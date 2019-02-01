
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE NULL

#include <stddef.h>
#include <stdint.h>
#include <debug.h>
#include <cpu.h>

const char *exception_text[] =
{
    "divide error",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "BOUND range exceeded",
    "undefined opcode",
    "device not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid TSS",
    "segment not present",
    "stack segment fault",
    "general protection fault",
    "page fault",
    "reserved",
    "x87 floating-point exception",
    "alignment check",
    "machine check",
    "SIMD floating-point exception",
    "virtualization exception",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "security exception",
    "reserved"
};

void handle_exception(int number, uint64_t code, uint64_t rip)
{
    ERROR("Exception 0x%02X at RIP 0x%016lX: %s\n", number, rip, exception_text[number]);
    if(number == 14)
    {
        /* page fault */
        ERROR("Error code 0x%08lX: (%s %s %s)\n", code,
            code & 4 ? "user" : "kernel",
            code & 2 ? "write" : "read",
            code & 1 ? "present" : "non-present");
        ERROR("Linear address of fault: 0x%016lX\n", read_cr2());
    } else
    {
        ERROR("Error code 0x%08lX\n", code);
    }

    while(1);
}
