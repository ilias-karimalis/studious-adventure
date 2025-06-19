#include <octiron/trap.h>

#define TRAP_ASYNC(cause) (((cause) >> 63) & 0x1)

#define REGISTER_COUNT 32
#define FP_REGISTER_COUNT 32


struct trap_frame {
	u64 registers[REGISTER_COUNT];
};

//size_t machine_mode_trap(size_t epc)
