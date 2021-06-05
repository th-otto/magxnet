#include <stdio.h>
#include <stdarg.h>
#include "lib.h"


int vfprintf(fp, fmt, args)
FILE *fp;
const char *fmt;
va_list args;
{
	return (_doprnt(fputc, fp, fmt, args));
}
