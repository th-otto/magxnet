/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern int __mint;

int fclose(fp)
register FILE *fp;
{
	register int f;
	register int error = 0;

	if (fp == NULL)
		return (EOF);					/* NULL file pointer file */
	f = fp->_flag;
	if ((f & (_IORW | _IOREAD | _IOWRT)) == 0)
		return (EOF);					/* file not open! */
	if (f & _IOWRT)						/* only bother flushing for write */
		error = fflush(fp);
#ifdef __OLD__
	if (fp->_bsiz != BUFSIZ)			/* throw away non-standard buffer */
#else
	if (!(f & _IOMYBUF))				/* throw away non-standard buffer */
#endif
	{
		fp->_base = NULL;
		fp->_ptr = NULL;
		fp->_bsiz = 0;
	}
#ifndef __OLD__
	else
	{
		free(fp->_base);
		fp->_base = NULL;
		fp->_ptr = NULL;
		fp->_bsiz = 0;
	}
#endif
	fp->_flag = 0;						/* clear status */
	if (__mint == 0)
		if (f & _IODEV)					/* leave tty's alone */
			return (0);
	error |= close(fp->_file);
	return (error ? EOF : 0);
}
