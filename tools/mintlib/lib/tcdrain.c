/*
Public domain termios tcdrain() for the MiNT library
10 October 1993 entropy@terminator.rs.itd.umich.edu
*/

#include <errno.h>
#include <types.h>
#include <ioctl.h>
#include <mintbind.h>
#include <termios.h>

int tcdrain(fd)
int fd;
{
	long outq;
	long r;

	do
	{
		r = Fcntl((short) fd, &outq, TIOCOUTQ);
		if (r < 0)
		{
			errno = (int) -r;
			return -1;
		}
	} while (outq != 0);
	return 0;
}
