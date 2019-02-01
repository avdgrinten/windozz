
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "ioapic"

#include <debug.h>
#include <apic.h>
#include <mm.h>
#include <cpu.h>

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

static size_t get_ioapic(uint8_t gsi)
{
    size_t i;
    for(i = 0; i < ioapic_count; i++)
    {
        if(gsi >= ioapics[i].gsi_start && gsi <= ioapics[i].gsi_end)
            goto found;
    }

    ERROR("global interrupt line %d not available.\n");
    return 0xFF;

found:
    return i;
}

uint8_t irq_configure(uint8_t line, uintptr_t handler, uint8_t flags)
{
    for(int i = 0; i < override_count; i++)
    {
        if(overrides[i].source == line)
        {
            line = overrides[i].destination;
            flags = overrides[i].flags;
            break;
        }
    }

    size_t ioapic = get_ioapic(line);
    if(ioapic > ioapic_count)
        return 0xFF;

    DEBUG("configure GSI %d on IOAPIC %d with flags 0x%04X (%s %s)\n", line,
        ioapic, flags,
        flags & LEVEL ? "level" : "edge",
        flags & ACTIVE_LOW ? "low" : "high");

    idt_install(line + IRQ_BASE, handler);

    uint64_t config;
    config = ioapic_read(ioapic, IOAPIC_TABLES + (line * 2));
    config |= (uint64_t)((uint64_t)ioapic_read(ioapic, IOAPIC_TABLES + (line * 2) + 1) << 32);

    if(flags & ACTIVE_LOW)
        config |= IOAPIC_ACTIVE_LOW;
    else
        config &= ~IOAPIC_ACTIVE_LOW;

    if(flags & LEVEL)
        config |= IOAPIC_LEVEL;
    else
        config &= ~IOAPIC_LEVEL;

    ioapic_write(ioapic, IOAPIC_TABLES + (line * 2), (uint32_t)config);
    ioapic_write(ioapic, IOAPIC_TABLES + (line * 2) + 1, config >> 32);

    return line;
}

void irq_mask(uint8_t line)
{
    size_t ioapic = get_ioapic(line);
    if(ioapic > ioapic_count)
    {
        ERROR("attempt to mask non-existant IRQ %d\n", line);
        return;
    }

    uint32_t config;
    config = ioapic_read(ioapic, IOAPIC_TABLES + (line * 2));
    config |= IOAPIC_MASK;
    ioapic_write(ioapic, IOAPIC_TABLES + (line * 2), config);
}

void irq_unmask(uint8_t line)
{
    size_t ioapic = get_ioapic(line);
    if(ioapic > ioapic_count)
    {
        ERROR("attempt to unmask non-existant IRQ %d\n", line);
        return;
    }

    uint32_t config;
    config = ioapic_read(ioapic, IOAPIC_TABLES + (line * 2));
    config &= ~IOAPIC_MASK;
    ioapic_write(ioapic, IOAPIC_TABLES + (line * 2), config);
}


