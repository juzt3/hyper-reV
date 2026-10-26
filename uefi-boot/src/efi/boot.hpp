#pragma once
#include "file.hpp"
#include "obj.hpp"
#include "status.hpp"

namespace efi
{
	// the volume we booted from, resolved once at startup
	inline file boot_partition;

	// the device handle that volume lives on — needed to build device paths
	// (FileDevicePath) when loading sibling images with LoadImage
	inline handle boot_device = nullptr;

	// resolves our boot volume + device handle via LoadedImage->DeviceHandle
	status find_boot_partition() noexcept;
}
