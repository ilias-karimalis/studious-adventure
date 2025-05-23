#include <grouperlib/error.h>
#include <grouperlib/fmtprint.h>

static const char *grouper_error_str[] = {
	[ERR_OK] = "No error.",
	[ERR_NOT_IMPLEMENTED] = "Not implemented.",
	[ERR_NULL_ARGUMENT] = "Pointer argument to function was NULL.",

	// Slab allocator errors:
	[ERR_SLAB_REGION_TOO_SMALL] =
		"Slab region is too small to allocate a block from.",
	[ERR_SLAB_FOREIGN_BLOCK] =
		"Slab block being returned is not managed by this allocator.",

	// Physical memory manager errors:
	[ERR_PMM_INIT] = "Physical memory manager initialization failed.",
	[ERR_PMM_SLAB_ALLOC_FAILED] =
		"Physical memory manager slab allocator failed to allocate a block.",
	[ERR_PMM_ADD_REGION_TOO_SMALL] =
		"Attempted to add a region to the physical memory manager that was smaller than BASE_PAGE_SIZE.",
	[ERR_PMM_ADD_MANAGED_REGION] =
		"Attempted to add a region to the physical memory manager that was already being managed by it."
	// Add new error strings here as needed:
};

SASSERT(sizeof(enum grouperError) == 1, "enum grouperError must fit in a u8");
SASSERT(sizeof(grouper_error_str) / sizeof(grouper_error_str[0]) ==
		GROUPER_ERROR_GUARD_VALUE,
	"grouper_error_str must match the number of errors in enum grouper_error");

inline errval_t err_new(void)
{
	// Create a new empty error stack
	return 0;
}

inline errval_t err_push(errval_t errval, enum grouperError err)
{
	// Push an error value onto the stack
	errval <<= sizeof(enum grouperError);
	errval |= (u8)err;
	return errval;
}

inline errval_t err_pop(errval_t errval)
{
	// Pop the top error value off the stack
	return errval >> sizeof(enum grouperError);
}

inline enum grouperError err_top(errval_t errval)
{
	// Return the top error value from the stack
	return (enum grouperError)(errval & 0xFF);
}

inline bool err_is_ok(errval_t errval)
{
	// Check if the top error value is not an error
	return err_top(errval) == ERR_OK;
}

inline bool err_is_fail(errval_t errval)
{
	// Check if the top error value is an error
	return err_top(errval) != ERR_OK;
}

inline const char *err_str(errval_t errval)
{
	// Return the string representation of the top error value
	return grouper_error_str[err_top(errval)];
}

inline void err_print_top(errval_t errval)
{
	// Print the string representation of the top error value
	print("errval_t top error: ");
	println(err_str(errval));
}

inline void err_print_stack(errval_t errval)
{
	// Print the string representation of the entire error stack
	println("errval_t error stack: ");
	while (err_is_fail(errval)) {
		println(err_str(errval));
		errval = err_pop(errval);
	}
}
