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

#define ISDIGIT(c) ((c) >= '0' && (c) <= '9')
#define ISXDIGIT(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define ISLOWER(c) ((c) >= 'a' && (c) <= 'z')
#define ISASCII(c) ((c) <= 127)

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define MAGIC_ONLY 1

short __libc_newsockets = 1;

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	int r;

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


int getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
	long r;

	{
		struct getsockname_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = GETSOCKNAME_CMD;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = Fcntl(fd, (long) &cmd, SOCKETCALL);
		
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


int socket(int domain, int type, int proto)
{
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = (int)Fsocket(domain, type, proto);
		if (r != -ENOSYS)
		{
			if (r < 0)
			{
				__set_errno(-r);
				return -1;
			}
			return r;
		}
		__libc_newsockets = 0;
	}
#endif

	{
		struct socket_cmd cmd;
		int sockfd;
		
		sockfd = (int)Fopen(SOCKDEV, O_RDWR);
		if (sockfd < 0)
		{
			__set_errno (-sockfd);
			return -1;
		}
		
		cmd.cmd = SOCKET_CMD;
		cmd.domain = domain;
		cmd.type = type;
		cmd.protocol = proto;
		
		r = (int)Fcntl(sockfd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-r);
			Fclose(sockfd);
			return -1;
		}
		
		return sockfd;
	}
}


#ifdef __MINT__
int listen(int fd, unsigned int backlog)
#else
int listen(int fd, int backlog)
#endif
{
	long r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		r = Flisten(fd, backlog);
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
		
		r = Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return 0;
}


int sendto(int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *addr, socklen_t addrlen)
{
	long r;

	if (__libc_newsockets)
	{
		r = Fsendto(fd, buf, buflen, flags, addr, addrlen);
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
	{
		struct sendto_cmd cmd;
		
		cmd.cmd = addr ? SENDTO_CMD : SEND_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		cmd.addr = addr;
		cmd.addrlen = (short) addrlen;
		
		r = Fcntl(fd, (long) &cmd, SOCKETCALL);
		if (r < 0)
		{
			__set_errno(-(int)r);
			return -1;
		}
	}
	return (int)r;
}
	

int send(int fd, const void *buf, size_t buflen, int flags)
{
	return sendto(fd, buf, buflen, flags, NULL, 0);
}


int recvfrom(int fd, void *buf, size_t buflen, int flags, struct sockaddr *addr, socklen_t *addrlen)
{
	long r;
	unsigned long addrlen32;
	
	if (__libc_newsockets)
	{
		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = Frecvfrom(fd, buf, buflen, flags, addr, &addrlen32);
		} else
		{
			r = Frecvfrom(fd, buf, buflen, flags, addr, addrlen);
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

	{
		struct recvfrom_cmd cmd;
		short addrlen16;
		
		if (addrlen)
			addrlen16 = (short) *addrlen;
		
		cmd.cmd = addr ? RECVFROM_CMD : RECV_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		cmd.addr = addr;
		cmd.addrlen = &addrlen16;
		
		r = Fcntl(fd, (long) &cmd, SOCKETCALL);
		
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


int recv(int fd, void *buf, size_t buflen, int flags)
{
	return recvfrom(fd, buf, buflen, flags, NULL, 0);
}


int poll(struct pollfd *fds, unsigned long int nfds, int32_t __timeout)
{
	long int retval;
	unsigned long timeout = (unsigned long) __timeout;

	if (__timeout < 0)
	{
		timeout = ~0;
	}

	retval = Fpoll(fds, nfds, timeout);
	if (retval != -ENOSYS)
	{
		if (retval < 0)
		{
			__set_errno(-(int)retval);
			return -1;
		}
	} else
	{
		/* We must emulate the call via Fselect ().  First task is to
		   set up the file descriptor masks.    */
		long rfds = 0;
		long wfds = 0;
		long xfds = 0;
		unsigned long int i;
		struct pollfd *pfds = fds;

		for (i = 0; i < nfds; i++)
		{
			pfds[i].revents = 0;

			/* Older than 1.19 can't do more than 32 file descriptors.
			 * And we'd only get here if we're a very old kernel anyway.
			 */
			if (pfds[i].fd >= 32)
			{
				pfds[i].revents = POLLNVAL;
				continue;
			}
#define LEGAL_FLAGS (POLLIN | POLLPRI | POLLOUT | POLLRDNORM | POLLWRNORM | POLLRDBAND | POLLWRBAND)

			if ((pfds[i].events | LEGAL_FLAGS) != LEGAL_FLAGS)
			{
				pfds[i].revents = POLLNVAL;
				continue;
			}

			if (pfds[i].events & (POLLIN | POLLRDNORM))
				rfds |= (1L << (pfds[i].fd));
			if (pfds[i].events & POLLPRI)
				xfds |= (1L << (pfds[i].fd));
			if (pfds[i].events & (POLLOUT | POLLWRNORM))
				wfds |= (1L << (pfds[i].fd));
		}

		if (__timeout < 0)
		{
			retval = Fselect(0L, &rfds, &wfds, &xfds);
		} else if (timeout == 0)
		{
			retval = Fselect(1L, &rfds, &wfds, &xfds);
		} else if (timeout < USHRT_MAX)
		{
			/* The manpage Fselect(2) says that timeout is
			   signed.  But it is really unsigned.  */
			retval = Fselect(timeout, &rfds, &wfds, &xfds);
		} else
		{
			/* Thanks to the former kernel hackers we have
			   to loop in order to simulate longer timeouts
			   than USHRT_MAX.  */
			unsigned long saved_rfds;
			unsigned long saved_wfds;
			unsigned long saved_xfds;
			unsigned short int this_timeout;
			int last_round = 0;

			saved_rfds = rfds;
			saved_wfds = wfds;
			saved_xfds = xfds;

			do
			{
				if ((unsigned long) timeout > USHRT_MAX)
					this_timeout = USHRT_MAX;
				else
				{
					this_timeout = timeout;
					last_round = 1;
				}

				retval = Fselect(this_timeout, &rfds, &wfds, &xfds);

				if (retval != 0)
					break;

				timeout -= this_timeout;

				/* I don't know whether we can rely on the
				   masks not being clobbered on timeout.  */
				rfds = saved_rfds;
				wfds = saved_wfds;
				xfds = saved_xfds;
			} while (!last_round);
		}

		/* Now fill in the results in struct pollfd.    */
		for (i = 0; i < nfds; i++)
		{
			/* Older than 1.19 can't do more than 32 file descriptors. */
			if (pfds[i].fd >= 32)
				continue;
			if (rfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & (POLLIN | POLLRDNORM));
			if (wfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & (POLLOUT | POLLWRNORM));
			if (xfds & (1L << (pfds[i].fd)))
				pfds[i].revents |= (pfds[i].events & POLLPRI);
		}

		if (retval < 0)
		{
			__set_errno(-(int)retval);
			return -1;
		}
	}

	return (int)retval;
}


/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  

   The function is currently emulated by means of poll().  This is 
   sub-optimal as long as NFDS is less or equal 32 because then we 
   waste quite some time by copying the file descriptor masks into
   struct poll.  Better poll will only be called when the native
   Fselect is not able to handle the call.  For the time being I want
   to test poll() and therefore ignore this problem.  
   
   If poll() has set POLLERR, POLLHUP, POLLNVAL or POLLMSG for any of the 
   polled descriptors, we simply mark that descriptor as ready for reading,
   writing or urgent reading so that the caller will get informed.  */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	struct pollfd pfds[FD_SETSIZE];
	int i;
	long retval;
	long msec_timeout;
	int saved_errno = errno;

	if (nfds < 0 || nfds > FD_SETSIZE)
	{
		__set_errno(EINVAL);
		return -1;
#if 0
	} else if (nfds <= 32)
	{
		/* Implement a native call to Fselect.  */
#endif
	}

	memset(pfds, 0, nfds * sizeof *pfds);

	/* Three loops are more efficient than one here.  */
	if (readfds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, readfds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLIN;
			}
	}
	
	if (exceptfds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, exceptfds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLPRI;
			}
	}

	if (writefds != NULL)
	{
		for (i = 0; i < nfds; i++)
			if (FD_ISSET(i, writefds))
			{
				pfds[i].fd = i;
				pfds[i].events |= POLLOUT;
			}
	}

	if (timeout == NULL)
	{
		msec_timeout = -1;
	} else
	{
		msec_timeout = timeout->tv_sec * 1000;
		msec_timeout += (timeout->tv_usec + 999) / 1000;
	}

	retval = poll(pfds, nfds, msec_timeout);

	if (retval < 0)
	{
		return (int)retval;
	} else
	{
		unsigned int sz = (unsigned int)howmany(nfds, NFDBITS) * (unsigned int)sizeof(fd_mask);

		if (readfds)
			memset(readfds, 0, sz);
		if (exceptfds)
			memset(exceptfds, 0, sz);
		if (writefds)
			memset(writefds, 0, sz);

		if (retval)
			for (i = 0; i < nfds; i++)
			{
				if (pfds[i].revents & (POLLIN | POLLRDNORM | POLLRDBAND))
					if (readfds != NULL)
						FD_SET(pfds[i].fd, readfds);
				if (pfds[i].revents & POLLPRI)
					if (exceptfds != NULL)
						FD_SET(pfds[i].fd, exceptfds);
				if (pfds[i].revents & (POLLOUT | POLLWRNORM | POLLWRBAND))
					if (writefds != NULL)
						FD_SET(pfds[i].fd, writefds);
				if (pfds[i].revents & (POLLERR | POLLNVAL | POLLHUP | POLLMSG))
				{
					if (readfds != NULL && FD_ISSET(pfds[i].fd, readfds))
						FD_SET(pfds[i].fd, readfds);
					if (exceptfds != NULL && FD_ISSET(pfds[i].fd, exceptfds))
						FD_SET(pfds[i].fd, exceptfds);
					if (writefds != NULL && FD_ISSET(pfds[i].fd, writefds))
						FD_SET(pfds[i].fd, writefds);
				}
			}
	}

	__set_errno(saved_errno);
	return (int)retval;
}


int usleep(__useconds_t dt)
{
	clock_t t;
	clock_t tt;

	tt = ((clock_t) dt) / (((clock_t) 1000000UL) / CLOCKS_PER_SEC);
	t = clock();
	while ((clock() - t) < tt)
		;
	return 0;
}


/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
in_addr_t inet_aton(const char *cp, struct in_addr *addr)
{
	in_addr_t val;
	int base;
	int n;
	unsigned char c;
	unsigned long parts[4];
	unsigned long *pp = parts;

	c = *cp;
	for (;;)
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!ISDIGIT(c))
			return 0;
		base = 10;
		if (c == '0')
		{
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		val = 0;
		for (;;)
		{
			if (ISASCII(c) && ISDIGIT(c))
			{
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && ISASCII(c) && ISXDIGIT(c))
			{
				val = (val << 4) |
					(c + 10 - (ISLOWER(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.')
		{
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return 0;
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!ISASCII(c) || !isspace(c)))
		return 0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = (int)(pp - parts + 1);
	switch (n)
	{
	case 0:
		return 0;		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (parts[0] > 0xff || val > 0xffffffUL)
			return 0;
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || val > 0xffffUL)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (parts[0] > 0xff || parts[1] > 0xff || parts[2] > 0xff || val > 0xff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);

	return 1;
}


/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
in_addr_t inet_addr(const char *cp)
{
	struct in_addr val;

	if (inet_aton(cp, &val))
		return val.s_addr;
	return INADDR_NONE;
}


