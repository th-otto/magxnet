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


int sendto(int fd, const void *buf, size_t buflen, int flags, const struct sockaddr *addr, socklen_t addrlen)
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
		
		cmd.cmd = addr ? SENDTO_CMD : SEND_CMD;
		cmd.buf = buf;
		cmd.buflen = buflen;
		cmd.flags = flags;
		cmd.addr = addr;
		cmd.addrlen = (short) addrlen;
		
		r = (int)Fcntl(fd, (long) &cmd, SOCKETCALL);
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
	int r;

#if !MAGIC_ONLY
	if (__libc_newsockets)
	{
		unsigned long addrlen32;

		if (addrlen)
		{
			addrlen32 = *addrlen;
			r = (int)Frecvfrom(fd, buf, buflen, flags, addr, &addrlen32);
		} else
		{
			r = (int)Frecvfrom(fd, buf, buflen, flags, addr, addrlen);
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
