
; Windozz
; Copyright (C) 2018 by Omar Muhamed
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.

bits 64

section .text

; void acquire(mutex_t *)
align 8
global acquire
acquire:
	bt qword[rdi], 0
	jc acquire
	lock bts qword[rdi], 0
	jc acquire

	ret

; void release(mutex_t *)
align 8
global release
release:
	mov byte[rdi], 0
	ret

; void outb(uint16_t, uint8_t)
align 8
global outb
outb:
	mov rdx, rdi
	mov rax, rsi
	out dx, al
	ret

; void outw(uint16_t, uint16_t)
align 8
global outw
outw:
	mov rdx, rdi
	mov rax, rsi
	out dx, ax
	ret

; void outd(uint16_t, uint32_t)
align 8
global outd
outd:
	mov rdx, rdi
	mov rax, rsi
	out dx, eax
	ret

; uint8_t inb(uint16_t)
align 8
global inb
inb:
	mov rdx, rdi
	in al, dx
	ret

; uint16_t inw(uint16_t)
align 8
global inw
inw:
	mov rdx, rdi
	in al, dx
	ret

; uint32_t ind(uint32_t)
align 8
global ind
ind:
	mov rdx, rdi
	in eax, dx
	ret

; void iowait()
align 8
global iowait
iowait:
	nop
	jmp .next

.next:
	nop
	ret

; void write_cr0(uint64_t)
align 8
global write_cr0
write_cr0:
	mov cr0, rdi
	ret

; void write_cr3(uint64_t)
align 8
global write_cr3
write_cr3:
	mov cr3, rdi
	ret

; void write_cr4(uint64_t)
align 8
global write_cr4
write_cr4:
	mov cr4, rdi
	ret

; uint64_t read_cr0()
align 8
global read_cr0
read_cr0:
	mov rax, cr0
	ret

; uint64_t read_cr2()
align 8
global read_cr2
read_cr2:
	mov rax, cr2
	ret

; uint64_t read_cr3()
align 8
global read_cr3
read_cr3:
	mov rax, cr3
	ret

; uint64_t read_cr4()
align 8
global read_cr4
read_cr4:
	mov rax, cr4
	ret

; void write_msr(uint32_t, uint64_t)
align 8
global write_msr
write_msr:
	mov ecx, edi
	mov eax, esi
	mov rdx, rsi
	shr rdx, 32
	wrmsr

	ret

; uint64_t read_msr(uint32_t)
align 8
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




