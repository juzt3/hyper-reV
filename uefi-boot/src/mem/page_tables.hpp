#pragma once
#include "../arch/cpu.hpp"

namespace mem::pt
{
	void init();

	::cr3 identity_cr3();
}
