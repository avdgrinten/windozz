/*
* tables.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#include <stdint.h>
#include <string.h>
#include <acpi.h>
#include <mm.h>
#include <debug.h>

#define MODULE              "acpi"

acpi_status_t acpi_find_table(void **table, const char *signature, size_t index)
{
    acpi_header_t *header;
    size_t i;
    size_t current_index = 0;
    size_t table_count;

    if(acpi_instance.xsdt)
    {
        /*DEBUG("using XSDT to scan for '%s'\n", signature);*/

        /* use the XSDT whenever possible */
        table_count = (acpi_instance.xsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint64_t);
        for(i = 0; i < table_count; i++)
        {
            header = (acpi_header_t *)MAP_MEMORY(acpi_instance.xsdt->tables[i], sizeof(acpi_header_t));
            if(!header) return ACPI_MEMORY;

            if(!memcmp(header->signature, signature, 4))
                current_index++;

            if(current_index > index)
            {
                header = (acpi_header_t *)MAP_MEMORY(acpi_instance.xsdt->tables[i], header->length);
                if(!header) return ACPI_MEMORY;

                *table = (void *)header;
                return ACPI_SUCCESS;
            }
        }
    } else
    {
        /* and of course fall back to RSDT even tho practically no real hw still uses this */
        /*DEBUG("using RSDT to scan for '%s'\n", signature);*/

        table_count = (acpi_instance.rsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint32_t);
        for(i = 0; i < table_count; i++)
        {
            header = (acpi_header_t *)MAP_MEMORY(acpi_instance.rsdt->tables[i], sizeof(acpi_header_t));
            if(!header) return ACPI_MEMORY;

            if(!memcmp(header->signature, signature, 4))
                current_index++;

            if(current_index > index)
            {
                header = (acpi_header_t *)MAP_MEMORY(acpi_instance.rsdt->tables[i], header->length);
                if(!header) return ACPI_MEMORY;

                *table = (void *)header;
                return ACPI_SUCCESS;
            }
        }
    }

    return ACPI_NO_TABLE;
}