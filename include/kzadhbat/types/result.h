/// Defines a macro programmed Result<T, E> type and helper macros to interact with the type.
#pragma once

#include <kzadhbat/types/error.h>

/// The Result<T, E> type
#define RESULT(t) RESULT_STR(t)
#define RESULT_STR(t) Result_##t

/// Declares and defines a Result<T, E> type.
#define DEFINE_RESULT_TYPE(t)              \
	typedef struct {              \
		bool some;            \
		union {               \
			t val;        \
			errval_t err; \
		} u;                  \
	} RESULT(t)

/// Declares and defines a Result<struct T, E> type.
#define DEFINE_RESULT_STRUCT(t)       \
	typedef struct {              \
		bool some;            \
		union {               \
			struct t val; \
			errval_t err; \
		} u;                  \
	} RESULT(STRUCT(t))

/// Declares and defines a Result<T*, E> type.
#define DEFINE_RESULT_PTR(t)          \
	typedef struct {              \
		bool some;            \
		union {               \
			t *val;       \
			errval_t err; \
		} u;                  \
	} RESULT(t)

/// Declares and defines a Result<struct T*, E> type.
#define DEFINE_RESULT_STRUCT_PTR(t)    \
	typedef struct {               \
		bool some;             \
		union {                \
			struct t *val; \
			errval_t err;  \
		} u;                   \
	} RESULT(PTR(STRUCT(t)))

/// Creates a Result<T, E>::Ok(value) instance.
#define RESULT_MAKE_OK(t, value) ((RESULT(t)){ .some = true, .u.val = value })
/// Creates a Result<T, E>::Err(err) instance.
#define RESULT_MAKE_ERR(t, error) ((RESULT(t)){ .some = false, .u.err = error })
/// Returns true of the the Result<T, E> is Ok(_).
#define RESULT_IS_OK(result) ((result).some)
/// Returns true if the Result<T, E> is Err(_).
#define RESULT_IS_ERR(result) (!(result).some)
/// Gets the value of a Result<T, E> if it is Ok(_), undefined behavior if it is Err(_).
#define RESULT_OK(result) ((result).u.val)
/// Gets the error of a Result<T, E> if it is Err(_), undefined behavior if it is Ok(_).
#define RESULT_ERR(result) ((result).u.err)

#ifndef STRUCT
/// Used as a work around for macro defined types that contain a struct type
/// To define an RESULT(struct type) type, use RESULT(STRUCT(type)).
#define STRUCT(type) STRUCT_HELPER(type)
#define STRUCT_HELPER(type) struct_##type
#endif
#ifndef UNION
/// Used as a work around for macro defined types that contain a union type
//// To define an RESULT(union type) type, use RESULT(UNION(type)).
#define UNION(type) UNION_HELPER(type)
#define UNION_HELPER(type) union_##type
#endif
#ifndef PTR
#define PTR(type) PTR_HELPER(type)
#define PTR_HELPER(type) type##_ptr
#endif
