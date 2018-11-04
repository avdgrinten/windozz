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

#define PHYSICAL_MEMORY		0xFFFF800000000000
#define KERNEL_HEAP         0x0000040000000000
#define USER_HEAP           0x80000000

#define PAGE_SIZE			4096
#define PAGE_SIZE_SHIFT		12

/* Paging Flags */
#define PAGE_PRESENT		0x001
#define PAGE_WRITE			0x002
#define PAGE_USER			0x004
#define PAGE_LARGE			0x080
#define PAGE_GLOBAL			0x100

#define PAGE_MASK			(~(4095))

#define KERNEL_HEAP_FLAGS   (PAGE_PRESENT | PAGE_WRITE)
#define USER_HEAP_FLAGS     (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)

uint8_t *pmm_bitmap;
size_t total_pages, used_pages;

void mm_init();
void pmm_init();
void vmm_init();

void pmm_mark_page_used(uintptr_t);
void pmm_mark_page_free(uintptr_t);
bool pmm_get_page(uintptr_t);
uintptr_t pmm_alloc_page();
void pmm_free_page(uintptr_t);

uintptr_t vmm_get_page(uintptr_t);
void vmm_map_page(uintptr_t, uintptr_t, uintptr_t);
void vmm_free(uintptr_t, size_t);
uintptr_t vmm_alloc(uintptr_t, size_t, uintptr_t);

/* standard functions */
void *kmalloc(size_t);
void *kcalloc(size_t, size_t);
void *krealloc(void *, size_t);
void kfree(void *);
