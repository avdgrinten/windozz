
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE            "pmm"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <mm.h>
#include <debug.h>
#include <bootmgr.h>
#include <string.h>
#include <mutex.h>

mutex_t pmm_mutex = MUTEX_FREE;

extern uint8_t end;

uint8_t *pmm_bitmap = &end;
size_t pmm_bitmap_size = 0;
size_t total_pages = 0, used_pages = 0;

uintptr_t highest_usable_address = 0;

const char *e820_types[] = 
{
    "usable",
    "hardware-reserved",
    "ACPI tables",
    "ACPI NVS",
    "bad memory"
};

static void e820_add_entry(e820_t *);

void pmm_init()
{
    DEBUG("firmware-provided physical memory map: \n");
    DEBUG(" STARTING ADDRESS - ENDING ADDRESS   - TYPE\n");

    e820_t *e820 = (e820_t *)boot_info.e820_map;
    e820_t *e820_last = (e820_t *)((uintptr_t)boot_info.e820_map + (boot_info.e820_map_entries - 1) * 32);
    uint64_t last_address = e820_last->base + e820_last->length;

    pmm_bitmap_size = (last_address + PAGE_SIZE - 1) / PAGE_SIZE;
    pmm_bitmap_size += 7;
    pmm_bitmap_size /= 8;
    memset(pmm_bitmap, 0xFF, pmm_bitmap_size);

    uint64_t i;

    for(i = 0; i < boot_info.e820_map_entries; i++)
    {
        if(e820->type != 0 && e820->type <= 5)
        {
            DEBUG(" %016lX - %016lX - %s\n", e820->base, e820->base + e820->length - 1, e820_types[e820->type-1]);
            e820_add_entry(e820);
        } else
            DEBUG(" %016lX - %016lX - undefined type %d\n", e820->base, e820->base + e820->length - 1, e820->type);

        e820 = (e820_t *)((uintptr_t)e820 + 32);
    }

    DEBUG("highest usable address is 0x%016lX\n", highest_usable_address);
    DEBUG("%ld physical pages, of which %ld are unusable.\n", total_pages, used_pages);
    DEBUG("total memory is %ld MB, of which %ld MB are usable.\n", (total_pages * 4096) / 1024 / 1024, ((total_pages - used_pages) * 4096) / 1024 / 1024);
    DEBUG("physical memory bitmap size: %d\n", pmm_bitmap_size);

    if(((total_pages * 4096) / 1024 / 1024) < 16)
    {
        ERROR("Windozz requires at least 16 MB of memory to boot.\n");
        while(1);
    }

    /* mark lowest 8 MB as used */
    for(i = 0; i < 2048; i++)
    {
        pmm_mark_page_used(i << PAGE_SIZE_SHIFT);
    }
}

static void e820_add_entry(e820_t *e820)
{
    uint64_t base = e820->base;
    uint64_t length = e820->length;

    if(!length)
        return;

    /* ACPI 3.0 compatibility */
    if(e820->size >= 24 && !(e820->acpi3_flags & 1))
        return;

    size_t pages = (length + 4095) / 4096;
    if(e820->type != 1)
    {
        pages++;
    }

    if(e820->type == 1)
    {
        if(e820->base + e820->length > highest_usable_address)
            highest_usable_address = e820->base + e820->length - 1;
    }

    /*memset(pmm_bitmap + pmm_bitmap_size, 0xFF, (pages + 7) / 8);*/

    total_pages += pages;
    if(e820->type != 1)
    {
        used_pages += pages;
    }

    size_t i;
    for(i = 0; i < pages; i++)
    {
        if(e820->type == 1)
        {
            size_t used = used_pages;
            pmm_mark_page_free(base + (i * 4096));
            used_pages = used;
        } else
            pmm_mark_page_used(base + (i * 4096));
    }
}

void pmm_mark_page_free(uintptr_t addr)
{
    size_t byte = (addr / 4096) / 8;
    size_t bit = (addr / 4096) % 8;

    uint8_t flag = ~(1 << bit);

    if(!(pmm_bitmap[byte] & (1 << bit)))
        return;

    pmm_bitmap[byte] &= flag;

    used_pages--;
}

void pmm_mark_page_used(uintptr_t addr)
{
    size_t byte = (addr / 4096) / 8;
    size_t bit = (addr / 4096) % 8;

    uint8_t flag = (1 << bit);

    if(pmm_bitmap[byte] & (1 << bit))
        return;

    pmm_bitmap[byte] |= flag;

    used_pages++;
}

bool pmm_get_page(uintptr_t addr)
{
    if(addr >= highest_usable_address)
        return true;

    size_t byte = (addr / 4096) / 8;
    size_t bit = (addr / 4096) % 8;

    uint8_t flag = (1 << bit);

    if(pmm_bitmap[byte] & flag)
        return true;
    else
        return false;
}

uintptr_t pmm_find_page()
{
    uintptr_t page = 0;
    while(pmm_get_page(page) == true)
    {
        page += 4096;
        if(page >= highest_usable_address - 4096)
        {
            return NULL;
        }
    }

    return page;
}

uintptr_t pmm_find_in_range(uintptr_t start, uintptr_t end)
{
    uintptr_t page = start;

    while(pmm_get_page(page) == true)
    {
        page += 4096;

        if(page >= end || page >= highest_usable_address - 4096)
        {
            return NULL;
        }
    }

    return page;
}

uintptr_t pmm_alloc_page()
{
    acquire(&pmm_mutex);

    uintptr_t page = pmm_find_page();
    if(!page)
        return NULL;

    pmm_mark_page_used(page);

    release(&pmm_mutex);
    return page;
}

void pmm_free_page(uintptr_t page)
{
    acquire(&pmm_mutex);
    pmm_mark_page_free(page);
    release(&pmm_mutex);
}
