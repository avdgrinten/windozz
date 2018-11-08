/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

#pragma once

#include "windozz/windozz.h"

typedef unsigned int cat_status_t;

#define CAT_SUCCESS             0
#define CAT_INTEGRITY           1
#define CAT_MEMORY              2

typedef struct cat_rsdp_t
{
    /* ACPI 1.0 */
    char signature[8];
    uint8_t checksum;
    char oem[6];
    uint8_t revision;
    uint32_t rsdt;

    uint32_t length;
    uint64_t xsdt;
    uint8_t extended_checksum;
    uint8_t reserved[3];
}__attribute__((packed)) cat_rsdp_t;

cat_status_t cat_init(cat_rsdp_t *);