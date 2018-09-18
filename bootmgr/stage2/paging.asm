
; Windozz
; Copyright (C) 2018 by Omar Muhamed

bits 16

PML4			equ 0x8000
PDPT			equ 0x9000
PD			equ 0xA000

; This function creates the paging tables in the addresses defined above

setup_paging:
	call go32

bits 32

	mov edi, PML4
	xor eax, eax
	mov ecx, 1024
	rep stosd

	mov edi, PDPT
	mov ecx, 1024
	rep stosd

	; build the PML4 - each entry maps 512 GB
	; we only need entry 0 and entry 256 (offset 2048)
	mov edi, PML4
	mov eax, PDPT
	or eax, 3		; read, write
	stosd

	mov edi, PML4
	add edi, 2048
	stosd

	; now build the PDPT - each entry maps 1 GB
	mov edi, PDPT
	mov ebx, PD
	mov ecx, 4		; 4 GB

.pdpt_loop:
	mov eax, ebx
	or eax, 3
	stosd
	xor eax, eax
	stosd

	add ebx, 4096
	loop .pdpt_loop

	; now build the page directory - each entry maps 2 MB
	mov edi, PD
	mov ebx, 0
	mov ecx, 4096/2

.pd_loop:
	mov eax, ebx
	or eax, 0x83
	stosd
	xor eax, eax
	stosd

	add ebx, 4096
	cmp ebx, 0
	je .inc_high

	loop .pd_loop
	jmp .done

.inc_high:
	inc edx
	loop .pd_loop

.done:
	call go16

bits 16

	ret


