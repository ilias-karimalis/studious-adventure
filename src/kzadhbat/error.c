#include <kzadhbat/types/error.h>
#include <kzadhbat/fmtprint.h>

static const char *grouper_error_str[] = {
	[ERR_OK] = "No error.",
	[ERR_NOT_IMPLEMENTED] = "Not implemented.",
	[ERR_NULL_ARGUMENT] = "Pointer argument to function was NULL.",

	// Slab allocator errors:
	[ERR_SLAB_REGION_TOO_SMALL] = "Slab region is too small to allocate a block from.",
	[ERR_SLAB_FOREIGN_BLOCK] = "Slab block being returned is not managed by this allocator.",

	// Physical memory manager errors:
	[ERR_PMM_INIT] = "Physical memory manager initialization failed.",
	[ERR_PMM_SLAB_ALLOC_FAILED] = "Physical memory manager slab allocator failed to allocate a block.",
	[ERR_PMM_ADD_REGION_TOO_SMALL] =
		"Attempted to add a region to the physical memory manager that was smaller than BASE_PAGE_SIZE.",
	[ERR_PMM_ADD_MANAGED_REGION] =
		"Attempted to add a region to the physical memory manager that was already being managed by it.",
	[ERR_PMM_BAD_ALIGNMENT] =
		"Attempted to allocate memory with an invalid alignment. Alignment must be a power of two and at least BASE_PAGE_SIZE.",
	[ERR_PMM_OUT_OF_MEMORY] =
		"Physical memory manager does not have enough free memory to satisfy the allocation request.",
	[ERR_PMM_REGION_LIST_EMPTY] = "Physical memory manager region list is empty. No regions are managed.",
	[ERR_PMM_REGION_LIST_FULL] = "Physical memory manager region list is full. No more regions can be added.",

	// Paging errors
	[ERR_PAGING_UNALIGNED_ADDRESS] =
		"Attempted to map a page with an unaligned address. Address must be aligned to the page size.",
	[ERR_PAGING_INVALID_ADDRESS] =
		"Attempted to map a page with an invalid address. Address must be within the managed physical memory range.",
	[ERR_PAGING_INVALID_FLAGS] =
		"Attempted to map a page with invalid flags. Flags must be a combination of valid paging flags.",
	[ERR_PAGING_INVALID_TYPE] =
		"Attempted to map a page with an invalid leaf type. Type must be one of the defined page types (Page, MegaPage, GigaPage).",
	[ERR_PAGING_SETUP_TABLE] = "Failed to set up an intermediate page table.",
	[ERR_PAGING_MAPPING_EXISTS] =
		"Attempted to map a page that already exists in the page table. Use a different address or unmap the existing mapping first.",

	// Device Tree errors
	[ERR_DTB_MAGIC_NUMBER] =
		"Device Tree Blob magic number is invalid. The magic number must be 0xD00DFEED to be valid.",
	[ERR_DTB_MAPPING_FAILED] =
		"Failed to map the Device Tree Blob into the kernel's page table.",
	[ERR_DTB_UNCLOSED_ROOT_NODE] =
		"Device Tree Blob parsing failed because the root node was not closed properly. Ensure that the DTB is well-formed.",

	// Add new error strings here as needed:
};

SASSERT(sizeof(enum grouperError) == 1, "enum grouperError must fit in a u8");
SASSERT(sizeof(grouper_error_str) / sizeof(grouper_error_str[0]) == GROUPER_ERROR_GUARD_VALUE,
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
