#include "sockets.h"
#include "mxkernel.h"
#include "sockdev.h"
#include "mintsock.h"
#include <pcerrno.h>
#include <fcntl.h>

long mgx_socket(MX_DOSFD *f, short domain, short type, short protocol);
long mgx_socketpair(MX_DOSFD *f, short domain, short type, short protocol);
long mgx_bind(MX_DOSFD *f, void *addr, short addrlen);
long mgx_listen(MX_DOSFD *f, short backlog);
long mgx_accept(MX_DOSFD *f, void *addr, short *addrlen);
long mgx_connect(MX_DOSFD *f, void *addr, short addrlen);
long mgx_getsockname(MX_DOSFD *f, void *addr, short *addrlen);
long mgx_getpeername(MX_DOSFD *f, void *addr, short *addrlen);
long mgx_send(MX_DOSFD *f, const void *buf, long buflen, short flags);
long mgx_sendto(MX_DOSFD *f, const void *buf, long buflen, short flags, const void *addr, short addrlen);
long mgx_recv(MX_DOSFD *f, void *buf, long buflen, short flags);
long mgx_recvfrom(MX_DOSFD *f, void *buf, long buflen, short flags, void *addr, short *addrlen);
long mgx_setsockopt(MX_DOSFD *f, short level, short optname, void *optval, long optlen);
long mgx_getsockopt(MX_DOSFD *f, short level, short optname, void *optval, long *optlen);
long mgx_shutdown(MX_DOSFD *f, short how);
long mgx_sendmsg(MX_DOSFD *f, const struct msghdr *msg, short flags);
long mgx_recvmsg(MX_DOSFD *f, struct msghdr *msg, short flags);

struct socket *so_alloc(void);
long so_free(struct socket *so);
long so_release(struct socket *so);

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * Fcntl(SOCKETCALL)
 */
static long socketcall(MX_DOSFD *f, void *buf)
{
	switch (((struct generic_cmd *)buf)->cmd)
	{
	case SOCKET_CMD:
		{
			struct socket_cmd *cmd = (struct socket_cmd *)buf;
			return mgx_socket(f, cmd->domain, cmd->type, cmd->protocol);
		}
	case SOCKETPAIR_CMD:
		{
			struct socketpair_cmd *cmd = (struct socketpair_cmd *)buf;
			return mgx_socketpair(f, cmd->domain, cmd->type, cmd->protocol);
		}
	case BIND_CMD:
		{
			struct bind_cmd *cmd = (struct bind_cmd *)buf;
			return mgx_bind(f, cmd->addr, cmd->addrlen);
		}
	case LISTEN_CMD:
		{
			struct listen_cmd *cmd = (struct listen_cmd *)buf;
			return mgx_listen(f, cmd->backlog);
		}
	case ACCEPT_CMD:
		{
			struct accept_cmd *cmd = (struct accept_cmd *)buf;
			return mgx_accept(f, cmd->addr, cmd->addrlen);
		}
	case CONNECT_CMD:
		{
			struct connect_cmd *cmd = (struct connect_cmd *)buf;
			return mgx_connect(f, cmd->addr, cmd->addrlen);
		}
	case GETSOCKNAME_CMD:
		{
			struct getsockname_cmd *cmd = (struct getsockname_cmd *)buf;
			return mgx_getsockname(f, cmd->addr, cmd->addrlen);
		}
	case GETPEERNAME_CMD:
		{
			struct getpeername_cmd *cmd = (struct getpeername_cmd *)buf;
			return mgx_getpeername(f, cmd->addr, cmd->addrlen);
		}
	case SEND_CMD:
		{
			struct send_cmd *cmd = (struct send_cmd *)buf;
			return mgx_send(f, cmd->buf, cmd->buflen, cmd->flags);
		}
	case SENDTO_CMD:
		{
			struct sendto_cmd *cmd = (struct sendto_cmd *)buf;
			return mgx_sendto(f, cmd->buf, cmd->buflen, cmd->flags, cmd->addr, cmd->addrlen);
		}
	case RECV_CMD:
		{
			struct recv_cmd *cmd = (struct recv_cmd *)buf;
			return mgx_recv(f, cmd->buf, cmd->buflen, cmd->flags);
		}
	case RECVFROM_CMD:
		{
			struct recvfrom_cmd *cmd = (struct recvfrom_cmd *)buf;
			return mgx_recvfrom(f, cmd->buf, cmd->buflen, cmd->flags, cmd->addr, cmd->addrlen);
		}
	case SETSOCKOPT_CMD:
		{
			struct setsockopt_cmd *cmd = (struct setsockopt_cmd *)buf;
			return mgx_setsockopt(f, cmd->level, cmd->optname, cmd->optval, cmd->optlen);
		}
	case GETSOCKOPT_CMD:
		{
			struct getsockopt_cmd *cmd = (struct getsockopt_cmd *)buf;
			return mgx_getsockopt(f, cmd->level, cmd->optname, cmd->optval, cmd->optlen);
		}
	case SHUTDOWN_CMD:
		{
			struct shutdown_cmd *cmd = (struct shutdown_cmd *)buf;
			return mgx_shutdown(f, cmd->how);
		}
	case SENDMSG_CMD:
		{
			struct sendmsg_cmd *cmd = (struct sendmsg_cmd *)buf;
			return mgx_sendmsg(f, cmd->msg, cmd->flags);
		}
	case RECVMSG_CMD:
		{
			struct recvmsg_cmd *cmd = (struct recvmsg_cmd *)buf;
			return mgx_recvmsg(f, cmd->msg, cmd->flags);
		}
	}
	return -ENOSYS;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * device driver callbacks
 */

static long cdecl socket_open(MX_DOSFD *f)
{
	struct socket *s;
	
	s = so_alloc();
	if (s == NULL)
		return -ENOMEM;
	f->fd_user1 = (long)s;
	f->fd_mode |= OM_NOCHECK;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_write(MX_DOSFD *f, long len, void *buf)
{
#ifdef __PUREC__
	/* already handled by asm code and never called */
	UNUSED(f);
	UNUSED(buf);
	UNUSED(len);
#pragma warn -rvl
#else
	struct socket *so = (struct socket *) f->fd_user1;
	struct iovec iov[1];

	if (so->state == SS_VIRGIN)
		return EINVAL;

	iov[0].iov_base = NO_CONST(buf);
	iov[0].iov_len = len;

	return (*so->ops->send)(so, iov, 1, f->fd_mode & O_NDELAY, 0, 0, 0);
#endif
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_read(MX_DOSFD *f, long len, void *buf)
{
#ifdef __PUREC__
	/* already handled by asm code and never called */
	UNUSED(f);
	UNUSED(buf);
	UNUSED(len);
#else
	struct socket *so = (struct socket *) f->fd_user1;
	struct iovec iov[1];
	
	if (so->state == SS_VIRGIN)
		return EINVAL;

	iov[0].iov_base = buf;
	iov[0].iov_len = len;

	return (*so->ops->recv)(so, iov, 1, f->fd_mode & O_NDELAY, 0, 0, 0);
#endif
}

#ifdef __PUREC__
#pragma warn .rvl
#endif

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_seek(MX_DOSFD *f, long where, short whence)
{
	UNUSED(f);
	UNUSED(where);
	UNUSED(whence);
	return -EACCES; /* FIXME: MiNT return ESPIPE here */
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_ioctl(MX_DOSFD *f, short cmd, void *buf)
{
	long r;
	struct socket *so;
	
	if (cmd == SOCKETCALL)
		return socketcall(f, buf);
	switch (cmd)
	{
	case F_GETFL:
		r = f->fd_mode & 0xfff;
		if (buf)
			*((long *)buf) = r;
		return r;
	case F_SETFL:
		f->fd_mode = (short)(*((long *)buf));
		return 0;
	case (('S' << 8) | 101):
		*((void **)buf) = f;
		return 0;
	case SIOCSPGRP:
		((struct socket *)f->fd_user1)->pgrp = (short)(*((long *)buf));
		return 0;
	case SIOCGPGRP:
		*((long *)buf) = ((struct socket *)f->fd_user1)->pgrp;
		return 0;
	default:
		/* fall through to domain ioctl */
		so = (struct socket *)f->fd_user1;
		if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
			return -ENOSYS;
		return so->ops->ioctl(so, cmd, buf);
	}
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_datime(MX_DOSFD *f, short *timeptr, short rwflag)
{
	struct socket *so = (struct socket *)f->fd_user1;
	
	if (rwflag)
	{
		so->time = timeptr[0];
		so->date = timeptr[1];
	} else
	{
		timeptr[0] = so->time;
		timeptr[1] = so->date;
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_close(MX_DOSFD *f)
{
	struct socket *so = (struct socket *)f->fd_user1;
	int pid = 0; /* hmpf */
	short flags;
	
	/* Wake anyone waiting on the socket. */
	so_wakersel(so);
	so_wakewsel(so);
	so_wakexsel(so);
	wake(IO_Q, (long)so);

	if ((f->fd_mode & O_LOCK) && ((so->lockpid == pid) /* || (f->fd_refcnt <= 0) */))
	{
		f->fd_mode &= ~O_LOCK;
		wake(IO_Q, (long)&so->lockpid);
	}
	if (f->fd_refcnt <= 1)
	{
		flags = so->flags;
		if (so_release(so) == 0)
		{
			if ((flags & SO_CLOSING) && f->fd_refcnt < 0)
				f->fd_refcnt = 0;
			p_kernel->mfree(so);
		}
	}
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_delete(MX_DOSFD *f, MX_DOSDIR *dir)
{
	UNUSED(f);
	UNUSED(dir);
	x114ce();
	p_kernel->Pfree(_BasPag);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl socket_stat(MX_DOSFD *f, MAGX_UNSEL *unsel, short rwflag, long appl)
{
	struct socket *so = (struct socket *)f->fd_user1;
	long r;
	
	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
	{
		r = 1;
	} else
	{
		r = so->ops->select(so, rwflag, appl);
	}
	if (unsel != NULL)
		unsel->unsel.status = r;
	switch (rwflag)
	{
	case O_RDONLY:
		if ((long)so->rsel == appl)
			so->rsel = 0;
		break;
	case O_WRONLY:
		if ((long)so->wsel == appl)
			so->wsel = 0;
		break;
	case O_RDWR:
		if ((long)so->xsel == appl)
			so->xsel = 0;
		break;
	}
	return r;
}

/*** ---------------------------------------------------------------------- ***/

MX_DDEV cdecl_socket_dev = {
	socket_open,
	socket_close,
	socket_read,
	socket_write,
	socket_stat,
	socket_seek,
	socket_datime,
	socket_ioctl,
	socket_delete,
	0, /* getc */
	0, /* getline */
	0  /* putc */
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

long mgx_socket(MX_DOSFD *f, short domain, short type, short protocol)
{
	(void)f;
	(void)domain;
	(void)type;
	(void)protocol;
	return 0;
}


long mgx_socketpair(MX_DOSFD *f, short domain, short type, short protocol)
{
	(void)f;
	(void)domain;
	(void)type;
	(void)protocol;
	return 0;
}


long mgx_bind(MX_DOSFD *f, void *addr, short addrlen)
{
	(void)f;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_listen(MX_DOSFD *f, short backlog)
{
	(void)f;
	(void)backlog;
	return 0;
}


long mgx_accept(MX_DOSFD *f, void *addr, short *addrlen)
{
	(void)f;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_connect(MX_DOSFD *f, void *addr, short addrlen)
{
	(void)f;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_getsockname(MX_DOSFD *f, void *addr, short *addrlen)
{
	(void)f;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_getpeername(MX_DOSFD *f, void *addr, short *addrlen)
{
	(void)f;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_send(MX_DOSFD *f, const void *buf, long buflen, short flags)
{
	(void)f;
	(void)buf;
	(void)buflen;
	(void)flags;
	return 0;
}


long mgx_sendto(MX_DOSFD *f, const void *buf, long buflen, short flags, const void *addr, short addrlen)
{
	(void)f;
	(void)buf;
	(void)buflen;
	(void)flags;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_recv(MX_DOSFD *f, void *buf, long buflen, short flags)
{
	(void)f;
	(void)buf;
	(void)buflen;
	(void)flags;
	return 0;
}


long mgx_recvfrom(MX_DOSFD *f, void *buf, long buflen, short flags, void *addr, short *addrlen)
{
	(void)f;
	(void)buf;
	(void)buflen;
	(void)flags;
	(void)addr;
	(void)addrlen;
	return 0;
}


long mgx_setsockopt(MX_DOSFD *f, short level, short optname, void *optval, long optlen)
{
	(void)f;
	(void)level;
	(void)optname;
	(void)optval;
	(void)optlen;
	return 0;
}


long mgx_getsockopt(MX_DOSFD *f, short level, short optname, void *optval, long *optlen)
{
	(void)f;
	(void)level;
	(void)optname;
	(void)optval;
	(void)optlen;
	return 0;
}


long mgx_shutdown(MX_DOSFD *f, short how)
{
	(void)f;
	(void)how;
	return 0;
}


long mgx_sendmsg(MX_DOSFD *f, const struct msghdr *msg, short flags)
{
	(void)f;
	(void)msg;
	(void)flags;
	return 0;
}


long mgx_recvmsg(MX_DOSFD *f, struct msghdr *msg, short flags)
{
	(void)f;
	(void)msg;
	(void)flags;
	return 0;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

struct socket *so_alloc(void)
{
	struct socket *s;
	
	s = p_kernel->mxalloc(sizeof(*s), MX_PREFTTRAM, _BasPag);
	if (s == NULL)
		return NULL;
	s->type = SOCK_NONE;
	s->state = SS_VIRGIN;
	s->flags = 0;
	s->conn = NULL;
	s->iconn_q = NULL;
	s->next = NULL;
	s->ops = NULL;
	s->data = NULL;
	s->error = 0;
	s->pgrp = 0;
	s->rsel = 0;
	s->wsel = 0;
	s->xsel = 0;
	/* BUG: date/time not initialized */
	s->lockpid = 0;
	return s;
}

/*** ---------------------------------------------------------------------- ***/

long so_release(struct socket *so)
{
	(void)so;
	return 0;
}
