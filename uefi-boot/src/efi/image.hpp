#pragma once
extern "C" {
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <IndustryStandard/PeImage.h>
#include <Protocol/LoadedImage.h>
}

#include "obj.hpp"
#include "status.hpp"
#include <expected.hpp>
#include <span.hpp>
#include <cstdint>
#include <cstddef>

namespace efi
{
	class image : public handle_obj
	{
	public:
		using handle_obj::handle_obj;

		struct info
		{
			std::uint32_t revision;
			handle parent_handle;
			EFI_SYSTEM_TABLE* system_table;
			handle device_handle;
			EFI_DEVICE_PATH_PROTOCOL* file_path;
			void* reserved;
			std::uint32_t load_options_size;
			void* load_options;
			void* image_base;
			std::uint64_t image_size;
			EFI_MEMORY_TYPE image_code_type;
			EFI_MEMORY_TYPE image_data_type;
			EFI_IMAGE_UNLOAD unload;
		};

		[[nodiscard]] cstd::expected<info*, status> info() const noexcept
		{
			EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

			struct info* inf = nullptr;

			const status stat = gBS->HandleProtocol(native_, &guid, reinterpret_cast<void**>(&inf));

			if (!stat)
				return cstd::unexpected(stat);

			return inf;
		}

		[[nodiscard]] struct info* operator->() const noexcept
		{
			const auto inf = info();

			// todo, handle inf being unexpected

			return *inf;
		}

		[[nodiscard]] void* base_addr() const noexcept
		{
			return (*this)->image_base;
		}

		[[nodiscard]] std::size_t size() const noexcept
		{
			return (*this)->image_size;
		}

		static image curr() noexcept
		{
			return image{ gImageHandle };
		}

		static cstd::expected<image, status> load(const cstd::span<const std::uint8_t> buf) noexcept
		{
			EFI_HANDLE img_handle = nullptr;

			const status stat = gBS->LoadImage(FALSE, gImageHandle, nullptr, const_cast<std::uint8_t*>(buf.data()),
			                                   buf.size(), &img_handle);

			if (!stat)
				return cstd::unexpected(stat);

			return image{ img_handle };
		}

		status start() noexcept
		{
			return gBS->StartImage(native_, nullptr, nullptr);
		}
	};
}
