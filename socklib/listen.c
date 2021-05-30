#include "stsocket.h"
#include "mintsock.h"

#ifdef __MINT__
int listen(int fd, unsigned int backlog)
#else
int listen(int fd, int backlog)
#endif
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Flisten(fd, backlog);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int) r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif
	
	{
		struct listen_cmd cmd;
		
		cmd.cmd = LISTEN_CMD;
		cmd.backlog = backlog;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}


