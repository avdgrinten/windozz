/*
 * cat - cool ACPI thing
 * Copyright (C) 2018 by Omar Muhamed.
 */

#include "cat.h"

void *cat_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *destc = (uint8_t *)dest;
    uint8_t *srcc = (uint8_t *)src;
    size_t i;
    for(i = 0; i < n; i++)
    {
        destc[i] = srcc[i];
    }

    return dest;
}

void *cat_memcpy(void *dest, const void *src, size_t n)
{
    return cat_memmove(dest, src, n);
}

void *cat_memset(void *dest, int val, size_t n)
{
    uint8_t *destc = (uint8_t *)dest;
    size_t i;
    for(i = 0; i < n; i++)
    {
        destc[i] = (uint8_t)val;
    }

    return dest;
}

int cat_memcmp(const void *p1, const void *p2, size_t n)
{
    signed char *p1c = (signed char *)p1;
    signed char *p2c = (signed char *)p2;

    size_t i;
    for(i = 0; i < n; i++)
    {
        if(p1c[i] != p2c[i])
            return p1c[i] - p2c[i];
    }

    return 0;
}

size_t cat_strlen(const char *str)
{
    size_t i = 0;
    while(str[i])
        i++;

    return i;
}

char *cat_strcpy(char *dest, const char *src)
{
    return (char *)cat_memmove(dest, src, cat_strlen(src) + 1);
}