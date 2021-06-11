#ifdef __TURBOC__
#include <sys\types.h>
#else
#include <sys/types.h>
#endif
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>
#include <ioctl.h>
#include <fcntl.h>
#include "lib.h"

static short have_pgrp = 1;

pid_t getpgrp()
{
	int r;

	if (have_pgrp)
	{
		r = Pgetpgrp();
		if (r == -ENOSYS)
			have_pgrp = 0;
		else
			return (pid_t) r;
	}
	return 0;
}

int setpgrp()
{
	int r;

	if (have_pgrp)
	{
		r = Psetpgrp(0, 0);
		if (r == -ENOSYS)
			have_pgrp = 0;
		else
			return r;
	}
	return 0;
}

int _bsd_setpgrp(pid, grp)
int pid,
	grp;
{
	int r = 0;

	if (have_pgrp)
	{
		r = Psetpgrp(pid, grp);
		if (r == -ENOSYS)
		{
			if (grp != -1)
				have_pgrp = 0;
			r = 0;
		} else if (r < 0)
		{
			errno = -r;
			r = -1;
		}
	}
	return r;
}

int _bsd_getpgrp(pid)
int pid;
{
	return _bsd_setpgrp(pid, -1);
}

int setpgid(pid, pgid)
pid_t pid,
	pgid;
{
	return (_bsd_setpgrp((int) pid, (int) pgid));
}

pid_t setsid()
{
	long prc_pgrp;
	int tty_fd;
	int rc = -1;

	if (have_pgrp)
	{
		prc_pgrp = Pgetpgrp();
		if (prc_pgrp == -ENOSYS)
		{
			have_pgrp = 0;
			errno = ENOSYS;
		} else
		{
			if (prc_pgrp != Pgetpid())
			{
				if (isatty(-1))
				{
					tty_fd = open("/dev/tty", O_RDWR | O_NOCTTY);
					if (tty_fd < __SMALLEST_VALID_HANDLE)
						return (pid_t) - 1;
					if (ioctl(tty_fd, TIOCNOTTY, 0) < 0)
						return -1;
					(void) close(tty_fd);
				}
			}
			rc = (int) Psetpgrp(0, 0);
			if (rc < 0)
			{
				errno = -rc;
				rc = -1;
			}
		}
	} else
		errno = ENOSYS;

	return (pid_t) rc;
}
