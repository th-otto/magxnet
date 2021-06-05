/* telldir routine */

/* under MiNT (v0.9 or better) these use the appropriate system calls.
 * under TOS or older versions of MiNT, they use Fsfirst/Fsnext
 *
 * Written by Eric R. Smith and placed in the public domain
 */

#include <types.h>
#include <limits.h>
#include <dirent.h>
#include "lib.h"

/* not POSIX */

off_t telldir(dirp)
DIR *dirp;
{
	return dirp->buf.d_off;
}
