/* something like the origonal
 * from Dale Schumacher's dLibs
 */

#include <stdio.h>
#include <unistd.h>

long ftell(fp)
FILE *fp;
{
	long rv,
	 count = fp->_cnt,
		adjust = 0;
	unsigned int f = fp->_flag;

	if (
#if 0									/* NOT ONLY IF _IOBIN! (MJK) */
		   ((f & _IOREAD) && (!(f & _IOBIN))) ||
#endif
		   (count == 0) || (f & _IONBF))
	{
		fflush(fp);
		rv = lseek(fp->_file, 0L, SEEK_CUR);
	} else
	{
		if (f & _IOREAD)
			adjust = -count;
		else if (f & (_IOWRT | _IORW))
		{
			if (f & _IOWRT)
				adjust = count;
		} else
			return -1L;

		rv = lseek(fp->_file, 0L, SEEK_CUR);
	}
	return (rv < 0) ? -1L : rv + adjust;
}
