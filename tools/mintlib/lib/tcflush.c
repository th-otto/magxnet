/*
Public domain termios tcflush() for the MiNT library
10 October 1993 entropy@terminator.rs.itd.umich.edu -- first attempt
*/

#include <mintbind.h>
#include <errno.h>
#include <file.h>
#include <ioctl.h>
#include <types.h>
#include <termios.h>

int tcflush(fd, action)
int fd;
int action;
{
	long flushtype;
	long r;

	switch (action)
	{
	case TCIFLUSH:
		flushtype = FREAD;
		break;
	case TCOFLUSH:
		flushtype = FWRITE;
		break;
	case TCIOFLUSH:
		flushtype = 0;
		break;
	default:
		errno = ENOSYS;
		return -1;
	}
	r = Fcntl((short) fd, &flushtype, TIOCFLUSH);
	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}
	return 0;
}
