#include <stdio.h>
#include <stdarg.h>
#include "lib.h"
int vprintf(fmt, args)
const char *fmt;
va_list args;
{
	return (_doprnt(fputc, stdout, fmt, args));
}
