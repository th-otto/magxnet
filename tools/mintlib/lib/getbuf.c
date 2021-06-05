/* from the dLibs getbuf.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib.h"

extern size_t __DEFAULT_BUFSIZ__;

void _getbuf(fp)						/* allocate a buffer for a stream */
register FILE *fp;
{
	unsigned int f = fp->_flag;

	if (f & _IOLBF)
		fp->_bsiz = (__DEFAULT_BUFSIZ__ < BUFSIZ) ? __DEFAULT_BUFSIZ__ : BUFSIZ;
	else
		fp->_bsiz = __DEFAULT_BUFSIZ__;

	if ((f & _IONBF)					/* risky!! but works ok with gnu.may change */
		|| ((fp->_base = (unsigned char *) malloc((size_t) fp->_bsiz)) == 0))
	{
		fp->_flag &= ~(_IOFBF | _IOLBF | _IONBF);
		fp->_flag |= _IONBF;
		fp->_base = &(fp->_ch);			/* use tiny buffer */
		fp->_bsiz = 1;
	} else
		fp->_flag |= _IOMYBUF;			/* use big buffer */
	fp->_ptr = fp->_base;
	fp->_cnt = 0;						/* start out with an empty buffer */
}
