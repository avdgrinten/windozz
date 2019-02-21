
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#include <stdint.h>
#include <io.h>
#include <debug.h>
#include <version.h>
#include <stdarg.h>
#include <string.h>
#include <timer.h>
#include <mutex.h>
#include <stddef.h>
#include <screen.h>
#include <bootmgr.h>
#include <stdbool.h>

bool e9_precense = false;
mutex_t e9_mutex = MUTEX_FREE;

char tmp_debug_buffer[32768];
char *debug_buffer = tmp_debug_buffer;
size_t debug_buffer_size = 0;
int display_debug = 0;

static char *pad(char *destination, const char *source, size_t new_size, char val)
{
    if(strlen(source) >= new_size)
    {
        strcpy(destination, source);
        return destination;
    }

    memset(destination, val, new_size);
    destination[new_size] = 0;

    if(source[0] != '-')
        strcpy(destination + new_size - strlen(source), source);
    else
    {
        strcpy(destination + new_size - strlen(source) + 1, source);
        destination[new_size - strlen(source) + 1] = val;
        destination[0] = '-';
    }

    return destination;
}

size_t copy_number(char *destination, const char *source)
{
    size_t i;
    for(i = 0; source[i] >= '0' && source[i] <= '9'; i++)
    {
        destination[i] = source[i];
    }

    destination[i] = 0;

    return i;
}

void debug_init()
{
    if(inb(0xE9) == 0xE9)
        e9_precense = true;

    debug_printf(LEVEL_DEBUG, NULL, VERSION);
}

void debug_putc(char val)
{
    if(e9_precense)
    {
        if(val == 10)
        {
            outb(0xE9, 13);
        }

        outb(0xE9, val);
    }

    if(display_debug)
    {
        putc(get_bootfb(), val);
    }

    debug_buffer[debug_buffer_size] = val;
    /*debug_buffer_size++;*/
}

int debug_puts(const char *string)
{
    int i;
    for(i = 0; string[i]; i++)
    {
        debug_putc(string[i]);
    }

    return i;
}

int debug_printf(int level, const char *module, const char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
	int status = debug_vprintf(level, module, fmt, args);
	va_end(args);
	return status;
}

int debug_vprintf(int level, const char *module, const char *fmt, va_list args)
{
    acquire(&e9_mutex);

    int size = 0;

    char integer_buffer[96];
    char padded_buffer[96];

    switch(level)
    {
    case LEVEL_ERROR:
        debug_puts("\e[0m\e[1m\e[31m");
        break;
    case LEVEL_WARN:
        debug_puts("\e[0m\e[1m\e[33m");
        break;
    case LEVEL_DEBUG:
    default:
        debug_puts("\e[0m\e[1m\e[34m");
        break;
    }

    debug_puts("[");
    ltoa((long)timer_ticks / TIMER_FREQUENCY, integer_buffer, DECIMAL);
    pad(padded_buffer, integer_buffer, 6, ' ');
    debug_puts(padded_buffer);

    debug_puts(".");
    ltoa((long)timer_ticks % TIMER_FREQUENCY, integer_buffer, DECIMAL);
    pad(padded_buffer, integer_buffer, 3, '0');
    debug_puts(padded_buffer);
    debug_puts("]\e[0m ");

    if(module)
    {
        debug_puts("\e[35m");
        debug_puts(module);
        debug_puts(":\e[0m ");
    }

    size_t i = 0;

    int integer;
    long long integer64;
    uint32_t hex;
    uint64_t hex64;

    char pad_character;
    int pad_spaces;
    char number[16];
    char *string;

    while(fmt[i])
    {
        if(fmt[i] == '%' && fmt[i+1] != '%')
        {
            pad_character = ' ';
            pad_spaces = 0;

            i++;

            if(fmt[i] == '0')
            {
                pad_character = '0';
                i++;
            }

            if(fmt[i] >= '0' && fmt[i] <= '9')
            {
                i += copy_number(number, &fmt[i]);
                pad_spaces = atoi(number);
            }

            switch(fmt[i])
            {
            case 'c':
                integer = va_arg(args, int);
                debug_putc((char)integer);
                size++;

                i++;
                break;

            case 's':
                string = va_arg(args, char *);
                size += debug_puts(string);

                i++;
                break;

            case 'd':
                integer = va_arg(args, int);
                itoa(integer, integer_buffer, 10);
                pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                size += debug_puts(padded_buffer);

                i++;
                break;

            case 'x':
                hex = va_arg(args, uint32_t);
                itoa(hex, integer_buffer, 16);

                pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                lowercase(padded_buffer);
                size += debug_puts(padded_buffer);

                i++;
                break;

            case 'X':
                hex = va_arg(args, uint32_t);
                itoa(hex, integer_buffer, 16);

                pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                uppercase(padded_buffer);
                size += debug_puts(padded_buffer);

                i++;
                break;

            case 'l':
                i++;

                switch(fmt[i])
                {
                case 'd':
                    integer64 = va_arg(args, long long);
                    ltoa(integer64, integer_buffer, DECIMAL);
                    pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                    size += debug_puts(padded_buffer);

                    i++;
                    break;

                case 'x':
                    hex64 = va_arg(args, uint64_t);
                    ltoa(hex64, integer_buffer, HEX);

                    pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                    lowercase(padded_buffer);
                    size += debug_puts(padded_buffer);

                    i++;
                    break;

                case 'X':
                    hex64 = va_arg(args, uint64_t);
                    ltoa(hex64, integer_buffer, HEX);

                    pad(padded_buffer, integer_buffer, pad_spaces, pad_character);
                    uppercase(padded_buffer);
                    size += debug_puts(padded_buffer);

                    i++;
                    break;
                }

                break;
            }

            continue;
        }

        debug_putc(fmt[i]);
        size++;
        i++;
    }

    release(&e9_mutex);
    return size;
}
