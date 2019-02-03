
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "smpboot"

#include <apic.h>
#include <debug.h>
#include <cpu.h>
#include <timer.h>
#include <mm.h>
#include <string.h>

static volatile size_t current_cpu;
static void smp_boot_ap(size_t);
volatile static int cpu_booted;
extern uint8_t smp_trampoline16[];
extern uint16_t smp_trampoline16_size[];
extern void smp_trampoline();

void smp_boot()
{
    size_t ap_count = cpu_count - 1;
    if(!ap_count) return;

    volatile uintptr_t *zero = (volatile uintptr_t *)PHYSICAL_MEMORY;
    zero[0] = (uintptr_t)&smp_trampoline;
    zero[1] = (uintptr_t)read_cr3();

    volatile uintptr_t *pml4 = (volatile uintptr_t *)((uintptr_t)read_cr3() + PHYSICAL_MEMORY);
    pml4[0] = pml4[256];

    memcpy((void *)PHYSICAL_MEMORY + 0x1000, smp_trampoline16, *smp_trampoline16_size);

    for(current_cpu = 1; current_cpu < cpu_count; current_cpu++)
        smp_boot_ap(current_cpu);
}

static void smp_boot_ap(size_t index)
{
    int i;
    uint8_t apic_id = cpus[index].apic_id;

    cpu_booted = 0;

    /* init IPI */
    lapic_write(LAPIC_COMMAND_HIGH, (uint32_t)apic_id << 24);
    lapic_write(LAPIC_COMMAND_LOW, LAPIC_COMMAND_INIT | LAPIC_COMMAND_ASSERT);
    timer_sleep(1);

    i = 0;
    while(lapic_read(LAPIC_COMMAND_LOW) & LAPIC_COMMAND_DELIVERY)
    {
        timer_sleep(1);
        i++;
        if(i > TIMER_FREQUENCY * 2)
        {
            ERROR("local APIC ID 0x%02X is not responding.\n", apic_id);
            while(1);
        }
    }

    /* startup IPI */
    lapic_write(LAPIC_COMMAND_HIGH, (uint32_t)apic_id << 24);
    lapic_write(LAPIC_COMMAND_LOW, LAPIC_COMMAND_SIPI | LAPIC_COMMAND_ASSERT | 0x01);
    timer_sleep(1);

    i = 0;
    while(!cpu_booted)
    {
        timer_sleep(1);
        i++;
        if(i > TIMER_FREQUENCY * 2)
        {
            ERROR("local APIC ID 0x%02X is not responding.\n", apic_id);
            while(1);
        }
    }

    DEBUG("CPU index %d running and responded after %dms.\n", index, i);
}

void smp_configure_cpu(size_t index)
{
    gdt_init();
}

void smp_kmain()
{
    smp_configure_cpu(current_cpu);
    load_idt(idtr);
    lapic_configure();

    cpu_booted = 1;

    while(1)
        asm volatile ("sti\nhlt");
}


