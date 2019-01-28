/*
* pci.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#pragma once

#include <stdint.h>

typedef struct pci_dev_t
{
    uint8_t bus, slot, function;
} pci_dev_t;

uint32_t pci_read(pci_dev_t *, uint32_t);
void pci_write(pci_dev_t *, uint32_t, uint32_t);