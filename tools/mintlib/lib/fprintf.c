#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

/* revised 4/15/92 sb -- moved __eprintf to a separate file to avoid conflict
   with libm.a's printf */

#if __STDC__
int fprintf(FILE * fp, const char *fmt, ...)
#else
int fprintf(fp, fmt)
FILE *fp;
const char *fmt;
#endif
{
	int r;
	va_list args;

	va_start(args, fmt);
	r = _doprnt(fputc, fp, fmt, args);
	va_end(args);
	return r;
}
