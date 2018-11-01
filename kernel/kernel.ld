/*
* kernel.ld
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

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