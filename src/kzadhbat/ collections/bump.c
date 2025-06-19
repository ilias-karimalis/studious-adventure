#include <kzadhbat/collections/bump.h>
#include "kzadhbat/fmtprint.h"

errval_t bump_init(struct bump *allocator, u8 *memory, size_t size)
{
	if (allocator == NULL || memory == 0 || size == 0) {
		return ERR_NULL_ARGUMENT;
	}

	allocator->memory = memory;
	allocator->size = size;
	allocator->index = 0;
	return ERR_OK;
}

void *bump_alloc(struct bump *allocator, size_t size)
{
	if (allocator == NULL || size == 0 || allocator->index + size > allocator->size) {
		println("allocator: %x, size: %x, allocator_index + size: %d, allocator_size: %d", allocator, size, allocator->index + size, allocator->size);
		return NULL;
	}

	void *ptr = (void *)(allocator->memory + allocator->index);
	allocator->index += size;
	return ptr;
}
