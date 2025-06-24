#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/types/error.h>
#include <kzadhbat/types/result.h>
#include <kzadhbat/collections/bump_allocator.h>

#include <octiron/collections/array.h>

// Struct forward declarations
struct dt;
struct dtNode;
struct dtReservedRegion;
struct dtProperty;
struct dtPropRaw;
struct dtPropStatus;
struct dtPropReg;

errval_t dt_parse(paddr_t dtb_base_addr);


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
	/// The number of <u32> cells used to encode the address field in this property
	u32 address_cells;
	/// The number of <u32> cells used to encode the size field in this property
	u32 size_cells;
	/// The number of (address, size) pairs.
	u32 n_pairs;
	/// List of addresses. NULL if address_cells == 0.
	void* addresses;
	/// List of sizes corresponding to the addresses. NULL if size_cells == 0.
	void* sizes;
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

	} data;
};

struct dtNode {
	/// Name of the node
	const char *name;
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
};
