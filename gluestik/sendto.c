#include "stsocket.h"
#include "mintsock.h"

int sendto(int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *addr, __mint_socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsendto(fd, buf, buflen, flags, addr, addrlen);
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
		struct sendto_cmd cmd;
		
		cmd.addr = addr;
		cmd.addrlen = (short) addrlen;
		cmd.cmd = SENDTO_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


