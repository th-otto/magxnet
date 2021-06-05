/* something like the origonal
 * from Dale Schumacher's dLibs
 */

#include <stdio.h>
#include <unistd.h>

void rewind(fp)
register FILE *fp;
{
	fflush(fp);
	(void) lseek(fp->_file, 0L, SEEK_SET);
	fp->_flag &= ~(_IOEOF | _IOERR);
}
