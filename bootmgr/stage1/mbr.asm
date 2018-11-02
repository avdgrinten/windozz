; mbr.asm
; Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
; Use of this source code is governed by a license that can be
; found in the LICENSE.md file, in the root directory of
; the source package.

bits 16
org 0

main:
	; relocate MBR to 0x0060:0x0000
	cli
	cld
	xor ax, ax
	mov ss, ax
	mov sp, ax
	mov ds, ax
	mov es, ax
	mov si, 0x7c00
	mov di, 0x0600
	mov cx, 512
	rep movsb

	jmp 0x0060:.next

.next:
	mov ax, 0x0060
	mov ds, ax
	mov es, ax

	mov [bootdisk], dl

	sti

	xor ax, ax
	mov dl, [bootdisk]
	int 0x13
	jc disk_error

	mov si, partition1
	mov cx, 4

.loop:
	test byte[si], 0x80		; active?
	jnz .boot			; -- yes

	add si, 16
	loop .loop

	jmp no_boot

.boot:
	push si

	mov eax, [si+8]			; LBA
	mov dword[dap.lba], eax

	mov ah, 0x42
	mov dl, [bootdisk]
	mov si, dap
	int 0x13
	jc disk_error

	pop si				; ds:si and ds:bp both point to the
	mov bp, si			; booted partition, for compatibility
	mov dl, [bootdisk]
	jmp 0x0000:0x7C00

disk_error:
	mov si, .msg
	jmp error

	.msg				db "Disk I/O error.", 0

no_boot:
	mov si, .msg
	jmp error

	.msg				db "No active partition.", 0

error:
	lodsb
	cmp al, 0
	je .hang
	mov ah, 0x0E
	int 0x10
	jmp error

.hang:
	sti
	hlt
	jmp .hang

	; data
	bootdisk:			db 0

	align 4
	dap:
		.size			dw 16
		.sectors		dw 1
		.offset			dw 0x7C00
		.segment		dw 0
		.lba			dq 0

	times 0x1BE - ($-$$) db 0

; partition table
partition1:
	.active				db 0x80
	.start_chs			db 0
					db 0
					db 0
	.type				db 0xF3
	.end_chs			db 0
					db 0
					db 0
	.lba				dd 63
	.size				dd 20480

partition2:
	.active				db 0
	.start_chs			db 0
					db 0
					db 0
	.type				db 0
	.end_chs			db 0
					db 0
					db 0
	.lba				dd 0
	.size				dd 0

partition3:
	.active				db 0
	.start_chs			db 0
					db 0
					db 0
	.type				db 0
	.end_chs			db 0
					db 0
					db 0
	.lba				dd 0
	.size				dd 0

partition4:
	.active				db 0
	.start_chs			db 0
					db 0
					db 0
	.type				db 0
	.end_chs			db 0
					db 0
					db 0
	.lba				dd 0
	.size				dd 0

	boot_signature:			dw 0xAA55