
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

	mov byte[boot_info.bootdisk], dl

	sti
	nop

	mov ah, 0x0F
	int 0x10
	cmp al, 3
	je .mode_good

	mov ax, 3
	int 0x10

	; show signs of life
	;mov si, banner
	;call print
	;mov si, newline
	;call print

.mode_good:
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

	cli
	hlt

	%include			"stage2/io.asm"
	%include			"stage2/system.asm"
	%include			"stage2/echfs.asm"
	%include			"stage2/vbe.asm"

	banner				db "Windozz Boot Manager", 0
	newline				db 10, 0
	no_64				db "CPU is not 64-bit capable.",0
	cd_msg				db "optical disc boot not implemented yet.",0
	bad_fs_msg			db "unsupported HDD filesystem.",0
	kernel_filename			db "winkern", 0

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

	align 16
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
		.bootdisk		dq 0		; only low byte is significant
		.mbr_partition:		dq partition	; pointer to MBR partition info, only valid for HDD ofc
		.bios_edd_info		dq 0		; BIOS EDD information

		.vbe_bios_info		dq 0		; pointer to VBE BIOS Info
		.vbe_mode_info		dq 0		; pointer to VBE Mode Info

		; TO-DO: Define UEFI fields here.
		.reserved_for_uefi:	times 16 dq 0

	align 16
	e820_map:


