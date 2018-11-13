/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

#include "cat.h"

cat_status_t cat_find_table(void **table, const char *signature, size_t index)
{
    cat_header_t *header;
    size_t i;
    size_t current_index = 0;
    size_t table_count;

    if(cat_instance.xsdt)
    {
        /*ACPI_DEBUG("using XSDT to scan for '%s'\n", signature);*/

        /* use the XSDT whenever possible */
        table_count = (cat_instance.xsdt->header.length - sizeof(cat_header_t)) / sizeof(uint64_t);
        for(i = 0; i < table_count; i++)
        {
            header = (cat_header_t *)cat_map_memory(cat_instance.xsdt->tables[i], sizeof(cat_header_t));
            if(!header) return CAT_MEMORY;

            if(!cat_memcmp(header->signature, signature, 4))
                current_index++;

            if(current_index > index)
            {
                header = (cat_header_t *)cat_map_memory(cat_instance.xsdt->tables[i], header->length);
                if(!header) return CAT_MEMORY;

                *table = (void *)header;
                return CAT_SUCCESS;
            }
        }
    } else
    {
        /* and of course fall back to RSDT even tho practically no real hw still uses this */
        /*ACPI_DEBUG("using RSDT to scan for '%s'\n", signature);*/

        table_count = (cat_instance.rsdt->header.length - sizeof(cat_header_t)) / sizeof(uint32_t);
        for(i = 0; i < table_count; i++)
        {
            header = (cat_header_t *)cat_map_memory(cat_instance.rsdt->tables[i], sizeof(cat_header_t));
            if(!header) return CAT_MEMORY;

            if(!cat_memcmp(header->signature, signature, 4))
                current_index++;

            if(current_index > index)
            {
                header = (cat_header_t *)cat_map_memory(cat_instance.rsdt->tables[i], header->length);
                if(!header) return CAT_MEMORY;

                *table = (void *)header;
                return CAT_SUCCESS;
            }
        }
    }

    return CAT_NO_TABLE;
}