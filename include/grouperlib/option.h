/// Defines a macro programmed Option<T> type and helper macros to interact with the type.
#pragma once

#include <stdbool.h>

/// Defines 
#define OPT(t) Option_##t

#define DEFINE_OPTION_TYPE(t)               \
    typedef struct {                        \
        bool some;                          \
        t    val;                           \
    }  OPT(t)

#define OPT_SOME(t, value) ((OPT(t)) { .some = true, .val = value })
#define OPT_NONE(t) ((OPT(t)) { .some = false })

#define OPT_EQ(opt, value) ((opt).some && (opt).val == (value))