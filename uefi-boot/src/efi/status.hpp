#pragma once
extern "C" {
#include <Uefi.h>
}

namespace efi
{
	class status
	{
	public:
		using value_type = EFI_STATUS;

		status() noexcept = default;
		status(const value_type value) noexcept
			:	value_(value) { }

		[[nodiscard]] operator bool() const noexcept
		{
			return EFI_SUCCESS(value_);
		}

		[[nodiscard]] value_type value() const noexcept
		{
			return value_;
		}

	protected:
		value_type value_;
	};

}
