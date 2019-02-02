
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE NULL

#include <bootmgr.h>
#include <string.h>
#include <debug.h>
#include <screen.h>
#include <mm.h>
#include <cpu.h>
#include <acpi.h>
#include <apic.h>
#include <stddef.h>
#include <lai.h>
#include <timer.h>
#include <pci.h>

boot_info_t boot_info;

void kmain(boot_info_t *boot_info_tmp)
{
    memcpy(&boot_info, boot_info_tmp, sizeof(boot_info_t));
    debug_init();
    screen_init();
    mm_init();
    smp_configure_cpu(0);
    idt_init();
    acpi_init((rsdp_t *)boot_info.acpi_rsdp);
    apic_init();
    timer_init();
    acpi_create_namespace(acpi_instance.dsdt);
    acpi_install_irq();

    /* show IRQs of all devices on PCI bus 0 */
    pci_dev_t dev;
    dev.bus = 0;
    dev.function = 0;
    acpi_resource_t resource;

    for(int i = 0; i < 32; i++)
    {
        dev.slot = i;
        if(pci_read(&dev, 0) == 0xFFFFFFFF)
            break;

        acpi_pci_route(&resource, dev.bus, dev.slot, dev.function);
    }

    DEBUG("Boot finished, %d MB used and %d MB free.\n", used_pages / 256, (total_pages - used_pages) / 256);
    while(1);
}
