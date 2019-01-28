/*
* pci.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#include <pci.h>
#include <io.h>

uint32_t pci_read(pci_dev_t *dev, uint32_t offset)
{
    uint32_t addr;
    addr = (dev->bus << 16) | (dev->slot << 11) | (dev->function << 8) + (offset & 0xFC) | 0x80000000;
    outd(0xCF8, addr);
    iowait();
    return ind(0xCFC);
}

void pci_write(pci_dev_t *dev, uint32_t offset, uint32_t value)
{
    uint32_t addr;
    addr = (dev->bus << 16) | (dev->slot << 11) | (dev->function << 8) + (offset & 0xFC) | 0x80000000;
    outd(0xCF8, addr);
    iowait();
    outd(0xCFC, value);
    iowait();
}