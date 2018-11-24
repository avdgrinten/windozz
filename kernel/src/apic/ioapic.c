/*
* ioapic.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE "ioapic"

#include <debug.h>
#include <apic.h>
#include <mm.h>

uint32_t ioapic_read(size_t ioapic, size_t index)
{
    volatile uint32_t *idx, *dat;
    idx = (volatile uint32_t *)ioapics[ioapic].mmio;
    dat = (volatile uint32_t *)((uintptr_t)ioapics[ioapic].mmio + 16);

    *idx = index;
    asm volatile ("pause");
    return *dat;
}