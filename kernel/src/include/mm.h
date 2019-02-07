
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */
 
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAP_MEMORY(x, y)       ((uintptr_t)x + PHYSICAL_MEMORY)
#define GET_PHYS(x)           ((uintptr_t)x - PHYSICAL_MEMORY)

#define PHYSICAL_MEMORY	   0xFFFF800000000000  /* pml[256] */
#define MMIO_REGION         0xFFFF808000000000  /* pml[257] */
#define KERNEL_HEAP         0xFFFF810000000000  /* pml[258] */
#define USER_HEAP           0x80000000

#define MAX_MMIO            32768

#define PAGE_SIZE		    	4096
#define PAGE_SIZE_SHIFT		12

/* Paging Flags */
#define PAGE_PRESENT	    	0x001
#define PAGE_WRITE			0x002
#define PAGE_USER		    	0x004
#define PAGE_UNCACHEABLE    0x010
#define PAGE_WRITE_COMBINE  0x08
#define PAGE_LARGE		 	0x080
#define PAGE_GLOBAL			0x100

#define PAGE_PAT            7
#define PAGE_PDC            4
#define PAGE_PWT            3

#define PAGE_MASK			(~(4095))

#define KERNEL_HEAP_FLAGS   (PAGE_PRESENT | PAGE_WRITE)
#define USER_HEAP_FLAGS     (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)

typedef struct mmio_t
{
    char device[64];
    uintptr_t physical, virtual;
    size_t pages;
} mmio_t;

mmio_t *mmio_regions;
size_t mmio_region_count;

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

uintptr_t *bsp_pml4;
uintptr_t vmm_get_page(uintptr_t);
void vmm_map_page(uintptr_t, uintptr_t, uintptr_t);
void vmm_free(uintptr_t, size_t);
uintptr_t vmm_alloc(uintptr_t, size_t, uintptr_t);
uintptr_t vmm_create_mmio(uintptr_t, size_t, char *);
void vmm_set_wc(uintptr_t, size_t);

/* standard functions */
void *kmalloc(size_t);
void *kcalloc(size_t, size_t);
void *krealloc(void *, size_t);
void kfree(void *);
