#pragma once
#include "../arch/cpu.hpp"

namespace mem
{
	void setup_identity_map();

	::cr3 identity_cr3();
}
