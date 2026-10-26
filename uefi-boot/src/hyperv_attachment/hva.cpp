#include "hva.hpp"
#include "../arch/cpu.hpp"
#include "../efi/boot.hpp"
#include "../mem/heap.hpp"

#include <pe/image.hpp>

#define HVA_PATH L"\\efi\\microsoft\\boot\\hyperv-attachment.dll"

namespace
{
	cstd::expected<cstd::vector<std::uint8_t>, efi::status> read_and_delete() noexcept
	{
		auto file = efi::boot_partition.open(HVA_PATH);

		if (!file)
			return cstd::unexpected(file.error());

		const auto file_buf = file->read();

		file->delete_();

		return file_buf;
	}

	void alloc_and_map(const pe::image* const file_img)
	{
		const phys_addr base_addr = mem::heap::alloc_page_aligned(file_img->size());
		const auto virt_img = reinterpret_cast<pe::image*>(base_addr);
		
		const std::size_t hdrs_size = file_img->nt_hdrs()->optional_hdr.size_of_headers;

		cstd::crt::memcpy(virt_img->as(), file_img->as(), hdrs_size);

		for (const auto& sec : file_img->sections())
		{
			const auto dest = virt_img->as() + sec.virtual_address;
			const auto src = file_img->as<const std::uint8_t*>() + sec.pointer_to_raw_data;

			cstd::crt::memcpy(dest, src, sec.size_of_raw_data);
		}

		const std::uintptr_t delta = virt_img->as<std::uintptr_t>() - file_img->base_addr();

		for (const auto reloc : file_img->relocs())
		{
			if (reloc.type != pe::reloc_type::dir64)
				continue;

			*reinterpret_cast<std::uintptr_t*>(virt_img->as() + reloc.loc.rva()) += delta;
		}
	}
}

efi::status hva::load_from_disk()
{
	const auto file_buf = read_and_delete();

	if (!file_buf)
		return file_buf.error();

	const auto file_img = reinterpret_cast<const pe::image*>(file_buf->data());

	alloc_and_map(file_img);

	return efi::success;
}
