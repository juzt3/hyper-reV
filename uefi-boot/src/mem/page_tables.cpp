#include "page_tables.hpp"
#include "heap.hpp"

namespace
{
	::cr3 identity_cr3 = { };
	pml4e_64 identity_pml4e = { };
}

void mem::pt::init()
{
	const phys_addr pml4_base = heap::alloc_pages(1);
	const phys_addr pdpt_base = heap::alloc_pages(1);

	auto* pml4 = reinterpret_cast<pml4e_64*>(pml4_base);
	auto* pdpt = reinterpret_cast<pdpte_1gb_64*>(pdpt_base);

	pml4e_64& pml4e = pml4[0];

	pml4e.flags = 0;
	pml4e.page_frame_number = pdpt_base >> page_shift;
	pml4e.present = true;
	pml4e.write = true;

	for (std::size_t i = 0; i < pte_count; i++)
	{
		pdpte_1gb_64& pdpte = pdpt[i];

		pdpte.flags = 0;
		pdpte.page_frame_number = i;
		pdpte.present = true;
		pdpte.write = true;
		pdpte.large_page = true;
	}

	::identity_cr3.flags = 0;
	::identity_cr3.address_of_page_directory = pml4_base >> page_shift;

	::identity_pml4e = pml4e;
}

pml4e_64 mem::pt::identity_pml4e()
{
	return ::identity_pml4e;
}

pml4e_64 mem::pt::insert_pml4e(const cr3 cr3, const pml4e_64 pml4e, const std::size_t idx)
{
	const auto pml4 = map_phys<pml4e_64*>(cr3.address_of_page_directory << page_shift);

	const pml4e_64 orig_pml4e = pml4[idx];

	pml4[idx] = pml4e;

	return orig_pml4e;
}

pml4e_64 mem::pt::insert_identity_pml4e(const cr3 cr3, const std::size_t idx)
{
	return insert_pml4e(cr3, ::identity_pml4e, idx);
}

::cr3 mem::pt::identity_cr3()
{
	return ::identity_cr3;
}

::cr3 mem::pt::load_identity_cr3()
{
	::cr3 old_cr3 = arch::cr3();

	arch::set_cr3(::identity_cr3);

	return old_cr3;
}
