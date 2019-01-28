
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "smpboot"

#include <apic.h>
#include <debug.h>

static size_t current_cpu;
static void smp_boot_ap(size_t);

void smp_boot()
{
    size_t ap_count = cpu_count - 1;
    if(!ap_count)
    {
        /* TO-DO: configure BSP here */
        return;
    }

    /* TO-DO */
    while(1);

    for(current_cpu = 1; current_cpu < cpu_count; current_cpu++)
    {
        smp_boot_ap(current_cpu);
    }
}

static void smp_boot_ap(size_t index)
{
    DEBUG("booting AP %d with APIC ID 0x%02X...\n", index, cpus[index].apic_id);
}

