/*
 *	setsockopt() emulation for MiNT-Net, (w) '93, kay roemer
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

#include "stsocket.h"
#include "mintsock.h"


int setsockopt(int fd, int level, int optname, const void *optval, __mint_socklen_t optlen)
{
	int r;

#if !defined(MAGIC_ONLY)
	if (__libc_newsockets)
	{
		r = (int)Fsetsockopt(fd, level, optname, optval, optlen);

		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct setsockopt_cmd cmd;

		cmd.cmd = SETSOCKOPT_CMD;
		cmd.level = level;
		cmd.optname = optname;
		cmd.optval = (void *) optval;
		cmd.optlen = optlen;

		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-r);
			return -1;
		}
		return 0;
	}
}


int getsockopt(int fd, int level, int optname, void *optval, __mint_socklen_t *optlen)
{
	int r;

#if !defined(MAGIC_ONLY)
	if (__libc_newsockets)
	{
		r = (int)Fgetsockopt(fd, level, optname, optval, optlen);

		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-r);
				return -1;
			}
			return 0;
		}
	}
#endif

	{
		struct getsockopt_cmd cmd;
		long optlen32;

		if (optlen)
			optlen32 = *optlen;

		cmd.cmd = GETSOCKOPT_CMD;
		cmd.level = level;
		cmd.optname = optname;
		cmd.optval = optval;
		cmd.optlen = &optlen32;

		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);

		if (optlen)
			*optlen = optlen32;

		if (r < 0)
		{
			__set_errno(-r);
			return -1;
		}
	}
	return 0;
}
