#pragma once
#include "file.hpp"
#include "status.hpp"

namespace efi
{
	// the volume we booted from, resolved once at startup
	inline file boot_partition;

	// resolves the volume our image was loaded from and stores it in boot_partition
	status find_boot_partition() noexcept;
}
