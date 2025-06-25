#include <octiron/devices/syscon/reboot.h>
#include <octiron/devices/device_tree/device_tree.h>

struct {
	/// The base address of the syscon device
	paddr_t base_address;
	/// The size of the syscon device in bytes
	size_t size;
	/// The reboot register offset within the syscon device
	size_t reboot_register_offset;

} syscon_reboot_device;


errval_t syscon_reboot_initialize()
{
	if (!dt_is_initialized()) {
		return ERR_NOT_IMPLEMENTED;
	}

	return ERR_NOT_IMPLEMENTED;
}