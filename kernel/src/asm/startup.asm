
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 64

KSTACK              equ 131072

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
stack_bottom:            resb KSTACK
stack_top:

section .rodata

align 16
global smp_trampoline16
global smp_trampoline16_size

smp_trampoline16:
    incbin "src/asm/smpboot.bin"
end_smp_trampoline16:

align 16
smp_trampoline16_size:  dw end_smp_trampoline16 - smp_trampoline16

section .text

global smp_trampoline
align 16
smp_trampoline:
    mov rdi, KSTACK          ; good kernel stack
    extern kmalloc
    call kmalloc
    add rax, KSTACK
    mov rsp, rax

    extern smp_kmain
    call smp_kmain

    ; impossible to reach here

.hang:
    hlt
    jmp .hang

