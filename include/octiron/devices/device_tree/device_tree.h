#pragma once

#include <kzadhbat/types/numeric_types.h>
#include <kzadhbat/types/error.h>

// Struct forward declarations
struct dtNode;
struct dtProperty;

/// Initializes the device tree structure by parsing the device tree blob loacated at the given physical address.
/// This procedure allocates it's own memory for it's internal structures and cleans up the dtb region memory after
/// parsing the device tree blob.
errval_t dt_initialize(paddr_t dtb_base_addr);

/// Returns true if the device tree has been initialized, false otherwise.
bool dt_is_initialized(void);

/// Looks up a node in the device tree by its path and returns a reference to it.
struct dtNode * dt_lookup_node(const char* path);