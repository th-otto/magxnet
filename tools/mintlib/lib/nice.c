/*
 * nice() `emulation' by Dave Gymer
 * I've never been too sure what this should return; my SysV quick
 * ref seems to say "-20 to +19,  or -1 on error". Hmm.
 */

#include <errno.h>
#include <mintbind.h>
#include <support.h>

int nice(p)
int p;
{
	int r;
	static short have_pnice = 1;

	if (have_pnice)
	{
		r = Pnice(p);
		if (r == -ENOSYS)
			have_pnice = 0;
		else
			return -(int) (short) r;
	}
	errno = ENOSYS;
	return -1;
}
