#pragma once
#include <cstdint>
#include <structures/trap_frame.h>

namespace cpuid
{
	void spoof(trap_frame_t* frame, std::uint32_t leaf, std::uint32_t subleaf);
}
