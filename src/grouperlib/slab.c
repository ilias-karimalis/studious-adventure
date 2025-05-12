#include <grouperlib/slab.h>

struct slabBlock {
	struct slabBlock *next;
};

errval_t slab_init(struct slabAllocator *slabs, size_t blocksize)
{
	errval_t err = err_new();
	if (slabs == NULL) {
		return err_push(err, ERR_NULL_ARGUMENT);
	}

	slabs->blocksize = SLAB_BLOCKSIZE(blocksize);
	slabs->regions = NULL;
	slabs->total = 0;
	slabs->free = 0;
	return err;
}

errval_t slab_grow(struct slabAllocator *slabs, void *buf, size_t len)
{
	errval_t err = err_new();
	if (slabs == NULL || buf == NULL) {
		return err_push(err, ERR_NULL_ARGUMENT);
	}

	// Check that the region is large enough to allocate at least a single block from.
	if (len < SLAB_REGION_SIZE(1, slabs->blocksize)) {
		return err_push(err, ERR_SLAB_REGION_TOO_SMALL);
	}

	// Setup slabRegion structure
	struct slabRegion *region = buf;
	len -= sizeof(struct slabRegion);
	buf = (u8 *)buf - len;

	// Calculate the number of blocks in the buffer
	region->free = region->total = len / slabs->blocksize;
	slabs->total += region->total;
	slabs->free += region->free;

	// Enqueue the slabBlocks in the region free list
	struct slabBlock *block = region->blocks = buf;
	for (size_t i = 0; i < region->total; i++) {
		buf = (u8 *)buf + slabs->blocksize;
		block->next = buf;
		block = buf;
	}
	block->next = NULL;

	// Add the Region to the allocator
	region->next = slabs->regions;
	slabs->regions = region;
	return err;
}

void *slab_alloc(struct slabAllocator *slabs)
{
	if (slabs->free == 0) {
		return NULL;
	}

	// Find the first region that isn't fully allocated.
	struct slabRegion *r;
	for (r = slabs->regions; r != NULL && r->free == 0; r = r->next)
		;
	if (r == NULL) {
		return NULL;
	}

	// Dequeue the first block from the Region free list
	struct slabBlock *sb = r->blocks;
	r->blocks = sb->next;
	r->free--;
	slabs->free--;
	return sb;
}

errval_t slab_free(struct slabAllocator *slabs, void *block)
{
	errval_t err = err_new();
	if (slabs == NULL || block == NULL) {
		return err_push(err, ERR_NULL_ARGUMENT);
	}
	struct slabBlock *sb = (struct slabBlock *)block;

	// Find the matching slabRegion
	struct slabRegion *region;
	size_t blocksize = blocksize;
	for (region = slabs->regions; region != NULL; region = region->next) {
		// Check if the block falls inside this slab
		bool LOWER_BOUND = (uintptr_t)region < (uintptr_t)sb;
		bool UPPER_BOUND = (uintptr_t)sb <
				   (uintptr_t)region +
					   sizeof(struct slabRegion) +
					   region->total * slabs->blocksize;
		if (LOWER_BOUND && UPPER_BOUND)
			break;
	}

	if (region == NULL)
		return err_push(err, ERR_SLAB_FOREIGN_BLOCK);

	sb->next = region->blocks;
	region->blocks = sb;
	region->free++;
	slabs->free++;
	return err;
}

size_t slab_freecount(struct slabAllocator *slabs)
{
	return slabs->free;
}
