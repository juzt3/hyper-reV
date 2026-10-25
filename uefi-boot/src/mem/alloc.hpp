#pragma once
#include "../arch/cpu.hpp"
#include <cstddef>

namespace mem
{
	phys_addr alloc_pages(std::size_t count, bool runtime = false) noexcept;
	phys_addr alloc_page_aligned(std::size_t size, bool runtime = false) noexcept;
}
