/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "lib.h"

extern int __mint;

extern int __default_mode__;

FILE *_fopen_i(filename, mode, fp)
const char *filename;
const char *mode;
FILE *fp;

/*
 *	INTERNAL FUNCTION.  Attempt to open <filename> in the given
 *	<mode> and attach it to the stream <fp>
 */
{
	register int h,
	 i,
	 iomode = 0,
		f = __default_mode__;

	while (*mode)
	{
		switch (*mode++)
		{
		case 'r':
			f |= _IOREAD;
			break;
		case 'w':
			f |= _IOWRT;
			iomode |= (O_CREAT | O_TRUNC);
			break;
		case 'a':
			f |= _IOWRT;
			iomode |= (O_CREAT | O_APPEND);
			break;
		case '+':
			f &= ~(_IOREAD | _IOWRT);
			f |= _IORW;
			break;
		case 'b':
			f |= _IOBIN;
			break;
		case 't':
			f &= ~_IOBIN;
			break;
		default:
			return (NULL);
		}
	}
	if ((i = (f & (_IORW | _IOREAD | _IOWRT))) == 0)
		return (NULL);
	else if (i == _IOREAD)
		iomode |= O_RDONLY;
	else if (i == _IOWRT)
		iomode |= O_WRONLY;
	else
		iomode |= O_RDWR;
	iomode |= O_NOCTTY;
	h = open(filename, iomode, 0666);
	if (h < __SMALLEST_VALID_HANDLE)
	{
		return (NULL);					/* file open/create error */
	}
	if (isatty(h))
		f |= __mint ? (_IODEV | _IONBF | _IOBIN) : (_IODEV | _IONBF);
	else
		f |= _IOFBF;
	fp->_file = h;						/* file handle */
	fp->_flag = f;						/* file status flags */
#if 0
	if (iomode & O_APPEND)
	{
		fp->_cnt = 0;					/* required for fseek */
		(void) fseek(fp, 0L, SEEK_END);
	}
#endif

	return (fp);
}
