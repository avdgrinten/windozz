
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
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