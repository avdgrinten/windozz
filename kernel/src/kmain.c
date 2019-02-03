
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

boot_info_t boot_info;

void kmain(boot_info_t *boot_info_tmp)
{
    memcpy(&boot_info, boot_info_tmp, sizeof(boot_info_t));
    debug_init();
    screen_init();
    mm_init();
    smp_configure_cpu(0);
    idt_init();
    screen_setup_buffer();
    acpi_init((rsdp_t *)boot_info.acpi_rsdp);
    apic_init();
    timer_init();
    acpi_create_namespace(acpi_instance.dsdt);
    acpi_install_irq();
    smp_boot();

    DEBUG("Boot finished, %d MB used and %d MB free.\n", used_pages / 256, (total_pages - used_pages) / 256);
    while(1);
}
