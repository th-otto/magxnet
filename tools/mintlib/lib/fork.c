/* public domain fork() for MiNT */

#include <unistd.h>
#include <errno.h>
#include <mintbind.h>

int fork()
{
	int r;

	r = (int) Pfork();
	if (r < 0)
	{
		errno = -r;
		return -1;
	}
	return r;
}
