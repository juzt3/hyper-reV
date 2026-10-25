#include "alloc.hpp"
#include "../efi/status.hpp"

extern "C" {
#include <Library/UefiBootServicesTableLib.h>
}

phys_addr mem::alloc_pages(const std::size_t count, const bool runtime) noexcept
{
	const EFI_MEMORY_TYPE type = runtime
		? EfiRuntimeServicesData
		: EfiBootServicesData;

	EFI_PHYSICAL_ADDRESS addr = 0;

	const efi::status stat = gBS->AllocatePages(AllocateAnyPages, type, count, &addr);

	if (!stat)
		return 0;

	return static_cast<phys_addr>(addr);
}

phys_addr mem::alloc_page_aligned(const std::size_t size, const bool runtime) noexcept
{
	const std::size_t page_count = (size + page_size - 1) / page_size;

	return alloc_pages(page_count, runtime);
}
