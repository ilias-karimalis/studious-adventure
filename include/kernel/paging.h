#pragma once

#include <grouperlib/numeric_types.h>
#include <grouperlib/error.h>
#include <grouperlib/option.h>
#include <grouperlib/assert.h>
#include <kernel/pmm.h>

// sv39 Paging Implementation

#define SATP_MODE_SV39_FLAG ((u64)8 << 60)
#define SATP_PPN_MASK(addr) ((u64)(addr) >> 12)

#define SV39_TableEntryCount 512
#define SV39_Kernel_VA_Base 0xFFFFFFF800000000

DEFINE_OPTION_TYPE(paddr_t);

typedef u64 sv39_tableEntry;
typedef sv39_tableEntry sv39_pageTable[SV39_TableEntryCount];

/// The types of Leaf pages in the SV39 paging system.
enum sv39_pageType {
    sv39_Page,		///< 4 KiB Page
    sv39_MegaPage,	///< 2 MiB Page
    sv39_GigaPage,	///< 1 GiB Page
};

/// The flags for the SV39 table entry.
enum sv39_tableEntryFlags {
	SV39_FLAGS_VALID 	= 0b00000001, 	///< Valid bit
	SV39_FLAGS_READ 	= 0b00000010, 	///< Read bit
	SV39_FLAGS_WRITE 	= 0b00000100, 	///< Write bit
	SV39_FLAGS_EXECUTE 	= 0b00001000, 	///< Execute bit
	SV39_FLAGS_USER 	= 0b00010000, 	///< User bit
	SV39_FLAGS_GLOBAL 	= 0b00100000, 	///< Global bit
	SV39_FLAGS_ACCESSED = 0b01000000, 	///< Accessed bit
	SV39_FLAGS_DIRTY 	= 0b10000000, 	///< Dirty bit

	// Ensures that the enum is 8 bytes wide and matches the size of sv39_tableEntry
	SV39_FLAGS_MAKE_ENUM_64_BIT = 0xFFFFFFFFFFFFFFFF,
};
SASSERT(sizeof(enum sv39_tableEntryFlags) == sizeof(sv39_tableEntry), "sv39_tableEntryFlags must be the same size as sv39_tableEntry");

// #define SV39_KERNEL_BASE 0xFFFFFFF800000000

/// Initializes the SV39 paging system and returns a pointer to the root page table.
sv39_tableEntry* sv39_paging_init();
/// Maps a page into the virtual address space, given a root page table. If any level of the page table does not exist, it is created.
errval_t sv39_map(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags, enum sv39_pageType type);
/// Unmaps a page from the virtual address space, given a root page table.
errval_t sv39_unmap(sv39_tableEntry* root, vaddr_t va);
/// Walks the paage table translating a va to a pa if the mapping is present.
OPT(paddr_t) sv39_virt_to_phys(sv39_tableEntry* root, vaddr_t va);

/// Prints the contents of the page table for debugging purposes.
void sv39_print_page_table(sv39_tableEntry* root);
void sv39_print_table_entry(sv39_tableEntry pte);
