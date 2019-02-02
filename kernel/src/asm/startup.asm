
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 64

section .startup

; kernel entry from boot loader
global _start
_start:
    cld

    ; clear the BSS
    extern bss
    extern bssend
    mov rdi, bss
    mov rcx, bssend
    sub rcx, rdi
    xor rax, rax
    rep stosb

    mov rsp, stack_top

    mov rdi, rbp
    extern kmain
    call kmain

    ; shouldn't be possible to ever reach here
.hang:
    hlt
    jmp .hang

section .bss

align 16
stack_bottom:            resb 131072
stack_top:
