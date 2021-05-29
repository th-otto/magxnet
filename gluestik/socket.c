#ifdef __GNUC__
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <mint/mintbind.h>
#include "mintsock.h"
#undef ENOSYS
#define ENOSYS 32

#undef __set_errno
#define __set_errno(e) (errno = (e))

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define MAGIC_ONLY 1

#if !MAGIC_ONLY
static short __libc_newsockets = 1;
#endif

#if !MAGIC_ONLY
static const char *h_errlist[] =
{
	"Resolver Error 0 (no error)",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
#define h_nerr (int)(sizeof h_errlist / sizeof h_errlist[0])


const char *hstrerror(int err)
{
	if (err < 0)
		return "Resolver internal error";
	else if (err < h_nerr)
		return h_errlist[err];
	
	return "Unknown resolver error";
}
#endif


int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

#if !MAGIC_ONLY
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


int bind(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fbind(fd, addr, addrlen);
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
		struct bind_cmd cmd;
		
		cmd.addr = (struct sockaddr *)addr;
		cmd.addrlen = (short) addrlen;
		cmd.cmd = BIND_CMD;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int) r);
			return -1;
		}
	}
	return 0;
}


int connect(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fconnect(fd, addr, addrlen);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-(int)r);
				return -1;
			}
			return 0;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct connect_cmd cmd;
		
		cmd.addr = (struct sockaddr *)addr;
		cmd.addrlen = (short) addrlen;
		cmd.cmd = CONNECT_CMD;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}
