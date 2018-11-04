/*
* mm.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#define MODULE			"mm"

#include <mm.h>
#include <mutex.h>
#include <string.h>
#include <stddef.h>
#include <debug.h>

mutex_t heap_mutex = MUTEX_FREE;

void mm_init()
{
	pmm_init();
	vmm_init();
}

void *kmalloc(size_t n)
{
	acquire(&heap_mutex);

	size_t act_size = n + (2 * sizeof(size_t));
	size_t pages = (act_size + PAGE_SIZE - 1) >> PAGE_SIZE_SHIFT;

	/*DEBUG("attempt to allocate %d bytes, %d pages\n", n, pages);*/

	size_t *ptr = (size_t *)vmm_alloc(KERNEL_HEAP, pages, KERNEL_HEAP_FLAGS);

	if(!ptr)
	{
		release(&heap_mutex);
		return NULL;
	}

	memset(ptr, 0, act_size);

	ptr[0] = pages;		/* store size in pages so we can free it */

	release(&heap_mutex);
	return (void *)((uintptr_t)ptr + 16);
}

void *kcalloc(size_t n1, size_t n2)
{
	/* no need to do anything because kmalloc() already clears the memory */
	return kmalloc(n1 * n2);
}

void kfree(void *ptr)
{
	size_t *size = (size_t *)((uintptr_t)ptr - 16);
	vmm_free((uintptr_t)ptr & PAGE_MASK, size[0]);
}