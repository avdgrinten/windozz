; bootmgr.asm
; Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
; Use of this source code is governed by a license that can be
; found in the LICENSE.md file, in the root directory of
; the source package.

bits 16
org 0x1000

	; Magic number for stage 1
	magic				db "WNDZ"	; WiNDoZz

; execution starts here at offset 4

start:
	cli
	cld

	xor ax, ax
	mov es, ax
	mov di, partition
	mov cx, 16
	rep movsb

	mov ds, ax
	mov fs, ax
	mov gs, ax
	mov sp, 0x8000

	mov ax, 0x1000
	mov ss, ax

	mov byte[boot_info.bootdisk], dl

	sti
	nop

	mov ah, 0x0F
	int 0x10
	cmp al, 3
	je .mode_good

	mov ax, 3
	int 0x10

.mode_good:
	; show signs of life
	mov si, banner
	call print
	mov si, newline
	call print

	; booting from CD or HDD?
	cmp byte[boot_info.bootdisk], 0xE0
	jge .cd

.hdd:
	mov byte[boot_info.bios_optical], 0
	jmp .next

.cd:
	mov byte[boot_info.bios_optical], 1

.next:
	; 64-bit CPU?
	mov eax, 0x80000000
	cpuid

	cmp eax, 0x80000001
	mov si, no_64
	jl error

	mov eax, 0x80000001
	cpuid

	and edx, 0x20000000
	mov si, no_64
	jz error

	; detect EDD info
	mov ah, 0x48
	mov dl, byte[boot_info.bootdisk]
	mov si, edd_info
	int 0x13
	mov si, edd_error_msg
	jc error

	call detect_memory
	call do_a20

	cmp byte[boot_info.bios_optical], 0
	je .load_hdd

	; CD here
	mov si, cd_msg
	jmp error

.load_hdd:
	cmp byte[partition.type], 0xF3
	mov si, bad_fs_msg
	jne error

	mov si, kernel_filename
	mov edi, 0x100000
	call echfs_load

.scan_acpi:
	; scan for ACPI RSDP
	call go32

bits 32

	mov esi, 0xE0000

.rsdp_loop:
	push esi
	mov edi, rsdp_signature
	mov ecx, 8
	cld
	rep cmpsb
	je .found_rsdp

	pop esi
	add esi, 16
	cmp esi, 0xFFFF0
	jge .no_rsdp

	jmp .rsdp_loop

.found_rsdp:
	pop esi
	mov dword[boot_info.acpi_rsdp], esi

	call go16

bits 16

	; setup paging structs and VBE mode
	call setup_paging
	call do_vbe

	; tell the BIOS we're entering long mode, don't even know what this does tbh
	mov eax, 0xEC00
	mov ebx, 2
	int 0x15

	; mask the PICs allowing BIOS to handle any pending IRQs
	mov al, 0xFF
	out 0x21, al
	out 0xA1, al

	mov ecx, 0xFFFF

.pic_wait:
	sti
	nop
	nop
	nop
	nop
	loop .pic_wait

	cli
	lgdt [gdtr]

	; enable PAE, PSE and SSE
	mov eax, 0x630
	mov cr4, eax

	mov eax, cr0
	and eax, 0xFFFFFFFB
	or eax, 2
	mov cr0, eax

	mov eax, PML4
	mov cr3, eax

	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x100
	wrmsr

	; enable long mode, caching, paging, write security
	mov eax, cr0
	or eax, 0x80010001
	and eax, 0x9FFFFFFF
	mov cr0, eax

	jmp 0x28:lmode

bits 32

.no_rsdp:
	call go16

bits 16

	mov si, rsdp_msg
	jmp error

bits 64

lmode:
	mov rax, 0x30
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	movzx rsp, sp
	add rsp, 0x10000

	mov rbp, boot_info + 0xFFFF800000000000
	mov rax, 0xFFFF800000100000
	jmp rax

	%include			"stage2/io.asm"
	%include			"stage2/system.asm"
	%include			"stage2/echfs.asm"
	%include			"stage2/vbe.asm"
	%include			"stage2/paging.asm"

	banner				db "Windozz Boot Manager", 0
	newline				db 10, 0
	no_64				db "CPU is not 64-bit capable.",0
	cd_msg				db "optical disc boot not implemented yet.",0
	bad_fs_msg			db "unsupported filesystem.",0
	edd_error_msg			db "BIOS EDD geometry function failed.",0
	rsdp_msg			db "ACPI root table not found.", 0
	kernel_filename			db "winkern", 0
	rsdp_signature			db "RSD PTR "

	align 8
	partition:
		.active			db 0
		.start_chs		db 0
					db 0
					db 0
		.type			db 0
		.end_chs		db 0
					db 0
					db 0
		.lba			dd 0
		.size			dd 0

	align 8
	edd_info:
		; all versions
		.size			dw 0x42		; requested size
		.flags			dw 0
		.cylinders		dd 0
		.heads			dd 0
		.sectors		dd 0
		.total_sectors		dq 0
		.bytes_per_sector	dw 0

		; EDD 2.0+
		.edd_config_parameters	dd 0

		; EDD 3.0
		.signature		dw 0
		.device_path_length	db 0
		.reserved:		times 3 db 0
		.host_bus:		times 4 db 0
		.interface_type:	times 8 db 0
		.interface_path:	times 8 db 0
		.reserved2		db 0
		.checksum		db 0

	align 16
	boot_info:
		.signature		db "WNDZ"	; magic
		.version		dd 0x00010000	; high = major, low = minor
		.uefi			dq 0		; 0 = BIOS, 1 = UEFI
							; someday I'll need a UEFI loader

		.e820_map		dq e820_map + 0xFFFF800000000000
		.e820_map_size		dq 0		; in bytes
		.e820_map_entries	dq 0		; in entries

		; ACPI RSDP table, search for it in bootloader
		; because differences between BIOS and UEFI
		.acpi_rsdp		dq 0xFFFF800000000000

		; same for SMBIOS
		.smbios			dq 0xFFFF800000000000

		; the following fields are only valid if boot_info.uefi == 0 (i.e. BIOS systems)
		.bios_optical		dq 0		; boot medium, 0 = HDD, 1 = optical disc
		.bootdisk		dq 0		; only low byte is significant
		.mbr_partition:		dq partition + 0xFFFF800000000000
		.bios_edd_info		dq edd_info + 0xFFFF800000000000

		; VBE BIOS/mode info structs
		.vbe_bios_info		dq vbe_bios_info + 0xFFFF800000000000
		.vbe_mode_info		dq vbe_mode_info + 0xFFFF800000000000

		; TO-DO: Define UEFI fields here.
		.reserved_for_uefi:	times 16 dq 0

	align 16
	e820_map: