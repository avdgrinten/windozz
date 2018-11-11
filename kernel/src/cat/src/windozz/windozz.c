/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

/* cat interface for windozz */

#define MODULE          "acpi"

#include <io.h>
#include <mm.h>
#include <stdint.h>
#include <debug.h>

uintptr_t cat_map_memory(uintptr_t physical, size_t n)
{
    if((physical + n) < 0x200000000L)
        return physical + PHYSICAL_MEMORY;
    else
    {
        /* TO-DO */
        ERROR("to do.\n");
        while(1);
    }
}

uintptr_t cat_get_phys(uintptr_t virtual)
{
    return virtual - PHYSICAL_MEMORY;
}