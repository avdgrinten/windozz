
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE            "vmm"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <debug.h>
#include <cpu.h>
#include <mm.h>
#include <mutex.h>
#include <string.h>

mutex_t vmm_mutex = MUTEX_FREE;
mmio_t *mmio_regions;

void vmm_init()
{
    uintptr_t *bsp_pml4 = (uintptr_t *)((uintptr_t)read_cr3() + PHYSICAL_MEMORY);
    DEBUG("BSP PML4 starts at 0x%016lX\n", bsp_pml4);

    bsp_pml4[0] = 0;        /* unmap the lowest 512 GB */
    write_cr3(read_cr3());

    mmio_regions = kcalloc(sizeof(mmio_t), MAX_MMIO);
    if(!mmio_regions)
    {
        ERROR("unable to allocate memory.\n");
        while(1);
    }

    mmio_region_count = 0;
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

void vmm_set_wc(uintptr_t page, size_t count)
{
    uintptr_t physical;
    uintptr_t flags;
    for(size_t i = 0; i < count; i++)
    {
        physical = vmm_get_page(page + (i * PAGE_SIZE));
        flags = physical & 0xFFF;
        physical &= PAGE_MASK;

        flags &= ~PAGE_UNCACHEABLE;
        flags |= PAGE_WRITE_COMBINE;

        vmm_map_page(page + (i * PAGE_SIZE), physical, flags);
    }
}

bool vmm_get_physical(uintptr_t *destination, uintptr_t page)
{
    if(page >= PHYSICAL_MEMORY && page < MMIO_REGION)
    {
        *destination = page - PHYSICAL_MEMORY;
        return true;
    }

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

void vmm_map_page(uintptr_t virtual, uintptr_t physical, uintptr_t flags)
{
    virtual &= 0xFFFFFFFFFFFFL;

    uintptr_t *pml4 = (uintptr_t *)((uintptr_t)(read_cr3() & PAGE_MASK) + PHYSICAL_MEMORY);
    uintptr_t pdpt_ptr = pml4[(virtual >> 39) & 511];
    if(!(pdpt_ptr & PAGE_PRESENT))
    {
        /* allocate a PDPT */
        pdpt_ptr = pmm_alloc_page();
        if(!pdpt_ptr)
        {
            /* for now until i implement a panic() function */
            ERROR("out of memory.\n");
            while(1);
        }

        pml4[(virtual >> 39) & 511] = pdpt_ptr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        memset((void *)pdpt_ptr + PHYSICAL_MEMORY, 0, PAGE_SIZE);

        /*DEBUG("allocated a PDPT at 0x%016lX\n", pdpt_ptr);*/
    }

    pdpt_ptr &= PAGE_MASK;

    uintptr_t *pdpt = (uintptr_t *)((uintptr_t)pdpt_ptr + PHYSICAL_MEMORY);
    uintptr_t pd_ptr = pdpt[(virtual >> 30) & 511];
    if(!(pd_ptr & PAGE_PRESENT))
    {
        /* allocate a PD */
        pd_ptr = pmm_alloc_page();
        if(!pd_ptr)
        {
            ERROR("out of memory.\n");
            while(1);
        }

        pdpt[(virtual >> 30) & 511] = pd_ptr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        memset((void *)pd_ptr + PHYSICAL_MEMORY, 0, PAGE_SIZE);

        /*DEBUG("allocated a PD at 0x%016lX\n", pd_ptr);*/
    }

    pd_ptr &= PAGE_MASK;

    uintptr_t *pd = (uintptr_t *)((uintptr_t)pd_ptr + PHYSICAL_MEMORY);
    uintptr_t pt_ptr = pd[(virtual >> 21) & 511];
    if(!(pt_ptr & PAGE_PRESENT))
    {
        /* allocate a page table */
        pt_ptr = pmm_alloc_page();
        if(!pt_ptr)
        {
            ERROR("out of memory.\n");
            while(1);
        }

        pd[(virtual >> 21) & 511] = pt_ptr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        memset((void *)pt_ptr + PHYSICAL_MEMORY, 0, PAGE_SIZE);

        /*DEBUG("allocated a PT at 0x%016lX\n", pt_ptr);*/
    }

    pt_ptr &= PAGE_MASK;

    uintptr_t *pt = (uintptr_t *)((uintptr_t)pt_ptr + PHYSICAL_MEMORY);
    pt[(virtual >> 12) & 511] = physical | flags;

    flush_tlb(virtual);
}

void vmm_free(uintptr_t virtual, size_t count)
{
    acquire(&vmm_mutex);

    size_t i;
    uintptr_t phys;
    for(i = 0; i < count; i++)
    {
        if(vmm_get_physical(&phys, virtual + (i << PAGE_SIZE_SHIFT)))
        {
            pmm_free_page(phys);
            vmm_map_page(virtual + (i << PAGE_SIZE_SHIFT), 0, 0);
        }
    }

    release(&vmm_mutex);
}

/* vmm_find: finds contiguous pages but does NOT map them */
uintptr_t vmm_find(uintptr_t start, size_t count)
{
    size_t i;
    uintptr_t ptr = start, dummy;

start:
    i = 0;

    while((ptr + (count << PAGE_SIZE_SHIFT) < PHYSICAL_MEMORY) || ptr >= MMIO_REGION)
    {
        if(!vmm_get_physical(&dummy, ptr + (i << PAGE_SIZE_SHIFT)))
        {
            i++;
            if(i >= count)
                return ptr;
        } else
        {
            ptr += PAGE_SIZE;
            goto start;
        }
    }

    return NULL;
}

uintptr_t vmm_alloc(uintptr_t start, size_t count, uintptr_t flags)
{
    if((total_pages - used_pages) <= count)
    {
        /* out of memory */
        return NULL;
    }

    uintptr_t ptr = vmm_find(start, count);
    if(!ptr)
    {
        return NULL;    /* realistically this will never happen tbh */
    }

    /*DEBUG("attempt to allocate %d pages at linear addr 0x%016lX, flags 0x%02lX\n", count, ptr, flags);*/

    acquire(&vmm_mutex);

    /* now find physical pages */
    size_t i;
    uintptr_t phys;

    for(i = 0; i < count; i++)
    {
        phys = pmm_alloc_page();
        if(!phys)
        {
            release(&vmm_mutex);
            return NULL;
        }

        /*DEBUG("%016lX : %016lX \n", ptr + (i << PAGE_SIZE_SHIFT), phys);*/

        vmm_map_page(ptr + (i << PAGE_SIZE_SHIFT), phys, flags);
    }

    release(&vmm_mutex);
    return ptr;    
}

uintptr_t vmm_create_mmio(uintptr_t physical, size_t pages, char *name)
{
    acquire(&vmm_mutex);

    uintptr_t virtual = vmm_find(MMIO_REGION, pages);
    if(!virtual)
    {
        ERROR("unable to map MMIO.\n");
        while(1);
    }

    for(size_t i = 0; i < pages; i++)
        vmm_map_page(virtual + (i * PAGE_SIZE), physical + (i * PAGE_SIZE), PAGE_PRESENT | PAGE_WRITE | PAGE_UNCACHEABLE);

    mmio_regions[mmio_region_count].virtual = virtual;
    mmio_regions[mmio_region_count].physical = physical;
    mmio_regions[mmio_region_count].pages = pages;
    strcpy(mmio_regions[mmio_region_count].device, name);

    mmio_region_count++;

    release(&vmm_mutex);
    return virtual;
}
