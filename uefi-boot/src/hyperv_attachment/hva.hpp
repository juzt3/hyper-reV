#pragma once
#include "../efi/status.hpp"

// hva = [H]yper[V][A]ttachment
namespace hva
{
	efi::status load_from_disk();

	void call_entry_point();
}
