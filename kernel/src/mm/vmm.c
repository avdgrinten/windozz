
/*
 * Windozz
 * Copyright (C) 2018 by Omar Muhamed
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Virtual Memory Manager */

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


