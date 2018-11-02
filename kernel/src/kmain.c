/*
* kmain.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#include <bootmgr.h>
#include <string.h>
#include <debug.h>
#include <screen.h>
#include <mm.h>

#define MODULE "kmain"

boot_info_t boot_info;

void kmain(boot_info_t *boot_info_tmp)
{
	memcpy(&boot_info, boot_info_tmp, sizeof(boot_info_t));
	debug_init();
	screen_init();
	mm_init();

	/* just for testing */
	vmm_map_page(0x1000, 0x2000, 0x03);
	vmm_map_page(0x8000, 0x4000, 0x03);

	while(1);
}