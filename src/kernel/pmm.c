#include <kernel/pmm.h>

#include <grouperlib/error.h>
#include <grouperlib/arch/riscv.h>
#include <grouperlib/bitmacros.h>

#include "../../include/grouperlib/error.h"

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
	if (err_is_fail((err = slab_grow(&pmm->region_allocator, slab_buf,
					 slab_len)))) {
		return err_push(err, ERR_PMM_INIT);
	}

	// Initialize the pmm structure
	pmm->regions = NULL;
	pmm->region_count = 0;
	pmm->total = 0;
	pmm->free = 0;
	return err;
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
	region->base = aligned_base;
	region->size = aligned_size;
	region->free = aligned_size;
	region->used_blocks = NULL;
	region->free_blocks = (struct pmmBlock *)aligned_base;
	region->free_blocks->next = NULL;
	region->free_blocks->size = aligned_size;

	// Check if we're already managing this region
	struct pmmRegion *prev = NULL;
	struct pmmRegion *cur;
	for (cur = pmm->regions; cur != NULL && cur->base <= base_address;
	     prev = cur, cur = cur->next) {
		bool UP_BOUND = (uintptr_t)aligned_base + aligned_size <=
				(uintptr_t)cur->base + cur->size;
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

errval_t pmm_alloc(struct pmm* pmm, size_t size, u8 **ret)
{
	errval_t err;

	// Check for null arguments
	if (pmm == NULL || ret == NULL) {
		return ERR_NULL_ARGUMENT;
	}

	// Round up the size to the nearest multiple of BASE_PAGE_SIZE
	size = ALIGN_UP(size, BASE_PAGE_SIZE);

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
