#pragma once

#include <grouperlib/numeric_types.h>

#define SASSERT(expr, msg) _Static_assert((expr), msg)

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

	/// @brief  Used to compute the number of enum values in enum grouper_error. Do not use this as an error value.
	GROUPER_ERROR_GUARD_VALUE,
} __attribute__((packed));

/// @brief errval_t serves as a register storable error value, that can store a stack of cascading errors of depth 8.
typedef u64 errval_t;

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
