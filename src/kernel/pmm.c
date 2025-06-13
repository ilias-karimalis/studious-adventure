#include <kernel/pmm.h>

#include <grouperlib/assert.h>
#include <grouperlib/error.h>
#include <grouperlib/arch/riscv.h>
#include <grouperlib/bitmacros.h>
#include <grouperlib/error.h>
#include <grouperlib/fmtprint.h>
#include <grouperlib/libc/string.h>

struct pmmBlock {
	/// @brief  Pointer to the next free block in the region.
	struct pmmBlock *next;
	/// @brief  Size of the block in bytes.
	size_t size;
};

errval_t pmm_initialize(struct pmm *pmm, u8 *slab_buf, size_t slab_len)
{
	errval_t err = err_new();
	if (pmm == NULL || slab_buf == NULL) {
		return err_push(err, ERR_NULL_ARGUMENT);
	}

	// Initialize the slab allocator for the regions
	if (err_is_fail((err = slab_init(&pmm->region_allocator,
					 sizeof(struct pmmRegion))))) {
		return err_push(err, ERR_PMM_INIT);
	}

	// Grow the slab allocator with the provided buffer
	if (err_is_fail((err = slab_grow(&pmm->region_allocator, slab_buf, slab_len)))) {
		return err_push(err, ERR_PMM_INIT);
	}

	// Initialize the pmm structure
	pmm->regions = NULL;
	pmm->region_count = 0;
	pmm->total = 0;
	pmm->free = 0;
	pmm->policy = PMM_POLICY_FIRST_FIT;
	return ERR_OK;
}

errval_t pmm_add_region(struct pmm *pmm, void* base, size_t size)
{
	if (pmm == NULL || base == NULL) {
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

	// Create and instantiate the region struct
	struct pmmRegion *region =
		(struct pmmRegion *)slab_alloc(&pmm->region_allocator);
	if (region == NULL) {
		return ERR_PMM_SLAB_ALLOC_FAILED;
	}
	region->next = NULL;
	region->base = (u8*) aligned_base;
	region->size = aligned_size;
	region->free = aligned_size;
	region->free_blocks = (struct pmmBlock *)aligned_base;
	region->free_blocks->next = NULL;
	region->free_blocks->size = aligned_size;

	// Check if we're already managing this region
	struct pmmRegion *prev = NULL;
	struct pmmRegion *cur;
	for (cur = pmm->regions; cur != NULL && (size_t) cur->base <= base_address;
	     prev = cur, cur = cur->next) {
		bool UP_BOUND = aligned_base + aligned_size <= (size_t) cur->base + cur->size;
		if (UP_BOUND) {
			return ERR_PMM_ADD_MANAGED_REGION;
		}
	}

	// We know that the region is valid and we're adding it to the region list, so we can update the usage statistics.
	pmm->total += region->size;
	pmm->free += region->size;

	// The region we want to add should be the first in the region list. Either it's the lowest addressable region, or
	// it's the first region to be added.
	if (prev == NULL) {
		pmm->regions = region;
		return ERR_OK;
	}

	prev->next = region;
	region->next = cur;
	return ERR_OK;
}

errval_t pmm_alloc_aligned(struct pmm* pmm, size_t size, size_t alignment, u8** ret)
{
	// Check for null arguments
	if (pmm == NULL || ret == NULL) {
        *ret = NULL;
		return ERR_NULL_ARGUMENT;
	}

	// Check that the requested alignment is a power of two aand at least BASE_PAGE_SIZE
    if (alignment < BASE_PAGE_SIZE || (alignment & (alignment -1 )) != 0) {
        *ret = NULL;
        return ERR_PMM_BAD_ALIGNMENT;
    }

	// Round up the size to the nearest multiple of BASE_PAGE_SIZE
    size = ALIGN_UP(size, BASE_PAGE_SIZE);

    // Check that the allocator has enough memory
    if (pmm->free < size) {
        *ret = NULL;
        return ERR_PMM_OUT_OF_MEMORY;
    }
    
    // Check whether we need to refill the slab Allocator
    //
    // For this we probably need paging to work!
    // TODO
    
    // Find a large enough region as per our allocation policy:
    switch (pmm->policy) {
        case PMM_POLICY_FIRST_FIT:
            break;
        case PMM_POLICY_BEST_FIT:
        case PMM_POLICY_WORST_FIT:
            *ret = NULL;
            return ERR_NOT_IMPLEMENTED;
    }

    ASSERT(pmm->policy == PMM_POLICY_FIRST_FIT, "Starting by implementing only First Fit policy");
    for (struct pmmRegion* region = pmm->regions; region != NULL; region = region->next)
    {
        // Is this regions's free pool big enough for the requuest, if not early exit
        if (region->free < size) {
            continue;
        }

        struct pmmBlock* block = region->free_blocks;
        struct pmmBlock* prev = NULL;
        while (block != NULL)
        {
            size_t block_size = block->size;
            size_t block_base = (size_t) block;

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

                    struct pmmBlock* extra = (struct pmmBlock*) (aligned_base + size);
                    extra->size = block_base + block_size - (aligned_base + size);
                    extra->next = block->next;
                    block->next = extra;
                } else if (EXISTS_PRECEEDING) {
                    block->size = offset;
                } else if (EXISTS_POSTCEEDING) {
                    size_t new_base = aligned_base + size;
                    struct pmmBlock* new_block = (struct pmmBlock*) new_base;
                    new_block->size = (block_base + block_size) - new_base;
                    new_block->next = block->next;
                    prev->next = new_block;
                } 

                region->free -= size;
                pmm->free -= size;
                *ret = (u8*) aligned_base;    

			    // Zero out the page being given out
#define ZERO_OUT_PMM_PAGE
#ifdef ZERO_OUT_PMM_PAGE
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


errval_t pmm_alloc(struct pmm* pmm, size_t size, u8 **ret)
{
	return pmm_alloc_aligned(pmm, size, BASE_PAGE_SIZE, ret);
}

errval_t pmm_free(struct pmm* pmm, u8* ret)
{
	return ERR_NOT_IMPLEMENTED;
}

size_t pmm_total_mem(struct pmm *pmm)
{
	if (pmm == NULL)
		return 0;
	return pmm->total;
}

size_t pmm_free_mem(struct pmm *pmm)
{
	if (pmm == NULL)
		return 0;
	return pmm->free;
}
