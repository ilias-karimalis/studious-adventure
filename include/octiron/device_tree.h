#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/types/error.h>
#include <kzadhbat/types/result.h>
#include <kzadhbat/collections/bump.h>

#include <octiron/collections/array.h>

struct dtb_reservedRegion {
	/// The physical address of the reserved region
	paddr_t address;
	/// The size of the reserved region in bytes
	size_t size;
	/// The next reserved region in the linked list
	struct dtb_reservedRegion *next;
};

struct dtb_property {
	/// The name of the property
	const char *name;
	/// The bytestring value of the property
	void *value;
	/// The length of the value in bytes
	u32 value_len;
	/// The next property in the linked list
	struct dtb_property *next;
};

struct dtb_node {
	/// Name of the node
	const char *name;
	/// Properties of the node
	struct dtb_property *properties;
	/// Pointer to the parent node (NULL for root)
	struct dtb_node *parent;
	/// Pointer to the first child node (NULL if no children)
	struct dtb_node *children;
	/// Pointer to the next sibling node (NULL if no siblings)
	struct dtb_node *sibling;
};

DEFINE_ARRAY_STRUCT(dtb_reservedRegion);
DEFINE_ARRAY_STRUCT(dtb_node);
DEFINE_ARRAY_STRUCT(dtb_property);
struct dtb_state {
	/// List of reserved memory regions
	ARRAY(STRUCT(dtb_reservedRegion)) reserved_memory;
	/// List of all device tree nodes. nodes[0] is the root node.
	ARRAY(STRUCT(dtb_node)) nodes;
	/// List of all device tree properties
	ARRAY(STRUCT(dtb_property)) properties;
	/// bump allocator used to allocate dtb structs and strings
	struct bump allocator;
};

DEFINE_RESULT_STRUCT_PTR(dtb_state);
RESULT(PTR(STRUCT(dtb_state))) dtb_parse(paddr_t dtb_base_addr);
