
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

/* Physical memory manager */

#define MODULE			"pmm"

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

uintptr_t highest_usable_address;

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

	if(((total_pages * 4096) / 1024 / 1024) < 16)
	{
		ERROR("Windozz requires at least 16 MB of memory to boot.\n");
		while(1);
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

	memset(pmm_bitmap + pmm_bitmap_size, 0xFF, (pages + 7) / 8);

	total_pages += pages;
	pmm_bitmap_size += ((pages + 7) / 8);

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
	pmm_bitmap[byte] &= flag;

	used_pages--;
}

void pmm_mark_page_used(uintptr_t addr)
{
	size_t byte = (addr / 4096) / 8;
	size_t bit = (addr / 4096) % 8;

	uint8_t flag = (1 << bit);
	pmm_bitmap[byte] |= flag;

	used_pages++;
}

bool pmm_get_page(uintptr_t addr)
{
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
	acquire(&pmm_mutex);

	uintptr_t page = 0;
	while(pmm_get_page(page) == true)
	{
		page += 4096;
		if(page >= highest_usable_address - 4096)
		{
			release(&pmm_mutex);
			return NULL;
		}
	}

	release(&pmm_mutex);
	return page;
}

uintptr_t pmm_find_in_range(uintptr_t start, uintptr_t end)
{
	acquire(&pmm_mutex);

	uintptr_t page = start;

	while(pmm_get_page(page) == true)
	{
		page += 4096;

		if(page >= end || page >= highest_usable_address - 4096)
		{
			release(&pmm_mutex);
			return NULL;
		}
	}

	release(&pmm_mutex);
	return page;
}





