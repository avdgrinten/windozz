
/*
 * Windozz
 * Copyright (C) 2018 by Omar Muhamed
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
	uint16_t x_max, y_max;		/* for debug log */
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
void putc(screen_t *, char);
void puts(screen_t *, const char *);
screen_t *get_bootfb();





