/*
* vmm.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE			"vmm"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <debug.h>
#include <cpu.h>
#include <mm.h>

void vmm_init()
{
	/* honestly nothing to do here except this one thing */
	uintptr_t *bsp_pml4 = (uintptr_t *)((uintptr_t)read_cr3() + PHYSICAL_MEMORY);
	DEBUG("BSP PML4 starts at 0x%016lX\n", bsp_pml4);

	bsp_pml4[0] = 0;		/* unmap the lowest 512 GB */
	write_cr3(read_cr3());
}

uintptr_t vmm_get_page(uintptr_t virtual)
{
	virtual &= 0xFFFFFFFFFFFFL;

	uintptr_t *pml4 = (uintptr_t *)((uintptr_t)(read_cr3() & PAGE_MASK) + PHYSICAL_MEMORY);
	uintptr_t pdpt_ptr = pml4[(virtual >> 39) & 511];
	if(!(pdpt_ptr & PAGE_PRESENT))
		return 0;

	if(pdpt_ptr & PAGE_LARGE)
		return pdpt_ptr;

	pdpt_ptr &= PAGE_MASK;

	uintptr_t *pdpt = (uintptr_t *)((uintptr_t)pdpt_ptr + PHYSICAL_MEMORY);
	uintptr_t pd_ptr = pdpt[(virtual >> 30) & 511];
	if(!(pd_ptr & PAGE_PRESENT))
		return 0;

	if(pd_ptr & PAGE_LARGE)
		return pd_ptr;

	pd_ptr &= PAGE_MASK;

	uintptr_t *pd = (uintptr_t *)((uintptr_t)pd_ptr + PHYSICAL_MEMORY);
	uintptr_t pt_ptr = pd[(virtual >> 21) & 511];
	if(!(pt_ptr & PAGE_PRESENT))
		return 0;

	if(pt_ptr & PAGE_LARGE)
		return pt_ptr;

	pt_ptr &= PAGE_MASK;

	uintptr_t *pt = (uintptr_t *)((uintptr_t)pt_ptr + PHYSICAL_MEMORY);
	uintptr_t page = pt[(virtual >> 12) & 511];

	return page;
}

bool vmm_get_physical(uintptr_t *destination, uintptr_t page)
{
	uintptr_t phys = vmm_get_page(page);
	if(phys & PAGE_PRESENT)
	{
		*destination = phys & PAGE_MASK;
		*destination += (page & 0xFFF);
		return true;
	} else
	{
		return false;
	}
}