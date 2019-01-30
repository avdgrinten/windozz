
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 16

DISK_BUFFER                equ 0x8000
DISK_BUFFER_SEGMENT            equ 0x800

align 8
echfs_directory_size            dq 0        ; in blocks
echfs_allocation_size            dd 0        ; in blocks
echfs_directory                dd 0        ; starting block
echfs_block_size            dw 0        ; in 512-byte sectors
echfs_alloc_per_block            dw 0        ; allocation table entries per one block
echfs_entries_per_block            db 0        ; directory entries per one block of directory

align 4
dap:
    .size            dw 16
    .sectors        dw 0
    .offset            dw 0
    .segment        dw 0
    .lba            dq 0

; Read sectors into high memory
; in: eax = lba
; in: ecx = count
; in: edi = 32-bit absolute address
; out: flags.cf = 0 on success

hdd_read_sector:
    mov [.lba], eax
    mov [.count], ecx
    mov [.address], edi

    xor ax, ax
    mov dl, [boot_info.bootdisk]
    int 0x13
    jc .error

.loop:
    mov eax, [.lba]
    mov dword[dap.lba], eax

    mov ecx, [.count]
    cmp ecx, 16
    jle .final_read

.shrink_size:
    mov ecx, 16

.final_read:
    pusha

.dot_loop:
    push cx
    mov ah, 0x0E
    mov al, '.'
    int 0x10
    pop cx
    loop .dot_loop

    popa

    mov [dap.sectors], cx
    mov [.current_count], ecx

    sub [.count], ecx
    add [.lba], ecx

    mov word[dap.offset], 0
    mov word[dap.segment], DISK_BUFFER_SEGMENT

    mov ah, 0x42
    mov dl, [boot_info.bootdisk]
    mov si, dap
    int 0x13
    jc .error

    call go32

bits 32

    mov esi, DISK_BUFFER
    mov edi, [.address]
    mov ecx, [.current_count]
    shl ecx, 9        ; mul 512
    add [.address], ecx

    cld
    rep movsb

    call go16

bits 16

    cmp dword[.count], 0
    jne .loop

    ; done
    clc
    ret

.error:
    stc
    ret

.lba                dd 0
.count                dd 0
.address            dd 0
.current_count            dd 0

; Load one block
; in: eax = block number
; in: edi = absolute address to load
; out: eflags.cf = 0 on success

echfs_read_block:
    mov [.block], eax
    mov [.address], edi

    mov eax, [.block]
    movzx ebx, word[echfs_block_size]
    mul ebx
    add eax, [partition.lba]

    mov edi, [.address]
    movzx ecx, word[echfs_block_size]
    call hdd_read_sector
    jc .error

    ret

.error:
    stc
    ret

align 4
.block                dd 0
.address            dd 0

; Detect the next block in the EchFS chain
; in: eax = block number
; out: eflags.cf = 0 on success
; out: eax = next block number

echfs_next_block:
    mov [.block], eax

    xor edx, edx
    movzx ebx, word[echfs_alloc_per_block]
    div ebx

    add eax, 16
    mov [.alloc], eax
    shl edx, 3
    mov [.entry], edx

    mov eax, [.alloc]
    mov edi, DISK_BUFFER
    call echfs_read_block
    jc .error

    mov ebp, [.entry]
    mov eax, [ds:DISK_BUFFER+bp]

    ret

.error:
    stc
    ret

align 4
.block                dd 0
.alloc                dd 0
.return                dd 0
.entry                dd 0

; Load file from echfs volume into high memory
; in: ds:si = filename
; in: edi = absolute address to load at
; out: only returns on success

echfs_load:
    mov [.filename], si
    mov [.address], edi

    mov si, .loading_msg
    call print
    mov si, [.filename]
    call print
    mov si, .loading_msg2
    call print

    mov cx, 0

.count_filename_size:
    lodsb
    cmp al, 0
    je .filename_size_done

    inc cx
    jmp .count_filename_size

.filename_size_done:
    inc cx
    mov [.filename_size], cx

    ; read the boot sector
    mov eax, [partition.lba]
    mov ecx, 1
    mov edi, DISK_BUFFER
    call hdd_read_sector
    mov si, .disk_msg
    jc error

    mov si, DISK_BUFFER
    add si, 4
    mov di, .echfs_signature
    mov cx, 8
    rep cmpsb
    mov si, .bad_fs
    jne error

    mov si, DISK_BUFFER
    mov eax, [si+20]        ; directory size
    mov dword[echfs_directory_size], eax
    mov eax, [si+24]
    mov dword[echfs_directory_size+4], eax
    mov eax, [si+28]
    shr eax, 9            ; div 512
    mov word[echfs_block_size], ax

    mov eax, [si+12]        ; total block count, low qword
    mov edx, [si+16]        ; high dword
    shl edx, 3
    mov ecx, eax
    shr ecx, 28
    or edx, ecx
    shl eax, 3
    mov ebx, [si+28]        ; block size
    div ebx
    cmp edx, 0            ; remainder
    je .read_directory

    inc eax

.read_directory:
    mov dword[echfs_allocation_size], eax

    add eax, 16
    mov dword[echfs_directory], eax

    mov ax, word[echfs_block_size]
    shl ax, 1            ; mul 2 (mul 512, div 256)
    mov [echfs_entries_per_block], al

.scan_loop:
    ; read the directory to find the file
    mov eax, dword[echfs_directory]
    mov edi, DISK_BUFFER
    call echfs_read_block
    mov si, .disk_msg
    jc error

    movzx ecx, byte[echfs_entries_per_block]
    mov si, DISK_BUFFER

.scan_block_loop:
    xor eax, eax
    not eax            ; eax = 0xFFFFFFFF
    cmp dword[si], eax
    jne .next_entry

    cmp dword[si+4], eax
    jne .next_entry

    push si
    push cx
    add si, 9
    mov di, [.filename]
    mov cx, [.filename_size]
    cld
    rep cmpsb
    je .found

    pop cx
    pop si

.next_entry:
    add si, 256
    loop .scan_block_loop

.end_of_block:
    ; next block
    inc dword[echfs_directory]
    inc dword[.block_count]
    mov eax, [.block_count]
    cmp eax, dword[echfs_directory_size]
    mov si, .not_found_msg
    jge error

    jmp .scan_loop

.found:
    ; okay, we've found the entry
    pop cx
    pop si        ; si = echfs directory entry

    ; make sure it's a file, not a directory
    cmp byte[si+8], 0x00
    push si
    mov si, .not_found_msg
    jne error

    pop si

    ; okay, safe to load
    mov eax, [si+240]        ; starting block
    mov [.block], eax

    mov dword[.block_count], 0
    movzx eax, word[echfs_block_size]
    shl eax, 6            ; mul 512, div 8
    mov [echfs_alloc_per_block], ax

.read_file_loop:
    mov eax, [.block]
    mov edi, [.address]
    call echfs_read_block
    mov si, .disk_msg
    jc error

    mov eax, [.block]
    call echfs_next_block
    mov si, .disk_msg
    jc error

    mov [.block], eax
    movzx eax, word[echfs_block_size]
    shl eax, 9            ; mul 512
    add [.address], eax

    xor eax, eax
    not eax
    cmp [.block], eax
    jne .read_file_loop

    ; done!
    mov si, newline
    call print

    ret

align 4
.block                dd 0
.block_count            dd 0
.filename            dw 0
.address            dd 0
.filename_size            dw 0
.loading_msg            db "loading ", 0
.loading_msg2            db "...", 0
.disk_msg            db "disk I/O error.", 0
.not_found_msg            db "kernel binary is missing.", 0
.echfs_signature        db "_ECH_FS_"
.bad_fs                db "echfs volume is corrupt.", 0






