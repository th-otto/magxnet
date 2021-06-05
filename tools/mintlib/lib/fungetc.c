/* from dlibs */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fungetc(c, fp)
int c;
register FILE *fp;
{
	if ((fp->_flag & (_IOERR | _IOEOF))	/* error or eof */
		|| (fp->_ptr <= fp->_base)		/* or too many ungets */
		|| (c < 0))						/* or trying to unget EOF */
		return (EOF);
	++(fp->_cnt);
	return (*--(fp->_ptr) = c);
}
