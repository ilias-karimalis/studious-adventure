#pragma once

#include <grouperlib/numeric_types.h>
#include <grouperlib/slab.h>

// forward declarations
struct pmm;
struct pmmRegion;
struct pmmBlock;

/// @brief  Initializes the physical memory manager.
errval_t pmm_initialize(struct pmm *pmm, u8 *slab_buf, size_t slab_len);
/// @brief  Adds a new memory region to the physical memory manager.
errval_t pmm_add_region(struct pmm *pmm, uintptr_t base, size_t size);
/// @brief  Allocates a region of memory with the requested size and alignment.
errval_t pmm_alloc_aligned(struct pmm *pmm, size_t size, size_t alignment, u8 **ret);
/// @brief  Allocates a region of memory with the requested size and BASE_PAGE_SIZE alignment.
errval_t pmm_alloc(struct pmm *pmm, size_t size, u8 **ret);
/// @brief  Returns a previously allocated memory region to the allocator.
errval_t pmm_free(struct pmm *pmm, u8 *ret);
/// @brief  Returns the total amount of memory this pmm manages.
size_t pmm_total_mem(struct pmm *pmm);
/// @brief  Returns the total amount of free memory this pmm manages.
size_t pmm_free_mem(struct pmm *pmm);

struct pmm {
	/// @brief  List of contiguous regions from which memory can be allocated.
	struct pmmRegion *regions;
	/// @brief  Slab allocator for mmRegion structures.
	struct slabAllocator region_allocator;
	/// @brief  Number of regions in the list.
	size_t region_count;
	/// @brief  Total amount of memory managed by the allocator.
	size_t total;
	/// @brief  Total amount of free memory managed by the allocator.
	size_t free;
};

struct pmmRegion {
	/// @brief The next region in the list.
	struct pmmRegion *next;
	/// @brief  The base address of the region.
	uintptr_t base;
	/// @brief  Length of the region in bytes.
	size_t size;
	/// @brief  Count of total bytes free in this region.
	size_t free;
	/// @brief  Free list of blocks in this region.
	struct pmmBlock *free_blocks;
	/// @briref Used list of blocks in this region.
	struct pmmBlock *used_blocks;
};
