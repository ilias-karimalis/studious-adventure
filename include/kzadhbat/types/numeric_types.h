#pragma once

#include <kzadhbat/freestanding.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

/// Virtual Address Type
typedef u64 vaddr_t;
/// Physical Address Type
typedef u64 paddr_t;

#define ENDIANNESS_FLIP_U16(x) (((x) >> 8) | ((x) << 8))
#define ENDIANNESS_FLIP_U32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000))
#define ENDIANNESS_FLIP_U64(x) ( \
	(((x) >> 56) & 0x00000000000000FFULL) | \
	(((x) >> 40) & 0x000000000000FF00ULL) | \
	(((x) >> 24) & 0x0000000000FF0000ULL) | \
	(((x) >> 8)  & 0x00000000FF000000ULL) | \
	(((x) << 8)  & 0x000000FF00000000ULL) | \
	(((x) << 24) & 0x0000FF0000000000ULL) | \
	(((x) << 40) & 0x00FF000000000000ULL) | \
	(((x) << 56) & 0xFF00000000000000ULL))

