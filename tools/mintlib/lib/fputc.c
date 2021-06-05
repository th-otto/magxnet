/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include "lib.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

int fputc(c, fp)
register int c;
register FILE *fp;
{
	register int nl = FALSE;
	register long m;
	unsigned int f = fp->_flag;

	if (f & _IORW)
	{
		fp->_flag |= _IOWRT;
		f = (fp->_flag &= ~(_IOREAD | _IOEOF));
	}
	if (!(f & _IOWRT)					/* not opened for write? */
		|| (f & (_IOERR | _IOEOF)))		/* error/eof conditions? */
		return (EOF);

	if (!(f & _IOBIN) && (c == '\n'))	/* NL -> CR+LF ? */
	{
		c = '\r';
		nl = TRUE;
	}
  _fputc:
	*(fp->_ptr)++ = c;
	fp->_cnt++;
	if ((fp->_cnt >= fp->_bsiz) || ((f & _IOLBF) && (c == '\n')))	/* flush line buffd stream on \n */
	{
		m = fp->_cnt;
		fp->_cnt = 0;
		fp->_ptr = fp->_base;
		if (_write(fp->_file, fp->_base, m) != m)
		{
			fp->_flag |= _IOERR;
			return (EOF);
		}
	}
	if (nl && (c == '\r'))
	{									/* NL -> CR+LF ? */
		c = '\n';
		goto _fputc;
	}
	return (c & 0xff);
}
