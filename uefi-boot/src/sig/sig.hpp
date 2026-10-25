#pragma once
#include <optional.hpp>
#include <cstddef>

namespace sig
{
	struct rel
	{
		std::int16_t rva_off;
		std::int16_t rip_off;
	};

	template <std::size_t N>
	struct info
	{
		char pattern[N];
		cstd::optional<rel> rel;
	};

	// ImgpLoadPeImage
	constexpr info load_pe_img = {
		.pattern = "48 89 44 24 ? E8 ? ? ? ? 8B D8 EB ? BB ? ? ? ? 48 8B",
		.rel = rel{ .rva_off = 6, .rip_off = 10 }
	};
}
