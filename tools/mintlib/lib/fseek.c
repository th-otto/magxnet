/* something like the origonal
 * from Dale Schumacher's dLibs
 */

#include <stdio.h>
#include <unistd.h>

int fseek(fp, offset, origin)
FILE *fp;
long offset;
int origin;
{
	long pos,
	 count;
	unsigned int f;
	int rv;

	/* Clear end of file flag */
	f = (fp->_flag &= ~_IOEOF);
	count = fp->_cnt;

	if (
#if 0									/* NOT ONLY IF _IOBIN! (MJK) */
		   (!(f & _IOBIN)) ||
#endif
		   (f & (_IOWRT)) || (count == 0) || (f & _IONBF) || (origin == SEEK_END))
	{
		rv = fflush(fp);
		return ((rv == EOF) || (lseek(fp->_file, offset, origin) < 0)) ? -1 : 0;
	}

	/* figure out if where we are going is still within the buffer */
	pos = offset;
	if (origin == SEEK_SET)
	{
		register long realpos;

		if ((realpos = tell(fp->_file)) < 0)
		{								/* no point carrying on */
			return -1;
		}
		pos += count - realpos;
	} else
		offset -= count;				/* we were already count ahead */

	if ((!(f & _IORW)) && (pos <= count) && (pos >= (fp->_base - fp->_ptr)))
	{
		fp->_ptr += pos;
		fp->_cnt -= pos;
		return 0;
	}
	/* otherwise */
	fp->_ptr = fp->_base;
	fp->_cnt = 0;
	if (f & _IORW)
		fp->_flag &= ~_IOREAD;
	return (lseek(fp->_file, offset, origin) < 0) ? -1 : 0;
}
