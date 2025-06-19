#include <octiron/paging.h>

#define IS_ALIGNED(addr, alignment) (((addr) & ((alignment) - 1)) == 0)

#define SV39_PTE_VALID(pte) ((paddr_t)(pte) & 0x1)
#define SV39_PTE_READABLE(pte) (((sv39_tableEntry)(pte)) & 0x2)
#define SV39_PTE_WRITABLE(pte) (((sv39_tableEntry)(pte)) & 0x4)
#define SV39_PTE_EXECUTABLE(pte) (((sv39_tableEntry)(pte)) & 0x8)
#define SV39_PTE_USER(pte) (((sv39_tableEntry)(pte)) & 0x10)
#define SV39_PTE_GLOBAL(pte) (((sv39_tableEntry)(pte)) & 0x20)
#define SV39_PTE_ACCESSED(pte) (((sv39_tableEntry)(pte)) & 0x40)
#define SV39_PTE_DIRTY(pte) (((sv39_tableEntry)(pte)) & 0x80)
#define SV39_PTE_LEAF(pte) (SV39_PTE_READABLE(pte) || SV39_PTE_WRITABLE(pte) || SV39_PTE_EXECUTABLE(pte))
#define SV39_PTE_PPN(pte) (((sv39_tableEntry)(pte) >> 10) & 0xFFFFFFFFFFFULL)

/// The sizes of leaf pages in the SV39 paging system.
const u64 sv39_pageSizes[] = {
	0x1000, ///< 4 KiBs
	0x200000, ///< 2 MiBs
	0x40000000, ///< 1 GiB
};

/// The root page table for the kernel, initialized to zero.
alignas(0x1000) sv39_tableEntry kernel_root[SV39_TableEntryCount] = { 0 };

sv39_pageTable *sv39_kernel_page_table()
{
	return &kernel_root;
}

// Forward declarations of internal functions
errval_t sv39_map_small_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags);
errval_t sv39_map_mega_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags);
errval_t sv39_map_giga_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags);

errval_t sv39_map(sv39_pageTable *root, vaddr_t va, paddr_t pa, u64 flags, enum sv39_pageType type)
{
	if (root == NULL) {
		return ERR_NULL_ARGUMENT;
	}
	sv39_tableEntry* table = (sv39_tableEntry *) root;

	switch (type) {
	case sv39_Page:
		return sv39_map_small_page(table, va, pa, flags);
	case sv39_MegaPage:
		return sv39_map_mega_page(table, va, pa, flags);
	case sv39_GigaPage:
		return sv39_map_giga_page(table, va, pa, flags);
	default:
		return ERR_PAGING_INVALID_TYPE;
	}
}

errval_t sv39_map_small_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags)
{
	errval_t err = err_new();
	if (!IS_ALIGNED(va, sv39_pageSizes[sv39_Page]) || !IS_ALIGNED(pa, sv39_pageSizes[sv39_Page])) {
		return ERR_PAGING_UNALIGNED_ADDRESS;
	}

	vaddr_t vpn[] = {
		(va >> 12) & 0x1FF, // Level 0 index
		(va >> 21) & 0x1FF, // Level 1 index
		(va >> 30) & 0x1FF // Level 2 index
	};
	paddr_t ppn[] = {
		(pa >> 12) & 0x1FF, // PPN[0] = pa[20:12]
		(pa >> 21) & 0x1FF, // PPN[1] = pa[29:21]
		(pa >> 30) & 0x3FFFFFF, // PPN[2] = pa[55:30]
	};

	sv39_tableEntry *l2 = root;
	sv39_tableEntry *l1 = NULL;
	sv39_tableEntry l2_entry = l2[vpn[2]];
	if (!SV39_PTE_VALID(l2_entry)) {
		u8 *page = NULL;
		err = pmm_alloc(sizeof(sv39_pageTable), (u8 **)&page);
		if (err_is_fail(err)) {
			return err_push(err, ERR_PAGING_SETUP_TABLE);
		}
		l2[vpn[2]] = ((sv39_tableEntry)page >> 2) | SV39_FLAGS_VALID;
	}
	l1 = (sv39_tableEntry *)((l2[vpn[2]] >> 10) << 12);

	sv39_tableEntry l1_entry = l1[vpn[1]];
	if (!SV39_PTE_VALID(l1_entry)) {
		u8 *page = NULL;
		err = pmm_alloc(sizeof(sv39_pageTable), (u8 **)&page);
		if (err_is_fail(err)) {
			return err_push(err, ERR_PAGING_SETUP_TABLE);
		}
		l1[vpn[1]] = ((sv39_tableEntry)page >> 2) | SV39_FLAGS_VALID;
	}
	sv39_tableEntry *l0 = (sv39_tableEntry *)((l1[vpn[1]] >> 10) << 12);

	if (SV39_PTE_VALID(l0[vpn[0]])) {
		return err_push(err, ERR_PAGING_MAPPING_EXISTS);
	}
	l0[vpn[0]] = (ppn[0] << 10) | (ppn[1] << 19) | (ppn[2] << 28) | flags | SV39_FLAGS_VALID;
	return ERR_OK;
}

errval_t sv39_map_mega_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags)
{
	(void)root;
	(void)va;
	(void)pa;
	(void)flags;
	return ERR_NOT_IMPLEMENTED;
}

errval_t sv39_map_giga_page(sv39_tableEntry *root, vaddr_t va, paddr_t pa, u64 flags)
{
	(void)root;
	(void)va;
	(void)pa;
	(void)flags;
	return ERR_NOT_IMPLEMENTED;
}

errval_t sv39_unmap(sv39_pageTable *root, vaddr_t va)
{
	(void)root;
	(void)va;
	return ERR_NOT_IMPLEMENTED;
}

OPT(paddr_t) sv39_virt_to_phys(sv39_pageTable *root, vaddr_t va)
{
	vaddr_t vpn[] = {
		(va >> 12) & 0x1FF,
		(va >> 21) & 0x1FF,
		(va >> 30) & 0x1FF,
	};
	paddr_t page_offset = va & (sv39_pageSizes[sv39_Page]);

	sv39_tableEntry *table = (sv39_tableEntry *) root;
	for (int level = 2; level >= 0; level--) {
		sv39_tableEntry pte = table[vpn[level]];
		// Fail if we hit an invalid pte
		if (!SV39_PTE_VALID(pte)) {
			return OPT_NONE(paddr_t);
		}
		// Return early if we hit a leaf
		if (SV39_PTE_LEAF(pte)) {
			paddr_t ppn = SV39_PTE_PPN(pte);
			paddr_t pa;
			switch (level) {
			case 2:
				pa = (ppn << 30) | (vpn[1] << 21) | (vpn[0] << 12);
				break;
			case 1:
				pa = (ppn << 21) | (vpn[0] << 12);
				break;
			default:
				pa = ppn << 12;
				break;
			}
			return OPT_SOME(paddr_t, pa | page_offset);
		}
		// Non leaf
		paddr_t next_table_addr = SV39_PTE_PPN(pte) << 12;
		table = (sv39_tableEntry *)next_table_addr;
	}
	__builtin_unreachable();
}

void sv39_print_page_table(sv39_tableEntry *root)
{
	TODO("Implement this function such that it recursively prints out all the subsequent page tables.\n");
	if (root == NULL) {
		println("sv39_print_page_table: root is NULL");
		return;
	}

	println("SV39 Page Table:");
	for (int i = 0; i < SV39_TableEntryCount; i++) {
		sv39_tableEntry entry = root[i];
		if (SV39_PTE_VALID(entry)) {
			print("Entry %d: PPN: %x, Flags: %x\n", i, entry >> 10, entry & 0x3FF);
		}
	}
}

void sv39_print_table_entry(sv39_tableEntry pte)
{
	uint64_t ppn = (pte >> 10) & 0xFFFFFFFFFFFULL; // bits 10–53
	u64 phys_addr = (ppn << 12);
	int D = (pte >> 7) & 1;
	int A = (pte >> 6) & 1;
	int G = (pte >> 5) & 1;
	int U = (pte >> 4) & 1;
	int X = (pte >> 3) & 1;
	int W = (pte >> 2) & 1;
	int R = (pte >> 1) & 1;
	int V = (pte >> 0) & 1;

	print("SV39 Page Table Entry (raw): %x\n", pte);
	print("  Valid     (V): %d\n", V);
	print("  Read      (R): %d\n", R);
	print("  Write     (W): %d\n", W);
	print("  Execute   (X): %d\n", X);
	print("  User      (U): %d\n", U);
	print("  Global    (G): %d\n", G);
	print("  Accessed  (A): %d\n", A);
	print("  Dirty     (D): %d\n", D);
	print("  PPN          : %x\n", ppn);
	print("  Phys Addr.   : %x\n", phys_addr);
}
