
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

#define TIMER_FREQUENCY 1000

extern void timer_irq_stub();

uint64_t timer_ticks;
uint8_t timer_irq_line;

void timer_init();
void pit_init();
void hpet_init();
