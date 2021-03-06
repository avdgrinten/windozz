
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#include <string.h>
#include <stdint.h>
#include <debug.h>
#include <acpi.h>
#include <mm.h>

#define MODULE              "acpi"

acpi_instance_t acpi_instance;

static acpi_status_t show_tables();
static acpi_status_t init_fadt(fadt_t *);

acpi_status_t acpi_init(rsdp_t *rsdp)
{
    memset(&acpi_instance, 0, sizeof(acpi_instance_t));

    char str1[7];
    char str2[5];

    memmove(str1, rsdp->oem, 6);
    str1[6] = 0;

    DEBUG("'RSD PTR ' v%02d @ 0x%016lX '%s'\n", rsdp->revision, GET_PHYS((uintptr_t)rsdp), str1);
    acpi_instance.rsdp = rsdp;
    acpi_instance.rsdt = (rsdt_t *)MAP_MEMORY(rsdp->rsdt, sizeof(acpi_sdth_t));
    if(!acpi_instance.rsdt)
    {
        ERROR("unable to map memory, ACPI initialization failed.\n");
        return ACPI_MEMORY;
    }

    memmove(str1, acpi_instance.rsdt->header.oem_id, 6);
    str1[6] = 0;
    memmove(str2, acpi_instance.rsdt->header.creator_id, 4);
    str2[4] = 0;

    DEBUG("'RSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", acpi_instance.rsdt->header.revision, GET_PHYS((uintptr_t)acpi_instance.rsdt), acpi_instance.rsdt->header.length, str1, str2, acpi_instance.rsdt->header.creator_revision);
    acpi_instance.rsdt = (rsdt_t *)MAP_MEMORY(rsdp->rsdt, acpi_instance.rsdt->header.length);
    if(!acpi_instance.rsdt)
    {
        ERROR("unable to map memory, ACPI initialization failed.\n");
        return ACPI_MEMORY;
    }

    if(rsdp->revision != 0)
    {
        acpi_instance.xsdt = (xsdt_t *)MAP_MEMORY(rsdp->xsdt, sizeof(acpi_sdth_t));
        if(!acpi_instance.xsdt)
        {
            ERROR("unable to map memory, ACPI initialization failed.\n");
            return ACPI_MEMORY;
        }

        memmove(str1, acpi_instance.xsdt->header.oem_id, 6);
        str1[6] = 0;
        memmove(str2, acpi_instance.xsdt->header.creator_id, 4);
        str2[4] = 0;

        DEBUG("'XSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", acpi_instance.xsdt->header.revision, GET_PHYS((uintptr_t)acpi_instance.xsdt), acpi_instance.xsdt->header.length, str1, str2, acpi_instance.xsdt->header.creator_revision);
        acpi_instance.xsdt = (xsdt_t *)MAP_MEMORY(rsdp->xsdt, acpi_instance.xsdt->header.length);
        if(!acpi_instance.xsdt)
        {
            ERROR("unable to map memory, ACPI initialization failed.\n");
            return ACPI_MEMORY;
        }
    }

    return show_tables();
}

static acpi_status_t show_tables()
{
    acpi_status_t status;
    size_t count, i;
    char name[5];
    char oem[7];
    char creator[5];
    acpi_sdth_t *header;

    memset(name, 0, 5);
    memset(oem, 0, 7);
    memset(creator, 0, 5);

    if(acpi_instance.xsdt)
    {
        count = (acpi_instance.xsdt->header.length - sizeof(acpi_sdth_t)) / sizeof(uint64_t);
        for(i = 0; i < count; i++)
        {
            header = (acpi_sdth_t *)MAP_MEMORY(acpi_instance.xsdt->tables[i], sizeof(acpi_sdth_t));
            if(!header)
            {
                ERROR("unable to map memory, ACPI initialization failed.\n");
                return ACPI_MEMORY;
            }

            memmove(name, header->signature, 4);
            memmove(oem, header->oem_id, 6);
            memmove(creator, header->creator_id, 4);

            DEBUG("'%s' v%02d @ 0x%016lX %06d (%s %s %08X) \n", name, header->revision, GET_PHYS((uintptr_t)header), header->length, oem, creator, header->creator_revision);
            if(!memcmp(header->signature, "FACP", 4))
            {
                status = init_fadt((fadt_t *)header);
                if(status != ACPI_SUCCESS)
                    return status;
            }
        }
    } else
    {
        count = (acpi_instance.rsdt->header.length - sizeof(acpi_sdth_t)) / sizeof(uint32_t);
        for(i = 0; i < count; i++)
        {
            header = (acpi_sdth_t *)MAP_MEMORY(acpi_instance.rsdt->tables[i], sizeof(acpi_sdth_t));
            if(!header)
            {
                ERROR("unable to map memory, ACPI initialization failed.\n");
                return ACPI_MEMORY;
            }

            memmove(name, header->signature, 4);
            memmove(oem, header->oem_id, 6);
            memmove(creator, header->creator_id, 4);

            DEBUG("'%s' v%02d @ 0x%016lX %06d (%s %s %08X) \n", name, header->revision, GET_PHYS((uintptr_t)header), header->length, oem, creator, header->creator_revision);
            if(!memcmp(header->signature, "FACP", 4))
            {
                status = init_fadt((fadt_t *)header);
                if(status != ACPI_SUCCESS)
                    return status;
            }
        }
    }

    return ACPI_SUCCESS;
}

static acpi_status_t init_fadt(fadt_t *fadt)
{
    acpi_instance.fadt = (fadt_t *)MAP_MEMORY(GET_PHYS((uintptr_t)fadt), fadt->header.length);
    if(!acpi_instance.fadt)
    {
        ERROR("unable to map memory, ACPI initialization failed.\n");
        return ACPI_MEMORY;
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

    acpi_instance.dsdt = (dsdt_t *)MAP_MEMORY(dsdt, sizeof(acpi_sdth_t));
    if(!acpi_instance.dsdt)
    {
        ERROR("unable to map memory, ACPI initialization failed.\n");
        return ACPI_MEMORY;
    }

    acpi_instance.dsdt = (dsdt_t *)MAP_MEMORY(dsdt, acpi_instance.dsdt->header.length);
    if(!acpi_instance.dsdt)
    {
        ERROR("unable to map memory, ACPI initialization failed.\n");
        return ACPI_MEMORY;
    }

    char oem[7];
    char creator[5];
    memset(oem, 0, 7);
    memset(creator, 0, 5);

    memmove(oem, acpi_instance.dsdt->header.oem_id, 6);
    memmove(creator, acpi_instance.dsdt->header.creator_id, 4);

    DEBUG("'DSDT' v%02d @ 0x%016lX %06d (%s %s %08X) \n", acpi_instance.dsdt->header.revision, dsdt, acpi_instance.dsdt->header.length, oem, creator, acpi_instance.dsdt->header.creator_revision);
    return ACPI_SUCCESS;
}
