
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

/* ACPI Root Pointer */
typedef struct rsdp_t
{
    /* ACPI 1.0 */
    char signature[8];
    uint8_t checksum;
    char oem[6];
    uint8_t revision;
    uint32_t rsdt;

    /* ACPI 2.0+ */
    uint32_t length;
    uint64_t xsdt;
    uint8_t extended_checksum;
    uint8_t reserved[3];
}__attribute__((packed)) rsdp_t;

/* Header that precedes all ACPI tables */
typedef struct acpi_sdth_t
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    char creator_id[4];
    uint32_t creator_revision;
}__attribute__((packed)) acpi_sdth_t;

/* RSDT -- 32-bit root table */
typedef struct rsdt_t
{
    acpi_sdth_t header;
    uint32_t tables[];
}__attribute__((packed)) rsdt_t;

/* XSDT -- 64-bit root table */
typedef struct xsdt_t
{
    acpi_sdth_t header;
    uint64_t tables[];
}__attribute__((packed)) xsdt_t;

/* Generic Address Structure used by the FADT, also used by HPET but thats irrelevant */
typedef struct acpi_address_t
{
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
}__attribute__((packed)) acpi_address_t;

typedef struct fadt_t
{
    acpi_sdth_t header;
    uint32_t firmware_control;
    uint32_t dsdt;      /* legacy pointer, use 64-bit pointer when possible */

    uint8_t reserved1;

    /* basic ACPI register locations */
    uint8_t pm_profile;
    uint16_t sci_irq;
    uint32_t smi_command;
    uint8_t enable;
    uint8_t disable;
    uint8_t s4bios_request;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cpu_state_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;

    /* non-standard CMOS register offsets */
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    /* ACPI 2.0+ starts here */
    uint16_t boot_flags;
    uint8_t reserved2;
    uint32_t flags;

    /* ACPI reset command */
    acpi_address_t reset_register;
    uint8_t reset_command;
    uint8_t reserved3[3];

    /* 64-bit pointers */
    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    /* if ACPI 2.0+, use the following fields instead of the ones above */
    acpi_address_t x_pm1a_event_block;
    acpi_address_t x_pm1b_event_block;
    acpi_address_t x_pm1a_control_block;
    acpi_address_t x_pm1b_control_block;
    acpi_address_t x_pm2_control_block;
    acpi_address_t x_pm_timer_block;
    acpi_address_t x_gpe0_block;
    acpi_address_t x_gpe1_block;
}__attribute__((packed)) fadt_t;

typedef struct dsdt_t
{
    acpi_sdth_t header;
    uint8_t aml[];
}__attribute__((packed)) dsdt_t;

typedef struct ssdt_t
{
    acpi_sdth_t header;
    uint8_t aml[];
}__attribute__((packed)) ssdt_t;
