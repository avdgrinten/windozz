/*
* io.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
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