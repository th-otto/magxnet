/*
 * accept() emulation for MiNT-Net, (w) '93, kay roemer.
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

#include "stsocket.h"
#include "mintsock.h"

int accept(int fd, struct sockaddr *addr, __mint_socklen_t *addrlen)
{
	int r;

#if !defined(MAGIC_ONLY)
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = (int)Faccept(fd, addr, &addrlen32);
		} else
		{
			r = (int)Faccept(fd, addr, addrlen);
		}
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			if (addrlen)
				*addrlen = addrlen32;
			return (int)r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct accept_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = ACCEPT_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		
		if (addrlen)
			*addrlen = addrlen16;
		
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}


