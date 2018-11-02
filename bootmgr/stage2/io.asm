; io.asm
; Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
; Use of this source code is governed by a license that can be
; found in the LICENSE.md file, in the root directory of
; the source package.

bits 16

print:
	pusha

.loop:
	lodsb
	cmp al, 0
	je .done
	cmp al, 10
	je .newline
	mov ah, 0x0E
	int 0x10
	jmp .loop

.newline:
	mov ah, 0x0E
	mov al, 13
	int 0x10
	mov ah, 0x0E
	mov al, 10
	int 0x10
	jmp .loop

.done:
	popa
	ret

error:
	push si
	mov ah, 5
	mov al, 0
	int 0x10

	mov ah, 1
	mov cx, 0x2807
	int 0x10

	mov ah, 2
	mov bh, 0
	xor dx, dx
	int 0x10

	mov ah, 9
	xor al, al
	xor bh, bh
	mov bl, 0x1F
	mov cx, 80*25
	int 0x10

	mov ah, 9
	xor al, al
	xor bh, bh
	mov bl, 0x17
	mov cx, 80*7
	int 0x10

	mov ah, 9
	xor al, al
	xor bh, bh
	mov bl, 0x70
	mov cx, 80
	int 0x10

	mov ah, 0x0E
	mov al, 0
	int 0x10

	mov si, banner
	call print

	mov si, newline
	call print
	call print

	mov si, .msg
	call print

	pop si
	call print

.hang:
	sti
	hlt
	jmp .hang

.msg:				db " Windozz has failed to start. This is most likely caused due to incompatible", 10
				db " hardware. Please report this error at http://windozz.github.io/ for it to be", 10
				db " reviewed and solved. Press Ctrl+Alt+Delete to reboot your PC.", 10, 10
				db " Error description: ", 10
				db "  *** ", 0