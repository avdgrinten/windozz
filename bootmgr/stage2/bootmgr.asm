
; Windozz
; Copyright (C) 2018 by Omar Muhamed

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

	mov [bootdisk], dl

	sti
	nop

	mov si, banner
	call print

	; booting from CD or HDD?
	cmp byte[bootdisk], 0xE0
	jge .cd

.hdd:
	mov byte[boot_info.bios_optical], 0
	jmp .next

.cd:
	mov byte[boot_info.bios_optical], 1

.next:
	call detect_memory
	call do_a20
	call setup_paging

	jmp $

	%include			"stage2/io.asm"
	%include			"stage2/system.asm"

	; Data area
	banner				db "Windozz Boot Manager", 0
	newline				db 10, 0

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

	bootdisk			db 0

	; Data passed to kernel
	align 8
	boot_info:
		.signature		db "WNDZ"	; magic
		.version		dd 0x00010000	; high = major, low = minor
		.uefi			dq 0		; 0 = BIOS, 1 = UEFI
							; someday I'll need a UEFI loader
		.e820_map		dq e820_map
		.e820_map_size		dq 0		; in bytes
		.e820_map_entries	dq 0		; in entries

		.acpi_rsdp		dq 0		; ACPI RSDP table, search for it in bootloader
							; because differences between BIOS and UEFI
		.smbios			dq 0

		; the following fields are only valid if boot_info.uefi == 0 (i.e. BIOS systems)
		.bios_optical		dq 0		; boot medium, 0 = HDD, 1 = optical disc
		.mbr_partition:		dq partition	; pointer to MBR partition info, only valid for HDD ofc
		.bios_edd_info		dq 0		; BIOS EDD information

		.vbe_mode_info		dq 0		; pointer to VBE Mode Info
		.vbe_bios_info		dq 0		; pointer to VBE BIOS Info

		; TO-DO: Define UEFI fields here.
		.reserved_for_uefi:	times 16 dq 0

	align 8
	e820_map:
		; empty reserved space



