#include "utils.h"
#include <stdarg.h>

// splits a string line in two
// according to the splitter character
char *
split_line(char *buf, char splitter)
{
	int i = 0;

	while (buf[i] != splitter && buf[i] != END_STRING)
		i++;

	buf[i++] = END_STRING;

	while (buf[i] == SPACE)
		i++;

	return &buf[i];
}

// looks in a block for the 'c' character
// and returns the index in which it is, or -1
// in other case
int
block_contains(char *buf, char c)
{
	for (size_t i = 0; i < strlen(buf); i++)
		if (buf[i] == c)
			return i;

	return -1;
}

// Printf wrappers for debug purposes so that they don't
// show when shell is compiled in non-interactive way
int
printf_debug(char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vprintf(format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

int
fprintf_debug(FILE *file, char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vfprintf(file, format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

// async-signal-safe implementation of integer to string conversion
// title:        itoa_safe
// author:       Ciro Santilli
// date:         Sep 03, 2018
// availability:
// https://github.com/cirosantilli/cpp-cheat/blob/6b4896846dfdca69e8d3f38fd3ede14c517438ef/posix/signal_write_safe.c
//               https://stackoverflow.com/questions/14573000/print-int-from-signal-handler-using-write-or-async-safe-functions/52111436#52111436
// license:      https://creativecommons.org/licenses/by-sa/4.0/
char *
itoa_safe(intmax_t value, char *result, int base)
{
	intmax_t tmp_value;
	char *ptr, *ptr2, tmp_char;
	if (base < 2 || base > 36) {
		return NULL;
	}

	ptr = result;
	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGH"
		         "IJKLMNOPQRSTUVWXYZ"[35 + (tmp_value - value * base)];
	} while (value);
	if (tmp_value < 0)
		*ptr++ = '-';
	ptr2 = result;
	result = ptr;
	*ptr-- = '\0';
	while (ptr2 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr2;
		*ptr2++ = tmp_char;
	}
	return result;
}
