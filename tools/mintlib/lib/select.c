/*
 * select() emulation for MiNT. Written by Eric R. Smith and placed in the
 * public domain
 */

#include <errno.h>
#include <mintbind.h>
#include <types.h>
#include <time.h>

int select(junk, rfds, wfds, xfds, timeout)
int junk;
fd_set *rfds,
*wfds,
*xfds;
struct timeval *timeout;
{
/* the only tricky part, really, is figuring out the timeout value.
   a null pointer means indefinite timeout, which is the same as a 0
   value under MiNT. A non-null pointer to a 0 valued struct means
   to poll; in MiNT we simulate this with a minimum timeout value.
 */
	unsigned long mtime;
	unsigned short stime;
	int rval;
	fd_set save_rfds = 0,
		save_wfds = 0,
		save_xfds = 0;

	(void)junk;
	if (rfds)
		save_rfds = *rfds;
	if (wfds)
		save_wfds = *wfds;
	if (xfds)
		save_xfds = *xfds;
	if (timeout)
	{
		mtime = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
		if (mtime == 0)
			mtime = 1;
	} else
		mtime = 0;

	/* Unfortunately, Fselect can only handle at most 65535ms timeout.
	   We have to loop for a bigger timeout. */
	for (;;)
	{
		if (mtime > 65535U)
			stime = 65535U;
		else
			stime = (unsigned short) mtime;
		mtime -= stime;
		rval = Fselect(stime, (long *) rfds, (long *) wfds, (long *) xfds);
		if (rval < 0)
		{
			errno = -rval;
			return -1;
		}
		if (rval == 0 && mtime > 0)
		{
			if (rfds)
				*rfds = save_rfds;
			if (wfds)
				*wfds = save_wfds;
			if (xfds)
				*xfds = save_xfds;
		} else
			return rval;
	}
}
