/*
* mutex.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#pragma once

#include <stdint.h>

typedef uint64_t mutex_t;

#define MUTEX_BUSY			0x01
#define MUTEX_FREE			0x00

void acquire(mutex_t *);
void release(mutex_t *);