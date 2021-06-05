/* chown -- change the owner and group of a file */
/* written by Eric R. Smith and placed in the public domain */
/* this is faked if MiNT is not running */

#include <types.h>
#include <stat.h>
#include <osbind.h>
#include <mintbind.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include "lib.h"


int chown(_name, uid, gid)
const char *_name;
int uid,
	gid;
{
	int r;
	char name[PATH_MAX];

	(void) _unx2dos(_name, name, sizeof(name));
	r = (int) Fchown(name, uid, gid);
	if (r && (r != -EINVAL))
	{
		errno = -r;
		return -1;
	}
	return 0;
}
