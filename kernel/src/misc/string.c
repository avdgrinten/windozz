/*
* string.c
* Copyright (C) 2018 by the Windozz authors (AUTHORS.md). All rights reserved.
* Use of this source code is governed by a license that can be
* found in the LICENSE.md file, in the root directory of
* the source package.
*/

#include <string.h>

size_t strlen(const char *str)
{
	size_t i = 0;
	while(str[i])
	{
		i++;
	}

	return i;
}

void *memmove(void *destination, const void *source, size_t num)
{
	uint8_t *dest = (uint8_t *)destination;
	uint8_t *src = (uint8_t *)source;

	size_t i;
	for(i = 0; i < num; i++)
	{
		dest[i] = src[i];
	}

	return destination;
}

char *strcpy(char *destination, const char *source)
{
	return memmove(destination, source, strlen(source) + 1);
}

char *ltoa(long num, char *buffer, int radix)
{
	/* IBM says radix can only be the constants DECIMAL, OCTAL or HEX,
	 * but nobody gives shit, we'll define those constants and still give
	 * valid results if they are not used. */

	size_t i = 0;
	long divider = (long)radix;
	long tmp;

	int is_negative = 0;

	uint64_t number;

	if(num < 0 && radix == DECIMAL)
	{
		is_negative = 1;
		i = 1;
		buffer[0] = '-';
		num *= -1;
		number = (uint64_t)num;
	} else
	{
		number = (uint64_t)num;
	}

	do
	{
		tmp = number % divider;
		if(tmp <= 9)
		{
			buffer[i] = tmp + '0';
		} else
		{
			buffer[i] = tmp - 10 + 'A';
		}

		number /= divider;
		i++;
	} while(number);

	buffer[i] = 0;

	/* reverse the string */
	i = strlen(buffer) - 1;
	size_t i2 = (size_t)is_negative;
	char tmpchar;

	while(i2 < i)
	{
		tmpchar = buffer[i2];
		buffer[i2] = buffer[i];
		buffer[i] = tmpchar;

		i2++;
		i--;
	}

	return buffer;
}

inline char *itoa(int num, char *buffer, int base)
{
	return ltoa((long)num & 0xFFFFFFFF, buffer, base);
}

int atoi(const char *str)
{
	ssize_t i = strlen(str) - 1;
	int value = 0;
	int multiplier = 1;

	int is_negative = 0;
	if(str[0] == '-')
	{
		is_negative = 1;
	} else
	{
		is_negative = 0;
	}

	while(i >= is_negative)
	{
		value += (str[i] - '0') * multiplier;
		multiplier *= 10;
		i--;
	}

	if(is_negative)
	{
		value *= -1;
	}

	return value;
}

void *memset(void *ptr, int value, size_t num)
{
	size_t i;
	uint8_t *byte_ptr = ptr;
	for(i = 0; i < num; i++)
	{
		byte_ptr[i] = (uint8_t)value;
	}

	return ptr;
}

char *lowercase(char *str)
{
	size_t size = strlen(str);
	size_t i;
	for(i = 0; i < size; i++)
	{
		if(str[i] >= 'A' && str[i] <= 'Z')
		{
			str[i] += 0x20;
		}
	}

	return str;
}

char *uppercase(char *str)
{
	size_t size = strlen(str);
	size_t i;
	for(i = 0; i < size; i++)
	{
		if(str[i] >= 'a' && str[i] <= 'z')
		{
			str[i] -= 0x20;
		}
	}

	return str;
}