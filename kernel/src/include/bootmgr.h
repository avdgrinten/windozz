
/*
 * Windozz
 * Copyright (C) 2018 by Omar Muhamed
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdint.h>

/* This boot information is passed from the boot loader to the kernel */

typedef struct boot_info_t
{
	/* Shared fields */
	char signature[4];
	uint32_t version;
	uint64_t uefi;
	uint64_t e820_map;
	uint64_t e820_map_size;
	uint64_t e820_map_entries;
	uint64_t acpi_rsdp;
	uint64_t smbios;

	/* BIOS-specific fields */
	uint64_t bios_optical;
	uint64_t bios_bootdisk;
	uint64_t mbr_partition;
	uint64_t bios_edd_info;
	uint64_t vbe_bios_info;
	uint64_t vbe_mode_info;

	/* UEFI-specific fields: to be implemented */
	uint64_t reserved_for_uefi[16];
}__attribute__((packed)) boot_info_t;

typedef struct vbe_bios_info_t
{
	char signature[4];
	uint16_t version;
	uint32_t oem;
	uint32_t capabilities;
	uint32_t video_modes;
	uint16_t vram_size;
	uint16_t software_rev;
	uint16_t vendor_string_offset;
	uint16_t vendor_string_segment;
	uint16_t product_string_offset;
	uint16_t product_string_segment;
	uint32_t product_rev;

	uint8_t reserved[222];
	uint8_t oem_data[256];
}__attribute__((packed)) vbe_bios_info_t;

typedef struct vbe_mode_info_t
{
	uint16_t attributes;
	uint8_t window_a;
	uint8_t window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;
	uint8_t bpp;
	uint8_t banks;
	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved0;

	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t reserved;
	uint8_t direct_color;

	uint32_t framebuffer;
	uint32_t invisible_vram;
	uint16_t invisible_vram_size;

	uint8_t reserved1[206];
}__attribute__((packed)) vbe_mode_info_t;

boot_info_t boot_info;




