/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <unistd.h>
#include "lib.h"
#ifndef _COMPILER_H
#include <compiler.h>
#endif

static int _fflush __PROTO((FILE * fp));

int fflush(fp)
register FILE *fp;

/*
 *	implementation note:  This function has the side effect of
 *	re-aligning the virtual file pointer (in the buffer) with
 *	the actual file pointer (in the file) and is therefore used
 *	in other functions to accomplish this re-sync operation.
 */
{
	register int f,
	 i;

	if (fp)
		return (_fflush(fp));
	else
	{
		for (i = 0; i < _NFILE; ++i)
		{
			f = _iob[i]._flag;
			if (f & (_IOREAD | _IOWRT | _IORW))
				_fflush(&_iob[i]);
		}
		return (0);
	}
}

static int _fflush(fp)
register FILE *fp;
{
	register int f,
	 rv = 0;
	register long offset;

	if (fp == NULL)
		return (0);
	f = fp->_flag;
	if (!(f & (_IORW | _IOREAD | _IOWRT)))	/* file not open */
		return (EOF);
	if (fp->_cnt > 0)					/* data in the buffer */
	{
		if (f & _IOWRT)					/* writing */
		{
			register long todo;

			/* _cnt is cleared before writing to avoid */
			/* loop if fflush is recursively called by */
			/* exit if ^C is pressed during this write */
			todo = fp->_cnt;
			fp->_cnt = 0;
			if (_write(fp->_file, fp->_base, todo) != todo)
			{
				fp->_flag |= _IOERR;
				rv = EOF;
			}
		} else if (f & _IOREAD)			/* reading */
		{
			offset = -(fp->_cnt);
			if (lseek(fp->_file, offset, 1) < 0)
				if (!(f & _IODEV))
					rv = EOF;
		}
	}
	if (f & _IORW)
		fp->_flag &= ~(_IOREAD | _IOWRT);
	fp->_ptr = fp->_base;
	fp->_cnt = 0;
	return (rv);
}
