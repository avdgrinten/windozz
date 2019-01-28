
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

extern void outb(uint16_t, uint8_t);
extern void outw(uint16_t, uint16_t);
extern void outd(uint16_t, uint32_t);

extern uint8_t inb(uint16_t);
extern uint16_t inw(uint16_t);
extern uint32_t ind(uint16_t);

extern void iowait();