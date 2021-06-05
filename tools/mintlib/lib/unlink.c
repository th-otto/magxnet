/* unlink.c: by ERS. This routine is in the public domain */

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <osbind.h>
#include <fcntl.h>
#include "lib.h"

/* remove provided for ansi compatibility */
#ifndef __GNUC__
int remove(filename)
const char *filename;
{
	return unlink(filename);
}
#endif

#ifdef __GNUC__
asm(".text; .even; .globl _remove; _remove:");	/* dept of dirty tricks */
#endif
int unlink(filename)
const char *filename;
{
	char name[PATH_MAX];
	int r;

	_unx2dos(filename, name, sizeof(name));

	r = (int) Fdelete(name);

	if (r < 0)
	{
		if ((r == -EPATH) && (_enoent(name)))
			r = -ENOENT;
		errno = -r;
		return -1;
	}
	return 0;
}
