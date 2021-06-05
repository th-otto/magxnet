/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

#if __STDC__
int fscanf(FILE * fp, const char *fmt, ...)
{
	int retval;
	va_list arg;

	va_start(arg, fmt);
	retval = _scanf(fp, fgetc, fungetc, fmt, arg);
	va_end(arg);
	return retval;
}
#else
int fscanf(fp, fmt, arg)
FILE *fp;
const char *fmt;
char *arg;
{
	return (_scanf(fp, fgetc, fungetc, fmt, &arg));
}
#endif /* __STDC__ */

#if __STDC__
int scanf(const char *fmt, ...)
{
	int retval;
	va_list arg;

	va_start(arg, fmt);
	retval = _scanf(stdin, fgetc, fungetc, fmt, arg);
	va_end(arg);
	return retval;
}
#else
int scanf(fmt, arg)
const char *fmt;
char *arg;
{
	return (_scanf(stdin, fgetc, fungetc, fmt, &arg));
}
#endif /* __STDC__ */
