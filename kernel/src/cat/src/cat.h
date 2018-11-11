/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

#pragma once

#include "windozz/windozz.h"
#include "tables.h"

typedef unsigned int cat_status_t;

#define CAT_SUCCESS             0
#define CAT_INTEGRITY           1
#define CAT_MEMORY              2

/* OS-specific functions */
uintptr_t cat_map_memory(uintptr_t, size_t);
uintptr_t cat_get_phys(uintptr_t);
void *cat_malloc(size_t);
void *cat_realloc(void *, size_t);
void cat_free(void *);
uint64_t cat_io_read(uintptr_t, size_t);
void cat_io_write(uintptr_t, size_t, uint64_t);
uint64_t cat_mmio_read(uintptr_t, size_t);
void cat_mmio_write(uintptr_t, size_t, uint64_t);

/* cat internal string functions -- the OS does not provide these */
void *cat_memmove(void *, const void *, size_t);
void *cat_memcpy(void *, const void *, size_t);
void *cat_memset(void *, int, size_t);
int cat_memcmp(const void *, const void *, size_t);
size_t cat_strlen(const char *);
char *cat_strcpy(char *, const char *);

typedef struct cat_instance_t
{
    cat_rsdp_t *rsdp;
    cat_rsdt_t *rsdt;
    cat_xsdt_t *xsdt;
    cat_fadt_t *fadt;
    cat_dsdt_t *dsdt;
} cat_instance_t;

cat_status_t cat_init(cat_rsdp_t *);