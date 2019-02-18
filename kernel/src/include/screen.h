
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <mutex.h>

extern uint8_t font[4096];

typedef struct screen_t
{
    mutex_t mutex;
    bool using_back_buffer;
    bool locked;
    uint16_t width, height, pitch;
    size_t size;
    uint16_t x_max, y_max;        /* for debug log */
    uint16_t x, y;
    uint32_t bg, fg;
    uintptr_t framebuffer;
    uintptr_t back_buffer;
    char escape_seq_buffer[16];
    bool is_escape_seq;
    size_t escape_seq_size;

    char *adapter_vendor, *adapter_product;
} screen_t;

void screen_init();
void clear_screen(screen_t *, uint32_t);
void putc(screen_t *, char);
void puts(screen_t *, const char *);
screen_t *get_bootfb();
void screen_setup_buffer();

