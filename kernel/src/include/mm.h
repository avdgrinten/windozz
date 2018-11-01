/*
* mm.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PHYSICAL_MEMORY			0xFFFF800000000000

/* Paging Flags */
#define PAGE_PRESENT			0x001
#define PAGE_WRITE			0x002
#define PAGE_USER			0x004
#define PAGE_LARGE			0x080
#define PAGE_GLOBAL			0x100

#define PAGE_MASK			(~(4095))

uint8_t *pmm_bitmap;

void mm_init();
void pmm_init();
void vmm_init();

void pmm_mark_page_used(uintptr_t);
void pmm_mark_page_free(uintptr_t);
bool pmm_get_page(uintptr_t);

uintptr_t vmm_get_page(uintptr_t);