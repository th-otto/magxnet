/*
 * fcntl() emulation for MiNT; written by Eric R. Smith and placed
 * in the public domain
 */

#include <errno.h>
#include <mintbind.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef __STDC__
int fcntl(int f, int cmd, ...)
#else
int fcntl(f, cmd)
int f;
int cmd;
#endif
{
	long r;
	va_list argp;

	va_start(argp, cmd);

	r = Fcntl(f, va_arg(argp, void *), cmd);

	if (r == -ELOCKED)
		r = -EACCES;
	if (r < 0)
	{
		errno = (int) -r;
		r = -1L;
	}
	return (int) r;
}
