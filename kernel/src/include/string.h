
/*
 * The Windozz Project
 * Copyright (C) 2018-2019 by the Windozz authors.
 */

#pragma once

#include <stdint.h>

/* for ltoa() */
#define OCTAL       8
#define DECIMAL     10
#define HEX         16

size_t strlen(const char *);
void *memmove(void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
char *strcpy(char *, const char *);
int strcmp(const char *, const char *);
char *itoa(int, char *, int);
char *ltoa(long, char *, int);
int atoi(const char *);
void *memset(void *, int, size_t);
char *lowercase(char *);
char *uppercase(char *);
