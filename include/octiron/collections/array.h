/// Defines a macro programmed Dynamic Array<T> type and helper macros to interact with the type.
#pragma once

#include <kzadhbat/libc/string.h>
#include <octiron/pmm.h>


/// The Array<T> type
#define ARRAY(t) ARRAY_STR(t)
#define ARRAY_STR(t) Array_##t

/// Declares and defines an Array<T> type.
#define DEFINE_ARRAY(t) \
	typedef struct { \
		t *data; \
		size_t size; \
		size_t capacity; \
	} ARRAY(t)

/// Declares and defines an Array<struct T> type.
#define DEFINE_ARRAY_STRUCT(t) \
	typedef struct { \
		struct t *data; \
		size_t size; \
		size_t capacity; \
	} ARRAY(STRUCT(t))

/// Initializes an Array<T> with a capacity of 0
#define ARRAY_INIT(type) (ARRAY_INIT_HELPER(type)) { .data = NULL, .size = 0, .capacity = 0 }
#define ARRAY_INIT_HELPER(type) ARRAY(type)
/// Initializes an Array<T> with a given capacity.
#define ARRAY_INIT_CAPACITY(type, cap) ARRAY(type) { .data = NULL, .size = 0, .capacity = (cap) }

#define ARRAY_PUSH(array, value) do { \
	if ((array).size >= (array).capacity) { \
		size_t new_capacity = (array).capacity ? (array).capacity * 2 : BASE_PAGE_SIZE / sizeof(*(array).data); \
		u8 *new_data = NULL; \
		if (err_is_fail(pmm_alloc_aligned(new_capacity * sizeof(*(array).data), BASE_PAGE_SIZE, &new_data))) { \
			PANIC_LOOP("ARRAY failed to allocate memory from the kernel pmm."); \
		} \
		if ((array).data != NULL) { \
			memcpy((array).data, new_data, (array).size * sizeof(*(array).data)); \
		} \
		(array).capacity = new_capacity; \
		(array).data = (typeof((array).data))new_data; \
	} \
	(array).data[(array).size++] = value; \
} while(0)

#define ARRAY_SIZE(array) ((array).size)
#define ARRAY_CAPACITY(array) ((array).capacity)

#ifndef STRUCT
/// Used as a work around for macro defined types that contain a struct type
/// To define an ARRAY(struct type) type, use ARRAY(STRUCT(type)).
#define STRUCT(type) STRUCT_HELPER(type)
#define STRUCT_HELPER(type) struct_##type
#endif
#ifndef UNION
/// Used as a work around for macro defined types that contain a union type
//// To define an ARRAY(union type) type, use ARRAY(UNION(type)).
#define UNION(type) UNION_HELPER(type)
#define UNION_HELPER(type) union_##type
#endif
#ifndef PTR
/// Used as a work around for macro defined types that contain a pointer type
/// To define an ARRAY(pointer type) type, use ARRAY(PTR(type)).
#define PTR(type) PTR_HELPER(type)
#define PTR_HELPER(type) type##_ptr
#endif