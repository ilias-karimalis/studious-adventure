#include <kernel/paging.h>

#define IS_ALIGNED(addr, alignment) (((addr) & ((alignment) - 1)) == 0)

#define SV39_PTE_IS_VALID(entry) (((entry) & SV39_FLAGS_VALID) != 0)

/// The sizes of leaf pages in the SV39 paging system.
u64 sv39_pageSizes[] = {
	0x1000, 	///< 4 KiBs
	0x200000, 	///< 2 MiBs
	0x40000000, ///< 1 GiB
};

/// The root page table for the kernel, initialized to zero.
sv39_tableEntry kernel_root[SV39_TableEntryCount] = { 0 };

sv39_tableEntry* sv39_paging_init() {
    ASSERT(kernel_root[0] == 0, "root page table is not 0 initialized");
    return &kernel_root;
}

// Forward declarations of internal functions
errval_t sv39_map_small_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags);
errval_t sv39_map_mega_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags);
errval_t sv39_map_giga_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags);

errval_t sv39_map(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags, enum sv39_pageType type) {
    if (root == NULL) {
        return ERR_NULL_ARGUMENT;
    }

    switch (type) {
        case sv39_Page:
            return sv39_map_small_page(root, va, pa, flags);
        case sv39_MegaPage:
            return sv39_map_mega_page(root, va, pa, flags);
        case sv39_GigaPage:
            return sv39_map_giga_page(root, va, pa, flags);
        default:
            return ERR_PAGING_INVALID_TYPE;
    }
}

errval_t sv39_map_small_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags) {
    errval_t err = err_new();
    if (!IS_ALIGNED(va, sv39_pageSizes[sv39_Page]) || !IS_ALIGNED(pa, sv39_pageSizes[sv39_Page])) {
        return ERR_PAGING_UNALIGNED_ADDRESS;
    }
    fmtprint("[sv39_map_small_page] Mapping VA: %x to PA: %x with flags: %x\n", va, pa, flags);

    vaddr_t vpn[] = {
        (va >> 12) & 0x1FF, // Level 0 index
        (va >> 21) & 0x1FF, // Level 1 index
        (va >> 30) & 0x1FF  // Level 2 index
    };
    paddr_t ppn[] = {
        (pa >> 12) & 0x1FF,     // PPN[0] = pa[20:12]
        (pa >> 21) & 0x1FF,     // PPN[1] = pa[29:21]
        (pa >> 30) & 0x3FFFFFF, // PPN[2] = pa[55:30]
    };

    sv39_tableEntry* l2 = root;
    sv39_tableEntry* l1 = NULL;
    sv39_tableEntry  l2_entry = l2[vpn[2]];
    if (!SV39_PTE_IS_VALID (l2_entry)) {
        // Allocate a new Level 1 page table if it doesn't exist
        u8* page = NULL;
        err = pmm_alloc(sizeof(sv39_pageTable), (u8**)&page);
        if (err_is_fail(err)) {
            return err_push(err, ERR_PAGING_SETUP_TABLE);
        }
        // Place the new Level 1 page table in the Level 2 table
        l2[vpn[2]] = ((sv39_tableEntry)page >> 2) | SV39_FLAGS_VALID;
    }
    l1 = (sv39_tableEntry*) ((l2[vpn[2]] >> 10) << 12);

    sv39_tableEntry l1_entry = l1[vpn[1]];
    if (!SV39_PTE_IS_VALID(l1_entry)) {
        // Allocate a new Level 0 page table if it doesn't exist
        u8* page = NULL;
        err = pmm_alloc(sizeof(sv39_pageTable), (u8**)&page);
        if (err_is_fail(err)) {
            return err_push(err, ERR_PAGING_SETUP_TABLE);
        }
        // Place the new Level 0 page table in the Level 1 table
        l1[vpn[1]] = ((sv39_tableEntry)page >> 2) | SV39_FLAGS_VALID;
    }
    sv39_tableEntry* l0 = (sv39_tableEntry*) ((l1[vpn[1]] >> 10) << 12);

    // Place the new page in the Level 0 table.
    if (SV39_PTE_IS_VALID(l0[vpn[0]])) {
        return err_push(err, ERR_PAGING_MAPPING_EXISTS);
    }
    l0[vpn[0]] = (ppn[0] << 10) | (ppn[1] << 19) | (ppn[2] << 28) | flags | SV39_FLAGS_VALID;
    sv39_print_page_table(root);
    return ERR_OK;
}

errval_t sv39_map_mega_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags) {
    return ERR_NOT_IMPLEMENTED;
}

errval_t sv39_map_giga_page(sv39_tableEntry* root, vaddr_t va, paddr_t pa, u64 flags) {
    return ERR_NOT_IMPLEMENTED;
}

errval_t sv39_unmap(sv39_tableEntry* root, vaddr_t va) {
    return ERR_NOT_IMPLEMENTED;
}

void sv39_print_page_table(sv39_tableEntry* root) {
    if (root == NULL) {
        println("sv39_print_page_table: root is NULL");
        return;
    }

    println("SV39 Page Table:");
    for (int i = 0; i < SV39_TableEntryCount; i++) {
        sv39_tableEntry entry = root[i];
        if (SV39_PTE_IS_VALID(entry)) {
            fmtprint("Entry %d: PPN: %x, Flags: %x\n", i, entry >> 10, entry & 0x3FF);
        } 
    }
}