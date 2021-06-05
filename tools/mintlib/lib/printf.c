#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

#if __STDC__
int printf(const char *fmt, ...)
#else
int printf(fmt)
const char *fmt;
#endif
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = _doprnt(fputc, stdout, fmt, args);
	va_end(args);
	return r;
}
