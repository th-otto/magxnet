/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

int vfscanf(fp, fmt, arg)
FILE *fp;
const char *fmt;
va_list arg;
{
	return (_scanf(fp, fgetc, fungetc, fmt, arg));
}
