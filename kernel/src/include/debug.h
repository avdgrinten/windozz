
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

#define LEVEL_DEBUG			1
#define LEVEL_WARN			2
#define LEVEL_ERROR			3

#define DEBUG(...)			debug_printf(LEVEL_DEBUG, MODULE, __VA_ARGS__)
#define WARN(...)			debug_printf(LEVEL_WARN, MODULE, __VA_ARGS__)
#define ERROR(...)			debug_printf(LEVEL_ERROR, MODULE, __VA_ARGS__)

char *debug_buffer;

void debug_init();
int debug_printf(int, const char *, const char *, ...);





