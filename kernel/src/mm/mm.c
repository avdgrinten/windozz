
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE            "mm"

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

    ptr[0] = pages;        /* store size in pages so we can free it */
    ptr[1] = n;            /* and size in bytes for realloc */

    release(&heap_mutex);
    return (void *)((uintptr_t)ptr + 16);
}

void *kcalloc(size_t n1, size_t n2)
{
    /* no need to do anything because kmalloc() already clears the memory */
    return kmalloc(n1 * n2);
}

void *krealloc(void *old, size_t new_size)
{
    void *new = kmalloc(new_size);
    if(!new)
        return NULL;

    size_t *s_old;
    s_old = (size_t *)((uintptr_t)old - 16);

    memcpy(new, old, s_old[1]);

    kfree(old);
    return new;
}

void kfree(void *ptr)
{
    size_t *size = (size_t *)((uintptr_t)ptr - 16);
    vmm_free((uintptr_t)ptr & PAGE_MASK, size[0]);
}
