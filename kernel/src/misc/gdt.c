
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#include <cpu.h>
#include <mm.h>
#include <apic.h>   /* MAX_CPUS */

/* 1 NULL, 2 kernel code/data, 2 user code/data, 2 for each CPU */
#define GDT_ENTRIES     (5 + (2 * MAX_CPUS))

gdtr_t *gdtr;
gdt_t *gdt;

void gdt_init()
{
    gdtr = kcalloc(sizeof(gdtr_t), 1);
    gdt = kcalloc(sizeof(gdt_t), GDT_ENTRIES);
}
