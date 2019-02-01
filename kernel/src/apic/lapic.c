
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "lapic"

#include <apic.h>
#include <cpu.h>
#include <debug.h>

extern void lapic_spurious_stub();

size_t lapic_spurious_count = 0;

uint32_t lapic_read(size_t index)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uintptr_t)lapic + index);
    return (uint32_t)*reg;
}

void lapic_write(size_t index, uint32_t value)
{
    volatile uint32_t *reg = (volatile uint32_t *)((uintptr_t)lapic + index);
    *reg = value;
}

void lapic_configure()
{
    /*DEBUG("configure lapic\n");*/
    idt_install(0xFF, (uintptr_t)&lapic_spurious_stub);
    lapic_write(LAPIC_SPURIOUS, LAPIC_SPURIOUS_ENABLE | 0xFF);  /* enable spurious IRQ */
    lapic_write(LAPIC_TPR, 0);      /* enable all external interrupts */
    lapic_write(LAPIC_EOI, 0);      /* in case there was anything pending */
}

void lapic_spurious()
{
    lapic_spurious_count++;
    WARN("spurious IRQ, total spurious IRQ count is %d\n", lapic_spurious_count);
}

void lapic_eoi()
{
    lapic_write(LAPIC_EOI, 0);
}
