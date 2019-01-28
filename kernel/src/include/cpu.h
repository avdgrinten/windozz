
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

#define IA32_APIC_BASE                  0x1B

#define IA32_APIC_BASE_BSP              (1 << 8)
#define IA32_APIC_BASE_ENABLED          (1 << 11)

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