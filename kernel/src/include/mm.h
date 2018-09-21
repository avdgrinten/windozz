
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



