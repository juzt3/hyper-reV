#include "page_tables.hpp"
#include "heap.hpp"

namespace
{
	::cr3 identity_cr3 = { };
}

void mem::setup_identity_map()
{
	const phys_addr pml4_base = heap::alloc_pages(1);
	const phys_addr pdpt_base = heap::alloc_pages(1);

	auto* pml4 = reinterpret_cast<pml4e_64*>(pml4_base);
	auto* pdpt = reinterpret_cast<pdpte_1gb_64*>(pdpt_base);

	pml4[0].flags = 0;
	pml4[0].page_frame_number = pdpt_base >> page_shift;
	pml4[0].present = true;
	pml4[0].write = true;

	for (std::size_t i = 0; i < pte_count; i++)
	{
		pdpt[i].flags = 0;
		pdpt[i].page_frame_number = i;
		pdpt[i].present = true;
		pdpt[i].write = true;
		pdpt[i].large_page = true;
	}

	::identity_cr3.flags = 0;
	::identity_cr3.address_of_page_directory = pml4_base >> page_shift;
}

::cr3 mem::identity_cr3()
{
	return ::identity_cr3;
}
