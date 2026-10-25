#include "hvloader.hpp"
#include "../hook/hook.hpp"
#include "../sig/sig.hpp"
#include "../arch/cpu.hpp"

using launch_hv_fn = void(*)(::cr3, virt_addr, void*, ::cr3);

namespace
{
	hook launch_hv_hk;

	void launch_hv_detour(const ::cr3 hyperv_cr3, const virt_addr hyperv_entry_point, void* const jmp_gadget, const ::cr3 kernel_cr3) noexcept
	{
		launch_hv_hk.disable();

		// set up hyperv hooks here

		launch_hv_hk.target<launch_hv_fn>()(hyperv_cr3, hyperv_entry_point, jmp_gadget, kernel_cr3);
	}
}

efi::status hvloader::place_hooks(pe::image* const img)
{
	const auto target = sig::hvloader_launch_hv.find(img);

	if (!target)
		return efi::not_found;

	launch_hv_hk = hook::create(target, reinterpret_cast<void*>(&launch_hv_detour));

	return efi::success;
}
