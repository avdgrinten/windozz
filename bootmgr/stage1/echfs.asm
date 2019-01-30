
; The Windozz Project
; Copyright (C) 2018-2019 by the Windozz authors.

bits 16
org 0

jmp short main

times 4 - ($-$$) nop

echfs_header:
    .magic            db "_ECH_FS_"
    .block_count        dq 0
    .rootdir_size        dq 0
    .bytes_per_block    dq 0

main:
    ; copy boot sector to 0x4000:0x0000 --
    ; -- leaving space for the second stage in low memory
    ; this prevents me from dealing with fucking conversion
    ; of segment:offset to linear for 64-bit mode
    cli
    cld
    push ds
    push si

    xor ax, ax
    mov ds, ax

    mov ax, 0x4000
    mov es, ax

    mov si, 0x7C00
    xor di, di
    mov cx, 512
    rep movsb

    jmp 0x4000:.next

.next:
    pop si
    pop ds
    mov di, boot_partition
    mov cx, 16
    rep movsb

    mov ds, ax
    mov ss, ax
    xor sp, sp

    mov [bootdisk], dl

    sti

    ; Windozz can only boot with block size >= 2048
    mov ax, word[echfs_header.bytes_per_block]
    cmp ax, 2048
    jl bad_block_size

    ; load stage 2 into 0x0000:0x1000
    shr ax, 9        ; div 512, bytes to sectors
    mov bx, 15        ; 15 reserved blocks
    mul bx
    mov [dap.sectors], ax

    mov eax, dword[echfs_header.bytes_per_block]
    shr eax, 9
    add eax, [boot_partition.lba]

    mov dword[dap.lba], eax

    xor ax, ax
    mov dl, [bootdisk]
    int 0x13
    jc disk_error

    mov ah, 0x42
    mov dl, [bootdisk]
    mov si, dap
    int 0x13
    jc disk_error

    xor ax, ax
    mov fs, ax

    cmp dword[fs:0x1000], "WNDZ"    ; magic number
    jne corrupt

    mov si, boot_partition
    mov dl, [bootdisk]
    jmp 0x0000:0x1004

disk_error:
    mov si, .msg
    jmp error

.msg                    db "Disk I/O error.", 0

corrupt:
    mov si, .msg
    jmp error

.msg                    db "Boot manager is corrupt.", 0

bad_block_size:
    mov si, .msg
    jmp error

.msg                    db "Block size must be >=2048.", 0

error:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    int 0x10
    jmp error

.done:
    sti
    hlt
    jmp .done

    ; data area

    align 4
    dap:
        .size            dw 16
        .sectors        dw 0
        .offset            dw 0x1000
        .segment        dw 0x0000
        .lba            dq 0

    bootdisk:            db 0
    boot_partition:
        .active            db 0
        .start_chs        db 0
                    db 0
                    db 0
        .type            db 0
        .end_chs        db 0
                    db 0
                    db 0
        .lba            dd 0
        .size            dd 0

    times 510 - ($-$$) db 0
    dw 0xAA55
