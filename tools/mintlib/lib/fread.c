/* nothing like the original from
 * from Dale Schumacher's dLibs
 */

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

size_t fread(_data, size, count, fp)
void *_data;
size_t size;
size_t count;
register FILE *fp;
{
#ifdef __SOZOBON__
	register unsigned long n;
#else
	register size_t n;
#endif
	register long l,
	 cnt;
	register unsigned int f;
	char *data = _data;
	char *ptr;

	f = fp->_flag;
	if (f & _IORW)
		f = (fp->_flag |= _IOREAD);
	if (!(f & _IOREAD) || (f & (_IOERR | _IOEOF)) || !size || !count)
		return (0);

	l = 0;
#ifdef __SOZOBON__
	n = (unsigned long) count *size;
#else
	n = count * size;
#endif
#if 0
	if (fflush(fp))						/* re-sync file pointers */
		return 0;
#endif
	if ((f & _IOBIN) || __FRW_BIN__)
	{
	  again:
		if ((cnt = fp->_cnt) > 0)
		{
			cnt = (cnt < n) ? cnt : n;
#ifdef __SOZOBON__
			_bcopy(fp->_ptr, data, cnt);
#else
			bcopy(fp->_ptr, data, cnt);
#endif
			fp->_cnt -= cnt;
			fp->_ptr += cnt;
			l += cnt;
			data = data + cnt;
			n -= cnt;
		}
		/* n == how much more */
		if (n > 0)
		{
			if (n < fp->_bsiz)
			{							/* read in fp->_bsiz bytes into fp->_base and do it again */
				fp->_ptr = fp->_base;
				if ((cnt = _read(fp->_file, fp->_base, (unsigned long) fp->_bsiz)) <= 0)
				{						/* EOF or error */
					fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
					goto ret;
				}
				fp->_cnt = cnt;
				goto again;
			} else
				while (n > 0)
				{						/* read in n bytes into data */
					if ((cnt = _read(fp->_file, data, (unsigned long) n)) <= 0)
					{					/* EOF or error */
						fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
						goto ret;
					}
					l += cnt;
					data = data + cnt;
					n -= cnt;
				}
		}
	} else
	{
		while (n > 0)
		{
			if ((cnt = fp->_cnt) > 0)
			{
				ptr = (char *) fp->_ptr;
				while (n > 0 && cnt > 0)
				{
					if (*ptr != '\r')
					{
						*data++ = *ptr++;
						cnt--;
						n--;
						l++;
					} else
					{
						ptr++;
						cnt--;
					}
				}
				fp->_cnt = cnt;
				fp->_ptr = (unsigned char *) ptr;
			}
			if (n == 0)
				break;					/* done */
			/* wanna have n more bytes */
			if (n < fp->_bsiz)
			{
				/* read in fp->_bsiz bytes into fp->_base and do it again */
				fp->_ptr = fp->_base;
				if ((cnt = _read(fp->_file, fp->_base, (unsigned long) fp->_bsiz)) <= 0)
				{						/* EOF or error */
					fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
					goto ret;
				}
				fp->_cnt = cnt;
			} else
			{
				/* read in n bytes into data */
				if ((cnt = _read(fp->_file, data, (unsigned long) n)) <= 0)
				{						/* EOF or error */
					fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
					goto ret;
				}
				/* Quickly move to first CR */
				ptr = memchr(data, '\r', cnt);
				if (ptr == NULL)
					ptr = data + cnt;
				cnt -= ptr - data;
				n -= ptr - data;
				l += ptr - data;
				data = ptr;
				while (cnt > 0)
				{
					if (*ptr != '\r')
					{
						*data++ = *ptr++;
						cnt--;
						n--;
						l++;
					} else
					{
						ptr++;
						cnt--;
					}
				}
			}
		}
	}

  ret:
#ifdef __SOZOBON__
	return ((l > 0) ? ((size_t) ((unsigned long) l / size)) : 0);
#else
	return ((l > 0) ? ((size_t) l / size) : 0);
#endif
}
