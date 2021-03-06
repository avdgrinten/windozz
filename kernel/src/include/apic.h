
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

#define ACTIVE_LOW      MADT_ACTIVE_LOW
#define LEVEL           MADT_LEVEL

#define ACTIVE_HIGH     0
#define EDGE            0

/* interrupt base */
#define IRQ_BASE            0x30
#define PIC_BASE            0x20    /* we need this for spurious irqs */

typedef struct cpu_t
{
    uint8_t apic_id, acpi_id;
    char vendor[13];
    char model[72];
    uint64_t clockspeed;
    int pid, tid;
    int sys_ready;
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
    acpi_sdth_t header;
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
override_t *overrides;
size_t cpu_count, ioapic_count, override_count;
size_t cpu_slot_count;
uintptr_t lapic_physical;
void *lapic;

void apic_init();
uint8_t get_apic_id();
cpu_t *get_cpu();
void smp_boot();
void smp_configure_cpu(size_t);

/* lapic stuff */
#define LAPIC_TPR               0x80
#define LAPIC_EOI               0xB0
#define LAPIC_SPURIOUS          0xF0
#define LAPIC_COMMAND_LOW       0x300
#define LAPIC_COMMAND_HIGH      0x310

#define LAPIC_SPURIOUS_ENABLE   (1 << 8)

#define LAPIC_COMMAND_INIT      (5 << 8)
#define LAPIC_COMMAND_SIPI      (6 << 8)
#define LAPIC_COMMAND_DELIVERY  (1 << 12)
#define LAPIC_COMMAND_ASSERT    (1 << 14)
#define LAPIC_COMMAND_DEASSERT  (1 << 15)

uint32_t lapic_read(size_t);
void lapic_write(size_t, uint32_t);
void lapic_configure();
void lapic_eoi();

/* ioapic stuff */
#define IOAPIC_ID               0
#define IOAPIC_VERSION          1
#define IOAPIC_TABLES           16

#define IOAPIC_ACTIVE_LOW       (1 << 13)
#define IOAPIC_LEVEL            (1 << 15)
#define IOAPIC_MASK             (1 << 16)

uint32_t ioapic_read(size_t, uint32_t);
void ioapic_write(size_t, uint32_t, uint32_t);
void ioapic_init();

uint8_t irq_configure(uint8_t, uintptr_t, uint8_t);
void irq_mask(uint8_t);
void irq_unmask(uint8_t);

