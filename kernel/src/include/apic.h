
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>
#include <acpi.h>

/* this limitation exists because the APIC ID field is 8 bits wide */
#define MAX_CPUS        256
#define MAX_IOAPICS     16
#define MAX_OVERRIDES   16

#define MADT_CPU        0
#define MADT_IOAPIC     1
#define MADT_OVERRIDE   2
#define MADT_NMI        4
#define MADT_LAPIC      5

#define MADT_ACTIVE_LOW 0x0002
#define MADT_LEVEL      0x0008

typedef struct cpu_t
{
    uint8_t apic_id, acpi_id;
    char vendor[13];
    char model[72];
    uint64_t clockspeed;
} cpu_t;

typedef struct ioapic_t
{
    uint8_t apic_id;
    uint32_t gsi_start, gsi_end;
    uintptr_t mmio_phys;
    void *mmio;
} ioapic_t;

typedef struct override_t
{
    uint8_t source, destination;
    uint16_t flags;
} override_t;

typedef struct acpi_madt_t
{
    acpi_header_t header;
    uint32_t local_apic;
    uint32_t flags;

    uint8_t data[];
}__attribute__((packed)) acpi_madt_t;

typedef struct madt_cpu_t
{
    uint8_t type;
    uint8_t length;
    uint8_t acpi_id;
    uint8_t apic_id;
    uint32_t flags;
}__attribute__((packed)) madt_cpu_t;

typedef struct madt_ioapic_t
{
    uint8_t type;
    uint8_t length;
    uint8_t apic_id;
    uint8_t reserved;
    uint32_t mmio;
    uint32_t gsi;
}__attribute__((packed)) madt_ioapic_t;

typedef struct madt_override_t
{
    uint8_t type;
    uint8_t length;
    uint8_t bus;
    uint8_t irq;
    uint32_t gsi;
    uint16_t flags;
}__attribute__((packed)) madt_override_t;

typedef struct madt_nmi_t
{
    uint8_t type;
    uint8_t length;
    uint8_t acpi_id;
    uint16_t flags;
    uint8_t lint;
}__attribute__((packed)) madt_nmi_t;

typedef struct madt_lapic_t
{
    uint8_t type;
    uint8_t length;
    uint16_t reserved;
    uint64_t mmio;
}__attribute__((packed)) madt_lapic_t;

cpu_t *cpus;
ioapic_t *ioapics;
size_t cpu_count, ioapic_count, override_count;
size_t cpu_slot_count;
uintptr_t lapic_physical;
void *lapic;

void apic_init();
void smp_boot();

/* lapic stuff */
uint32_t lapic_read(size_t);
void lapic_write(size_t, uint32_t);

/* ioapic stuff */
#define IOAPIC_ID               0
#define IOAPIC_VERSION          1
#define IOAPIC_TABLES           16

uint32_t ioapic_read(size_t, size_t);
void ioapic_write(size_t, size_t, uint32_t);