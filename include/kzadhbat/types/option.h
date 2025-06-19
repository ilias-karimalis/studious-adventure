/// Defines a macro programmed Option<T> type and helper macros to interact with the type.
#pragma once

#include <kzadhbat/freestanding.h>

/// The Option<T> type
#define OPT(t) OPT_STR(t)
#define OPT_STR(t) Option_##t

/// Declares and defines an Option<T> type.
#define DEFINE_OPTION_TYPE(t) \
	typedef struct {      \
		bool some;    \
		t val;        \
	} OPT(t)

/// Creates an Option<T>::Some(value) instance.
#define OPT_SOME(t, value) ((OPT(t)){ .some = true, .val = value })
/// Creates an Option<T>::None instance.
#define OPT_NONE(t) ((OPT(t)){ .some = false })
/// Returns true if the Option<T> is Some(_).
#define OPT_IS_SOME(opt) ((opt).some)
/// Returns true if the Option<T> is None.
#define OPT_IS_NONE(opt) (!(opt).some)
/// Returns true if the Option<T> is Some(value).
#define OPT_EQ(opt, value) ((opt).some && (opt).val == (value))



#ifndef STRUCT
/// Used as a work around for macro defined types that contain a struct type
/// To define an OPT(struct type) type, use OPT(STRUCT(type)).
#define STRUCT(type) struct_##type
#endif
#ifndef UNION
/// Used as a work around for macro defined types that contain a union type
//// To define an OPT(union type) type, use OPT(UNION(type)).
#define UNION(type) union_##type
#endif
