#include "smbios.h"

#include "../crt/crt.h"
#include "../memory_manager/memory_manager.h"
#include "../slat/cr3/cr3.h"

#include <ia32-doc/ia32.hpp>
#include <intrin.h>

namespace
{
	std::uint64_t rng_state = 0;

	std::uint32_t random_u32()
	{
		rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;

		return static_cast<std::uint32_t>(rng_state >> 33);
	}

	char random_char()
	{
		return (random_u32() & 1) ? static_cast<char>('A' + random_u32() % 26) : static_cast<char>('0' + random_u32() % 10);
	}

	std::uint8_t* at(const cr3 slat_cr3, const std::uint64_t guest_physical_address)
	{
		std::uint64_t size_left_of_page = 0;

		auto* const page = static_cast<std::uint8_t*>(memory_manager::map_guest_physical(slat_cr3, guest_physical_address & ~0xFFFULL, &size_left_of_page));
		if (!page)
			return nullptr;

		return page + (guest_physical_address & 0xFFF);
	}

	std::uint8_t index_at(const cr3 slat_cr3, const std::uint64_t guest_physical_address)
	{
		auto* const value = at(slat_cr3, guest_physical_address);

		return value ? *value : 0;
	}

	bool equal(const std::uint8_t* const data, const char* const value, const std::uint32_t length)
	{
		for (std::uint32_t i = 0; i < length; i++)
		{
			if (data[i] != static_cast<std::uint8_t>(value[i]))
				return false;
		}

		return true;
	}

	void randomize_string(const cr3 slat_cr3, const std::uint64_t guest_physical_address)
	{
		if (guest_physical_address == 0)
			return;

		for (std::uint32_t i = 0; i < 64; i++)
		{
			auto* const character = at(slat_cr3, guest_physical_address + i);
			if (!character || *character == 0)
				break;

			*character = random_char();
		}
	}

	std::uint64_t get_string(const cr3 slat_cr3, const std::uint64_t strings, const std::uint8_t index)
	{
		if (index == 0)
			return 0;

		std::uint64_t string = strings;

		for (std::uint8_t i = 1; i < index; i++)
		{
			for (std::uint32_t j = 0; j < 0x1000; j++)
			{
				auto* const character = at(slat_cr3, string);
				if (!character || *character == 0)
					break;

				string++;
			}

			string++;
		}

		return string;
	}

	// UEFI systems usually have no legacy SMBIOS entry point, so the table is located
	// by its first structure (type 0, handle 0) and validated up to the end-of-table marker
	std::uint64_t find_table(const cr3 slat_cr3, std::uint32_t* const size)
	{
		for (std::uint64_t page_address = 0x1000; page_address < 0x100000000; page_address += 0x1000)
		{
			if (page_address >= 0xA0000 && page_address < 0x100000)
				continue;

			std::uint64_t size_left_of_page = 0;

			auto* const page = static_cast<std::uint8_t*>(memory_manager::map_guest_physical(slat_cr3, page_address, &size_left_of_page));
			if (!page)
				continue;

			for (std::uint32_t offset = 0; offset < 0x1000; offset += 16)
			{
				auto* const candidate = page + offset;

				if (candidate[0] != 0 || candidate[2] != 0 || candidate[3] != 0 || candidate[4] != 1)
					continue;

				if (candidate[1] < 0x1A || candidate[1] > 0x40)
					continue;

				const std::uint64_t table = page_address + offset;
				std::uint64_t current = table;
				bool valid = false;

				for (std::uint32_t guard = 0; guard < 0x10000; guard++)
				{
					auto* const type = at(slat_cr3, current);
					auto* const length = at(slat_cr3, current + 1);

					if (!type || !length)
						break;

					if (*type == 127)
					{
						valid = true;
						break;
					}

					if (*length < 4)
						break;

					std::uint64_t string_end = current + *length;

					for (std::uint32_t string_guard = 0; string_guard < 0x10000; string_guard++)
					{
						auto* const first = at(slat_cr3, string_end);
						auto* const second = at(slat_cr3, string_end + 1);

						if (!first || !second)
							break;

						if (*first == 0 && *second == 0)
							break;

						string_end++;
					}

					current = string_end + 2;
				}

				if (valid)
				{
					*size = static_cast<std::uint32_t>(current - table) + 6;

					return table;
				}
			}
		}

		return 0;
	}
}

void smbios::spoof()
{
	rng_state = __rdtsc();

	const cr3 slat_cr3 = slat::hyperv_cr3();

	std::uint64_t table = 0;
	std::uint32_t size = 0;

	for (std::uint64_t page_address = 0xF0000; page_address < 0x100000; page_address += 0x1000)
	{
		std::uint64_t size_left_of_page = 0;

		auto* const page = static_cast<std::uint8_t*>(memory_manager::map_guest_physical(slat_cr3, page_address, &size_left_of_page));
		if (!page)
			continue;

		for (std::uint32_t offset = 0; offset + 0x20 <= 0x1000; offset += 16)
		{
			auto* const entry = page + offset;

			if (equal(entry, "_SM3_", 5))
			{
				table = *reinterpret_cast<std::uint64_t*>(entry + 0x10);
				size = *reinterpret_cast<std::uint32_t*>(entry + 0x0C);
				break;
			}

			if (equal(entry, "_SM_", 4))
			{
				table = *reinterpret_cast<std::uint32_t*>(entry + 0x18);
				size = *reinterpret_cast<std::uint8_t*>(entry + 0x16);
				break;
			}
		}

		if (table != 0)
			break;
	}

	if (table == 0)
		table = find_table(slat_cr3, &size);

	if (table == 0 || size == 0 || size > 0x10000)
		return;

	const std::uint64_t end = table + size;
	std::uint64_t current = table;

	while (current + 4 <= end)
	{
		auto* const type = at(slat_cr3, current);
		auto* const length = at(slat_cr3, current + 1);

		if (!type || !length)
			return;

		if (*type == 127)
			break;

		if (*length < 4 || current + *length > end)
			break;

		const std::uint64_t strings = current + *length;

		switch (*type)
		{
		case 0:
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 4)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 5)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 8)));
			break;
		case 1:
		case 2:
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 4)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 5)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 7)));
			break;
		case 3:
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 4)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 7)));

			if (*length > 5)
			{
				static const std::uint8_t chassis_types[] = { 3, 4, 5, 6, 7, 10, 23 };

				if (auto* const chassis = at(slat_cr3, current + 5))
					*chassis = chassis_types[random_u32() % sizeof(chassis_types)];
			}
			break;
		case 4:
			if (*length >= 16)
			{
				auto* const processor_id = at(slat_cr3, current + 8);
				if (processor_id)
				{
					*reinterpret_cast<std::uint32_t*>(processor_id) = 0x000B06A3;
					*reinterpret_cast<std::uint32_t*>(processor_id + 4) = 0;
				}
			}
			break;
		case 17:
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 0x17)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 0x18)));
			randomize_string(slat_cr3, get_string(slat_cr3, strings, index_at(slat_cr3, current + 0x1A)));
			break;
		default:
			break;
		}

		std::uint64_t string_end = strings;

		while (string_end + 1 < end)
		{
			auto* const first = at(slat_cr3, string_end);
			auto* const second = at(slat_cr3, string_end + 1);

			if (!first || !second)
				return;

			if (*first == 0 && *second == 0)
				break;

			string_end++;
		}

		current = string_end + 2;
	}
}
