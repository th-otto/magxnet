/*
 * fill and process an input buffer
 *	called only when fp->_cnt < 0
 *
 * more hacks, the initial impl bit!
 *
 *	++jrb	bammi@dsrgsun.ces.cwru.edu
 *
 * do not remember EOF if input comes from a tty (er)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "lib.h"

int _filbuf(fp)
FILE *fp;
{
	register unsigned int f;
	register long got;

	f = fp->_flag;
	if (f & _IORW)
		f = (fp->_flag |= _IOREAD);
	if (!(f & _IOREAD) || (f & (_IOERR | _IOEOF)))
		return (EOF);

	/* if this is stdin &  a tty, and stdout is line buffered, flush it */
	if ((fp == stdin) && (f & _IODEV) && (stdout->_flag & _IOLBF))
		(void) fflush(stdout);

	fp->_ptr = fp->_base;
	if ((got = _read(fp->_file, fp->_base, (unsigned long) fp->_bsiz)) <= 0)
	{									/* EOF or error */
		fp->_flag |= ((got == 0) ? ((f & _IODEV) ? 0 : _IOEOF) : _IOERR);
		fp->_cnt = 0;
		return EOF;
	}
	fp->_cnt = got - 1;
	return *(fp->_ptr)++;
}
