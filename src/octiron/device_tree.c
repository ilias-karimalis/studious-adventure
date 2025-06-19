#include <octiron/device_tree.h>
#include <octiron/paging.h>
//#include <octiron/array.h>
#include <kzadhbat/bitmacros.h>
#include <kzadhbat/arch/riscv.h>

/// The parsed state of the Device Tree Blob (DTB).
/// As of now, I'm assuming that the kernel will only ever need to parse a single DTB.
struct dtb_state dtb_state = { 0 };

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

#define READ_U32(ptr, offset) (ENDIANNESS_FLIP_U32(((u32*) (ptr))[offset / sizeof(u32)]))

#define FDT_BEGIN_NODE	((u32) 0x00000001)
#define FDT_END_NODE	((u32) 0x00000002)
#define FDT_PROP	((u32) 0x00000003)
#define FDT_NOP		((u32) 0x00000004)
#define FDT_END		((u32) 0x00000009)

size_t dtb_parse_node(struct dtb_node **curr, u8 *structures, size_t off);
size_t dtb_parse_property(struct dtb_node *curr, u8 *structures, u8 *strings, size_t off);
void dtb_print_tree(void);


RESULT(PTR(STRUCT(dtb_state))) dtb_parse(vaddr_t dtb_base_addr)
{
	println("[dtb_parse] Parsing DTB at address: %x", dtb_base_addr);

	// Map the DTB base address to the kernel's page table.
	sv39_pageTable *root = sv39_kernel_page_table();
	paddr_t aligned_base = ALIGN_DOWN(dtb_base_addr, BASE_PAGE_SIZE);
	errval_t err = sv39_map(root, aligned_base, aligned_base, SV39_FLAGS_READ | SV39_FLAGS_EXECUTE, sv39_Page);
	if (err_is_fail(err))
	{
		return RESULT_ERR(PTR(STRUCT(dtb_state)), err_push(err, ERR_DTB_MAPPING_FAILED));
	}

	// The dtb header integers are all stored in big-endian format, and with our system being little-endian,
	// we need to ensure that we read them correctly.
	struct dtb_header* header = (struct dtb_header *)dtb_base_addr;

	// Check if the dtb header is valid by checking the magic number
	if (0x0D00DFEED != ENDIANNESS_FLIP_U32(header->magic))
	{
		return RESULT_ERR(PTR(STRUCT(dtb_state)), ERR_DTB_MAGIC_NUMBER);
	}

	// Print the header information
	println("DTB Header:");
	println("  Magic: %x", (u64) ENDIANNESS_FLIP_U32(header->magic));
	println("  Total Size: %x bytes", ENDIANNESS_FLIP_U32(header->totalsize));
	println("  Off DT Struct: %x bytes", ENDIANNESS_FLIP_U32(header->off_dt_struct));
	println("  Off DT Strings: %x bytes", ENDIANNESS_FLIP_U32(header->off_dt_strings));
	println("  Off Mem Rsvmap: %x bytes", ENDIANNESS_FLIP_U32(header->off_mem_rsvmap));
	println("  Version: %d", ENDIANNESS_FLIP_U32(header->version));
	println("  Last Compiled Version: %d", ENDIANNESS_FLIP_U32(header->last_comp_version));
	println("  Boot CPU ID Phys: %d", ENDIANNESS_FLIP_U32(header->boot_cpuid_phys));
	println("  Size DT Strings: %x bytes", ENDIANNESS_FLIP_U32(header->size_dt_strings));
	println("  Size DT Struct: %x bytes", ENDIANNESS_FLIP_U32(header->size_dt_struct));

	// Given the length of the DTB, we may need to map more than one page into the kernel's vspace
	size_t dtb_size = ENDIANNESS_FLIP_U32(header->totalsize);
	for (paddr_t pa = aligned_base + BASE_PAGE_SIZE; pa < dtb_base_addr + dtb_size; pa += BASE_PAGE_SIZE)
	{
		println("[dtb_parse] Mapping additional DTB page at address: %x", pa);
		err = sv39_map(root, pa, pa, SV39_FLAGS_READ | SV39_FLAGS_EXECUTE, sv39_Page);
		if (err_is_fail(err))
		{
			return RESULT_ERR(PTR(STRUCT(dtb_state)), err_push(err, ERR_DTB_MAPPING_FAILED));
		}
	}

	// Initialize the dtb_state structure
	dtb_state.reserved_memory = ARRAY_INIT(STRUCT(dtb_reservedRegion));
	dtb_state.nodes = ARRAY_INIT(STRUCT(dtb_node));
	dtb_state.properties = ARRAY_INIT(STRUCT(dtb_property));

	// Allocate memory for the string bump allocator, which will be used to store the strings in the DTB.
	u8* allocator_buf = NULL;
	err = pmm_alloc(32 * BASE_PAGE_SIZE, &allocator_buf);
	if (err_is_fail(err)) { return RESULT_ERR(PTR(STRUCT(dtb_state)), err); }
	bump_init(&dtb_state.allocator, allocator_buf, 2 * BASE_PAGE_SIZE);


	// Parse the Memory Reservation Block
	u64* mem_rsvmap = (u64 *) (dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_mem_rsvmap));
	while (mem_rsvmap[0] != 0 && mem_rsvmap[1] != 0) {
		struct dtb_reservedRegion rr;
		rr.address = ENDIANNESS_FLIP_U64(mem_rsvmap[0]);
		rr.size    = ENDIANNESS_FLIP_U64(mem_rsvmap[1]);
		ARRAY_PUSH(dtb_state.reserved_memory, rr);
	}

	// Parse the structure and string blocks.
	u8* structures = (u8 *)(dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_dt_struct));
	u8* strings = (u8 *)(dtb_base_addr + ENDIANNESS_FLIP_U32(header->off_dt_strings));

	// Place and initialize the root node.
	ARRAY_PUSH(dtb_state.nodes, ((struct dtb_node) {
					    .name = NULL,
					    .properties = NULL,
					    .parent = NULL,
					    .children = NULL,
					    .sibling = NULL
				    }));
	struct dtb_node* root_node = &dtb_state.nodes.data[0];
	size_t off = 0;
	size_t depth = 0;

	struct dtb_node* curr = root_node;
	for (;;) {
		ASSERT(off % 4 == 0, "Accesses must be 4 bytes aligned.");
		u32 token = READ_U32(structures, off);
		off += sizeof(u32);

		switch (token) {
		case FDT_BEGIN_NODE:
//			println("FDT_BEGIN_NODE Token");
			off = dtb_parse_node(&curr, structures, off);
			depth++;
			break;

		case FDT_END_NODE:
//			println("FDT_END_NODE Token");
			curr = curr->parent;
			depth--;
			break;

		case FDT_PROP:
//			println("FDT_PROP Token");
			off = dtb_parse_property(curr, structures, strings, off);
			break;

		case FDT_NOP:
//			println("FDT_NOP Token");
			break;

		case FDT_END:
//			println("FDT_END Token");
			if (curr != root_node) {
				PANIC_LOOP("FDT_END token found, but current node is not the root node. Depth: %d", depth);
			}
			dtb_print_tree();
			TODO("DTB Parsing isn't complete!");
			return RESULT_OK(PTR(STRUCT(dtb_state)), &dtb_state);
		default:
			PANIC_LOOP("Unknown structure type: %x", token);
		}
	}
}


size_t dtb_parse_property(struct dtb_node *curr, u8 *structures, u8 *strings, size_t off)
{
	u32 prop_len = READ_U32(structures, off);
	off += sizeof(u32);
	u32 name_offset = READ_U32(structures, off);
	off += sizeof(u32);

	size_t prop_name_len = strlen((const char *)&strings[name_offset]) + 1;
	char *prop_name = (char *)bump_alloc(&dtb_state.allocator, prop_name_len);
	ASSERT(prop_name != NULL, "Bump alloc should not return NULL");
	memcpy(prop_name, &strings[name_offset], prop_name_len);

	void* prop_value = NULL;
	if (prop_len != 0) {
		prop_value = bump_alloc(&dtb_state.allocator, prop_len);
		ASSERT(prop_value != NULL, "Bump alloc should not return NULL");
	}
	memcpy(prop_value, &structures[off], prop_len);
	off += ALIGN_UP(prop_len, sizeof(u32));

	ARRAY_PUSH(dtb_state.properties, ((struct dtb_property){
		.name = prop_name,
		.value = prop_value,
		.value_len = prop_len,
		.next = NULL
	}));
	struct dtb_property *new = &dtb_state.properties.data[ARRAY_SIZE(dtb_state.properties) - 1];

	new->next = curr->properties;
	curr->properties = new;
//	if (curr->properties == NULL) {
//		curr->properties = new;
//	} else {
//		struct dtb_property *last_prop = curr->properties;
//		while (last_prop->next != NULL) {
//			last_prop = last_prop->next;
//		}
//		last_prop->next = new;
//	}
	return off;
}

size_t dtb_parse_node(struct dtb_node **curr, u8 *structures, size_t off)
{
	const char *node_name = (const char *)&structures[off];
	size_t name_len = strlen(node_name) + 1;
	char *name_buf = bump_alloc(&dtb_state.allocator, name_len);
	ASSERT(name_buf != NULL, "Failed to allocate memory for node name.");
	memcpy(name_buf, node_name, name_len);

	ARRAY_PUSH(dtb_state.nodes,
		   ((struct dtb_node){
			   .name = name_buf,
			   .properties = NULL,
			   .parent = *curr,
			   .children = NULL,
			   .sibling = NULL
		   }));
	struct dtb_node *new = &dtb_state.nodes.data[ARRAY_SIZE(dtb_state.nodes) - 1];

	if ((*curr)->children == NULL) {
		(*curr)->children = new;
	} else {
		struct dtb_node *last_child = (*curr)->children;
		while (last_child->sibling != NULL) {
			last_child = last_child->sibling;
		}
		last_child->sibling = new;
	}
	*curr = new;
	return off + ALIGN_UP(name_len, sizeof(u32)); // Move the offset past the node name
}

void dtb_recursive_print(size_t depth, struct dtb_node *node) {
	if (node == NULL) {
		return;
	}

	// Print the current node's name with indentation based on depth
	for (size_t i = 0; i < depth * 2; i++) {
		print(" ");
	}
	println("Node: %s", node->name);

	// Print properties of the current node
	for (struct dtb_property *prop = node->properties; prop != NULL; prop = prop->next) {
		for (size_t i = 0; i < depth * 2; i++) {
			print(" ");
		}
		if (prop->value_len == 0) {
			println("\tProperty: %s, Value length: 0, Value: <empty>", prop->name);
		} else {
			println("\tProperty: %s, Value length: %d, Value: %s", prop->name, prop->value_len, prop->value);
		}
	}

	// Recursively print child nodes
	for (struct dtb_node *child = node->children; child != NULL; child = child->sibling) {
		dtb_recursive_print(depth + 1, child);
	}

}


void dtb_print_tree(void) {
	println("[dtb_print_tree] Printing the device tree structure:");

	// Recursively print the device tree structure
	dtb_recursive_print(0, &dtb_state.nodes.data[0]);
}


//void dtb_recursive_print(u32* structure_block, u32* string_block, size_t i)
//{
//	switch (ENDIANNESS_FLIP_U32(structure_block[i]))
//	{
//	case FDT_BEGIN_NODE:
//		i++;
//		const char* node_name = (const char*) &structure_block[i];
//		i += (strlen(node_name) + 3) / 4; // Move to the next word after the node name
//		println("FDT_BEGIN_NODE Token [ %s ]");
//		dtb_recursive_print(structure_block, string_block, i);
//		break;
//	case FDT_END_NODE:
//		println("FDT_END_NODE Token");
//		dtb_recursive_print(structure_block, string_block, ++i);
//		return;
//	case FDT_PROP:
//		i++;
//		u32 prop_len = ENDIANNESS_FLIP_U32(structure_block[i]);
//		i++;
//		u32 prop_nameoff = ENDIANNESS_FLIP_U32(structure_block[i]);
//		i++;
//		if (prop_len > 0) {
//			char* prop_value = bump_alloc(&dtb_state.allocator, prop_len + 1);
//			memcpy(prop_value, &structure_block[i], prop_len);
//			prop_value[prop_len] = '\0';
//			println("FDT_PROP Token [ %s, %s, %d ]", &string_block[prop_nameoff], prop_value, prop_len);
//		} else {
//			println("FDT_PROP Token [ %s, <empty> ]", &string_block[prop_nameoff]);
//		}
//		dtb_recursive_print(structure_block, string_block, i + ((prop_len + 3) / 4));
//		break;
//	case FDT_END:
//		println("FDT_END Token");
//		return;
//	default:
//		println("Unknown structure type: %x", ENDIANNESS_FLIP_U32(structure_block[i]));
//		dtb_recursive_print(structure_block, string_block, ++i);
//		break;
//	}
//
//}
