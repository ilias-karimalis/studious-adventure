#pragma once

#include <kzadhbat/types/numeric_types.h>

#define SASSERT(expr, msg) _Static_assert((expr), msg)

/// @brief errval_t serves as a register storable error value, that can store a stack of cascading errors of depth 8.
typedef u64 errval_t;

enum grouperError {
	ERR_OK = 0,
	ERR_NOT_IMPLEMENTED,

	ERR_NULL_ARGUMENT,
	// Slab allocator errors:
	ERR_SLAB_REGION_TOO_SMALL,
	ERR_SLAB_FOREIGN_BLOCK,

	// Physical memory manager errors:
	ERR_PMM_INIT,
	ERR_PMM_SLAB_ALLOC_FAILED,
	ERR_PMM_ADD_REGION_TOO_SMALL,
	ERR_PMM_ADD_MANAGED_REGION,
	ERR_PMM_BAD_ALIGNMENT,
	ERR_PMM_OUT_OF_MEMORY,
	ERR_PMM_REGION_LIST_EMPTY,
	ERR_PMM_REGION_LIST_FULL,
	ERR_PMM_REGION_ALLOCATED_FROM,
	ERR_PMM_REGION_NOT_MANAGED,

	// Paging errors
	ERR_PAGING_UNALIGNED_ADDRESS,
	ERR_PAGING_INVALID_ADDRESS,
	ERR_PAGING_INVALID_FLAGS,
	ERR_PAGING_INVALID_TYPE,
	ERR_PAGING_SETUP_TABLE,
	ERR_PAGING_MAPPING_EXISTS,

	// Device Tree errors
	ERR_DTB_MAGIC_NUMBER,
	ERR_DTB_MAPPING_FAILED,
	ERR_DTB_UNCLOSED_ROOT_NODE,
	ERR_DTB_NO_NODES,
	ERR_DTB_ADDRESS_CELLS_TOO_LARGE,
	ERR_DTB_SIZE_CELLS_TOO_LARGE,
	ERR_DTB_REWRITE_FAILED,

	/// @brief  Used to compute the number of enum values in enum grouper_error. Do not use this as an error value.
	GROUPER_ERROR_GUARD_VALUE,
} __attribute__((packed));


/// @brief  Creates a new empty error stack.
errval_t err_new(void);
/// @brief  Pushes an error value onto the stack.
errval_t err_push(errval_t errval, enum grouperError err);
/// @brief  Pops the top error value off the stack.
errval_t err_pop(errval_t errval);
/// @brief  Returns the top error value from the stack.
enum grouperError err_top(errval_t errval);
/// @brief  Returns true if the top error value is not an error.
bool err_is_ok(errval_t errval);
/// @brief  Returns true if the top error value is an error.
bool err_is_fail(errval_t errval);
/// @brief  Returns the string representation of the top error value.
const char *err_str(errval_t errval);
/// @brief  Prints the string representation of the top error value.
void err_print_top(errval_t errval);
/// @brief  Prints the string representation of the entire error stack.
void err_print_stack(errval_t errval);

