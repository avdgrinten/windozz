
/*
 * Lux ACPI Implementation
 * Copyright (C) 2018-2019 by Omar Muhamed
 */

/* Windozz-specific OS functions */

#include <debug.h>
#include <string.h>
#include <lai/host.h>
#include <mm.h>
#include <io.h>
#include <pci.h>
#include <acpi.h>
#include <timer.h>

// Any OS using lai must provide implementations of the following functions

void laihost_log(int level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    switch(level)
    {
    case LAI_DEBUG_LOG:
        debug_vprintf(LEVEL_DEBUG, "acpi", fmt, args);
        break;
    default:
        debug_vprintf(LEVEL_WARN, "acpi", fmt, args);
    }
	va_end(args);
}

void laihost_panic(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    debug_printf(LEVEL_ERROR, "acpi", fmt, args);
	va_end(args);
    while(1)
        ;
}

void *laihost_scan(char *name, size_t index)
{
    void *ptr;
    acpi_status_t status = acpi_find_table(&ptr, name, index);
    if(status == ACPI_SUCCESS)
        return ptr;
    else
        return NULL;
}

void *laihost_malloc(size_t count)
{
    return kmalloc(count);
}

void *laihost_realloc(void *ptr, size_t count)
{
    return krealloc(ptr, count);
}

void laihost_free(void *ptr)
{
    kfree(ptr);
}

void *laihost_map(size_t physical, size_t count)
{
    count += PAGE_SIZE - 1;
    count >>= PAGE_SIZE_SHIFT;
    return (void*)vmm_create_mmio(physical, count, "acpi");
}

void laihost_outb(uint16_t port, uint8_t data)
{
    outb(port, data);
}

void laihost_outw(uint16_t port, uint16_t data)
{
    outw(port, data);
}

void laihost_outd(uint16_t port, uint32_t data)
{
    outd(port, data);
}

uint8_t laihost_inb(uint16_t port)
{
    return inb(port);
}

uint16_t laihost_inw(uint16_t port)
{
    return inw(port);
}

uint32_t laihost_ind(uint16_t port)
{
    return ind(port);
}

void laihost_pci_write(uint8_t bus, uint8_t slot, uint8_t function, uint16_t offset, uint32_t data)
{
    pci_dev_t dev;
    dev.bus = bus;
    dev.slot = slot;
    dev.function = function;

    pci_write(&dev, offset, data);
}

uint32_t laihost_pci_read(uint8_t bus, uint8_t slot, uint8_t function, uint16_t offset)
{
    pci_dev_t dev;
    dev.bus = bus;
    dev.slot = slot;
    dev.function = function;

    return pci_read(&dev, offset);
}

void laihost_sleep(uint64_t time)
{
    timer_sleep(time);
}

