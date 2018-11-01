; vbe.asm
; Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
; Use of this source code is governed by a license that can be
; found in the LICENSE.md file, in the root directory of
; the source package.

bits 16

DEFAULT_WIDTH			equ 800
DEFAULT_HEIGHT			equ 600

vbe_width			dw DEFAULT_WIDTH
vbe_height			dw DEFAULT_HEIGHT

align 16
vbe_bios_info:
	.signature		db "VBE2"
	.version		dw 0
	.oem			dd 0
	.capabilities		dd 0
	.video_modes		dd 0
	.vram_size		dw 0
	.software_revision	dw 0
	.vendor			dd 0
	.product		dd 0
	.product_revision	dd 0
	.reserved:		times 222 db 0
	.oem_data:		times 256 db 0

align 16
vbe_mode_info:
	.attributes		dw 0
	.window_a		db 0
	.window_b		db 0
	.granularity		dw 0
	.window_size		dw 0
	.segment_a		dw 0
	.segment_b		dw 0
	.win_func_ptr		dd 0
	.pitch			dw 0
	.width			dw 0
	.height			dw 0
	.w_char			db 0
	.y_char			db 0
	.planes			db 0
	.bpp			db 0
	.banks			db 0
	.memory_model		db 0
	.bank_size		db 0
	.image_pages		db 0
	.reserved0		db 0

	.red_mask		db 0
	.red_position		db 0
	.green_mask		db 0
	.green_position		db 0
	.blue_mask		db 0
	.blue_position		db 0
	.reserved_mask		db 0
	.reserved_position	db 0
	.direct_color		db 0

	.framebuffer		dd 0
	.invisible_vram		dd 0
	.invisible_vram_size	dw 0
	.reserved1:		times 206 db 0

align 16
vbe_edid:
	.header:		times 8 db 0
	.vendor_product:	times 10 db 0
	.edid_version		db 0
	.edid_revision		db 0
	.display_params:	times 5 db 0
	.color_params:		times 10 db 0
	.established_timings:	times 3 db 0
	.standard_timings:	times 16 db 0	; 8*2 bytes
	.detailed_timings:	times 72 db 0	; 4*18 bytes
	.extension_flag		db 0
	.checksum		db 0

do_edid:
	mov ax, 0x4F15
	mov bx, 1
	xor cx, cx
	mov dx, cx
	mov di, vbe_edid
	int 0x10

	cmp ax, 0x4F
	jne .error

	movzx ax, byte[vbe_edid.detailed_timings+2]
	movzx bx, byte[vbe_edid.detailed_timings+4]
	shl bx, 4
	or ax, bx

	mov [vbe_width], ax

	movzx ax, byte[vbe_edid.detailed_timings+5]
	movzx bx, byte[vbe_edid.detailed_timings+7]
	shl bx, 4
	or ax, bx

	mov [vbe_height], ax

	cmp word[vbe_width], 0
	je .error

	cmp word[vbe_height], 0
	je .error

	ret

.error:
	mov si, .msg
	call print

	ret

.msg				db "warning: VBE EDID not supported", 10, 0

do_vbe:
	mov dword[vbe_bios_info.signature], "VBE2"
	mov ax, 0x4F00
	mov di, vbe_bios_info
	push es
	int 0x10
	pop es

	mov si, .no_vbe
	cmp ax, 0x4F
	jne error
	cmp dword[vbe_bios_info.signature], "VESA"
	jne error
	cmp word[vbe_bios_info.version], 0x200
	jl error

	call do_edid

	mov ax, [vbe_width]
	mov bx, [vbe_height]
	mov cl, 32		; 32bpp always
	call vbe_set_mode
	jc .try_default

	ret

.try_default:
	mov word[vbe_width], DEFAULT_WIDTH
	mov word[vbe_height], DEFAULT_HEIGHT

	mov ax, [vbe_width]
	mov bx, [vbe_height]
	mov cl, 32		; 32bpp always
	call vbe_set_mode

	mov si, .error
	jc error

.error				db "failed to set VBE mode.",0
.no_vbe				db "VBE 2.0+ not present.", 0

vbe_set_mode:		; in: ax/bx/cl = width/height/bpp
			; out: ax = mode number, flags.cf = 0 on success
	mov [.width], ax
	mov [.height], bx
	mov [.bpp], cl

	mov ax, [vbe_bios_info.video_modes+2]
	mov fs, ax
	mov si, [vbe_bios_info.video_modes]

.loop:
	mov cx, [fs:si]
	cmp cx, 0xFFFF		; end of list
	je .error

	mov [.mode], cx

	add si, 2
	mov [.offset], si

	mov ax, 0x4F01
	mov di, vbe_mode_info
	push es
	int 0x10
	pop es

	cmp ax, 0x4F
	jne .error

	mov ax, [.width]
	mov bx, [.height]
	mov cl, [.bpp]

	cmp ax, [vbe_mode_info.width]
	jne .next

	cmp bx, [vbe_mode_info.height]
	jne .next

	cmp cl, [vbe_mode_info.bpp]
	jne .next

	; set mode
	mov ax, 0x4F02
	mov bx, [.mode]
	or bx, 0x4000	; linear framebuffer
	push es
	int 0x10
	pop es

	cmp ax, 0x4F
	jne .error

	mov ax, [.mode]
	clc
	ret

.next:
	mov si, [.offset]
	jmp .loop

.error:
	xor ax, ax
	mov fs, ax
	stc
	ret

.width				dw 0
.height				dw 0
.bpp				db 0
.mode				dw 0
.offset				dw 0