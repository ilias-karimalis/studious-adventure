#pragma once


#define BASE_PAGE_SIZE 4096

extern size_t HEAP_START;
extern size_t HEAP_END;
extern size_t HEAP_SIZE;
extern size_t TEXT_START;
extern size_t TEXT_END;
extern size_t DATA_START;
extern size_t DATA_END;
extern size_t RODATA_START;
extern size_t RODATA_END;
extern size_t BSS_START;
extern size_t BSS_END;
extern size_t STACK_START;
extern size_t STACK_END;

[[noreturn]] inline void wfi(void) {
    asm volatile ("wfi");
    __builtin_unreachable();
}
