/*
 * sendmsg() emulation for MiNT-Net, (w) '93, kay roemer
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

#include "stsocket.h"
#include "mintsock.h"

int sendmsg(int fd, const struct msghdr *msg, int flags)
{
	int r;

#if !defined(MAGIC_ONLY)
	if (__libc_newsockets)
	{
		r = (int)Fsendmsg(fd, msg, flags);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct sendmsg_cmd cmd;
		
		cmd.msg = msg;
		cmd.cmd = SENDMSG_CMD;
		cmd.flags = flags;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return (int)r;
}


