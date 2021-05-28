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

#if 1
extern const unsigned int *_ctype;
#define	_IScntrl	0x01		/* control character */
#define	_ISdigit	0x02		/* numeric digit */
#define	_ISupper	0x04		/* upper case */
#define	_ISlower	0x08		/* lower case */
#define	_ISspace	0x10		/* whitespace */
#define	_ISpunct	0x20		/* punctuation */
#define	_ISxdigit	0x40		/* hexadecimal */
#define _ISblank	0x80		/* blank */
#define _ISgraph	0x100		/* graph */
#define _ISprint	0x200		/* print */
#undef isdigit
#define	isdigit(c)	(_ctype[(unsigned char)((c) + 1)]&_ISdigit)
#undef isxdigit
#define	isxdigit(c)	(_ctype[(unsigned char)((c) + 1)]&_ISxdigit)
#undef islower
#define	islower(c)	(_ctype[(unsigned char)((c) + 1)]&_ISlower)
#undef isspace
#define	isspace(c)	(_ctype[(unsigned char)((c) + 1)]&_ISspace)
#undef isascii
#define	isascii(c)	!((c)&~0x7F)
#endif

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define MAGIC_ONLY 1



int getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen)
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


