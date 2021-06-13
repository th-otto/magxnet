/*
 *	Inet domain toplevel.
 *
 *	started 01/20/94, kay roemer.
 *	07/01/99 masquerading code by Mario Becroft.
 */

#include "sockets.h"
#include "arp.h"
#include "inetdev.h"
#include "inet.h"
#include "bpf.h"
#include "inetutil.h"
#include "port.h"
#include "timer.h"
#include "mxkernel.h"
#include "icmp.h"
#include "igmp.h"
#include "tcp.h"
#include "udp.h"
#include "masquera.h"
#include "rawip.h"

#define SIGPIPE 13

#ifndef SIOCSIFHWADDR
#define SIOCSIFHWADDR	(('S' << 8) | 49)	/* set hardware address */
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void inet_autobind(struct in_data *data)
{
	if (!(data->flags & IN_ISBOUND))
	{
		data->src.port = (data->sock->type == SOCK_RAW) ? 0 : port_alloc(data);
		data->src.addr = INADDR_ANY;
		data->flags |= IN_ISBOUND;
	}
}

/*** ---------------------------------------------------------------------- ***/

static long inet_attach(struct socket *so, short proto)
{
	struct in_data *data;
	short handler;
	long r;

	switch (so->type)
	{
	case SOCK_STREAM:
		if (proto && proto != IPPROTO_TCP)
		{
			DEBUG(("inet_attach: %d: wrong stream protocol", proto));
			return EPROTOTYPE;
		}
		handler = proto = IPPROTO_TCP;
		break;

	case SOCK_DGRAM:
		if (proto && proto != IPPROTO_UDP)
		{
			DEBUG(("inet_attach: %d: wrong dgram protocol", proto));
			return EPROTOTYPE;
		}
		handler = proto = IPPROTO_UDP;
		break;

	case SOCK_RAW:
		handler = IPPROTO_RAW;
		break;

	default:
		DEBUG(("inet_attach: %d: unsupported socket type", so->type));
		return ESOCKTNOSUPPORT;
	}

	data = in_data_create();
	if (!data)
	{
		DEBUG(("inet_attach: No mem for socket data"));
		return ENOMEM;
	}

	data->protonum = proto;
	data->proto = in_proto_lookup(handler);
	data->sock = so;
	if (!data->proto)
	{
		DEBUG(("inet_attach: %d: No such protocol", handler));
		in_data_destroy(data, 0);
		return EPROTONOSUPPORT;
	}

	r = (*data->proto->soops.attach) (data);
	if (r)
	{
		in_data_destroy(data, 0);
		return r;
	}

	so->data = data;
	in_data_put(data);
	return 0;
}

static long inet_dup(struct socket *newso, struct socket *oldso)
{
	struct in_data *data = oldso->data;

	return inet_attach(newso, data->protonum);
}

static long inet_abort(struct socket *so, enum so_state ostate)
{
	struct in_data *data = so->data;
	long r;

	r = (*data->proto->soops.abort) (data, ostate);

#ifdef NOTYET
	/* wake anyone waiting on the socket */
	wake(IO_Q, (long) so);
	so_wakersel(so);
	so_wakewsel(so);
	so_wakexsel(so);
#endif

	return r;
}

static long inet_detach(struct socket *so)
{
	struct in_data *data = so->data;
	long r;

	r = (*data->proto->soops.detach) (data, 1);
	if (!r)
		so->data = 0;

	return r;
}

static long inet_bind(struct socket *so, struct sockaddr *addr, short addrlen)
{
	struct in_data *data = so->data;
	struct sockaddr_in *inaddr = (struct sockaddr_in *) addr;
	in_addr_t saddr;
	in_port_t port;

	if (!addr)
		return EDESTADDRREQ;

	if (data->flags & IN_ISBOUND)
	{
		DEBUG(("inet_bind: already bound"));
		return EINVAL;
	}

	if (addrlen != sizeof(struct sockaddr_in))
	{
		DEBUG(("inet_bind: invalid address"));
		return EINVAL;
	}

	if (addr->sa_family != AF_INET)
	{
		DEBUG(("inet_bind: invalid adr family"));
		return EAFNOSUPPORT;
	}

	saddr = inaddr->sin_addr.s_addr;
	if (saddr != INADDR_ANY)
	{
		/* Allow bind to local broadcast address. Fixes samba's nmbd. */
		if (!ip_is_local_addr(saddr) 
#ifdef NOTYET
			&& !ip_is_brdcst_addr(saddr)
#endif
			)
		{
			DEBUG(("inet_bind: %lx: no such local IP address", saddr));
			return EADDRNOTAVAIL;
		}
	}

	port = inaddr->sin_port;
	if (so->type == SOCK_RAW)
	{
		port = 0;
	} else if (port == 0)
	{
		port = port_alloc(data);
	} else
	{
		struct in_data *data2;

		if (port < IPPORT_RESERVED && p_geteuid() != 0)
		{
			DEBUG(("inet_bind: Permission denied"));
			return EACCES;
		}
		/*
		 * Reusing of local ports is allowed if:
		 * SOCK_STREAM: All sockets with same local port have
		 *      IN_REUSE set.
		 * SOCK_DGRAM:  All sockets with same local port have
		 *      IN_REUSE set and have different local
		 *      addresses
		 * binding to the same port with different ip addresses is
		 * always allowed, e.g. a local address and INADDR_ANY.
		 * for incoming packets sockets with exact address matches
		 * are preferred over INADDR_ANY matches.
		 */
		data2 = port_find_with_addr(data, port, saddr);
		if (data2)
		{
			if (!(data->flags & IN_REUSE) || !(data2->flags & IN_REUSE))
			{
				DEBUG(("inet_bind: duplicate local address"));
				return EADDRINUSE;
			}
			if (so->type != SOCK_STREAM && (saddr == INADDR_ANY
											|| data2->src.addr == INADDR_ANY || saddr == data2->src.addr))
			{
				DEBUG(("inet_bind: duplicate local address"));
				return EADDRINUSE;
			}
		}
	}

	data->src.addr = saddr;
	data->src.port = port;
	data->flags |= IN_ISBOUND;

	return 0;
}

static long inet_connect(struct socket *so, const struct sockaddr *addr, short addrlen, short nonblock)
{
	struct in_data *data = so->data;

	if (so->state == SS_ISCONNECTING)
		return EALREADY;

	if (addrlen != sizeof(struct sockaddr_in) || !addr)
	{
		DEBUG(("inet_connect: invalid address"));
		if (so->type != SOCK_STREAM)
			data->flags &= ~IN_ISCONNECTED;

		return EINVAL;
	}
	if (addr->sa_family != AF_INET)
	{
		DEBUG(("inet_connect: invalid adr family"));
		if (so->type != SOCK_STREAM)
			data->flags &= ~IN_ISCONNECTED;

		return EAFNOSUPPORT;
	}
	inet_autobind(data);
	return (*data->proto->soops.connect) (data, (const struct sockaddr_in *) addr, addrlen, nonblock);
}

static long inet_socketpair(struct socket *so1, struct socket *so2)
{
	UNUSED(so1);
	UNUSED(so2);
	return EOPNOTSUPP;
}

static long inet_accept(struct socket *server, struct socket *newso, short nonblock)
{
	struct in_data *sdata = server->data;
	struct in_data *cdata = newso->data;

	return (*sdata->proto->soops.accept) (sdata, cdata, nonblock);
}

static long inet_getname(struct socket *so, struct sockaddr *addr, short *addrlen, short peer)
{
	struct in_data *data = so->data;
	struct sockaddr_in in;
	long todo;

	if (!addr || !addrlen || *addrlen < 0)
	{
		DEBUG(("inet_getname: invalid addr/addrlen"));
		return EINVAL;
	}

	in.sin_family = AF_INET;
	if (peer == PEER_ADDR)
	{
		if (!(data->flags & IN_ISCONNECTED))
		{
			DEBUG(("inet_getname: not connected"));
			return ENOTCONN;
		}
		in.sin_port = data->dst.port;
		in.sin_addr.s_addr = data->dst.addr;
	} else
	{
		inet_autobind(data);
		in.sin_port = data->src.port;
		in.sin_addr.s_addr = (data->src.addr != INADDR_ANY)
			? data->src.addr : ip_local_addr((data->flags & IN_ISCONNECTED) ? data->dst.addr : INADDR_ANY);
	}

	todo = MIN(*addrlen, (long)sizeof(in));
	mint_bzero(in.sin_zero, sizeof(in.sin_zero));
	memcpy(addr, &in, todo);
	*addrlen = todo;

	return 0;
}

static long inet_select(struct socket *so, short how, long proc)
{
	struct in_data *data = so->data;

	if (so->type == SOCK_RAW)
		inet_autobind(data);

	return (*data->proto->soops.select) (data, how, proc);
}

static long inet_ioctl(struct socket *so, short cmd, void *buf)
{
	struct in_data *data = so->data;

	switch (cmd)
	{
	case SIOCSIFLINK:
	case SIOCGIFNAME:
	case SIOCGIFCONF:
	case SIOCGIFFLAGS:
	case SIOCSIFFLAGS:
	case SIOCGIFMETRIC:
	case SIOCSIFMETRIC:
	case SIOCSIFMTU:
	case SIOCGIFMTU:
	case SIOCSIFADDR:
	case SIOCGIFADDR:
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
	case SIOCGIFSTATS:
	case SIOCGLNKFLAGS:
	case SIOCSLNKFLAGS:
#ifdef NOTYET
	case SIOCSIFHWADDR:
#endif
	case SIOCGIFHWADDR:
	case SIOCGLNKSTATS:
	case SIOCSIFOPT:
		return if_ioctl(cmd, (long) buf);

	case SIOCADDRT:
	case SIOCDELRT:
		return route_ioctl(cmd, (long) buf);

	case SIOCDARP:
	case SIOCGARP:
	case SIOCSARP:
		return arp_ioctl(cmd, buf);
	}

	return (*data->proto->soops.ioctl) (data, cmd, buf);
}

static long inet_listen(struct socket *so, short backlog)
{
	struct in_data *data = so->data;

	if (so->type != SOCK_STREAM)
	{
		DEBUG(("inet_listen: Not supp. for datagram sockets"));
		return EOPNOTSUPP;
	}
	inet_autobind(data);
	data->backlog = backlog;
	return (*data->proto->soops.listen) (data);
}

static long
inet_send(struct socket *so, const struct iovec *iov, short niov, short nonblock,
		  short flags, const struct sockaddr *addr, short addrlen)
{
	struct in_data *data = so->data;
	long r;

	if (so->state == SS_ISDISCONNECTING || so->state == SS_ISDISCONNECTED)
	{
		DEBUG(("inet_send: Socket shut down"));
		p_kill(p_getpid(), SIGPIPE);
		return EPIPE;
	}

	if (data->err)
	{
		r = data->err;
		data->err = 0;
		return r;
	}

	if (so->flags & SO_CANTSNDMORE)
	{
		DEBUG(("inet_send: shut down"));
		p_kill(p_getpid(), SIGPIPE);
		return EPIPE;
	}

	if (addr)
	{
		if (addrlen != sizeof(struct sockaddr_in))
		{
			DEBUG(("inet_send: invalid address"));
			return EINVAL;
		}
		if (addr->sa_family != AF_INET)
		{
			DEBUG(("inet_send: invalid adr family"));
			return EAFNOSUPPORT;
		}
	}

	inet_autobind(data);
	return (*data->proto->soops.send) (data, iov, niov, nonblock, flags, (const struct sockaddr_in *) addr, addrlen);
}

static long
inet_recv(struct socket *so, const struct iovec *iov, short niov, short nonblock,
		  short flags, struct sockaddr *addr, short *addrlen)
{
	struct in_data *data = so->data;
	long r;

	if (so->state == SS_ISDISCONNECTING || so->state == SS_ISDISCONNECTED)
	{
		DEBUG(("inet_recv: Socket shut down"));
		return 0;
	}

	if (data->err)
	{
		r = data->err;
		data->err = 0;
		return r;
	}

	inet_autobind(data);
	return (*data->proto->soops.recv) (data, iov, niov, nonblock, flags, (struct sockaddr_in *) addr, addrlen);
}

static long inet_shutdown(struct socket *so, short how)
{
	struct in_data *data = so->data;
	long r;

	inet_autobind(data);
	r = (*data->proto->soops.shutdown) (data, how);

#define SO_CANTDOMORE	(SO_CANTSNDMORE|SO_CANTRCVMORE)

	/* Note that sock_shutdown() has already set so->flags for us. */
	if ((so->flags & SO_CANTDOMORE) == SO_CANTDOMORE)
	{
		DEBUG(("inet_shutdown: releasing socket"));
		so_release(so);
	}

	return r;
}

static long inet_setsockopt(struct socket *so, short level, short optname, char *optval, long optlen)
{
	struct in_data *data = so->data;
	long val;

	switch (level)
	{
	case IPPROTO_IP:					/* SOL_IP */
		return ip_setsockopt(&data->opts, level, optname, optval, optlen);

	case (short)SOL_SOCKET:
		break;

	default:
		return (*data->proto->soops.setsockopt) (data, level, optname, optval, optlen);
	}

#ifdef NOTYET
	if (optval == NULL)
	{
		DEBUG(("inet_setsockopt: invalid optval"));
		return EFAULT;
	}
	if ((unsigned long)optlen >= sizeof(long))
	{
		val = *((long *) optval);
	} else if ((unsigned long)optlen >= sizeof(short))
	{
		val = *((short *) optval);
	} else if ((unsigned long)optlen >= sizeof(char))
	{
		val = *((unsigned char *) optval);
	} else
	{
		DEBUG(("inet_setsockopt: invalid optval/optlen"));
		return EINVAL;
	}
#else
	if (optval == NULL || (unsigned long)optlen < sizeof(long))
	{
		DEBUG(("inet_setsockopt: invalid optval/optlen"));
		return EINVAL;
	}
	val = *((long *) optval);
#endif

	switch (optname)
	{
	case SO_DEBUG:
		break;

	case SO_REUSEADDR:
		if (val)
			data->flags |= IN_REUSE;
		else
			data->flags &= ~IN_REUSE;
		break;

	case SO_DONTROUTE:
		if (val)
			data->flags |= IN_DONTROUTE;
		else
			data->flags &= ~IN_DONTROUTE;
		break;

	case SO_BROADCAST:
		if (val)
			data->flags |= IN_BROADCAST;
		else
			data->flags &= ~IN_BROADCAST;
		break;

	case SO_SNDBUF:
		if (val > IN_MAX_WSPACE)
			val = IN_MAX_WSPACE;
		else if (val < IN_MIN_WSPACE)
			val = IN_MIN_WSPACE;

		if (so->type == SOCK_STREAM && val < data->snd.curdatalen)
		{
			DEBUG(("inet_setsockopt: sndbuf size invalid"));
			return EINVAL;
		}
		data->snd.maxdatalen = val;
		break;

	case SO_RCVBUF:
		if (val > IN_MAX_RSPACE)
			val = IN_MAX_RSPACE;
		else if (val < IN_MIN_RSPACE)
			val = IN_MIN_RSPACE;

		if (so->type == SOCK_STREAM && val < data->rcv.curdatalen)
		{
			DEBUG(("inet_setsockopt: rcvbuf size invalid"));
			return EINVAL;
		}
		data->rcv.maxdatalen = val;
		break;

	case SO_KEEPALIVE:
		if (val)
			data->flags |= IN_KEEPALIVE;
		else
			data->flags &= ~IN_KEEPALIVE;
		break;

	case SO_OOBINLINE:
		if (val)
			data->flags |= IN_OOBINLINE;
		else
			data->flags &= ~IN_OOBINLINE;
		break;

	case SO_LINGER:
		{
			struct linger l;

			if ((unsigned long)optlen < sizeof(struct linger))
			{
				DEBUG(("inet_setsockopt: optlen to small"));
				return EINVAL;
			}

			l = *(struct linger *) optval;
			if (l.l_onoff)
			{
				data->flags |= IN_LINGER;
				data->linger = l.l_linger;
			} else
			{
				data->flags &= ~IN_LINGER;
			}
		}
		break;
	case SO_CHKSUM:
		if (val)
			data->flags |= IN_CHECKSUM;
		else
			data->flags &= ~IN_CHECKSUM;
		break;

	default:
		DEBUG(("inet_setsockopt: %d: invalid option", optname));
		return EOPNOTSUPP;
	}

	return 0;
}

static long inet_getsockopt(struct socket *so, short level, short optname, char *optval, long *optlen)
{
	struct in_data *data = so->data;
	long val;

	switch (level)
	{
	case IPPROTO_IP:
		return ip_getsockopt(&data->opts, level, optname, optval, optlen);

	case (short)SOL_SOCKET:
		break;

	default:
		return (*data->proto->soops.getsockopt) (data, level, optname, optval, optlen);
	}

	if (!optlen || !optval)
	{
		DEBUG(("inet_getsockopt: invalid optval/optlen"));
		return EINVAL;
	}

	switch (optname)
	{
	case SO_DEBUG:
		val = 0;
		break;

	case SO_TYPE:
		val = so->type;
		break;

	case SO_ERROR:
		val = data->err;
		data->err = 0;
		break;

	case SO_REUSEADDR:
		val = (data->flags & IN_REUSE) ? 1 : 0;
		break;

	case SO_KEEPALIVE:
		val = (data->flags & IN_KEEPALIVE) ? 1 : 0;
		break;

	case SO_DONTROUTE:
		val = (data->flags & IN_DONTROUTE) ? 1 : 0;
		break;

	case SO_LINGER:
		{
			struct linger l;

			if ((unsigned long)*optlen < sizeof(struct linger))
			{
				DEBUG(("inet_setsockopt: optlen < sizeof linger"));
				return EINVAL;
			}

			if (data->flags & IN_LINGER)
			{
				l.l_onoff = 1;
				l.l_linger = data->linger;
			} else
			{
				l.l_onoff = 0;
				l.l_linger = 0;
			}

			*(struct linger *) optval = l;
			*optlen = sizeof(struct linger);

			return 0;
		}
	case SO_BROADCAST:
		val = (data->flags & IN_BROADCAST) ? 1 : 0;
		break;

	case SO_OOBINLINE:
		val = (data->flags & IN_OOBINLINE) ? 1 : 0;
		break;

	case SO_SNDBUF:
		val = data->snd.maxdatalen;
		break;

	case SO_RCVBUF:
		val = data->rcv.maxdatalen;
		break;

	case SO_CHKSUM:
		val = (data->flags & IN_CHECKSUM) ? 1 : 0;
		break;

#ifdef IGMP_SUPPORT
	case SO_ACCEPTCONN:
		return (*data->proto->soops.getsockopt) (data, level, optname, optval, optlen);
#endif

	default:
		return EOPNOTSUPP;
	}

#ifdef NOTYET
	if (*optlen == sizeof(short))
	{
		*((short *) optval) = val;
	} else if (*optlen == sizeof(char))
	{
		*((unsigned char *) optval) = val;
	} else if (*optlen == sizeof(long))
	{
		*((long *) optval) = val;
	} else
	{
		DEBUG(("inet_getsockopt: optlen < sizeof long"));
		return EINVAL;
	}
#else
	if ((unsigned long)*optlen < sizeof(long))
	{
		DEBUG(("inet_getsockopt: optlen < sizeof long"));
		return EINVAL;
	}
	*((long *) optval) = val;
	*optlen = sizeof(long);
#endif

	return 0;
}


static struct dom_ops inet_ops = {
	AF_INET,
	NULL,
	inet_attach,
	inet_dup,
	inet_abort,
	inet_detach,
	inet_bind,
	inet_connect,
	inet_socketpair,
	inet_accept,
	inet_getname,
	inet_select,
	inet_ioctl,
	inet_listen,
	inet_send,
	inet_recv,
	inet_shutdown,
	inet_setsockopt,
	inet_getsockopt
};


void inet4_init(void)
{
	/* initialize buf allocator */
	buf_init();

	/* load all interfaces */
	if_init();
	
	/* initialize IP router & control device */
	route_init();
	
	/* initialize raw IP driver; must be first */
	rip_init();
	
	/* initialize ICMP protocol */
	icmp_init();

	/* initialize UDP protocol */
	udp_init();

	/* initialize TCP protocol */
	tcp_init();
#ifdef IGMP_SUPPORT
	/* initialize IGMP protocol */
	igmp_init();
#endif

	/* register our domain */
	so_register(AF_INET, &inet_ops);
	inetdev_init();

	/* initialize masquerade support */
	masq_init();
}
