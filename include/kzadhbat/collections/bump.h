 /// Implementation of a bump allocator
#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/types/error.h>

// Forward Declarations
struct bump;
struct bumpRegion;

/// Creates a new bump allocator.
errval_t bump_init(struct bump* allocator, u8* memory, size_t size);
/// Allocates a block of memory of size size from the bump allocator, return NULL if allocation fails.
void* bump_alloc(struct bump* allocator, size_t size);

struct bump {
	/// The base address of the bump allocator's memory region.
	u8 *memory;
	/// The total size of the bump allocator's memory region.
	size_t size;
	/// The current index of the bump allocator.
	size_t index;
};