
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

#define LEVEL_DEBUG			1
#define LEVEL_WARN			2
#define LEVEL_ERROR			3

#define DEBUG(...)			debug_printf(LEVEL_DEBUG, MODULE, __VA_ARGS__)
#define WARN(...)			debug_printf(LEVEL_WARN, MODULE, __VA_ARGS__)
#define ERROR(...)			debug_printf(LEVEL_ERROR, MODULE, __VA_ARGS__)

char *debug_buffer;
int display_debug;

void debug_init();
int debug_printf(int, const char *, const char *, ...);
size_t copy_number(char *, const char *);