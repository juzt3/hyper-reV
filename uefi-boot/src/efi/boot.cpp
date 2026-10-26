#include "boot.hpp"
#include "image.hpp"

extern "C" {
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
}

efi::status efi::find_boot_partition() noexcept
{
	const handle dev = image::curr()->device_handle;

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* native = nullptr;

	const status stat = gBS->HandleProtocol(dev, &gEfiSimpleFileSystemProtocolGuid,
	                                        reinterpret_cast<void**>(&native));

	if (!stat)
		return stat;

	file_system fs{ native };

	auto vol = fs.volume();

	if (!vol)
		return vol.error();

	boot_partition = cstd::move(*vol);
	boot_device = dev;

	return success;
}
