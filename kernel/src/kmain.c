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
#include <string.h>

#define MODULE "kmain"

boot_info_t boot_info;

void kmain(boot_info_t *boot_info_tmp)
{
	memcpy(&boot_info, boot_info_tmp, sizeof(boot_info_t));
	debug_init();
	screen_init();
	mm_init();

	char string[] = "Hello, world!";
	char *ptr = kmalloc(strlen(string));

	if(!ptr)
	{
		ERROR("out of memory.\n");
	} else
	{
		DEBUG("allocated memory at 0x%016lX\n", ptr);
		strcpy(ptr, string);
		DEBUG("contents of memory: %s\n", ptr);
	}

	while(1);
}