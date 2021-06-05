/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

int vscanf(fmt, arg)
const char *fmt;
va_list arg;
{
	return (_scanf(stdin, fgetc, fungetc, fmt, arg));
}
