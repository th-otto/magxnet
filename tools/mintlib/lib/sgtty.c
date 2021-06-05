/*
 * Written by Eric R. Smith for the Atari ST
 */

#include <ioctl.h>

int stty(fd, _tty)
int fd;
struct sgttyb *_tty;
{
	return ioctl(fd, TIOCSETP, _tty);
}

int gtty(fd, _tty)
int fd;
struct sgttyb *_tty;
{
	return ioctl(fd, TIOCGETP, _tty);
}
