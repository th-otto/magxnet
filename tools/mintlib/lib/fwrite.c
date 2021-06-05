/* from Dale Schumacher's dLibs */

/* 5/26/93 sb -- Modified for HSC to account for the possibility that
 * size * count >= 64K.
 */

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include "lib.h"

extern short __FRW_BIN__;

size_t fwrite(_data, size, count, fp)
const void *_data;
size_t size;
size_t count;
register FILE *fp;
{
	const char *data = _data;

#ifdef __SOZOBON__
	register unsigned long n,
	 m;
#else
	register size_t n,
	 m;
#endif
	register long l = 0;
	long space;
	unsigned int f = fp->_flag;
	const char *restart_buf;
	int have_nl;
	int wrote_cr;
	int line_flush;

	if (f & _IORW)
	{
		fp->_flag |= _IOWRT;
		f = (fp->_flag &= ~(_IOREAD | _IOEOF));
	}

	if (!(f & _IOWRT)					/* not opened for write? */
		|| (f & (_IOERR | _IOEOF))		/* error/eof conditions? */
		|| !size || !count)				/* nothing to do? */
		return (0);

#ifdef __SOZOBON__
	n = (unsigned long) count *size;

	assert(n <= (unsigned_long) LONG_MAX);	/* otherwise impl will not work */
#else
	n = count * size;
#endif

	if ((f & _IOBIN) || __FRW_BIN__)
	{
		space = fp->_bsiz - fp->_cnt;
		while (n > 0)
		{
			m = (n > space) ? space : n;
#ifdef __SOZOBON__
			_bcopy(data, fp->_ptr, m);
#else
			bcopy(data, fp->_ptr, m);
#endif
			fp->_ptr += m;
			fp->_cnt += m;
			space -= m;
			if (space == 0)
			{
				if (fflush(fp))
					return 0;
				space = fp->_bsiz;
				if (f & _IORW)
					fp->_flag |= _IOWRT;	/* fflush resets this */
			}
			l += m;
			data += m;
			n -= m;
			if (n < space)
				continue;
			if ((m = _write(fp->_file, data, (unsigned long) n)) != (long) n)
			{
				fp->_flag |= _IOERR;
				return 0;
			}
			l += m;
			break;
		}
	} else
	{
		have_nl = 1;
		wrote_cr = 0;
		line_flush = 0;
		/* this relies on having at least one byte buffer,
		   otherwise we'll hang up when trying to write CRLF */
		while (n > 0)
		{
			space = fp->_bsiz - fp->_cnt;
			restart_buf = data;
			while (space > 0 && n > 0)
			{
				if (*data == '\n')
				{
					if (!wrote_cr)
					{
						if (f & _IOLBF)
							line_flush = 1;
						*fp->_ptr++ = '\r';
						wrote_cr = 1;
						have_nl = 1;
						l--;			/* compensate for the increment below */
					} else
					{
						*fp->_ptr++ = '\n';
						data++;
						wrote_cr = 0;
						n--;
					}
				} else
				{
					*fp->_ptr++ = *data++;
					n--;
				}
				space--;
				fp->_cnt++;
				l++;
			}

			if (space == 0)
			{
				if (have_nl)
				{
					if (fflush(fp))
						return 0;
					if (f & _IORW)
						fp->_flag |= _IOWRT;	/* fflush resets this */
					have_nl = 0;
				} else
				{
					/* this will probably happen in nonbuffered mode only:
					   try to write as much data in one go as possible */
					fp->_cnt = 0;
					fp->_ptr = fp->_base;
					while (n && *data != '\n')
					{
						n--;
						data++;
					}
					if ((m = _write(fp->_file, restart_buf, data - restart_buf)) != data - restart_buf)
					{
						fp->_flag |= _IOERR;
						return 0;
					}
					l += (m - 1);		/* we already counted one character, before
										 * decrementing 'space' to zero */
				}
			}
		}								/* while */
		if (line_flush)
		{
			if (fflush(fp))
				return 0;
			if (f & _IORW)
				fp->_flag |= _IOWRT;	/* fflush resets this */
		}
	}

#ifdef __SOZOBON
	return ((l > 0) ? ((size_t) ((unsigned long) l / size)) : 0);
#else
	return ((l > 0) ? ((size_t) l / size) : 0);
#endif
}
