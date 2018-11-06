# Windozz Boot Protocol
The Windozz kernel uses a custom boot loader to start, and is not multiboot compliant. Rather, it uses a custom boot protocol to deliver firmware information to the kernel.

### Kernel format
The kernel is a flat binary, named `winkern` in the root directory of the boot medium, and being a flat binary the entry point is the first byte of the binary. The kernel is expected to run in virtual address `0xFFFF800000100000` or at 1 MiB physical.

### Memory state
The kernel expects 8 GiB of physical memory, regardless of its presence or absence, identity mapped starting at zero, and the exact same mapping starting at `0xFFFF800000000000`, i.e. the kernel assumes `PML4[0] == PML4[256]`, to mirror both the higher and lower halves of the address space.

### Machine state
When the kernel starts, it expects the machine to be in a high resolution graphics mode, preferebly the monitor's optimal mode as specified by the EDID. It also expects the machine to be in long mode, with interrupts disabled, IDTR untouched, all segment registers set to ring 0 base 0 segments, FS/GS base registers untouched, caching and page write-protection enabled in CR0 (i.e. bits 30 and 29 cleared and bit 16 set,) and finally it expects SSE/SSE2 and floating point exceptions to be enabled. If PAE/NX is supported, the boot loader shall not enabled it. RIP is set to the kernel entry point, at higher-half address `0xFFFF800000100000` and RBP is set to the higher-half address of the boot information structure, which is defined below. The kernel shall set up a stack as soon as possible, as the boot loader is not required to provide a stack.

### Boot Information Structure
This information is passed to the kernel in the RBP register.  

| Offset | Description           | Size     |
|--------|-----------------------|----------|
| 0 | Signature. This field contains the characters 'WNDZ' in ASCII, without a null terminator. It serves as a magic number. | 4 bytes  |
| 4 | Version. This field encodes the version of the boot protocol in BCD, where the higher word is the major version, and the lower word is the minor version. As of the time of writing this document, this field must contain 1.0 encoded is `0x00010000`. | DWORD |
| 8 | UEFI/BIOS toggle. This field indicates whether the system is BIOS or UEFI, where zero indicates BIOS and any other value, but preferably one, indicates UEFI. The BIOS-specific fields of this structure are only valid if this value evaluates to false, and likewise the UEFI-specific fields of this structure are only valid if this value evalutes to true. | QWORD |
| 16 | Physical memory map. This field contains the higher-half pointer to the system memory map, which is an array of E820 map structures as defined in the ACPI specification. Each structure is extended to be 32 bytes, and its actual size is stored in a WORD before it, to allow the kernel to determine whether or not ACPI 3.0 flags are present. | QWORD |
| 24 | Physical memory map size in bytes. | QWORD |
| 32 | Physical memory map size in entries. Basically the size in bytes divided by 32. | QWORD |
| 40 | ACPI root pointer. This field contains the higher-half pointer to the ACPI RSDP table. Scanning for the table is done in the boot loader to hide differences between UEFI and BIOS. If ACPI is not supported, the boot loader shall display and error message and abort booting. | QWORD |
| 48 | SMBIOS root pointer. This field contains the higher-half pointer to the SMBIOS tables. If unsupported, this field shall be zeroed by the boot loader. | QWORD |
| 56 | **BIOS-specific:** HDD/optical medium toggle. This field toggles whether the boot medium was an optical disc or a hard disk, where zero indicates HDD and any other value, preferebly one, indicates optical disc. | QWORD |
| 64 | **BIOS-specific:** INT 13h drive number. This field indicates the drive number assigned to the boot device by the BIOS to be used with INT 13h. Only the lowest byte is significant, and the remaining of this field shall be zeroed, and only exists for alignment. | QWORD |
| 72 | **BIOS-specific:** MBR partition. This field contains the higher-half pointer to the MBR partition structure of the partition the OS is booting from. This field is only valid if the OS is booting from a hard disk, and shall be zeroed if it boots from optical medium. | QWORD |
| 80 | **BIOS-specific:** BIOS INT 13h EDD information. This field contains the higher-half pointer to the structure returned by BIOS function INT 13h AH = 0x48 for the boot device. The boot loader shall request the full EDD 3.0 structure, but shall accept whatever output the BIOS provides, providing the function does not fail. If it fails, the boot loader shall display an error message and abort. | QWORD |
| 88 | **BIOS-specific:** VBE BIOS information. This field contains the higher-half pointer to the VBE BIOS information structure, i.e. the structure returned by INT 10h AX = 0x4F00. | QWORD |
| 96 | **BIOS-specific:** VBE mode information. This field contains the higher-half pointer to the VBE mode information structure of the video mode the system is currently in, i.e. the structure returned by INT 10h AX = 0x4F01. | QWORD |
| 104 | **UEFI-specific:** These fields are reserved for future expansion, when a UEFI loader has been implemented. | 16 QWORDs |
