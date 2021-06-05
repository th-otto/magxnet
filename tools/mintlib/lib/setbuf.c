/* from Dale Schumacher's dLibs library */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void setbuf(fp, buf)
register FILE *fp;
char *buf;
{
	if (fp->_flag & _IOMYBUF)
		free(fp->_base);
	fp->_flag &= ~(_IOFBF | _IOLBF | _IONBF | _IOMYBUF);
	fp->_cnt = 0;
	if ((fp->_base = (unsigned char *) buf) != NULL)
	{
		fp->_flag |= _IOFBF;
		/* this is intentionally not __DEFAULT_BUFSIZ__ ++jrb */
		fp->_bsiz = BUFSIZ;
	} else
	{
		fp->_flag |= _IONBF;
		fp->_base = &(fp->_ch);			/* use tiny buffer */
		fp->_bsiz = 1;
	}
	fp->_ptr = fp->_base;
}

/*
 * bezerkly'ism
 * change the buffering on stream from block/unbuffered to line buffered.
 * should stream be flushed before change?? i think so.
 *	++jrb
 */

void setlinebuf(fp)
register FILE *fp;
{
#ifndef NDEBUG
	assert((fflush(fp) != EOF));
#else
	(void) fflush(fp);
#endif
	fp->_flag &= ~(_IOFBF | _IONBF);
	fp->_flag |= _IOLBF;
}
