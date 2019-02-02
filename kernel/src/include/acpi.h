
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
    rsdp_t *rsdp;
    rsdt_t *rsdt;
    xsdt_t *xsdt;
    fadt_t *fadt;
    dsdt_t *dsdt;
} acpi_instance_t;

acpi_instance_t acpi_instance;

acpi_status_t acpi_init(rsdp_t *);
acpi_status_t acpi_find_table(void **, const char *, size_t);
void acpi_install_irq();
