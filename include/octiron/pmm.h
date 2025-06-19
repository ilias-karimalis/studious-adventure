#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/collections/slab.h>

// forward declarations
enum pmmPolicy {
	PMM_POLICY_FIRST_FIT, ///< First fit allocation policy.
	PMM_POLICY_BEST_FIT, ///< Best fit allocation policy.
	PMM_POLICY_WORST_FIT, ///< Worst fit allocation policy.
};

/// @brief  Initializes the physical memory manager.
errval_t pmm_initialize(void);
/// @brief  Adds a new memory region to the physical memory manager.
errval_t pmm_add_region(void *base, size_t size);
/// @brief  Removes a contiguous region from the physical memory manager.
errval_t pmm_remove_region(paddr_t base, size_t size);
/// @brief  Allocates a region of memory with the requested size and alignment.
errval_t pmm_alloc_aligned(size_t size, size_t alignment, u8 **ret);
/// @brief  Allocates a region of memory with the requested size and BASE_PAGE_SIZE alignment.
errval_t pmm_alloc(size_t size, u8 **ret);
/// @brief  Returns a previously allocated memory region to the allocator.
errval_t pmm_free(u8 *ret);
/// @brief  Returns the total amount of memory this pmm manages.
size_t pmm_total_mem(void);
/// @brief  Returns the total amount of free memory this pmm manages.
size_t pmm_free_mem(void);