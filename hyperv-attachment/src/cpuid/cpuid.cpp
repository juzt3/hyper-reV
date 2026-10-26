#include "cpuid.h"
#include "../crt/crt.h"

namespace
{
	// fake CPU: 13th Gen Intel Core i5-13420H (same 8C/12T topology as the real i5-12450H)
	constexpr char brand[48] = "13th Gen Intel(R) Core(TM) i5-13420H";
	constexpr std::uint32_t signature = 0x000B06A3;

	void set_brand(trap_frame_t* const frame, const std::uint32_t index)
	{
		std::uint32_t value = 0;

		crt::copy_memory(&value, &brand[index * 16], sizeof(value));
		frame->rax = value;

		value = 0;
		crt::copy_memory(&value, &brand[index * 16 + 4], sizeof(value));
		frame->rbx = value;

		value = 0;
		crt::copy_memory(&value, &brand[index * 16 + 8], sizeof(value));
		frame->rcx = value;

		value = 0;
		crt::copy_memory(&value, &brand[index * 16 + 12], sizeof(value));
		frame->rdx = value;
	}
}

void cpuid::spoof(trap_frame_t* const frame, const std::uint32_t leaf, const std::uint32_t subleaf)
{
	(void)subleaf;

	switch (leaf)
	{
	case 0x00000000:
		frame->rbx = 0x756E6547; // Genu
		frame->rdx = 0x49656E69; // ineI
		frame->rcx = 0x6C65746E; // ntel
		break;
	case 0x00000001:
	case 0x80000001:
		frame->rax = signature;
		break;
	case 0x80000002:
		set_brand(frame, 0);
		break;
	case 0x80000003:
		set_brand(frame, 1);
		break;
	case 0x80000004:
		set_brand(frame, 2);
		break;
	default:
		break;
	}
}
