/*
Public domain termios tcflow() for the MiNT library
10 October 1993 entropy@terminator.rs.itd.umich.edu -- first attempt
*/

#include <mintbind.h>
#include <errno.h>
#include <ioctl.h>
#include <types.h>
#include <termios.h>

int tcflow(fd, action)
int fd;
int action;
{
	long r;

	switch (action)
	{
	case TCOOFF:
		r = Fcntl((short) fd, (long) 0, TIOCSTOP);
		break;
	case TCOON:
		r = Fcntl((short) fd, (long) 0, TIOCSTART);
		break;
	case TCIOFF:
		r = -EINVAL;
		break;
	case TCION:
		r = -EINVAL;
		break;
	default:
		r = -EINVAL;
		break;
	}
	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	return 0;
}
