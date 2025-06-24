 /// Implementation of a bumpAllocator allocator
#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/types/error.h>

// Forward Declarations
struct bumpAllocator;
struct bumpRegion;

/// Creates a new bumpAllocator allocator.
errval_t bump_init(struct bumpAllocator * allocator, u8* memory, size_t size);

struct bumpAllocator {
	/// The base address of the bumpAllocator allocator's memory region.
	u8 *memory;
	/// The total size of the bumpAllocator allocator's memory region.
	size_t size;
	/// The current index of the bumpAllocator allocator.
	size_t index;
	/// Allocates a block of memory of size size from the bumpAllocator allocator, return NULL if allocation fails.
	void* (*alloc)(struct bumpAllocator * alloc, size_t size);
	/// Copies a block of memory into the bumpAllocator allocator, returning a pointer to the copied memory.
	/// Returns NULL if the allocation fails, if the source is NULL, or if the size is zero.
	void* (*copy)(struct bumpAllocator * alloc, const void* src, size_t size);
	/// Copies a string into the bumpAllocator allocator, returning a pointer to the copied string.
	/// Returns NULL if the allocation fails or if the string is NULL.
	/// This operation is unsafe if the string is not null-terminated.
	const char* (*str_copy)(struct bumpAllocator * alloc, void* src);
	/// Allocates a block of memory of size size from the bumpAllocator allocator, ensuring that the allocated memory is
	/// aligned to the specified alignment.
	void* (*alloc_aligned)(struct bumpAllocator * alloc, size_t size, size_t alignment);
};