
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE "idt"

#include <cpu.h>
#include <mm.h>
#include <debug.h>
#include <string.h>

extern void int0_handler();
extern void int1_handler();
extern void int2_handler();
extern void int3_handler();
extern void int4_handler();
extern void int5_handler();
extern void int6_handler();
extern void int7_handler();
extern void int8_handler();
extern void int9_handler();
extern void int10_handler();
extern void int11_handler();
extern void int12_handler();
extern void int13_handler();
extern void int14_handler();
extern void int15_handler();
extern void int16_handler();
extern void int17_handler();
extern void int18_handler();
extern void int19_handler();
extern void int20_handler();
extern void int21_handler();
extern void int22_handler();
extern void int23_handler();
extern void int24_handler();
extern void int25_handler();
extern void int26_handler();
extern void int27_handler();
extern void int28_handler();
extern void int29_handler();
extern void int30_handler();
extern void int31_handler();

idt_t *idt;
idtr_t *idtr;

void idt_init()
{
    idt = kcalloc(sizeof(idt_t), 256);
    idtr = kcalloc(sizeof(idtr_t), 1);

    if(!idt || !idtr)
    {
        ERROR("unable to allocate memory.\n");
        while(1);
    }

    idtr->limit = (sizeof(idt_t) * 256) - 1;
    idtr->base = (uint64_t)idt;

    load_idt(idtr);

    /* install exception handlers */
    idt_install(0, (uintptr_t)&int0_handler);
    idt_install(1, (uintptr_t)&int1_handler);
    idt_install(2, (uintptr_t)&int2_handler);
    idt_install(3, (uintptr_t)&int3_handler);
    idt_install(4, (uintptr_t)&int4_handler);
    idt_install(5, (uintptr_t)&int5_handler);
    idt_install(6, (uintptr_t)&int6_handler);
    idt_install(7, (uintptr_t)&int7_handler);
    idt_install(8, (uintptr_t)&int8_handler);
    idt_install(9, (uintptr_t)&int9_handler);
    idt_install(10, (uintptr_t)&int10_handler);
    idt_install(11, (uintptr_t)&int11_handler);
    idt_install(12, (uintptr_t)&int12_handler);
    idt_install(13, (uintptr_t)&int13_handler);
    idt_install(14, (uintptr_t)&int14_handler);
    idt_install(15, (uintptr_t)&int15_handler);
    idt_install(16, (uintptr_t)&int16_handler);
    idt_install(17, (uintptr_t)&int17_handler);
    idt_install(18, (uintptr_t)&int18_handler);
    idt_install(19, (uintptr_t)&int19_handler);
    idt_install(20, (uintptr_t)&int20_handler);
    idt_install(21, (uintptr_t)&int21_handler);
    idt_install(22, (uintptr_t)&int22_handler);
    idt_install(23, (uintptr_t)&int23_handler);
    idt_install(24, (uintptr_t)&int24_handler);
    idt_install(25, (uintptr_t)&int25_handler);
    idt_install(26, (uintptr_t)&int26_handler);
    idt_install(27, (uintptr_t)&int27_handler);
    idt_install(28, (uintptr_t)&int28_handler);
    idt_install(29, (uintptr_t)&int29_handler);
    idt_install(30, (uintptr_t)&int30_handler);
    idt_install(31, (uintptr_t)&int31_handler);
}

void idt_install(uint8_t vector, uintptr_t handler)
{
    memset(&idt[vector], 0, sizeof(idt_t));

    idt[vector].handler_low = (uint16_t)handler;
    idt[vector].handler_middle = (uint16_t)(handler >> 16) & 0xFFFF;
    idt[vector].handler_high = (uint32_t)(handler >> 32) & 0xFFFFFFFF;
    idt[vector].segment = KCODE_SEGMENT;
    idt[vector].attributes = 0x8E;  /* bit 7 = present, 0Eh = interrupt gate */
}

