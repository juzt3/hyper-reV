#pragma once
#include "../efi/status.hpp"
#include <pe/image.hpp>

namespace hvloader
{
	efi::status place_hooks(pe::image* img);
}
