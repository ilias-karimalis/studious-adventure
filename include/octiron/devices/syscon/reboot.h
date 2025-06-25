#pragma once

#include <kzadhbat/types/error.h>

/// Initializes the syscon reboot device.
errval_t syscon_reboot_initialize();
/// Reboots the system using the syscon reboot device.
_Noreturn void syscon_reboot();