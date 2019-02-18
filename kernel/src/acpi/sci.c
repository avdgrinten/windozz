
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#define MODULE              "acpi"

#include <lai/core.h>
#include <acpi.h>
#include <apic.h>
#include <debug.h>

extern void acpi_sci_stub();
uint8_t acpi_sci_line;

void acpi_install_irq()
{
    /* install IRQ handler */
    acpi_sci_line = irq_configure(acpi_instance.fadt->sci_irq, (uintptr_t)&acpi_sci_stub, LEVEL | ACTIVE_LOW);
    irq_unmask(acpi_sci_line);
    acpi_enable(1);
}

void acpi_sci()
{
    acpi_read_event();
    /*DEBUG("ACPI SCI occurred, event data 0x%04X: %s %s %s\n", event,
        event & ACPI_POWER_BUTTON ? "power button" : "",
        event & ACPI_SLEEP_BUTTON ? "sleep button" : "",
        event & ACPI_WAKE ? "wake" : "");*/
}
