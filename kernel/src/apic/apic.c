/*
* apic.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE "apic"

#include <debug.h>
#include <apic.h>
#include <stdint.h>
#include <mm.h>
#include <acpi.h>
#include <cpu.h>

acpi_madt_t *madt;
size_t cpu_count, ioapic_count, override_count;
size_t cpu_slot_count;
uintptr_t lapic_physical;
void *lapic;
cpu_t *cpus;
ioapic_t *ioapics;
override_t *overrides;

static void create_cpu(madt_cpu_t *);
static void create_ioapic(madt_ioapic_t *);
static void create_override(madt_override_t *);

void apic_init()
{
    cpu_count = cpu_slot_count = ioapic_count = override_count = 0;
    cpus = kcalloc(sizeof(cpu_t), MAX_CPUS);
    ioapics = kcalloc(sizeof(ioapic_t), MAX_IOAPICS);
    overrides = kcalloc(sizeof(override_t), MAX_OVERRIDES);
    if(!cpus || !ioapics || !overrides)
    {
        ERROR("unable to allocate memory.\n");
        while(1);
    }

    if(acpi_find_table((void *)&madt, "APIC", 0) != ACPI_SUCCESS)
    {
        ERROR("ACPI MADT table not present.\n");
        while(1);
    }

    lapic_physical = madt->local_apic;
    lapic = (void *)vmm_create_mmio(lapic_physical, 1, "lapic");

    uint64_t apic_base = read_msr(IA32_APIC_BASE);
    apic_base |= IA32_APIC_BASE_ENABLED;
    apic_base &= 0xFFF;
    apic_base |= lapic_physical;
    write_msr(IA32_APIC_BASE, apic_base);

    DEBUG("MADT local APIC is at 0x%08X\n", madt->local_apic);
    DEBUG("MADT flags = 0x%08X\n", madt->flags);

    uint8_t *madt_data = (uint8_t *)madt->data;
    uint8_t *madt_end = (uint8_t *)madt + madt->header.length - 2;

    while(madt_data < madt_end)
    {
        if(madt_data[1] <= 2)
            break;

        switch(*madt_data)
        {
        case MADT_CPU:
            create_cpu((madt_cpu_t *)madt_data);
            madt_data += madt_data[1];
            break;

        case MADT_IOAPIC:
            create_ioapic((madt_ioapic_t *)madt_data);
            madt_data += madt_data[1];
            break;

        case MADT_OVERRIDE:
            create_override((madt_override_t *)madt_data);
            madt_data += madt_data[1];
            break;

        default:
            WARN("undefined MADT entry type %d size %d\n", madt_data[0], madt_data[1]);
            madt_data += madt_data[1];
            break;
        }
    }

    if(!ioapic_count)
    {
        ERROR("IOAPIC is not present.\n");
        while(1);
    }

    if(!cpu_count)
    {
        ERROR("MADT doesn't contain any CPU declarations.\n");
        while(1);
    }

    DEBUG("%d total CPU slot(s), out of which %d %s occupied.\n", cpu_slot_count, cpu_count, cpu_count == 1 ? "is" : "are");
    smp_boot();
}

static void create_cpu(madt_cpu_t *cpu)
{
    DEBUG("CPU local APIC ID 0x%02X ACPI ID 0x%02X flags 0x%08X\n", cpu->apic_id, cpu->acpi_id, cpu->flags);

    cpu_slot_count++;

    if(cpu->flags & 1)
    {
        cpus[cpu_count].apic_id = cpu->apic_id;
        cpus[cpu_count].acpi_id = cpu->acpi_id;
        cpu_count++;
    }
}

static void create_ioapic(madt_ioapic_t *ioapic)
{
    ioapics[ioapic_count].mmio_phys = ioapic->mmio;
    ioapics[ioapic_count].mmio = (void *)vmm_create_mmio(ioapic->mmio, 1, "ioapic");
    ioapics[ioapic_count].apic_id = ioapic->apic_id;
    ioapics[ioapic_count].gsi_start = ioapic->gsi;
    ioapics[ioapic_count].gsi_end = ioapic->gsi;
    ioapics[ioapic_count].gsi_end += (ioapic_read(ioapic_count, IOAPIC_VERSION) >> 16) & 0xFF;

    DEBUG("IOAPIC ID 0x%02X MMIO at 0x%08X IRQs %d -> %d\n", ioapics[ioapic_count].apic_id, ioapics[ioapic_count].mmio_phys, ioapics[ioapic_count].gsi_start, ioapics[ioapic_count].gsi_end);

    ioapic_count++;
}

static void create_override(madt_override_t *override)
{
    overrides[override_count].source = override->irq;
    overrides[override_count].destination = override->gsi;
    overrides[override_count].flags = override->flags;

    DEBUG("override for IRQ %d -> %d flags 0x%04X (%s %s)\n", override->irq, override->gsi, override->flags, override->flags & MADT_LEVEL ? "level" : "edge", override->flags & MADT_ACTIVE_LOW ? "low" : "high");

    override_count++;
}