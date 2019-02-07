
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
#include <sys.h>

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
    sched_init();

    while(1);
}

void idle()
{
    while(1) asm volatile ("sti\nhlt");
}

void kmain_late()
{
    DEBUG("Hello from a new task!\n");

    create_thread((uintptr_t)&idle);
    while(1);
}

