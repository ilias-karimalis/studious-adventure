#include <kzadhbat/collections/bump_allocator.h>
#include <kzadhbat/libc/string.h>
#include <kzadhbat/bitmacros.h>

// Forward declarations
void *bump_alloc(struct bumpAllocator *allocator, size_t size);
void *bump_copy(struct bumpAllocator *allocator, const void *src, size_t size);
const char *bump_copy_string(struct bumpAllocator *allocator, void *src);
void *bump_alloc_aligned(struct bumpAllocator *allocator, size_t size, size_t alignment);

errval_t bump_init(struct bumpAllocator *allocator, u8 *memory, size_t size)
{
	if (allocator == NULL || memory == 0 || size == 0) {
		return ERR_NULL_ARGUMENT;
	}

	allocator->memory = memory;
	allocator->size = size;
	allocator->index = 0;
	allocator->alloc = bump_alloc;
	allocator->copy = bump_copy;
	allocator->str_copy = bump_copy_string;
	allocator->alloc_aligned = bump_alloc_aligned;

	return ERR_OK;
}

void *bump_alloc(struct bumpAllocator *allocator, size_t size)
{
	if (allocator == NULL || size == 0 || allocator->index + size > allocator->size) {
		return NULL;
	}

	void *ptr = (void *)(allocator->memory + allocator->index);
	allocator->index += size;
	return ptr;
}

void *bump_copy(struct bumpAllocator *allocator, const void *src, size_t size)
{
	if (allocator == NULL || src == NULL || size == 0 || allocator->index + size > allocator->size) {
		return NULL;
	}

	void *dest = bump_alloc(allocator, size);
	if (dest == NULL) {
		return NULL;
	}

	memcpy(dest, src, size);
	return dest;
}

const char *bump_copy_string(struct bumpAllocator *allocator, void *src)
{
	if (allocator == NULL || src == NULL) {
		return NULL;
	}

	size_t len = strlen((const char *)src) + 1;
	if (allocator->index + len > allocator->size) {
		return NULL;
	}

	char *dest = (char *)(allocator->memory + allocator->index);
	memcpy(dest, src, len);
	allocator->index += len;
	return dest;
}

void *bump_alloc_aligned(struct bumpAllocator *allocator, size_t size, size_t alignment)
{
	if (allocator == NULL || size == 0 || alignment == 0 ||
	    ALIGN_UP(allocator->index, alignment) + size > allocator->size) {
		return NULL;
	}
	allocator->index = ALIGN_UP(allocator->index, alignment);

	void *ptr = (void *)(allocator->memory + allocator->index);
	allocator->index += size;
	return ptr;
}