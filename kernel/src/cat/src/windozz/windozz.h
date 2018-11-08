/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

/* spai interface for windozz */

#pragma once

#include <debug.h>
#include <stdint.h>

#define ACPI_DEBUG(...)      debug_printf(LEVEL_DEBUG, "acpi", __VA_ARGS__)
#define ACPI_WARN(...)       debug_printf(LEVEL_WARN, "acpi", __VA_ARGS__)
#define ACPI_ERROR(...)      debug_printf(LEVEL_ERROR, "acpi", __VA_ARGS__)