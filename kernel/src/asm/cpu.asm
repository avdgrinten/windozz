
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







