
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "ioapic"

#include <debug.h>
#include <apic.h>
#include <mm.h>

uint32_t ioapic_read(size_t ioapic, size_t index)
{
    volatile uint32_t *idx, *dat;
    idx = (volatile uint32_t *)ioapics[ioapic].mmio;
    dat = (volatile uint32_t *)((uintptr_t)ioapics[ioapic].mmio + 16);

    *idx = index;
    asm volatile ("pause");
    return *dat;
}