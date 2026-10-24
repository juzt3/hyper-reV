#pragma once
extern "C" {
#include <Uefi.h>
#include <Library/UefiLib.h>
}

#include "obj.hpp"
#include "status.hpp"
#include <expected.hpp>
#include <string_view.hpp>
#include <span.hpp>
#include <cstdint>
#include <cstddef>

namespace efi
{
	class file : public obj_wrap<EFI_FILE_PROTOCOL>
	{
	public:
		using obj_wrap::obj_wrap;

		using mode_base_t = std::uint64_t;
		using attr_base_t = std::uint64_t;

		enum mode : mode_base_t
		{
			mode_read = EFI_FILE_MODE_READ,	
			mode_write = EFI_FILE_MODE_WRITE,	
			mode_create = EFI_FILE_MODE_CREATE,
			mode_all = mode_read | mode_write | mode_create
		};

		enum attr : attr_base_t
		{
			attr_read_only = EFI_FILE_READ_ONLY,
			attr_hidden = EFI_FILE_HIDDEN,
			attr_file_system = EFI_FILE_SYSTEM,
			attr_directory = EFI_FILE_DIRECTORY,
			attr_archive = EFI_FILE_ARCHIVE
		};

		[[nodiscard]] cstd::expected<file, status> open(const cstd::wzstring_view name,
		                                                const mode_base_t mode = mode_all,
		                                                const attr_base_t attr = attr_file_system) const noexcept
		{
			EFI_FILE_PROTOCOL* root;

			const auto raw_name = const_cast<CHAR16*>(reinterpret_cast<const CHAR16*>(name.c_str()));
			const status status = native_->Open(native_, &root, raw_name, mode, attr);

			if (!status)
				return cstd::unexpected(status);

			return file{ root };
		}

		status read(void* const buf, const std::size_t size) const noexcept
		{
			UINTN s = static_cast<UINTN>(size);

			return status{ native_->Read(native_, &s, buf) };
		}

		status read(const cstd::span<std::uint8_t> buf) const noexcept
		{
			return read(buf.data(), buf.size());
		}

		status write(const void* const buf, const std::size_t size) noexcept
		{
			UINTN s = static_cast<UINTN>(size);

			return status{ native_->Write(native_, &s, const_cast<void*>(buf)) };
		}

		status write(const cstd::span<const std::uint8_t> buf) noexcept
		{
			return write(buf.data(), buf.size());
		}
	};

	class file_system : public obj_wrap<EFI_SIMPLE_FILE_SYSTEM_PROTOCOL>
	{
	public:
		using obj_wrap::obj_wrap;

		[[nodiscard]] cstd::expected<file, status> volume() const noexcept
		{
			EFI_FILE_PROTOCOL* root;

			const status status = native_->OpenVolume(native_, &root);

			if (!status)
				return cstd::unexpected(status);

			return file{ root };
		}
	};
}
