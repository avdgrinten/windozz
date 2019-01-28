
OUTPUT_ARCH("i386:x86-64")
OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
	. = 0xFFFF800000100000;
	.startup : {
		*(.startup)
		. = ALIGN(4096);
	}

	.text : {
		*(.text)
		. = ALIGN(4096);
	}

	.data : {
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

	.bss : {
		bss = .; _bss = .; __bss = .;
		*(.bss)
		bssend = .; _bssend = .; __bssend = .;
	}

	end = .;
}