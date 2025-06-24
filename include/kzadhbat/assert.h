#pragma once

#include <kzadhbat/fmtprint.h>

#ifdef ENABLE_ASSERTIONS
#define ASSERT(expr, msg, ...)                                 \
	do {                                                   \
		if (!(expr)) {                                 \
			print(msg __VA_OPT__(, ) __VA_ARGS__); \
			for (;;) {                             \
			}                                      \
		}                                              \
	} while (0)
#else
#define ASSERT(expr, msg, ...) ((void)(expr))
#endif

#define assert(expr) ASSERT(expr, "Assertion failed: " #expr)


#define TODO(msg, ...)                                          \
	do {                                                    \
		print("TODO: " msg __VA_OPT__(, ) __VA_ARGS__); \
		for (;;) {                                      \
		}                                               \
	} while (0)

#define PANIC_LOOP(msg, ...)                                            \
	do {                                                            \
		print("KERNEL PANIC: " msg __VA_OPT__(, ) __VA_ARGS__); \
		for (;;) {                                              \
		}                                                       \
	} while (0)

