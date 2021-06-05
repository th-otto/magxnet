/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "lib.h"

__EXTERN void _getbuf __PROTO((FILE *));

FILE *fopen(filename, mode)
const char *filename,
*mode;
{
	register int i;
	register FILE *fp = NULL;

	for (i = 0; (!fp && (i < _NFILE)); ++i)
		if (!(_iob[i]._flag & (_IORW | _IOREAD | _IOWRT)))
			fp = &_iob[i];				/* empty slot */
	if (fp != NULL)
	{
		if (_fopen_i(filename, mode, fp) == NULL)
			return NULL;
		_getbuf(fp);
		return fp;
	} else
	{
		errno = EMFILE;
		return NULL;
	}
}
