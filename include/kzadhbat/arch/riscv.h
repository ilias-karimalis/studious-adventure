#pragma once

#include <kzadhbat/freestanding.h>

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

_Noreturn inline __attribute__((always_inline)) void wfi(void)
{
	asm volatile("wfi");
	__builtin_unreachable();
}

#define GENERATE_CSR_FUNCTIONS(reg)                                             \
	static inline __attribute__((always_inline)) void csrw_##reg(u64 value) \
	{                                                                       \
		asm volatile("csrw " #reg ", %0 " : : "r"(value) : "memory");        \
	} \
	\
	static inline __attribute__((always_inline)) u64 csrr_##reg(void) \
	{                                                                       \
		u64 value;                                                       \
		asm volatile("csrr %0, " #reg : "=r"(value) : : "memory");       \
		return value;                                                   \
	}


// Generated CSR functions for machine mode registers:
GENERATE_CSR_FUNCTIONS(mideleg)
GENERATE_CSR_FUNCTIONS(pmpaddr0)
GENERATE_CSR_FUNCTIONS(pmpcfg0)

///////////////////////////////////////////////////////////////////////////////
// Supervisor mode functions:
///////////////////////////////////////////////////////////////////////////////

// Generated CSR functions for supervisor mode registers:
GENERATE_CSR_FUNCTIONS(sstatus)
GENERATE_CSR_FUNCTIONS(stvec)
GENERATE_CSR_FUNCTIONS(sip)
GENERATE_CSR_FUNCTIONS(sie)
GENERATE_CSR_FUNCTIONS(scounteren)
GENERATE_CSR_FUNCTIONS(sscratch)
GENERATE_CSR_FUNCTIONS(sepc)
GENERATE_CSR_FUNCTIONS(scause)
GENERATE_CSR_FUNCTIONS(stval)
GENERATE_CSR_FUNCTIONS(senvcfg)
GENERATE_CSR_FUNCTIONS(satp)

// Fences
static inline __attribute__((always_inline)) void sfence_vma(void)
{
	asm volatile("sfence.vma" : : : "memory");
}

// Return from trap in S-Mode
static inline __attribute__((always_inline)) void sret(void)
{
	asm volatile("sret" : : : "memory");
}


// Gernerated CSR functions for user mode registers:
GENERATE_CSR_FUNCTIONS(time)
GENERATE_CSR_FUNCTIONS(cycle)
GENERATE_CSR_FUNCTIONS(instret)

