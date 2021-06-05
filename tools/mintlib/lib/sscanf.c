#include <stdio.h>
#include <stdarg.h>
#include "lib.h"

static int sgetc __PROTO((FILE * s));
static int sungetc __PROTO((int c, FILE * s));

static int sgetc(s)
FILE *s;
{
	register unsigned char c;

	c = *(* (unsigned char **) s)++;
	return ((c == '\0') ? EOF : c);
}

static int sungetc(c, s)
int c;
FILE *s;
{
	--(* (unsigned char **) s);
	return c;
}

#ifdef __STDC__
int sscanf(const char *buf, const char *fmt, ...)
{
	int retval;
	va_list args;

	va_start(args, fmt);
	retval = _scanf((FILE *) & buf, sgetc, sungetc, fmt, args);
	va_end(args);
	return retval;
}
#else
int sscanf(buf, fmt, arg)
const char *buf,
*fmt;
int arg;
{
	return (_scanf(&buf, sgetc, sungetc, fmt, &arg));
}
#endif /* __STDC__ */

#ifdef __STDC__
int vsscanf(const char *buf, const char *fmt, va_list arg)
#else
int vsscanf(buf, fmt, arg)
const char *buf,
*fmt;
char *arg;
#endif /* __STDC__ */
{
	return (_scanf((FILE *) & buf, sgetc, sungetc, fmt, arg));
}
