#include <octiron/pmm.h>

#include <kzadhbat/assert.h>
#include <kzadhbat/types/error.h>
#include <kzadhbat/arch/riscv.h>
#include <kzadhbat/bitmacros.h>
#include <kzadhbat/fmtprint.h>
#include <kzadhbat/libc/string.h>

#define PMM_REGION_COUNT 16

struct pmmRegion {
	// /// @brief The next region in the list.
	// struct pmmRegion *next;
	/// @brief  The base address of the region.
	paddr_t base;
	/// @brief  Length of the region in bytes.
	size_t size;
	/// @brief  Count of total bytes free in this region.
	size_t free;
	/// @brief  Free list of blocks in this region.
	struct pmmBlock *free_blocks;
};

struct pmm {
	/// @brief  List of contiguous regions from which memory can be allocated.
	struct pmmRegion regions[PMM_REGION_COUNT];
	/// @brief  Number of regions in the list.
	size_t region_count;
	/// @brief  Slab allocator for mmBlock structures.
	struct slabAllocator block_allocator;
	/// @brief  Total amount of memory managed by the allocator.
	size_t total;
	/// @brief  Total amount of free memory managed by the allocator.
	size_t free;
	/// @brief  The policy used for allocation.
	enum pmmPolicy policy;
	/// @brief  Check whether the pmm is initialized.
	bool initialized;
} pmm;

struct pmmBlock {
	/// @brief  Base address of the block.
	paddr_t base;
	/// @brief  Size of the block in bytes.
	size_t size;
	/// @brief  Pointer to the next free block in the region.
	struct pmmBlock *next;
};
SASSERT(sizeof(struct pmmBlock) == 24, "pmmBlock must be 24 bytes wide");

#define INITIAL_PMM_SLAB_SZ SLAB_REGION_SIZE(64, sizeof(struct pmmBlock))
u8 pmm_initial_region_slab[INITIAL_PMM_SLAB_SZ];

errval_t pmm_initialize(void)
{
	errval_t err = err_new();
	// Initialize the slab allocator for the regions
	if (err_is_fail((err = slab_init(&pmm.block_allocator, sizeof(struct pmmBlock))))) {
		return err_push(err, ERR_PMM_INIT);
	}
	// Grow the slab allocator with the provided buffer
	if (err_is_fail((err = slab_grow(&pmm.block_allocator, pmm_initial_region_slab, INITIAL_PMM_SLAB_SZ)))) {
		return err_push(err, ERR_PMM_INIT);
	}
	// Initialize the pmm structure
	pmm.region_count = 0;
	pmm.total = 0;
	pmm.free = 0;
	pmm.policy = PMM_POLICY_FIRST_FIT;
	return ERR_OK;
}

errval_t pmm_add_region(void *base, size_t size)
{
	if (base == NULL) {
		return ERR_NULL_ARGUMENT;
	}
	size_t base_address = (size_t)base;
	// Check that the aligned base and size region is at least BASE_PAGE_SIZE
	size_t aligned_base = ALIGN_UP(base_address, BASE_PAGE_SIZE);
	size_t aligned_size = ALIGN_DOWN(size - (aligned_base - base_address), BASE_PAGE_SIZE);
	bool ALIGNED_REGION_FITS = aligned_base + aligned_size <= base_address + size;
	bool NEW_SIZE_NON_ZERO = aligned_size >= BASE_PAGE_SIZE;
	if (!ALIGNED_REGION_FITS || !NEW_SIZE_NON_ZERO) {
		return ERR_PMM_ADD_REGION_TOO_SMALL;
	}
	// Check that the region list is not full
	if (pmm.region_count >= PMM_REGION_COUNT) {
		return ERR_PMM_REGION_LIST_FULL;
	}
	// Check if we're already managing this region
	for (size_t i = 0; i < pmm.region_count; i++) {
		struct pmmRegion cur = pmm.regions[i];
		bool UP_BOUND = aligned_base + aligned_size <= (size_t)cur.base + cur.size;
		if (UP_BOUND) {
			return ERR_PMM_ADD_MANAGED_REGION;
		}
	}
	// Create and instantiate the region struct
	struct pmmBlock *free_block = (struct pmmBlock *)slab_alloc(&pmm.block_allocator);
	if (free_block == NULL) {
		return ERR_PMM_SLAB_ALLOC_FAILED;
	}
	pmm.regions[pmm.region_count].base = aligned_base;
	pmm.regions[pmm.region_count].size = aligned_size;
	pmm.regions[pmm.region_count].free = aligned_size;
	pmm.regions[pmm.region_count].free_blocks = free_block;
	free_block->base = aligned_base;
	free_block->size = aligned_size;
	free_block->next = NULL;
	pmm.region_count++;
	// Update the pmm's usage statistics
	pmm.total += aligned_size;
	pmm.free += aligned_size;
	return ERR_OK;
}

errval_t pmm_remove_region(paddr_t base, size_t size)
{
	paddr_t aligned_base = ALIGN_DOWN(base, BASE_PAGE_SIZE);
	size_t aligned_size = ALIGN_UP(size, BASE_PAGE_SIZE);

	// Check that the aligned base and size region is at least BASE_PAGE_SIZE
	bool ALIGNED_REGION_FITS = aligned_base + aligned_size <= base + size;
	ASSERT(ALIGNED_REGION_FITS, "Aligned region does not fit in the original region");
	bool NEW_SIZE_NON_ZERO = aligned_size >= BASE_PAGE_SIZE;
	if (!NEW_SIZE_NON_ZERO) {
		return ERR_OK;
	}

	// Check if the region is managed by the pmm, it could either be a whole region or a part of a region.
	for (size_t i = 0; i < pmm.region_count; i++) {
		struct pmmRegion *region = &pmm.regions[i];
		bool EXACT_FIT = aligned_base == region->base && aligned_size == region->size;
		if (EXACT_FIT) {
			// If the region has not been allocated from we can remove it safely
			if (region->free != region->size) {
				return ERR_PMM_REGION_ALLOCATED_FROM;
			}
			// Free up and clean this region then shift the rest of the regions down
			slab_free(&pmm.block_allocator, region->free_blocks);
			pmm.free -= region->size;
			pmm.total -= region->size;
			// Remove the region by shifting the rest of the regions down
			for (size_t j = i; j < pmm.region_count - 1; j++) {
				pmm.regions[j] = pmm.regions[j + 1];
			}
			return ERR_OK;

		}
		bool UNDER_BOUND_REGION = aligned_base >= region->base;
		bool UP_BOUND_REGION = aligned_base + aligned_size <= region->base + region->size;
		if (UNDER_BOUND_REGION && UP_BOUND_REGION) {
			// Search in the free blocks of the region for the block to remove
			struct pmmBlock *block = region->free_blocks;
			struct pmmBlock *prev = NULL;
			while (block != NULL) {
				bool UNDER_BOUND = aligned_base >= block->base;
				bool UP_BOUND = aligned_base + aligned_size <= block->base + block->size;
				if (UNDER_BOUND && UP_BOUND) {
					size_t offset = aligned_base - block->base;

					// Check if the current pmmBlock has at least a page preceeding or postceeding
					bool EXISTS_PRECEEDING = block->base != aligned_base;
					bool EXISTS_POSTCEEDING = block->base + block->size > aligned_base + aligned_size;
					if (EXISTS_PRECEEDING && EXISTS_POSTCEEDING) {
						// Need to make a new block
						block->size = offset;

						struct pmmBlock *extra = slab_alloc(&pmm.block_allocator);
						if (extra == NULL) {
							return ERR_PMM_SLAB_ALLOC_FAILED;
						}
						extra->base = aligned_base + aligned_size;
						extra->size = block->base + block->size - (aligned_base + aligned_size);
						extra->next = block->next;
						block->next = extra;
					} else if (EXISTS_PRECEEDING) {
						block->size = offset;
					} else if (EXISTS_POSTCEEDING) {
						block->base = aligned_base + aligned_size;
						block->size = block->base + block->size - (aligned_base + aligned_size);
					} else if (prev == NULL) {
						// This is the first block in the region, so we can just remove it
						region->free_blocks = block->next;
						slab_free(&pmm.block_allocator, block);
					} else {
						prev->next = block->next;
						slab_free(&pmm.block_allocator, block);
					}

					pmm.regions[i].free -= aligned_size;
					pmm.free -= aligned_size;
					return ERR_OK;
				} else if (UP_BOUND) {
					// We've gone past the block we are looking for, so we can stop searching
					return ERR_PMM_REGION_ALLOCATED_FROM;
				}
				prev = block;
				block = block->next;
			}
		}
	}

	return ERR_PMM_REGION_NOT_MANAGED;
}

errval_t pmm_alloc_aligned(size_t size, size_t alignment, u8 **ret)
{
	// Check for null arguments
	if (ret == NULL) {
		*ret = NULL;
		return ERR_NULL_ARGUMENT;
	}

	// Check that the requested alignment is a power of two aand at least BASE_PAGE_SIZE
	if (alignment < BASE_PAGE_SIZE || (alignment & (alignment - 1)) != 0) {
		*ret = NULL;
		return ERR_PMM_BAD_ALIGNMENT;
	}

	// Round up the size to the nearest multiple of BASE_PAGE_SIZE
	size = ALIGN_UP(size, BASE_PAGE_SIZE);

	// Check that the allocator has enough memory
	if (pmm.free < size) {
		*ret = NULL;
		return ERR_PMM_OUT_OF_MEMORY;
	}

	// Check whether we need to refill the slab Allocator
	//
	// For this we probably need paging to work!
	// TODO
	if (slab_freecount(&pmm.block_allocator) < 16) {
		TODO("Slab allocator needs to be refilled, but paging is not implemented yet.");
	}

	// Find a large enough region as per our allocation policy:
	switch (pmm.policy) {
	case PMM_POLICY_FIRST_FIT:
		break;
	case PMM_POLICY_BEST_FIT:
	case PMM_POLICY_WORST_FIT:
		*ret = NULL;
		return ERR_NOT_IMPLEMENTED;
	}

	ASSERT(pmm.policy == PMM_POLICY_FIRST_FIT, "Starting by implementing only First Fit policy");
	for (size_t i = 0; i < pmm.region_count; i++)
	// for (struct pmmRegion* region = pmm.regions; region != NULL; region = region->next)
	{
		struct pmmRegion region = pmm.regions[i];
		// Is this regions's free pool big enough for the requuest, if not early exit
		if (region.free < size) {
			continue;
		}

		struct pmmBlock *block = region.free_blocks;
		struct pmmBlock *prev = NULL;
		while (block != NULL) {
			size_t block_size = block->size;
			size_t block_base = block->base;

			size_t aligned_base = ALIGN_UP(block_base, alignment);
			bool SIZE_OK = block_base + block_size >= aligned_base + size;
			if (SIZE_OK) {
				size_t offset = aligned_base - block_base;

				// Check if the current pmmBlock has atleast a page preceeding or postceeding
				bool EXISTS_PRECEEDING = block_base != aligned_base;
				bool EXISTS_POSTCEEDING = block_base + block_size > aligned_base + size;
				if (EXISTS_PRECEEDING && EXISTS_POSTCEEDING) {
					// Need to make a new block
					block->size = offset;

					struct pmmBlock *extra = slab_alloc(&pmm.block_allocator);
					extra->base = aligned_base + size;
					extra->size = block_base + block_size - (aligned_base + size);
					extra->next = block->next;
					block->next = extra;
				} else if (EXISTS_PRECEEDING) {
					block->size = offset;
				} else if (EXISTS_POSTCEEDING) {
					block->base = aligned_base + size;
					block->size = block_base + block_size - (aligned_base + size);
				} else if (prev == NULL) {
					// This is the first block in the region, so we can just remove it
					pmm.regions[i].free_blocks = block->next;
					slab_free(&pmm.block_allocator, block);
				} else {
					prev->next = block->next;
					slab_free(&pmm.block_allocator, block);
				}

				pmm.regions[i].free -= size;
				pmm.free -= size;
				*ret = (u8 *)aligned_base;

#define ZERO_OUT_PMM_PAGE
#ifdef ZERO_OUT_PMM_PAGE
				// Zero out the page being given out
				memset(*ret, 0, size);
#endif
				return ERR_OK;
			}

			prev = block;
			block = block->next;
		}
	}

	// No large enough block was found.
	*ret = NULL;
	return ERR_PMM_OUT_OF_MEMORY;
}

errval_t pmm_alloc(size_t size, u8 **ret)
{
	return pmm_alloc_aligned(size, BASE_PAGE_SIZE, ret);
}

errval_t pmm_free(u8 *ret)
{
	(void)ret;
	return ERR_NOT_IMPLEMENTED;
}

size_t pmm_total_mem(void)
{
	return pmm.total;
}

size_t pmm_free_mem(void)
{
	return pmm.free;
}
