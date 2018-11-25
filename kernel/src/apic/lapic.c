/*
* lapic.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE "lapic"

#include <apic.h>

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