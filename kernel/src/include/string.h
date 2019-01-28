/*
* string.h
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#pragma once

#include <stdint.h>

/* for ltoa() */
#define OCTAL		8
#define DECIMAL     10
#define HEX			16

size_t strlen(const char *);
void *memmove(void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
char *strcpy(char *, const char *);
char *itoa(int, char *, int);
char *ltoa(long, char *, int);
int atoi(const char *);
void *memset(void *, int, size_t);
char *lowercase(char *);
char *uppercase(char *);