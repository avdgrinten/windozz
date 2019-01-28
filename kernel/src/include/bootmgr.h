
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
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

typedef struct e820_t
{
	uint16_t size;
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t acpi3_flags;
}__attribute__((packed)) e820_t;

boot_info_t boot_info;