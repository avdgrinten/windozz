
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 64

section .text

; void load_context(thread_t *)
align 16
global load_context
load_context:
    mov r15, rdi    ; r15 = thread struct

    ; make IRETQ frame
    mov rax, [r15+152]  ; ss
    push rax

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, [r15+128]  ; rsp
    push rax
    mov rax, [r15+136]  ; rflags
    push rax
    mov rax, [r15+144]  ; cs
    push rax
    mov rax, [r15+120]  ; rip
    push rax

    ; restore remaining state
    mov rax, [r15]
    mov rbx, [r15+8]
    mov rcx, [r15+16]
    mov rdx, [r15+24]
    mov rsi, [r15+32]
    mov rdi, [r15+40]
    mov rbp, [r15+48]
    mov r8, [r15+56]
    mov r9, [r15+64]
    mov r10, [r15+72]
    mov r11, [r15+80]
    mov r12, [r15+88]
    mov r13, [r15+96]
    mov r14, [r15+104]
    mov r15, [r15+112]

    iretq

