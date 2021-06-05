/* open a file, searching along the PATH environment variable for it */

/* rehacked by Uwe Ohse, 3.5.93: uses buffindfile now */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <support.h>

FILE *fopenp(name, mode)
const char *name,
*mode;
{
	char *fullname;
	char buffer[PATH_MAX];

	fullname = _buffindfile(name, getenv("PATH"), (char const **) 0, buffer);
	if (!fullname)
	{
		errno = ENOENT;
		return NULL;
	}
	return fopen(fullname, mode);
}
