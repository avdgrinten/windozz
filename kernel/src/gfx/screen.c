/*
* screen.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE			"screen"

#include <debug.h>
#include <bootmgr.h>
#include <screen.h>
#include <mutex.h>
#include <io.h>
#include <mm.h>
#include <string.h>

/* someday allow up to 8 monitors when there is a real graphics driver */
screen_t screens[8];

/* colors for ANSI escape sequences */
#define rgb(r, g, b) ((r << 16) + (g << 8) + b)
uint32_t ansi_colors[] = 
{
	rgb(0,0,0),		/* 0- black */
	rgb(222,56,43),		/* 1- red */
	rgb(57,181,74),		/* 2- green */
	rgb(255,199,6),		/* 3- yellow */
	rgb(0,111,184),		/* 4- blue */
	rgb(118,38,113),	/* 5- magenta */
	rgb(44,181,233),	/* 6- cyan */
	rgb(204,204,204),	/* 7- white */
};

void screen_init()
{
	screens[0].mutex = MUTEX_FREE;
	screens[0].bg = ansi_colors[0];
	screens[0].fg = ansi_colors[7];
	screens[0].x = 0;
	screens[0].y = 0;

	if(!boot_info.uefi)
	{
		vbe_bios_info_t *bios_info = (vbe_bios_info_t *)boot_info.vbe_bios_info;
		vbe_mode_info_t *mode_info = (vbe_mode_info_t *)boot_info.vbe_mode_info;

		screens[0].using_back_buffer = false;
		screens[0].locked = false;

		screens[0].width = mode_info->width;
		screens[0].height = mode_info->height;
		screens[0].pitch = mode_info->pitch;
		screens[0].x_max = (screens[0].width / 9) - 1;
		screens[0].y_max = (screens[0].height / 16) - 1;
		screens[0].framebuffer = (uintptr_t)mode_info->framebuffer + PHYSICAL_MEMORY;

		screens[0].adapter_vendor = (char *)((uintptr_t)bios_info->vendor_string_segment << 4) + bios_info->vendor_string_offset;
		screens[0].adapter_product = (char *)((uintptr_t)bios_info->product_string_segment << 4) + bios_info->product_string_offset;

		display_debug = 1;

		DEBUG("using VESA framebuffer for output\n");
		DEBUG("resolution is %dx%dx32bpp, %d bytes per line\n", screens[0].width, screens[0].height, screens[0].pitch);
		DEBUG("framebuffer is at 0x%08lX\n", mode_info->framebuffer);
		DEBUG("card name: %s, total %d MB of VRAM\n", screens[0].adapter_product, (bios_info->vram_size * 65536) / 1024 / 1024);
	} else
	{
		ERROR("UEFI not implemented yet.\n");
		while(1);
	}
}

screen_t *get_bootfb()
{
	return &screens[0];
}

static inline void *pixel_offset(screen_t *screen, uint16_t x, uint16_t y)
{
	uintptr_t ptr = (y * screen->pitch) + (x << 2);
	if(screen->using_back_buffer)
		ptr += screen->back_buffer;
	else
		ptr += screen->framebuffer;

	return (void *)ptr;
}

static void scroll(screen_t *screen)
{
	void *top = pixel_offset(screen, 0, 0);
	void *first_line = top + (screen->pitch << 4);
	size_t size = (screen->height - 16) * screen->pitch;

	memcpy(top, first_line, size);

	uint32_t *bottom = pixel_offset(screen, 0, screen->y_max * 16);
	int i, j;
	for(j = 0; j < 16; j++)
	{
		for(i = 0; i < screen->width; i++)
		{
			bottom[i] = screen->bg;
		}

		bottom = (uint32_t *)((uintptr_t)bottom + screen->pitch);
	}

	screen->x = 0;
	screen->y = screen->y_max;
}

static void parse_escape_sequence(screen_t *screen)
{
	char seq[16];
	char numstr[16];

	memcpy(seq, screen->escape_seq_buffer, 16);
	memset(numstr, 0, 16);

	int number;

	if(seq[1] == '[')
	{
		copy_number(numstr, seq+2);
		number = atoi(numstr);

		if(number == 0)		/* reset */
		{
			screen->bg = ansi_colors[0];
			screen->fg = ansi_colors[7];
		} else if(number >= 30 && number <= 37)
		{
			screen->fg = ansi_colors[number - 30];
		} else if(number >= 40 && number <= 47)
		{
			screen->bg = ansi_colors[number - 40];
		}
	}

}

void putc(screen_t *screen, char value)
{
	uint32_t *pixels = pixel_offset(screen, screen->x * 9, screen->y * 16);

	uint8_t font_data;

	if(value == 13)
	{
		screen->x = 0;
		return;
	}

	if(value == 10)
	{
		screen->x = 0;
		screen->y++;

		if(screen->y > screen->y_max)
		{
			scroll(screen);
		}

		return;
	}

	if(value == '\e')
	{
		memset(screen->escape_seq_buffer, 0, 16);
		screen->is_escape_seq = true;
		screen->escape_seq_size = 1;
		screen->escape_seq_buffer[0] = '\e';
		return;
	}

	if(screen->is_escape_seq)
	{
		screen->escape_seq_buffer[screen->escape_seq_size] = value;
		screen->escape_seq_size++;

		if(value == 'm')
		{
			screen->is_escape_seq = false;
			parse_escape_sequence(screen);
		}

		return;
	}

	int i, j;
	for(j = 0; j < 16; j++)
	{
		font_data = font[(value * 16) + j];

		for(i = 0; i < 8; i++)
		{
			if(font_data & 0x80)
				pixels[i] = screen->fg;
			else
				pixels[i] = screen->bg;

			font_data <<= 1;
		}

		pixels = (uint32_t *)((uintptr_t)pixels + screen->pitch);
	}

	screen->x++;
	if(screen->x > screen->x_max)
	{
		screen->x = 0;
		screen->y++;

		if(screen->y > screen->y_max)
		{
			scroll(screen);
		}
	}
}

void puts(screen_t *screen, const char *str)
{
	size_t i;
	for(i = 0; str[i]; i++)
	{
		putc(screen, str[i]);
	}
}