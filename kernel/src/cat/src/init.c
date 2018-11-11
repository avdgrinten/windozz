/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

#include "cat.h"

cat_instance_t cat_instance;

static cat_status_t show_tables();
static cat_status_t init_fadt(cat_fadt_t *);

cat_status_t cat_init(cat_rsdp_t *rsdp)
{
    cat_memset(&cat_instance, 0, sizeof(cat_instance_t));

    char str1[7];
    char str2[5];

    cat_memmove(str1, rsdp->oem, 6);
    str1[6] = 0;

    ACPI_DEBUG("'RSD PTR ' v%02d @ 0x%016lX '%s'\n", rsdp->revision, cat_get_phys((uintptr_t)rsdp), str1);
    cat_instance.rsdp = rsdp;
    cat_instance.rsdt = (cat_rsdt_t *)cat_map_memory(rsdp->rsdt, sizeof(cat_header_t));
    if(!cat_instance.rsdt)
    {
        ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
        return CAT_MEMORY;
    }

    cat_memmove(str1, cat_instance.rsdt->header.oem_id, 6);
    str1[6] = 0;
    cat_memmove(str2, cat_instance.rsdt->header.creator_id, 4);
    str2[4] = 0;

    ACPI_DEBUG("'RSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", cat_instance.rsdt->header.revision, cat_get_phys((uintptr_t)cat_instance.rsdt), cat_instance.rsdt->header.length, str1, str2, cat_instance.rsdt->header.creator_revision);
    cat_instance.rsdt = (cat_rsdt_t *)cat_map_memory(rsdp->rsdt, cat_instance.rsdt->header.length);
    if(!cat_instance.rsdt)
    {
        ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
        return CAT_MEMORY;
    }

    if(rsdp->revision != 0)
    {
        cat_instance.xsdt = (cat_xsdt_t *)cat_map_memory(rsdp->xsdt, sizeof(cat_header_t));
        if(!cat_instance.xsdt)
        {
            ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
            return CAT_MEMORY;
        }

        cat_memmove(str1, cat_instance.xsdt->header.oem_id, 6);
        str1[6] = 0;
        cat_memmove(str2, cat_instance.xsdt->header.creator_id, 4);
        str2[4] = 0;

        ACPI_DEBUG("'XSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", cat_instance.xsdt->header.revision, cat_get_phys((uintptr_t)cat_instance.xsdt), cat_instance.xsdt->header.length, str1, str2, cat_instance.xsdt->header.creator_revision);
        cat_instance.xsdt = (cat_xsdt_t *)cat_map_memory(rsdp->xsdt, cat_instance.xsdt->header.length);
        if(!cat_instance.xsdt)
        {
            ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
            return CAT_MEMORY;
        }
    }

    return show_tables();
}

static cat_status_t show_tables()
{
    cat_status_t status;
    size_t count, i;
    char name[5];
    char oem[7];
    char creator[5];
    cat_header_t *header;

    cat_memset(name, 0, 5);
    cat_memset(oem, 0, 7);
    cat_memset(creator, 0, 5);

    if(cat_instance.xsdt)
    {
        count = (cat_instance.xsdt->header.length - sizeof(cat_header_t)) / sizeof(uint64_t);
        for(i = 0; i < count; i++)
        {
            header = (cat_header_t *)cat_map_memory(cat_instance.xsdt->tables[i], sizeof(cat_header_t));
            if(!header)
            {
                ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
                return CAT_MEMORY;
            }

            cat_memmove(name, header->signature, 4);
            cat_memmove(oem, header->oem_id, 6);
            cat_memmove(creator, header->creator_id, 4);

            ACPI_DEBUG("'%s' v%02d @ 0x%016lX %06d (%s %s %08X) \n", name, header->revision, cat_get_phys((uintptr_t)header), header->length, oem, creator, header->creator_revision);
            if(!cat_memcmp(header->signature, "FACP", 4))
            {
                status = init_fadt((cat_fadt_t *)header);
                if(status != CAT_SUCCESS)
                    return status;
            }
        }
    } else
    {
        count = (cat_instance.rsdt->header.length - sizeof(cat_header_t)) / sizeof(uint32_t);
        for(i = 0; i < count; i++)
        {
            header = (cat_header_t *)cat_map_memory(cat_instance.rsdt->tables[i], sizeof(cat_header_t));
            if(!header)
            {
                ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
                return CAT_MEMORY;
            }

            cat_memmove(name, header->signature, 4);
            cat_memmove(oem, header->oem_id, 6);
            cat_memmove(creator, header->creator_id, 4);

            ACPI_DEBUG("'%s' v%02d @ 0x%016lX %06d (%s %s %08X) \n", name, header->revision, cat_get_phys((uintptr_t)header), header->length, oem, creator, header->creator_revision);
            if(!cat_memcmp(header->signature, "FACP", 4))
            {
                status = init_fadt((cat_fadt_t *)header);
                if(status != CAT_SUCCESS)
                    return status;
            }
        }
    }

    return CAT_SUCCESS;
}

static cat_status_t init_fadt(cat_fadt_t *fadt)
{
    cat_instance.fadt = (cat_fadt_t *)cat_map_memory(cat_get_phys((uintptr_t)fadt), fadt->header.length);
    if(!cat_instance.fadt)
    {
        ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
        return CAT_MEMORY;
    }

    uintptr_t dsdt;
    if(fadt->header.revision >= 2)      /* ACPI 2.0+ */
    {
        if(fadt->x_dsdt)
            dsdt = fadt->x_dsdt;
        else
            dsdt = fadt->dsdt;
    } else
    {
        dsdt = fadt->dsdt;
    }

    cat_instance.dsdt = (cat_dsdt_t *)cat_map_memory(dsdt, sizeof(cat_header_t));
    if(!cat_instance.dsdt)
    {
        ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
        return CAT_MEMORY;
    }

    cat_instance.dsdt = (cat_dsdt_t *)cat_map_memory(dsdt, cat_instance.dsdt->header.length);
    if(!cat_instance.dsdt)
    {
        ACPI_ERROR("unable to map memory, ACPI initialization failed.\n");
        return CAT_MEMORY;
    }

    char oem[7];
    char creator[5];
    cat_memset(oem, 0, 7);
    cat_memset(creator, 0, 5);

    cat_memmove(oem, cat_instance.dsdt->header.oem_id, 6);
    cat_memmove(creator, cat_instance.dsdt->header.creator_id, 4);

    ACPI_DEBUG("'DSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", cat_instance.dsdt->header.revision, dsdt, cat_instance.dsdt->header.length, oem, creator, cat_instance.dsdt->header.creator_revision);
    return CAT_SUCCESS;
}