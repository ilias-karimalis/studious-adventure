#pragma once

#include <kzadhbat/types/numeric_types.h>

#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

/// Detect endianness and fail if not little-endian.
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "This code requires little-endian architecture"
#endif
#else
#error "This code requires little-endian architecture"
#endif

/// Flip the endianness of a 16-bit unsigned integer.
#define ENDIANNESS_FLIP_U16(x) (((x) >> 8) | ((x) << 8))

/// Flip the endianness of a 32-bit unsigned integer.
#define ENDIANNESS_FLIP_U32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000))

/// Flip the endianness of a 64-bit unsigned integer.
#define ENDIANNESS_FLIP_U64(x) ( \
	(((x) >> 56) & 0x00000000000000FFULL) | \
	(((x) >> 40) & 0x000000000000FF00ULL) | \
	(((x) >> 24) & 0x0000000000FF0000ULL) | \
	(((x) >> 8)  & 0x00000000FF000000ULL) | \
	(((x) << 8)  & 0x000000FF00000000ULL) | \
	(((x) << 24) & 0x0000FF0000000000ULL) | \
	(((x) << 40) & 0x00FF000000000000ULL) | \
	(((x) << 56) & 0xFF00000000000000ULL))

/// Flip the endianness of a 128-bit unsigned integer.
static inline u128 endianness_flip_u128(u128 x)
{
	union {
		u128 val;
		u8 bytes[16];
	} src, dst;

	src.val = x;

	// Reverse the byte order
	for (int i = 0; i < 16; i++) {
		dst.bytes[i] = src.bytes[15 - i];
	}

	return dst.val;
}

#define ENDIANNESS_FLIP_U128(x) endianness_flip_u128(x)

/// Read a 32-bit big endian unsigned integer from a pointer.
#define READ_BIG_ENDIAN_U16(ptr) (ENDIANNESS_FLIP_U16(*(u16 *)(ptr)))

/// Read a 32-bit big endian unsigned integer from a pointer.
#define READ_BIG_ENDIAN_U32(ptr) (ENDIANNESS_FLIP_U32(*(u32 *)(ptr)))

/// Read a 64-bit big endian unsigned integer from a pointer.
#define READ_BIG_ENDIAN_U64(ptr) (ENDIANNESS_FLIP_U64(*(u64 *)(ptr)))

/// Read a 128-bit big endian unsigned integer from a pointer.
#define READ_BIG_ENDIAN_U128(ptr) (ENDIANNESS_FLIP_U128(*(u128 *)(ptr)))
