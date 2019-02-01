
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE          "timer"

#include <stdint.h>
#include <debug.h>
#include <timer.h>

uint64_t timer_ticks = 0;
uint8_t timer_irq_line;

void timer_init()
{
    DEBUG("using PIT as primary timer source...\n");
    pit_init();
}

void timer_irq()
{
    timer_ticks++;
}
