#pragma once
#include "../efi/status.hpp"
#include "../arch/cpu.hpp"
#include <pe/image.hpp>

namespace hvloader
{
	inline constexpr std::size_t hyperv_pml4e_idx = 255;

	constexpr std::uintptr_t hyperv_mapping_base()
	{
		return hyperv_pml4e_idx << pml4e_shift;
	}

	efi::status place_hooks(pe::image* img);
}
