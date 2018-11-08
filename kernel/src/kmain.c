/*
* kmain.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE "kmain"

#include <bootmgr.h>
#include <string.h>
#include <debug.h>
#include <screen.h>
#include <mm.h>
#include <cpu.h>
#include <cat.h>

boot_info_t boot_info;

void kmain(boot_info_t *boot_info_tmp)
{
	memcpy(&boot_info, boot_info_tmp, sizeof(boot_info_t));
	debug_init();
	screen_init();
	mm_init();
	cat_init((cat_rsdp_t *)boot_info.acpi_rsdp);

	while(1);
}