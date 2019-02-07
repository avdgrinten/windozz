
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>
#include <apic.h>

#define IA32_APIC_BASE                  0x1B
#define IA32_PAT                        0x277

#define IA32_APIC_BASE_BSP              (1 << 8)
#define IA32_APIC_BASE_ENABLED          (1 << 11)

#define IA32_PAT_UC                     0
#define IA32_PAT_WC                     1
#define IA32_PAT_WT                     4
#define IA32_PAT_WP                     5
#define IA32_PAT_WB                     6
#define IA32_PAT_UCD                    7

#define GDT_ENTRY_NULL                  0
#define GDT_ENTRY_KCODE                 1
#define GDT_ENTRY_KDATA                 2
#define GDT_ENTRY_UCODE                 3
#define GDT_ENTRY_UDATA                 4
#define GDT_ENTRY_TSS_LOW               5
#define GDT_ENTRY_TSS_HIGH              6

#define GDT_ENTRIES                     7

#define KCODE_SEGMENT                   0x08
#define KDATA_SEGMENT                   0x10
#define UCODE_SEGMENT                   0x1B
#define UDATA_SEGMENT                   0x23

#define GDT_ACCESS_PRESENT              (1 << 7)
#define GDT_ACCESS_USER                 (3 << 5)
#define GDT_ACCESS_NOT_TSS              (1 << 4)
#define GDT_ACCESS_EXECUTABLE           (1 << 3)
#define GDT_ACCESS_RW                   (1 << 1)

#define GDT_FLAGS_GRANULARITY           (1 << 3)
#define GDT_FLAGS_LONG_MODE             (1 << 1)

#define GDT_KCODE_ACCESS                (GDT_ACCESS_PRESENT | GDT_ACCESS_NOT_TSS | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RW)
#define GDT_KDATA_ACCESS                (GDT_ACCESS_PRESENT | GDT_ACCESS_NOT_TSS | GDT_ACCESS_RW)

#define GDT_UCODE_ACCESS                (GDT_KCODE_ACCESS | GDT_ACCESS_USER)
#define GDT_UDATA_ACCESS                (GDT_KDATA_ACCESS | GDT_ACCESS_USER)

typedef struct gdt_t
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
}__attribute__((packed)) gdt_t;

typedef struct idt_t
{
    uint16_t handler_low;
    uint16_t segment;
    uint8_t reserved0;
    uint8_t attributes;
    uint16_t handler_middle;
    uint32_t handler_high;
    uint32_t reserved1;
}__attribute__((packed)) idt_t;

typedef struct gdtr_t
{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed)) gdtr_t;

typedef struct idtr_t
{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed)) idtr_t;

idt_t *idt;
idtr_t *idtr;

void write_cr0(uint64_t);
void write_cr3(uint64_t);
void write_cr4(uint64_t);

uint64_t read_cr0();
uint64_t read_cr2();
uint64_t read_cr3();
uint64_t read_cr4();

void write_msr(uint32_t, uint64_t);
uint64_t read_msr(uint32_t);

void flush_tlb(uintptr_t);

void load_gdt(gdtr_t *);
void load_idt(idtr_t *);

void gdt_init();
void idt_init();
void idt_install(uint8_t, uintptr_t);

void reload_segments(uint16_t, uint16_t);
