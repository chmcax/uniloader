/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#include "stdbool.h"
#include "string.h"

void *memset (void *m, int c, size_t n)
{
	char *s = (char *) m;

	unsigned int i;
	unsigned long buffer;
	unsigned long *aligned_addr;
	unsigned int d = c & 0xff;

	while (UNALIGNED (s)) {
		if (n--)
			*s++ = (char) c;
		else
			return m;
	}

	if (!TOO_SMALL (n)) {
		/* If we get this far, we know that n is large and s is word-aligned. */
		aligned_addr = (unsigned long *) s;

		/*
		 * Store D into each char sized location in BUFFER so that
		 * we can set large blocks quickly.
		 */
		buffer = (d << 8) | d;
		buffer |= (buffer << 16);
		for (i = 32; i < LBLOCKSIZE * 8; i <<= 1)
			buffer = (buffer << i) | buffer;

		/* Unroll the loop. */
		while (n >= LBLOCKSIZE*4) {
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			*aligned_addr++ = buffer;
			n -= 4*LBLOCKSIZE;
		}

		while (n >= LBLOCKSIZE)	{
			*aligned_addr++ = buffer;
			n -= LBLOCKSIZE;
		}

		/* Pick up the remainder with a bytewise loop.	*/
		s = (char*)aligned_addr;
	}

	return m;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (n--) {
		if (*p1 != *p2) {
			return *p1 - *p2;
		}

		++p1;
		++p2;
	}

	return 0;
}

void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *p = (const unsigned char *)s;

	while (n--) {
		if (*p == (unsigned char)c) {
			return (void *)p;
		}

		++p;
	}

	return NULL;
}

char *strcpy(char *s1, const char *s2)
{
	char *rc = s1;

	while ((*s1++ = *s2++)) {
		/* EMPTY */
	}

	return rc;
}


char *strncpy(char *d, char *s, long n)
{
	int len = strlen(s);
	if (len > n)
		len = n;
	memcpy(d, s, len);
	memset(d + len, 0, n - len);
	return d;
}

char *strcat(char *d, char *s)
{
	strcpy(d + strlen(d), s);
	return d;
}

int strcmp(const char *s1, const char *s2)
{
	while ((*s1) && (*s1 == *s2)) {
		++s1;
		++s2;
	}

	return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n && *s1 && (*s1 == *s2)) {
		++s1;
		++s2;
		--n;
	}

	if (n == 0) {
		return 0;
	} else {
		return (*(unsigned char *)s1 - *(unsigned char *)s2);
	}
}

size_t strlen(const char *s)
{
	size_t rc = 0;

	while (s[rc]) {
		++rc;
	}

	return rc;
}

size_t strnlen(const char *s, size_t n)
{
	size_t rc = 0;

	while (rc < n && s[rc]) {
		++rc;
	}

	return rc;
}

char *strchr(const char *s, int c)
{
	do {
		if (*s == (char)c) {
			return (char *)s;
		}
	} while (*s++);

	return NULL;
}

char *strrchr(const char *s, int c)
{
	size_t i = 0;

	while (s[i++]) {
		/* EMPTY */
	}

	do {
		if (s[--i] == (char)c) {
			return (char *)s + i;
		}
	} while (i);

	return NULL;
}

/* Very naive, no attempt to check for errors */
long atol(const char *s)
{
	long val = 0;
	bool neg = false;

	if (*s == '-') {
		neg = true;
		s++;
	}

	while (*s >= '0' && *s <= '9')
		val = (val * 10) + (*s++ - '0');

	if (neg)
		val = -val;

	return val;
}

unsigned long power(unsigned char base, unsigned char exp) { // we need some limits
	double exp_tmp, result;

	result = 1;

	exp_tmp = exp;
	while (exp_tmp != 0) {
		result *= base;
		exp_tmp -= 1;
	}

	return result;
}

unsigned long strtoul(const char *str, char **endptr, int base)
{
	unsigned long result;
	int base_guessed, off, end;
	bool neg;

	if (str[0] == '+') {
		off = 1;
	} else if (str[0] == '-') {
		off = 1;
		neg = true;
	} else {
		off = 0;
	}

	// Guessing base
	// TODO: Add support for detecting more bases
	if (base == 0) {
		if (str[off] == '0' && (str[off + 1] == 'x' || str[off + 1] == 'X')) { // assuming hexadecimal
			base_guessed = 16;
			off += 2;
		} else if (str[off] == '0' && (str[off + 1] == 'b' || str[off + 1] == 'B')) { // assuming binary
			base_guessed = 2;
			off += 2;
		} else if (str[off] == '0') { // assuming octal
			base_guessed = 8;
			off += 1;
		} else { // assuming decimal, fix later
			base_guessed = 10;
		}
	} else {
		base_guessed = base;
	}

	// Basic error checking
	for (end = off; end < strlen(str); end++) {
		if (base_guessed > 10) {
			if (str[end] < '0' || ((str[end] > '9') && (str[end] < 'A')) ||
			    ((str[end] > (64 + base_guessed - 10)) && (str[end] < 'a')) || str[end] > (96 + base_guessed - 10))
				break;
		} else {
			if (str[end] < '0' || str[end] > 47 + base_guessed)
				break;
		}
	}


	end -= off;

	// actual thing now
	for (int i = 0; i < end; i++) {
		if (base_guessed > 10) {
			if (str[i + off] >= '0' && str[i + off] <= '9') {
				result += (str[i + off] - 48) * power(base_guessed, (end - i - 1));
			} else if (str[i + off] >= 'A' && str[i + off] <= (64 + base_guessed - 10)) {
				result += ((str[i + off] - 65) + 10) * power(base_guessed, (end - i - 1));
			} else if (str[i + off] >= 'a' && str[i + off] <= (96 + base_guessed - 10)) {
				result += ((str[i + off] - 97) + 10) * power(base_guessed, (end - i - 1));
			} else {
				error("wtf shouldnt get here\n");
			}
		} else {
			if (str[i + off] >= '0' && str[i + off] <= 47 + base_guessed) {
				result += (str[i + off] - 48) * power(base_guessed, (end - i - 1));
			} else {
				error("wtf shouldn't get here\n");
			}
		}
	}

	if (endptr != NULL)
		*endptr = str + end + off;

	if (neg)
		result = -result;

	return result;
}

void writel(unsigned int value, void* address)
{
	volatile unsigned int* ptr = (volatile unsigned int*)address;
	*ptr = value;
}

uint32_t readl(volatile uint32_t *addr)
{
	return *addr;
}
