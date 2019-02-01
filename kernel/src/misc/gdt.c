
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE          "gdt"

#include <cpu.h>
#include <mm.h>
#include <debug.h>
#include <string.h>

static void make_gdt_entry(gdt_t *entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    memset(entry, 0, sizeof(gdt_t));

    if(flags & GDT_FLAGS_GRANULARITY)
    {
        entry->limit_low = (uint16_t)limit;
        entry->flags = (uint8_t)(limit >> 16) & 0x0F;   /* limit high */
    } else
    {
        entry->limit_low = 0xFFFF;
        entry->flags = 0x0F;
    }

    entry->base_low = (uint16_t)base;
    entry->base_middle = (uint8_t)(base >> 16) & 0xFF;
    entry->base_high = (uint8_t)(base >> 24) & 0xFF;
    entry->access = access;
    entry->flags |= flags << 4;
}

void gdt_init()
{
    gdtr_t *gdtr = kcalloc(sizeof(gdtr_t), 1);
    gdt_t *gdt = kcalloc(sizeof(gdt_t), GDT_ENTRIES);

    if(!gdtr || !gdt)
    {
        ERROR("unable to allocate memory.\n");
        while(1);
    }

    gdtr->limit = (sizeof(gdt_t) * GDT_ENTRIES) - 1;
    gdtr->base = (uint64_t)gdt;

    make_gdt_entry(&gdt[GDT_ENTRY_KCODE], 0, 0xFFFFF, GDT_KCODE_ACCESS, GDT_FLAGS_GRANULARITY | GDT_FLAGS_LONG_MODE);
    make_gdt_entry(&gdt[GDT_ENTRY_KDATA], 0, 0xFFFFF, GDT_KDATA_ACCESS, GDT_FLAGS_GRANULARITY | GDT_FLAGS_LONG_MODE);
    make_gdt_entry(&gdt[GDT_ENTRY_UCODE], 0, 0xFFFFF, GDT_UCODE_ACCESS, GDT_FLAGS_GRANULARITY | GDT_FLAGS_LONG_MODE);
    make_gdt_entry(&gdt[GDT_ENTRY_UDATA], 0, 0xFFFFF, GDT_UDATA_ACCESS, GDT_FLAGS_GRANULARITY | GDT_FLAGS_LONG_MODE);

    load_gdt(gdtr);
    reload_segments(KCODE_SEGMENT, KDATA_SEGMENT);
}
