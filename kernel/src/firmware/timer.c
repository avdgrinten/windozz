
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE          "timer"

#include <stdint.h>
#include <debug.h>
#include <timer.h>
#include <sys.h>

uint64_t timer_ticks = 0;
uint8_t timer_irq_line;

void timer_init()
{
    DEBUG("using PIT as primary timer source...\n");
    pit_init();
}

void timer_irq(thread_t *context)
{
    timer_ticks++;
    if(sys_ready && (timer_ticks % 10))
        resched(context);
}

void timer_sleep(uint64_t ms)
{
    uint64_t time = timer_ticks;
    while(timer_ticks < time + ms)
        asm volatile ("sti; hlt");
}

