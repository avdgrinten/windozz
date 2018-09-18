
; Windozz
; Copyright (C) 2018 by Omar Muhamed

bits 16

detect_memory:
	; get the E820 memory map
	mov di, e820_map+2
	mov edx, 0x534D4150
	xor ebx, ebx

.loop:
	mov eax, 0xE820
	mov ecx, 24
	int 0x15
	jc .check_error

	cmp eax, 0x534D4150
	jne .error

	cmp ebx, 0
	je .check_error

	and cx, 0xFF
	mov [di-2], cx

	add word[boot_info.e820_map_size], 32
	inc word[boot_info.e820_map_entries]

	add di, 32
	jmp .loop

.check_error:
	cmp word[boot_info.e820_map_entries], 0
	je .error

	; done
	ret

.error:
	mov si, .msg
	jmp error

.msg				db "BIOS memory detection failure.", 0

check_a20:		; checks A20 status
			; CF = enabled
	mov di, 0x500
	xor eax, eax
	stosd

	mov ax, 0xFFFF
	mov es, ax
	mov di, 0x510
	mov eax, "WNDZ"
	stosd

	xor ax, ax
	mov es, ax
	mov si, 0x500
	lodsd

	cmp eax, "WNDZ"
	je .no

	; yes
	stc
	ret

.no:
	clc
	ret

a20_wait:
	mov ecx, 0xFF

.loop:
	nop
	nop
	loop .loop

	ret

do_a20:
	; check if we even need to do anything
	call check_a20
	jc .done

	; try BIOS
	mov ax, 0x2401
	int 0x15

	call a20_wait

	call check_a20
	jc .done

	; try fast A20 method
	in al, 0x92
	or al, 2
	and al, 0xFE
	out 0x92, al

	call a20_wait

	call check_a20
	jc .done

	call a20_wait		; this function may take a while --
				; -- so wait one more time

	call check_a20
	jc .done

	mov si, .msg
	jmp error

.done:
	ret

.msg				db "A20 gate is not responding.", 0

; Mode switching shit
align 16
gdt:
	; 0x00 - null descriptor
	dq 0

	; 0x08 - 32-bit code
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10011010b		; access byte
	db 11001111b		; flags and limit high
	db 0x00			; base high

	; 0x10 - 32-bit data
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10010010b		; access byte
	db 11001111b		; flags and limit high
	db 0x00			; base high

	; 0x18 - 16-bit code
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10011010b		; access byte
	db 10001111b		; flags and limit high
	db 0x00			; base high

	; 0x20 - 16-bit data
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10010010b		; access byte
	db 10001111b		; flags and limit high
	db 0x00			; base high

	; 0x28 - 64-bit code
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10011010b		; access byte
	db 10101111b		; flags and limit high
	db 0x00			; base high

	; 0x30 - 64-bit data
	dw 0xFFFF		; limit low
	dw 0x0000		; base low
	db 0x00			; base middle
	db 10010010b		; access byte
	db 10101111b		; flags and limit high
	db 0x00			; base high

end_of_gdt:

align 16
gdtr:
	dw end_of_gdt - gdt - 1
	dq gdt

;; to enter 32-bit mode

go32:
	cli
	pop bp

	lgdt [gdtr]

	mov eax, cr0
	or al, 1
	mov cr0, eax
	jmp 0x08:.next

bits 32

.next:
	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	movzx esp, sp
	add esp, 0x10000

	and ebp, 0xFFFF
	jmp ebp

;; and back to 16-bit mode

go16:
	pop ebp

	jmp 0x18:.next

bits 16

.next:
	mov ax, 0x20
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, cr0
	and al, 0xFE
	mov cr0, eax

	jmp 0x0000:.next2

.next2:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ax, 0x1000
	mov ss, ax
	and esp, 0xFFFF

	sti
	nop

	jmp bp



