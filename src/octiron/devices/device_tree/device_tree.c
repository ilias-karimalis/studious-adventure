#include <octiron/devices/device_tree/device_tree.h>
#include <octiron/paging.h>
#include <octiron/collections/array.h>

#include <kzadhbat/bitmacros.h>
#include <kzadhbat/arch/riscv.h>
#include <kzadhbat/collections/bump_allocator.h>

// Struct forward declarations
// struct dt;
// struct dtNode;
// struct dtReservedRegion;
// struct dtProperty;
// struct dtPropRaw;
// struct dtPropStatus;
// struct dtPropReg;

///////////////////////////////////////////////////////////////////////////////
// Device Tree Structures
///////////////////////////////////////////////////////////////////////////////

struct dtReservedRegion {
	/// The physical address of the reserved region
	paddr_t address;
	/// The size of the reserved region in bytes
	size_t size;
};

enum dtPropertyType {
	/// Used to mark properties that have not been fully evaluated yet.
	DTB_PROP_RAW,
	/// compatible property, consists of one or more strings that def
	DTB_PROP_COMPATIBLE,
	/// model property, specifies the manufacturer's model number of the device
	DTB_PROP_MODEL,
	/// phandle property, specifies a numerical identifer for a node that is unique within the devicetree
	DTB_PROP_PHANDLE,
	/// status property, describes the operational status of the device
	DTB_PROP_STATUS,
	/// #address-cells property, specifies the number of <u32> cells used to encode the address field in a child
	/// node's reg property
	DTB_PROP_ADDRESS_CELLS,
	/// #size-cells property, specifies the number of <u32> cells used to encode the size field in a child node's
	/// reg property
	DTB_PROP_SIZE_CELLS,
	/// dma-coherence property, specifies whether the device is capable of coherent DMA operations
	DTB_PROP_DMA_COHERENCE,
	/// device_type property, specifies the type of the device (deprecated)
	DTB_PROP_DEVICE_TYPE,
	/// reg property, describes the address of the device’s resources within the address space defined by its parent
	/// bus
	DTB_PROP_REG,
	/// ranges property, describes the address translation between the address space of the bus and the address
	/// space of the bus node's parent.
	DTB_PROP_RANGES,
	/// dma-ranges property, describes DMA address translation between child and parent bus address spaces as
	/// (child-address, parent-address, length) triplets
	DTB_PROP_DMA_RANGES,
};

struct dtPropRaw {
	/// The length of the property value in bytes
	u32 value_len;
	/// The bytestring value of the property
	void *value;
};

struct dtPropStatus {
	enum {
		/// The device is operational
		DTB_PROP_STATUS_OK,
		/// The device is not operational, but can be enabled
		DTB_PROP_STATUS_DISABLED,
		/// The device is operational, but should not be used. Typically this is used for devices that are
		/// controlled by the bootloader or other firmware.
		DTB_PROP_STATUS_RESERVED,
		/// The device is not operational and should not be used.
		DTB_PROP_STATUS_FAIL, ///< The device has failed
		/// The device is not operational, and there is a specific reason for it not being operational,
		/// described by the attached string.
		DTB_PROP_STATUS_FAIL_SSS, ///< The status of the device is unknown
	} value;
	/// A string that describes the reason for the device not being operational, if applicable.
	const char *reason;
};

struct dtPropReg {
	/// List of addresses. NULL if address_cells == 0.
	void* addresses;
	/// List of sizes corresponding to the addresses. NULL if size_cells == 0.
	void* sizes;
	/// The number of (address, size) pairs.
	u32 n_pairs;

};

struct dtPropRanges {
	/// List of child bus addresses. NULL if address_cells == 0.
	void* child_bus_addrs;
	/// List of parent bus addresses. NULL if address_cells == 0.
	void* parent_bus_addrs;
	/// List of region lengths. NULL if size_cells == 0.
	void* lengths;
	/// The number of (child-bus-addr, parent-bus-addr, length) triplets.
	u32 n_trips;
};

struct dtProperty {
	/// The name of the property
	const char *name;
	/// The next property in the linked list
	struct dtProperty *next;
	/// The type of this property
	enum dtPropertyType type;
	/// The property type specific data
	union {
		/// Raw property data, used for properties that have not been fully evaluated yet.
		struct dtPropRaw raw;
		/// List of strings specifying device compatibility for driver selection. NULL terminated.
		const char** compat;
		/// Model string specifying the manufacturer and model number of the device.
		const char *model;
		/// Numerical identifier unique within the devicetree for node referencing.
		u32 phandle;
		/// Status property indicating the operational state of a device.
		struct dtPropStatus status;
		/// Number of <u32> cells used to encode the address field in a child node's reg property.
		u32 address_cells;
		/// Number of <u32> cells used to encode the size field in a child node's reg property.
		u32 size_cells;
		/// Indicates whether the device is capable of coherent DMA operations.
		bool dma_coherence;
		/// Specifies the type of the device (deprecated).
		const char *device_type;
		/// Defines the address of the device’s resources within the address space defined by its parent bus.
		struct dtPropReg reg;
		/// Defines the address translation between the address space of the bus and the address space of the bus
		/// node's parent.
		struct dtPropRanges ranges;
		/// Defines DMA address translation between child and parent bus address spaces as (child-address, parent-address,
		/// length) triplets.
		struct dtPropRanges dma_ranges;
		u32 cpu_handle;
	} data;
};

struct dtNode {
	/// Name of the node
	const char *name;
	/// The number of <u32> cells used to encode the address field in this node's reg property
	u32 address_cells;
	/// The number of <u32> cells used to encode the size field in this node's reg property
	u32 size_cells;
	/// Properties of the node
	struct dtProperty *properties;
	/// Pointer to the parent node (NULL for root)
	struct dtNode *parent;
	/// Pointer to the first child node (NULL if no children)
	struct dtNode *children;
	/// Pointer to the next sibling node (NULL if no siblings)
	struct dtNode *sibling;
};

DEFINE_ARRAY_STRUCT(dtReservedRegion);
DEFINE_ARRAY_STRUCT(dtNode);
DEFINE_ARRAY_STRUCT(dtProperty);
struct dt {
	/// List of reserved memory regions
	ARRAY(STRUCT(dtReservedRegion)) reserved_memory;
	/// List of all device tree nodes. nodes[0] is the root node.
	ARRAY(STRUCT(dtNode)) nodes;
	/// List of all device tree properties
	ARRAY(STRUCT(dtProperty)) properties;
	/// bumpAllocator allocator used to allocate strings
	struct bumpAllocator bump;
	/// Pointer to the root node of the device tree
	struct dtNode *root;
	/// Indicates whether the device tree has been initialized
	bool initialized;
} state = { 0 };


struct dtb_header {
	u32 magic;
	u32 totalsize;
	u32 off_dt_struct;
	u32 off_dt_strings;
	u32 off_mem_rsvmap;
	u32 version;
	u32 last_comp_version;
	u32 boot_cpuid_phys;
	u32 size_dt_strings;
	u32 size_dt_struct;
};

enum dtb_StructureToken {
	FDT_BEGIN_NODE = (u32)0x01,
	FDT_END_NODE = (u32)0x02,
	FDT_PROP = (u32)0x03,
	FDT_NOP = (u32)0x04,
	FDT_END = (u32)0x09,
};

///////////////////////////////////////////////////////////////////////////////
// Device Tree Helper Functions
///////////////////////////////////////////////////////////////////////////////


size_t dtb_parse_property(struct dtNode *curr, u8 *structures, u8 *strings, size_t off)
{
	u32 prop_len = READ_BIG_ENDIAN_U32(structures + off);
	off += sizeof(u32);
	u32 name_offset = READ_BIG_ENDIAN_U32(structures + off);
	off += sizeof(u32);

	const char *prop_name = state.bump.str_copy(&state.bump, &strings[name_offset]);
	void *prop_value = state.bump.copy(&state.bump, &structures[off], prop_len);
	off += ALIGN_UP(prop_len, sizeof(u32));

	ARRAY_PUSH(state.properties, ((struct dtProperty){ .name = prop_name,
							   .next = curr->properties,
							   .type = DTB_PROP_RAW,
							   .data.raw = {
								   .value = prop_value,
								   .value_len = prop_len,
							   } }));
	struct dtProperty *new = &state.properties.data[ARRAY_SIZE(state.properties) - 1];
	curr->properties = new;
	return off;
}

size_t dtb_parse_node(struct dtNode **curr, u8 *structures, size_t off)
{
	const char *name_buf = state.bump.str_copy(&state.bump, (void *)&structures[off]);
	ASSERT(name_buf != NULL, "Failed to allocate memory for node name.");
	size_t name_len = strlen(name_buf) + 1;

	ARRAY_PUSH(state.nodes,
		   ((struct dtNode){
			   .name = name_buf, .properties = NULL, .parent = *curr, .children = NULL, .sibling = NULL }));
	struct dtNode *new = &state.nodes.data[ARRAY_SIZE(state.nodes) - 1];
	if (*curr == NULL) {
		*curr = new;
		return off + ALIGN_UP(name_len, sizeof(u32));
	}

	if ((*curr)->children == NULL) {
		(*curr)->children = new;
	} else {
		struct dtNode *last_child = (*curr)->children;
		while (last_child->sibling != NULL) {
			last_child = last_child->sibling;
		}
		last_child->sibling = new;
	}
	*curr = new;
	return off + ALIGN_UP(name_len, sizeof(u32)); // Move the offset past the node name
}

void dtb_rewrite_property_compatible(struct dtProperty *prop)
{
	ASSERT(prop->type == DTB_PROP_RAW, "At rewrite time, the property must be a pre-parse property.");
	u32 value_len = prop->data.raw.value_len;
	char *value = prop->data.raw.value;
	// Count the number of strings in the value
	size_t num_strings = 0;
	for (size_t i = 0; i < value_len; i++) {
		if (value[i] == '\0') {
			num_strings++;
		}
	}

	prop->data.compat = state.bump.alloc_aligned(&state.bump, sizeof(void *) * (num_strings + 1), sizeof(void *));
	ASSERT(prop->data.compat != NULL, "Failed to allocate memory for compatible property.");
	for (size_t i = 0, j = 0; j < value_len; i++) {
		prop->data.compat[i] = &value[j];
		j += strlen(&value[j]) + 1;
	}
	prop->data.compat[num_strings] = NULL;
	prop->type = DTB_PROP_COMPATIBLE;
}

void dtb_rewrite_property_status(struct dtProperty *prop)
{
	ASSERT(prop->type == DTB_PROP_RAW, "At rewrite time, the property must be a pre-parse property.");
	prop->type = DTB_PROP_STATUS;
	if (strcmp(prop->data.raw.value, "okay") == 0) {
		prop->data.status.value = DTB_PROP_STATUS_OK;
		prop->data.status.reason = NULL;
	} else if (strcmp(prop->data.raw.value, "disabled") == 0) {
		prop->data.status.value = DTB_PROP_STATUS_DISABLED;
		prop->data.status.reason = NULL;
	} else if (strcmp(prop->data.raw.value, "reserved") == 0) {
		prop->data.status.value = DTB_PROP_STATUS_RESERVED;
		prop->data.status.reason = NULL;
	} else if (strcmp(prop->data.raw.value, "fail") == 0) {
		prop->data.status.value = DTB_PROP_STATUS_FAIL;
		prop->data.status.reason = NULL;
	} else if (strncmp(prop->data.raw.value, "fail-", 5) == 0) {
		prop->data.status.value = DTB_PROP_STATUS_FAIL_SSS;
		prop->data.status.reason = prop->data.raw.value + 5; // Skip the "fail-" prefix
	} else {
		ASSERT(false, "[dtb_rewrite_property_status] Unknown status value: %s", prop->data.raw.value);
	}
}

void dtb_rewrite_property_reg(struct dtProperty *prop, u32 address_cells, u32 size_cells)
{
	void *value = prop->data.raw.value;
	u32 value_len = prop->data.raw.value_len;
	u32 address_size = sizeof(u32) * address_cells;
	u32 size_size = sizeof(u32) * size_cells;

	size_t n_pairs = value_len / (address_size + size_size);
	assert(n_pairs > 0);
	assert(value_len % (address_size + size_size) == 0);
	assert(address_cells <= 3);
	assert(size_cells <= 2);

	// Allocate memory for the reg property
	void *addresses = state.bump.alloc_aligned(&state.bump, address_size * n_pairs, address_size);
	void *sizes = state.bump.alloc_aligned(&state.bump, size_size * n_pairs, size_size);
	assert((address_size == 0) ? addresses == NULL : addresses != NULL);
	assert((size_size == 0) ? sizes == NULL : sizes != NULL);

	for (size_t i = 0, j = 0; i < n_pairs; i++) {
		// Read an address
		switch (address_cells) {
		case 0:
			break;
		case 1:
			((u32 *)addresses)[i] = READ_BIG_ENDIAN_U32(value + j);
			break;
		case 2:
			((u64 *)addresses)[i] = READ_BIG_ENDIAN_U64(value + j);
			break;
		case 3:
			((u128 *)addresses)[i] = READ_BIG_ENDIAN_U128(value + j);
			break;
		default:
			__builtin_unreachable();
		}
		j += address_size;

		// Read a size
		switch (size_cells) {
		case 0:
			break;
		case 1:
			((u32 *)sizes)[i] = READ_BIG_ENDIAN_U32(value + j);
			break;
		case 2:
			((u64 *)sizes)[i] = READ_BIG_ENDIAN_U64(value + j);
			break;
		default:
			__builtin_unreachable();
		}
		j += size_size;
	}

	prop->type = DTB_PROP_REG;
	// prop->data.reg.address_cells = address_cells;
	// prop->data.reg.size_cells = size_cells;
	prop->data.reg.n_pairs = n_pairs;
	prop->data.reg.addresses = addresses;
	prop->data.reg.sizes = sizes;
}

void dtb_rewrite_property_ranges(struct dtProperty *prop, u32 address_cells, u32 size_cells)
{
	void *value = prop->data.raw.value;
	u32 value_len = prop->data.raw.value_len;
	u32 address_size = sizeof(u32) * address_cells;
	u32 size_size = sizeof(u32) * size_cells;

	assert(address_size > 0);
	assert(size_size > 0);
	size_t n_trips = value_len / (address_size + address_size + size_size);

	// Allocate memory for the buffers
	void *child_bus_addrs = state.bump.alloc_aligned(&state.bump, address_size * n_trips, address_size);
	void *parent_bus_addrs = state.bump.alloc_aligned(&state.bump, address_size * n_trips, address_size);
	void *lengths = state.bump.alloc_aligned(&state.bump, size_size * n_trips, size_size);

	for (size_t i = 0, j = 0; i < n_trips; i++) {
		// Read a child bus address
		switch (address_cells) {
		case 1:
			((u32 *)child_bus_addrs)[i] = READ_BIG_ENDIAN_U32(value + j);
			break;
		case 2:
			((u64 *)child_bus_addrs)[i] = READ_BIG_ENDIAN_U64(value + j);
			break;
		case 3:
			((u128 *)child_bus_addrs)[i] = READ_BIG_ENDIAN_U128(value + j);
			break;
		default:
			__builtin_unreachable();
		}
		j += address_size;

		// Read a parent bus address
		switch (address_cells) {
		case 1:
			((u32 *)parent_bus_addrs)[i] = READ_BIG_ENDIAN_U32(value + j);
			break;
		case 2:
			((u64 *)parent_bus_addrs)[i] = READ_BIG_ENDIAN_U64(value + j);
			break;
		case 3:
			((u128 *)parent_bus_addrs)[i] = READ_BIG_ENDIAN_U128(value + j);
			break;
		default:
			__builtin_unreachable();
		}
		j += address_size;

		// Read a length
		switch (size_cells) {
		case 1:
			((u32 *)lengths)[i] = READ_BIG_ENDIAN_U32(value + j);
			break;
		case 2:
			((u64 *)lengths)[i] = READ_BIG_ENDIAN_U64(value + j);
			break;
		default:
			PANIC_LOOP("Unsupported size_cells: %d", size_cells);
		}
		j += size_size;
	}

	prop->type = DTB_PROP_RANGES;
	prop->data.ranges.child_bus_addrs = child_bus_addrs;
	prop->data.ranges.parent_bus_addrs = parent_bus_addrs;
	prop->data.ranges.lengths = lengths;
	prop->data.ranges.n_trips = n_trips;
}

errval_t dtb_recursive_property_rewrite(struct dtNode *node)
{
	errval_t err = ERR_OK;

	u32 next_address_cells = node->address_cells;
	u32 next_size_cells = node->size_cells;

	ASSERT(node != NULL, "Node must not be NULL for property rewriting.");
	// Rewrite properties of the current node
	for (struct dtProperty *prop = node->properties; prop != NULL; prop = prop->next) {
		if (strcmp(prop->name, "compatible") == 0) {
			dtb_rewrite_property_compatible(prop);
		} else if (strcmp(prop->name, "model") == 0) {
			const char *value = prop->data.raw.value;
			prop->type = DTB_PROP_MODEL;
			prop->data.model = value;
		} else if (strcmp(prop->name, "phandle") == 0 || strcmp(prop->name, "linux,phandle") == 0) {
			prop->type = DTB_PROP_PHANDLE;
			prop->data.phandle = READ_BIG_ENDIAN_U32(prop->data.raw.value);
		} else if (strcmp(prop->name, "status") == 0) {
			dtb_rewrite_property_status(prop);
		} else if (strcmp(prop->name, "#address-cells") == 0) {
			prop->type = DTB_PROP_ADDRESS_CELLS;
			prop->data.address_cells = READ_BIG_ENDIAN_U32(prop->data.raw.value);
			if (prop->data.address_cells > 3) {
				return ERR_DTB_ADDRESS_CELLS_TOO_LARGE;
			}
			next_address_cells = prop->data.address_cells;
		} else if (strcmp(prop->name, "#size-cells") == 0) {
			prop->type = DTB_PROP_SIZE_CELLS;
			prop->data.size_cells = READ_BIG_ENDIAN_U32(prop->data.raw.value);
			if (prop->data.size_cells > 2) {
				return ERR_DTB_SIZE_CELLS_TOO_LARGE;
			}
			next_size_cells = prop->data.size_cells;
		} else if (strcmp(prop->name, "dma-coherent") == 0) {
			prop->type = DTB_PROP_DMA_COHERENCE;
			prop->data.dma_coherence = true;
		} else if (strcmp(prop->name, "dma-noncoherent") == 0) {
			prop->type = DTB_PROP_DMA_COHERENCE;
			prop->data.dma_coherence = false;
		} else if (strcmp(prop->name, "device_type") == 0) {
			prop->type = DTB_PROP_DEVICE_TYPE;
			prop->data.device_type = prop->data.raw.value;
		} else if (strcmp(prop->name, "reg") == 0) {
			dtb_rewrite_property_reg(prop, node->address_cells, node->size_cells);
		} else if (strcmp(prop->name, "ranges") == 0) {
			dtb_rewrite_property_ranges(prop, node->address_cells, node->size_cells);
		} else if (strcmp(prop->name, "dma-ranges") == 0) {
			dtb_rewrite_property_ranges(prop, node->address_cells, node->size_cells);
			prop->type = DTB_PROP_DMA_RANGES;
		} else {
			println("[dtb_recursive_property_rewrite] Unhandled property: %s", prop->name);
		}
	}

	// Recursively rewrite properties for child nodes
	for (struct dtNode *child = node->children; child != NULL; child = child->sibling) {
		child->address_cells = next_address_cells;
		child->size_cells = next_size_cells;
		if (err_is_fail((err = dtb_recursive_property_rewrite(child)))) {
			return err;
		}
	}

	return ERR_OK;
}

void dtb_recursive_print(size_t depth, struct dtNode *node)
{
	if (node == NULL) {
		return;
	}

	// Print the current node's name with indentation based on depth
	for (size_t i = 0; i < depth * 2; i++) {
		print(" ");
	}
	println("Node: %s", node->name);

	// Print properties of the current node
	for (struct dtProperty *prop = node->properties; prop != NULL; prop = prop->next) {
		for (size_t i = 0; i < depth * 2; i++) {
			print(" ");
		}
		print("\t");
		switch (prop->type) {
		case DTB_PROP_RAW:
			println("Unprocessed Property: %s, Value length: %d", prop->name, prop->data.raw.value_len);
			break;
		case DTB_PROP_COMPATIBLE:
			print("Property: compatible, Values: [");
			for (size_t i = 0; prop->data.compat[i] != NULL; i++) {
				if (i > 0) {
					print(", ");
				}
				print("\"%s\"", prop->data.compat[i]);
			}
			println("]");
			break;
		case DTB_PROP_MODEL:
			println("Property: model, Value: \"%s\"", prop->data.model);
			break;
		case DTB_PROP_PHANDLE:
			println("Property: phandle, Value: %d", prop->data.phandle);
			break;
		case DTB_PROP_STATUS:
			switch (prop->data.status.value) {
			case DTB_PROP_STATUS_OK:
				println("Property: status, Value: okay");
				break;
			case DTB_PROP_STATUS_DISABLED:
				println("Property: status, Value: disabled");
				break;
			case DTB_PROP_STATUS_RESERVED:
				println("Property: status, Value: reserved");
				break;
			case DTB_PROP_STATUS_FAIL:
				println("Property: status, Value: fail");
				break;
			case DTB_PROP_STATUS_FAIL_SSS:
				println("Property: status, Value: fail - %s", prop->data.status.reason);
				break;
			}
			break;
		case DTB_PROP_ADDRESS_CELLS:
			println("Property: #address-cells, Value: %d", prop->data.address_cells);
			break;
		case DTB_PROP_SIZE_CELLS:
			println("Property: #size-cells, Value: %d", prop->data.size_cells);
			break;
		case DTB_PROP_DMA_COHERENCE:
			println("Property: dma-coherence, Value: %s", prop->data.dma_coherence ? "true" : "false");
			break;
		case DTB_PROP_DEVICE_TYPE:
			println("Property: device_type, Value: \"%s\"", prop->data.device_type);
			break;
		case DTB_PROP_REG:
			print("Property: reg, Address Cells: %d, Size Cells: %d, Pairs: [",
			      node->address_cells, node->size_cells);
			for (size_t i = 0; i < prop->data.reg.n_pairs; i++) {
				if (i > 0) {
					print(", ");
				}
				switch (node->address_cells) {
				case 1:
					print("(Addr: %x", ((u32 *)prop->data.reg.addresses)[i]);
					break;
				case 2:
					print("(Addr: %x", ((u64 *)prop->data.reg.addresses)[i]);
					break;
				case 3:
					print("(Addr: %x", ((u128 *)prop->data.reg.addresses)[i]);
					break;
				default:
					print("(Addr: N/A)");
				}

				switch (node->size_cells) {
				case 1:
					print(", Size: %x)", ((u32 *)prop->data.reg.sizes)[i]);
					break;
				case 2:
					print(", Size: %x)", ((u64 *)prop->data.reg.sizes)[i]);
					break;
				default:
					assert(prop->data.reg.sizes == NULL);
					print(", Size: N/A)");
					break;
				}
			}
			println("]");
			break;
		case DTB_PROP_RANGES:
			print("Property: ranges, Address Cells: %d, Size Cells: %d, Triplets: [",
			      node->address_cells, node->size_cells);
			for (size_t i = 0; i < prop->data.ranges.n_trips; i++) {
				if (i > 0) {
					print(", ");
				}

				switch (node->address_cells) {
				case 1:
					print("(Child: %x, Parent: %x",
					      ((u32 *)prop->data.ranges.child_bus_addrs)[i],
					      ((u32 *)prop->data.ranges.parent_bus_addrs)[i]);
					break;
				case 2:
					print("(Child: %x, Parent: %x",
					      ((u64 *)prop->data.ranges.child_bus_addrs)[i],
					      ((u64 *)prop->data.ranges.parent_bus_addrs)[i]);
						break;
				case 3:
					print("(Child: %x, Parent: %x",
					      ((u128 *)prop->data.ranges.child_bus_addrs)[i],
					      ((u128 *)prop->data.ranges.parent_bus_addrs)[i]);
					break;
				default:
					__builtin_unreachable();
				}

				switch (node->size_cells) {
				case 1:
					print(", Length: %x)", ((u32 *)prop->data.ranges.lengths)[i]);
					break;
				case 2:
					print(", Length: %x)", ((u64 *)prop->data.ranges.lengths)[i]);
					break;
				default:
					print(", Length: %x)", ((u128 *)prop->data.ranges.lengths)[i]);
					break;
			}
			}
			println("]");
			break;
		case DTB_PROP_DMA_RANGES:
			print("Property: dma-ranges, Address Cells: %d, Size Cells: %d, Triplets: [",
			      node->address_cells, node->size_cells);
			for (size_t i = 0; i < prop->data.ranges.n_trips; i++) {
				if (i > 0) {
					print(", ");
				}

				switch (node->address_cells) {
				case 1:
					print("(Child: %x, Parent: %x",
					      ((u32 *)prop->data.dma_ranges.child_bus_addrs)[i],
					      ((u32 *)prop->data.dma_ranges.parent_bus_addrs)[i]);
					break;
				case 2:
					print("(Child: %x, Parent: %x",
					      ((u64 *)prop->data.dma_ranges.child_bus_addrs)[i],
					      ((u64 *)prop->data.dma_ranges.parent_bus_addrs)[i]);
					break;
				case 3:
					print("(Child: %x, Parent: %x",
					      ((u128 *)prop->data.dma_ranges.child_bus_addrs)[i],
					      ((u128 *)prop->data.dma_ranges.parent_bus_addrs)[i]);
					break;
				default:
					__builtin_unreachable();
				}

				switch (node->size_cells) {
				case 1:
					print(", Length: %x)", ((u32 *)prop->data.dma_ranges.lengths)[i]);
					break;
				case 2:
					print(", Length: %x)", ((u64 *)prop->data.dma_ranges.lengths)[i]);
					break;
				default:
					print(", Length: N/A)");
					break;
				}
			}
			println("]");
			break;
		}
	}

	// Recursively print child nodes
	for (struct dtNode *child = node->children; child != NULL; child = child->sibling) {
		dtb_recursive_print(depth + 1, child);
	}
}

void dtb_print_tree(void)
{
	println("[dtb_print_tree] Printing the device tree structure:");

	// Recursively print the device tree structure
	dtb_recursive_print(0, &state.nodes.data[0]);
}

///////////////////////////////////////////////////////////////////////////////
// Device tree API
///////////////////////////////////////////////////////////////////////////////

errval_t dt_initialize(paddr_t dtb_base_addr)
{
	println("[dt_parse] Parsing DTB at address: %x", dtb_base_addr);

	// Map the DTB base address to the kernel's page table.
	sv39_pageTable *root = sv39_kernel_page_table();
	paddr_t aligned_base = ALIGN_DOWN(dtb_base_addr, BASE_PAGE_SIZE);
	errval_t err = sv39_map(root, aligned_base, aligned_base, SV39_FLAGS_READ, sv39_Page);
	if (err_is_fail(err)) {
		return err_push(err, ERR_DTB_MAPPING_FAILED);
	}

	// The dtb header integers are all stored in big-endian format, and with our system being little-endian,
	// we need to ensure that we read them correctly.
	struct dtb_header *header = (struct dtb_header *)dtb_base_addr;

	// Check if the dtb header is valid by checking the magic number
	if (0x0D00DFEED != ENDIANNESS_FLIP_U32(header->magic)) {
		return ERR_DTB_MAGIC_NUMBER;
	}

	// Given the length of the DTB, we may need to map more than one page into the kernel's vspace
	size_t dtb_size = ENDIANNESS_FLIP_U32(header->totalsize);
	for (paddr_t pa = aligned_base + BASE_PAGE_SIZE; pa < dtb_base_addr + dtb_size; pa += BASE_PAGE_SIZE) {
		err = sv39_map(root, pa, pa, SV39_FLAGS_READ, sv39_Page);
		if (err_is_fail(err)) {
			return err_push(err, ERR_DTB_MAPPING_FAILED);
		}
	}

	// Initialize the dt structure
	state.reserved_memory = ARRAY_INIT(STRUCT(dtReservedRegion));
	state.nodes = ARRAY_INIT(STRUCT(dtNode));
	state.properties = ARRAY_INIT(STRUCT(dtProperty));

	// Allocate memory for the bumpAllocator allocator, which we will use to allocate strings and other device
	// tree structures.
	u8 *allocator_buf = NULL;
	err = pmm_alloc(2 * BASE_PAGE_SIZE, &allocator_buf);
	if (err_is_fail(err))
		return err;
	bump_init(&state.bump, allocator_buf, 2 * BASE_PAGE_SIZE);

	// Parse the Memory Reservation Block
	u64 *mem_rsvmap = (u64 *)(dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_mem_rsvmap));
	while (mem_rsvmap[0] != 0 && mem_rsvmap[1] != 0) {
		struct dtReservedRegion rr;
		rr.address = ENDIANNESS_FLIP_U64(mem_rsvmap[0]);
		rr.size = ENDIANNESS_FLIP_U64(mem_rsvmap[1]);
		ARRAY_PUSH(state.reserved_memory, rr);
	}

	// Parse the structure and string blocks.
	u8 *structures = (u8 *)(dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_dt_struct));
	u8 *strings = (u8 *)(dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_dt_strings));

	struct dtNode *root_node = NULL;
	size_t off = 0;
	size_t depth = 0;

	struct dtNode *curr = root_node;
	for (;;) {
		ASSERT(off % 4 == 0, "Accesses must be 4 bytes aligned.");
		u32 token = READ_BIG_ENDIAN_U32(structures + off);
		off += sizeof(u32);

		switch (token) {
		case FDT_BEGIN_NODE:
			off = dtb_parse_node(&curr, structures, off);
			if (depth == 0 && curr->name[0] == '\0') {
				// We just parsed the root node, if it's not named we set it to "/"
				curr->name = "/";
			}
			depth++;
			break;

		case FDT_END_NODE:
			curr = curr->parent;
			depth--;
			break;

		case FDT_PROP:
			off = dtb_parse_property(curr, structures, strings, off);
			break;

		case FDT_NOP:
			break;

		case FDT_END:
			if (curr != NULL) {
				PANIC_LOOP("FDT_END token found, but current node is not the root node. Depth: %d",
					   depth);
			}
			goto dtb_rewrite_pass;
		default:
			PANIC_LOOP("Unknown structure type: %x", token);
		}
	}

dtb_rewrite_pass:
	// The first allocated node is the root node, so we can set it as the root of the device tree.
	if (ARRAY_SIZE(state.nodes) == 0) {
		return ERR_DTB_NO_NODES;
	}
	state.root = &state.nodes.data[0];

	// Now that we have parsed the device tree, we can rewrite properties as needed.
	state.root->address_cells = 2;
	state.root->size_cells = 1;
	err = dtb_recursive_property_rewrite(state.root);
	if (err_is_fail(err))
		return err_push(err, ERR_DTB_REWRITE_FAILED);

	dtb_print_tree();
	println("bump free memory: %x bytes", state.bump.size - state.bump.index);

	// Unmap the DTB pages from the kernel's page table.
	for (paddr_t pa = aligned_base; pa < dtb_base_addr + dtb_size; pa += BASE_PAGE_SIZE) {
		RESULT(paddr_t) res = sv39_unmap(root, pa);
		if (RESULT_IS_ERR(res)) {
			return err_push(RESULT_ERR(res), ERR_DTB_UNMAPPING_FAILED);
		}
	}

	return ERR_OK;
}

bool dt_is_initialized(void)
{
	return state.initialized;
}

struct dtNode *dt_lookup_node(const char *path)
{
	(void) path;
	// Walk through the path, starting from the root node.
	// struct dtNode *curr = state.root;
	return NULL; // Placeholder for the actual implementation

}