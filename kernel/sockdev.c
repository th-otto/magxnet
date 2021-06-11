#include "sockets.h"
#include "mxkernel.h"
#include "sockdev.h"
#include "mintsock.h"
#include <pcerrno.h>
#include <fcntl.h>

#define SIOCGDOSFD (('S' << 8) | 101) /* MagiCNet extension: returns dos file descriptor */


static long mgx_socket(MX_DOSFD *f, short domain, short type, short protocol);
static long mgx_socketpair(MX_DOSFD *f, short domain, short type, short protocol);
static long mgx_bind(MX_DOSFD *f, void *addr, short addrlen);
static long mgx_listen(MX_DOSFD *f, short backlog);
static long mgx_accept(MX_DOSFD *f, void *addr, short *addrlen);
static long mgx_connect(MX_DOSFD *f, void *addr, short addrlen);
static long mgx_getsockname(MX_DOSFD *f, void *addr, short *addrlen);
static long mgx_getpeername(MX_DOSFD *f, void *addr, short *addrlen);
static long mgx_send(MX_DOSFD *f, const void *buf, long buflen, short flags);
static long mgx_sendto(MX_DOSFD *f, const void *buf, long buflen, short flags, const void *addr, short addrlen);
static long mgx_recv(MX_DOSFD *f, void *buf, long buflen, short flags);
static long mgx_recvfrom(MX_DOSFD *f, void *buf, long buflen, short flags, void *addr, short *addrlen);
static long mgx_setsockopt(MX_DOSFD *f, short level, short optname, void *optval, long optlen);
static long mgx_getsockopt(MX_DOSFD *f, short level, short optname, void *optval, long *optlen);
static long mgx_shutdown(MX_DOSFD *f, short how);
static long mgx_sendmsg(MX_DOSFD *f, const struct msghdr *msg, short flags);
static long mgx_recvmsg(MX_DOSFD *f, struct msghdr *msg, short flags);

static long so_create(MX_DOSFD **f);

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
		/*
		 * BUG: Incompatible to MiNT:
		 * in MiNT, the arg is used as value, not as pointer to a value
		 */
		f->fd_mode = (short)(*((long *)buf));
		return 0;
	case SIOCGDOSFD:
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
			return -ENOSYS; /* BUG: should be EINVAL */
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

static long mgx_socket(MX_DOSFD *f, short domain, short type, short protocol)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct dom_ops *ops;
	long ret;

	if (so->state != SS_VIRGIN)
		return -ENOSYS;
	for (ops = alldomains; ops != NULL; ops = ops->next)
		if (ops->domain == domain)
			break;
	if (ops == NULL)
	{
		DEBUG(("so_create: domain %d not supported", domain));
		return -EAFNOSUPPORT;
	}
	switch (type)
	{
	case SOCK_DGRAM:
	case SOCK_STREAM:
	case SOCK_RAW:
	case SOCK_RDM:
	case SOCK_SEQPACKET:
		so->ops = ops;
		so->type = type;

		ret = (*so->ops->attach) (so, protocol);
		if (ret < 0)
		{
			DEBUG(("so_create: failed to attach protocol data (%li)", ret));
			return ret;
		}
		so->state = SS_ISUNCONNECTED;
		return 0;
	default:
		break;
	}

	return -ESOCKTNOSUPPORT;
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_socketpair(MX_DOSFD *f, short domain, short type, short protocol)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct socket *so2;
	long r;
	long fd;
	MX_DOSFD *pair;
	
	r = mgx_socket(f, domain, type, protocol);
	if (r < 0)
		return r;
	fd = so_create(&pair);
	if (fd < 0)
	{
		so_release(so);
		return fd;
	}
	r = mgx_socket(pair, domain, type, protocol);
	if (r < 0)
	{
		so_release(so);
		Fclose((int)fd);
		return r;
	}
	so2 = (struct socket *)pair->fd_user1;
	r = so->ops->socketpair(so, so2);
	if (r < 0)
	{
		so_release(so);
		Fclose((int)fd);
		return r;
	}
	return fd;
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_bind(MX_DOSFD *f, void *addr, short addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;
	
	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */
	
	return so->ops->bind(so, addr, addrlen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_listen(MX_DOSFD *f, short backlog)
{
	struct socket *so = (struct socket *)f->fd_user1;
	long r;
	
	if (so->state != SS_ISUNCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */
	if (backlog < 0)
		backlog = 0;
	r = so->ops->listen(so, backlog);
	if (r < 0)
	{
		DEBUG(("sys_listen: failing ..."));
		return r;
	}
	so->flags |= SO_ACCEPTCON;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static void so_drop(MX_DOSFD *f)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct socket *newso;
	
	if (!(so->flags & SO_DROP))
		return;
	newso = so_alloc();
	if (newso == NULL)
		return;
	newso->type = so->type;
	newso->ops = so->ops;
	if (so->ops->dup(newso, so) >= 0)
	{
		newso->state = SO_ACCEPTCON;
		so->ops->accept(so, newso, f->fd_mode & O_NDELAY);
		so_release(newso);
	}
	p_kernel->mfree(newso);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_accept(MX_DOSFD *f, void *addr, short *addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct socket *newso;
	MX_DOSFD *newfp;
	long fd;
	long ret;
	
	if (so->state != SS_ISUNCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */
	if (!(so->flags & SO_ACCEPTCON))
	{
		DEBUG(("accept: socket not listening"));
		return -ENOSYS; /* BUG: should be EINVAL */
	}
	
	fd = so_create(&newfp);
	if (fd < 0)
	{
		so_drop(f);
		return fd;
	}
	newso = (struct socket *)newfp->fd_user1;
	newso->type = so->type;
	newso->ops = so->ops;
	ret = so->ops->dup(newso, so);
	if (ret < 0)
	{
		Fclose((int)fd);
		so_drop(f);
		return ret;
	}
	newso->state = SO_ACCEPTCON;
	ret = so->ops->accept(so, newso, f->fd_mode & O_NDELAY);
	if (ret < 0)
	{
		Fclose((int)fd);
		DEBUG(("accept: cannot accept a connection"));
		return ret;
	}
	if (addr)
	{
		ret = (*newso->ops->getname)(newso, addr, addrlen, PEER_ADDR);
		if (ret < 0)
		{
			DEBUG (("sys_accept: getname failed"));
			*addrlen = 0;
		}
	}
	
	return fd;
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_connect(MX_DOSFD *f, void *addr, short addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;

	switch (so->state)
	{
	case SS_ISUNCONNECTED:
	case SS_ISCONNECTING:
		if (so->flags & SO_ACCEPTCON)
		{
			DEBUG(("connect: attempt to connect a listening socket"));
			return -ENOSYS; /* BUG: should be EINVAL */
		}
		return so->ops->connect(so, addr, addrlen, f->fd_mode & O_NDELAY);
	case SS_ISCONNECTED:
		/* Connectionless sockets can be connected several
		 * times. So their state must always be
		 * SS_ISUNCONNECTED.
		 */
		DEBUG(("connect: already connected"));
		return -EISCONN;
	case SS_ISDISCONNECTING:
	case SS_ISDISCONNECTED:
	case SS_VIRGIN:
		DEBUG(("connect: socket cannot connect"));
		return -ENOSYS; /* BUG: should be EINVAL */
	}

	DEBUG(("connect: invalid socket state %d", so->state));
	return -EINTERNAL;
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_getsockname(MX_DOSFD *f, void *addr, short *addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */

	return so->ops->getname(so, addr, addrlen, SOCK_ADDR);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_getpeername(MX_DOSFD *f, void *addr, short *addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */

	return so->ops->getname(so, addr, addrlen, PEER_ADDR);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_send(MX_DOSFD *f, const void *buf, long buflen, short flags)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct iovec iov[1];

	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	iov[0].iov_base = NO_CONST(buf);
	iov[0].iov_len = buflen;
	return so->ops->send(so, iov, 1, f->fd_mode & O_NDELAY, flags, NULL, 0);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_sendto(MX_DOSFD *f, const void *buf, long buflen, short flags, const void *addr, short addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct iovec iov[1];

	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	iov[0].iov_base = NO_CONST(buf);
	iov[0].iov_len = buflen;
	return so->ops->send(so, iov, 1, f->fd_mode & O_NDELAY, flags, addr, addrlen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_recv(MX_DOSFD *f, void *buf, long buflen, short flags)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct iovec iov[1];

	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	iov[0].iov_base = buf;
	iov[0].iov_len = buflen;
	return so->ops->recv(so, iov, 1, f->fd_mode & O_NDELAY, flags, NULL, 0);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_recvfrom(MX_DOSFD *f, void *buf, long buflen, short flags, void *addr, short *addrlen)
{
	struct socket *so = (struct socket *)f->fd_user1;
	struct iovec iov[1];

	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	iov[0].iov_base = buf;
	iov[0].iov_len = buflen;
	return so->ops->recv(so, iov, 1, f->fd_mode & O_NDELAY, flags, addr, addrlen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_setsockopt(MX_DOSFD *f, short level, short optname, void *optval, long optlen)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */
	
	if (level == (short)SOL_SOCKET)
	{
		switch (optname)
		{
		case SO_DROPCONN:
			if (!optval || optlen < sizeof(long))
				return -ENOSYS; /* BUG: should be EINVAL */

			if (*(long *) optval)
				so->flags |= SO_DROP;
			else
				so->flags &= ~SO_DROP;

			return 0;
		default:
			break;
		}
	}	
	return so->ops->setsockopt(so, level, optname, optval, optlen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_getsockopt(MX_DOSFD *f, short level, short optname, void *optval, long *optlen)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
	{
		DEBUG(("so_getsockopt: virgin state -> EINVAL"));
		return -ENOSYS; /* BUG: should be EINVAL */
	}
	
	if (level == (short)SOL_SOCKET)
	{
		switch (optname)
		{
		case SO_DROPCONN:
			if (!optval || !optlen || *optlen < sizeof(long))
				return -ENOSYS; /* BUG: should be EINVAL */

			*(long *) optval = (so->flags & SO_DROP) != 0;
			*optlen = sizeof(long);

			return 0;
		default:
			break;
		}
	}

	return so->ops->getsockopt(so, level, optname, optval, optlen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_shutdown(MX_DOSFD *f, short how)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN || so->state == SS_ISDISCONNECTED)
		return -ENOSYS; /* BUG: should be EINVAL */
	
	switch (how)
	{
	case 0:
		so->flags |= SO_CANTRCVMORE;
		break;
	case 1:
		so->flags |= SO_CANTSNDMORE;
		break;
	case 2:
		so->flags |= SO_CANTRCVMORE | SO_CANTSNDMORE;
		break;
#if 0 /* BUG: missing */
	default:
		return -ENOSYS;
#endif
	}
	
	return so->ops->shutdown(so, how);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_sendmsg(MX_DOSFD *f, const struct msghdr *msg, short flags)
{
	struct socket *so = (struct socket *)f->fd_user1;

	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	if (msg->msg_control && msg->msg_controllen)
	{
#if 0
		msg->msg_control = NULL;
		msg->msg_controllen = 0;
#else
		return -ENOSYS; /* BUG: should be EINVAL */
#endif
	}
	
	return so->ops->send(so, msg->msg_iov, msg->msg_iovlen,
					f->fd_mode & O_NDELAY, flags,
					msg->msg_name, msg->msg_namelen);
}

/*** ---------------------------------------------------------------------- ***/

static long mgx_recvmsg(MX_DOSFD *f, struct msghdr *msg, short flags)
{
	struct socket *so = (struct socket *)f->fd_user1;
	long r;
	short addrlen;
	
	addrlen = msg->msg_namelen;
	if (so->state == SS_VIRGIN)
		return -ENOSYS; /* BUG: should be EINVAL */

	if (msg->msg_control && msg->msg_controllen)
	{
		msg->msg_controllen = 0;
	}
	
	r = so->ops->recv(so, msg->msg_iov, msg->msg_iovlen,
					f->fd_mode & O_NDELAY, flags,
					msg->msg_name, &addrlen);
	msg->msg_namelen = addrlen;
	return r;
}

/*** ---------------------------------------------------------------------- ***/

static long so_create(MX_DOSFD **f)
{
	long fd;
	long r;

	/* note: reentering GEMDOS here */	
	fd = Fopen(socket_devname, O_RDWR);
	if (fd >= 0)
	{
		r = Fcntl(fd, (long)f, SIOCGDOSFD);
		if (r < 0)
		{
			Fclose((int)fd);
			return r;
		}
	}
	return fd;
}
