#include "stsocket.h"
#include "mintsock.h"



int getsockname(int fd, struct sockaddr *addr, __mint_socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		addrlen32 = *addrlen;
		r = (int)Fgetsockname(fd, addr, &addrlen32);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			*addrlen = addrlen32;
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct getsockname_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = GETSOCKNAME_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;

		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return 0;
}


