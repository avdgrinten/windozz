
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE          "pit"

#include <stdint.h>
#include <debug.h>
#include <timer.h>
#include <io.h>
#include <apic.h>

void pit_init()
{
    timer_irq_line = irq_configure(0, (uintptr_t)&timer_irq_stub, EDGE | ACTIVE_HIGH);
    DEBUG("timer IRQ line is %d\n", timer_irq_line);

    uint16_t divider = 1193182 / TIMER_FREQUENCY;

    /* channel 0, high byte and low byte in one access, square wave generator */
    outb(0x43, 0x36);
    iowait();
    outb(0x40, (uint8_t)divider & 0xFF);
    iowait();
    outb(0x40, (uint8_t)(divider >> 8) & 0xFF);
    iowait();

    irq_unmask(timer_irq_line);
}

