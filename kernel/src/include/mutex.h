
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

typedef uint64_t mutex_t;

#define MUTEX_BUSY            0x01
#define MUTEX_FREE            0x00

void acquire(mutex_t *);
void release(mutex_t *);
