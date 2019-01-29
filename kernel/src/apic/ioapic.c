
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "ioapic"

#include <debug.h>
#include <apic.h>
#include <mm.h>

uint32_t ioapic_read(size_t ioapic, uint32_t index)
{
    volatile uint32_t *idx, *dat;
    idx = (volatile uint32_t *)ioapics[ioapic].mmio;
    dat = (volatile uint32_t *)((uintptr_t)ioapics[ioapic].mmio + 16);

    *idx = index;
    asm volatile ("pause");
    return *dat;
}

void ioapic_write(size_t ioapic, uint32_t index, uint32_t value)
{
    volatile uint32_t *idx, *dat;
    idx = (volatile uint32_t *)ioapics[ioapic].mmio;
    dat = (volatile uint32_t *)((uintptr_t)ioapics[ioapic].mmio + 16);

    *idx = index;
    asm volatile ("pause");
    *dat = value;
    asm volatile ("pause");
}

void ioapic_init()
{
    size_t i, j;
    size_t max_irq;
    uint64_t config;

    for(i = 0; i < ioapic_count; i++)
    {
        max_irq = (ioapic_read(i, IOAPIC_VERSION) >> 16) & 0xFF;
        max_irq++;

        for(j = 0; j < max_irq; j++)
        {
            config = ioapics[i].gsi_start + IRQ_BASE + j;
            config |= IOAPIC_MASK;
            ioapic_write(i, (j * 2) + IOAPIC_TABLES, (uint32_t)config);
            ioapic_write(i, (j * 2) + IOAPIC_TABLES + 1, config >> 32);

            /*DEBUG("configure IRQ line %d as 0x%08lX\n", j, config);*/
        }
    }
}