/* from Dale Schumacher's dLibs */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "lib.h"

extern int __mint;

/*
 * re-coded,  foobared code deleted
 *
 *	++jrb
 */
FILE *freopen(filename, mode, fp)
const char *filename,
*mode;
FILE *fp;
{
	unsigned int f;

	if (fp == NULL)
		return NULL;

	f = fp->_flag;
	if ((f & (_IORW | _IOREAD | _IOWRT)) != 0)
	{									/* file not closed, close it */
#if 0
		if (fflush(fp) != 0)
			return NULL;				/* flush err */
		if (close(fp->_file) != 0)
			return NULL;				/* close err */
#else
		fflush(fp);						/* ANSI says ignore errors */
		if (__mint || !(f & _IODEV))	/* leave tty's alone */
			close(fp->_file);
#endif
	}
	/* save buffer discipline and setbuf settings, and _IOWRT just for
	   determinining line buffering after the next _fopen_i */
	f &= (_IOFBF | _IOLBF | _IONBF | _IOMYBUF | _IOWRT);

	/* open the new file according to mode */
	if ((fp = _fopen_i(filename, mode, fp)) == NULL)
		return NULL;
	if (fp->_flag & _IODEV)
	{									/* we are re-opening to a tty */
		if ((f & _IOFBF) && (f & _IOWRT) && (f & _IOMYBUF))
		{								/* was a FBF & WRT & !setvbuff'ed  */
			/* reset to line buffering */
			f &= ~_IOFBF;
			f |= _IOLBF;
		}
	}
	f &= ~_IOWRT;						/* get rid of saved _IOWRT flag */

	/* put buffering and discipline flags in new fp->_flag */
	fp->_flag &= ~(_IONBF | _IOLBF | _IOFBF | _IOMYBUF);
	fp->_flag |= f;

	if (fp->_base == NULL)
		/* get new buffer if file was closed */
		_getbuf(fp);

	return fp;
}
