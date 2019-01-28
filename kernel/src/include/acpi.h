
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <acpi_tables.h>

typedef unsigned int acpi_status_t;

#define ACPI_SUCCESS             0
#define ACPI_INTEGRITY           1
#define ACPI_MEMORY              2
#define ACPI_NO_TABLE            3

typedef struct acpi_instance_t
{
    acpi_rsdp_t *rsdp;
    acpi_rsdt_t *rsdt;
    acpi_xsdt_t *xsdt;
    acpi_fadt_t *fadt;
    acpi_dsdt_t *dsdt;
} acpi_instance_t;

acpi_instance_t acpi_instance;

acpi_status_t acpi_init(acpi_rsdp_t *);
acpi_status_t acpi_find_table(void **, const char *, size_t);
