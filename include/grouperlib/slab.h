// A simple slab allocator
//
// Largely inspired by the implementation of a slab allocator found in the Barrelfish operating system.
#pragma once

#include <grouperlib/numeric_types.h>
#include <grouperlib/error.h>

// forward declarations
struct slabAllocator;
struct slabRegion;

/// If defined, the slab allocator will zero out the memory of allocated blocks.
#define ZERO_OUT_SLAB_BLOCKS

/// Creates a new allocator with the given block size.
errval_t slab_init(struct slabAllocator *slabs, size_t blocksize);
/// Adds the buf region (with length len) to the allocator.
errval_t slab_grow(struct slabAllocator *slabs, void *buf, size_t len);
/// Allocate a block from the allocator.
void *slab_alloc(struct slabAllocator *slabs);
/// Frees block returning it to the allocator.
errval_t slab_free(struct slabAllocator *slabs, void *block);
/// The free count of blocks managed by the allocator.
size_t slab_freecount(struct slabAllocator *slabs);

// Size of block header
#define SLAB_HEADER_SIZE (sizeof(void *))
// The size of a block must be at least as big as the header size
#define SLAB_BLOCKSIZE(blocksize) \
	(((blocksize) > SLAB_HEADER_SIZE) ? (blocksize) : SLAB_HEADER_SIZE)
// Macro to compute the static buffer size required to fit a Slab Region with n elements
#define SLAB_REGION_SIZE(n, blocksize) \
	((n)*SLAB_BLOCKSIZE(blocksize) + sizeof(struct slabRegion))

struct slabAllocator {
	/// Size of blocks managed by this allocator
	size_t blocksize;
	/// Pointer to a list of slab regions
	struct slabRegion *regions;
	/// Count of total blocks managed by this allocator
	u64 total;
	/// Count of free blocks managed by this allocator
	u64 free;
};

struct slabRegion {
	/// Next slabRegion in the slabAllocator list.
	struct slabRegion *next;
	/// Count of total blocks in this Region
	u64 total;
	/// Count of free blocks in this Region
	u64 free;
	/// Free list of blocks in this Region
	struct slabBlock *blocks;
};
