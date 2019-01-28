
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 64

section .text

; void *memcpy(void *, const void *, size_t)
align 16
global memcpy
memcpy:
	mov rax, rdi
	mov rcx, rdx
	push rcx
	shr rcx, 3		; div 8
	cld
	rep movsq
	pop rcx
	and rcx, 7		; mod 8
	rep movsb

	ret

; void acquire(mutex_t *)
align 16
global acquire
acquire:
	bt qword[rdi], 0
	jc acquire
	lock bts qword[rdi], 0
	jc acquire

	ret

; void release(mutex_t *)
align 16
global release
release:
	mov byte[rdi], 0
	ret

; void outb(uint16_t, uint8_t)
align 16
global outb
outb:
	mov rdx, rdi
	mov rax, rsi
	out dx, al
	ret

; void outw(uint16_t, uint16_t)
align 16
global outw
outw:
	mov rdx, rdi
	mov rax, rsi
	out dx, ax
	ret

; void outd(uint16_t, uint32_t)
align 16
global outd
outd:
	mov rdx, rdi
	mov rax, rsi
	out dx, eax
	ret

; uint8_t inb(uint16_t)
align 16
global inb
inb:
	mov rdx, rdi
	in al, dx
	ret

; uint16_t inw(uint16_t)
align 16
global inw
inw:
	mov rdx, rdi
	in al, dx
	ret

; uint32_t ind(uint32_t)
align 16
global ind
ind:
	mov rdx, rdi
	in eax, dx
	ret

; void iowait()
align 16
global iowait
iowait:
	nop
	jmp .next

.next:
	nop
	ret

; void write_cr0(uint64_t)
align 16
global write_cr0
write_cr0:
	mov cr0, rdi
	ret

; void write_cr3(uint64_t)
align 16
global write_cr3
write_cr3:
	mov cr3, rdi
	ret

; void write_cr4(uint64_t)
align 16
global write_cr4
write_cr4:
	mov cr4, rdi
	ret

; uint64_t read_cr0()
align 16
global read_cr0
read_cr0:
	mov rax, cr0
	ret

; uint64_t read_cr2()
align 16
global read_cr2
read_cr2:
	mov rax, cr2
	ret

; uint64_t read_cr3()
align 16
global read_cr3
read_cr3:
	mov rax, cr3
	ret

; uint64_t read_cr4()
align 16
global read_cr4
read_cr4:
	mov rax, cr4
	ret

; void write_msr(uint32_t, uint64_t)
align 16
global write_msr
write_msr:
	mov ecx, edi
	mov eax, esi
	mov rdx, rsi
	shr rdx, 32
	wrmsr

	ret

; uint64_t read_msr(uint32_t)
align 16
global read_msr
read_msr:
	mov ecx, edi
	rdmsr

	xor rcx, rcx
	not ecx		; ecx = all ones

	and rax, rcx
	shl rdx, 32
	or rax, rdx
	ret

; void flush_tlb(uintptr_t)
align 16
global flush_tlb
flush_tlb:
	invlpg [rdi]
	ret

; void load_gdt(gdtr_t *)
align 16
global load_gdt
load_gdt:
	lgdt [rdi]
	ret

; void load_idt(idtr_t *)
align 16
global load_idt
load_idt:
	lidt [rdi]
	ret
