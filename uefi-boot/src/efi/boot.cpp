#include "boot.hpp"
#include "image.hpp"

extern "C" {
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>
}

efi::status efi::find_boot_partition() noexcept
{
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* native = nullptr;

	const status stat = gBS->HandleProtocol(image::curr()->device_handle, &gEfiSimpleFileSystemProtocolGuid,
	                                        reinterpret_cast<void**>(&native));

	if (!stat)
		return stat;

	file_system fs{ native };

	auto vol = fs.volume();

	if (!vol)
		return vol.error();

	boot_partition = cstd::move(*vol);

	return success;
}
