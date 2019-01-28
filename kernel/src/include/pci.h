
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

typedef struct pci_dev_t
{
    uint8_t bus, slot, function;
} pci_dev_t;

uint32_t pci_read(pci_dev_t *, uint32_t);
void pci_write(pci_dev_t *, uint32_t, uint32_t);