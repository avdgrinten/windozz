
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 64

section .text

; void handle_exception(int number, uint64_t code, uint64_t rip)
extern handle_exception
extern lapic_eoi

global int0_handler
global int1_handler
global int2_handler
global int3_handler
global int4_handler
global int5_handler
global int6_handler
global int7_handler
global int8_handler
global int9_handler
global int10_handler
global int11_handler
global int12_handler
global int13_handler
global int14_handler
global int15_handler
global int16_handler
global int17_handler
global int18_handler
global int19_handler
global int20_handler
global int21_handler
global int22_handler
global int23_handler
global int24_handler
global int25_handler
global int26_handler
global int27_handler
global int28_handler
global int29_handler
global int30_handler
global int31_handler

%macro pushaq 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro exception 1
    pushaq

    mov rdi, %1
    mov rsi, 0
    mov rdx, [rsp+120]  ; rip
    call handle_exception

    popaq
    iretq
%endmacro

%macro exception_code 1
    pushaq

    mov rdi, %1
    mov rsi, [rsp+120]  ; error code
    mov rdx, [rsp+128]  ; rip
    call handle_exception

    popaq
    add rsp, 8
    iretq
%endmacro

int0_handler:
    exception 0

int1_handler:
    exception 1

int2_handler:
    exception 2

int3_handler:
    exception 3

int4_handler:
    exception 4

int5_handler:
    exception 5

int6_handler:
    exception 6

int7_handler:
    exception 7

int8_handler:
    exception_code 8

int9_handler:
    exception 9

int10_handler:
    exception_code 10

int11_handler:
    exception_code 11

int12_handler:
    exception_code 12

int13_handler:
    exception_code 13

int14_handler:
    exception_code 14

int15_handler:
    exception 15

int16_handler:
    exception 16

int17_handler:
    exception_code 17

int18_handler:
    exception 18

int19_handler:
    exception 19

int20_handler:
    exception 20

int21_handler:
    exception 21

int22_handler:
    exception 22

int23_handler:
    exception 23

int24_handler:
    exception 24

int25_handler:
    exception 25

int26_handler:
    exception 26

int27_handler:
    exception 27

int28_handler:
    exception 28

int29_handler:
    exception 29

int30_handler:
    exception_code 30

int31_handler:
    exception 31

global pic0_spurious_stub
pic0_spurious_stub:
    pushaq

    extern pic0_spurious
    call pic0_spurious

    popaq
    iretq

global pic1_spurious_stub
pic1_spurious_stub:
    pushaq

    extern pic1_spurious
    call pic1_spurious

    popaq
    iretq

global lapic_spurious_stub
lapic_spurious_stub:
    pushaq

    extern lapic_spurious
    call lapic_spurious

    popaq
    iretq

global timer_irq_stub
timer_irq_stub:
    cli
    pushaq

    extern timer_irq
    call timer_irq

    call lapic_eoi

    popaq
    iretq

global acpi_sci_stub
acpi_sci_stub:
    cli
    pushaq

    extern acpi_sci
    call acpi_sci

    call lapic_eoi

    popaq
    iretq

