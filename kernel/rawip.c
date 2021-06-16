/*
 *	Raw IP driver.
 *
 *	07/21/94, kay roemer.
 */

#include "sockets.h"
#include "rawip.h"
#include <fcntl.h>

#include "if.h"
#include <netinet/in.h>
#include "inet.h"
#include "inetutil.h"
#include "ip.h"
#include "bpf.h"
#include "mxkernel.h"

#include "buf.h"
#include "iov.h"

#define IS_INET_PROTO(p) ((p) == IPPROTO_ICMP || (p) == IPPROTO_TCP || (p) == IPPROTO_UDP)


#define RIP_RESERVE	100

static long rip_attach(struct in_data *);
static long rip_abort(struct in_data *, short);
static long rip_detach(struct in_data *, short);
static long rip_connect(struct in_data *, const struct sockaddr_in *, short, short);
static long rip_accept(struct in_data *, struct in_data *, short);
static long rip_ioctl(struct in_data *, short, void *);
static long rip_select(struct in_data *, short, long);
static long rip_send(struct in_data *, const struct iovec *, short, short, short, const struct sockaddr_in *, short);
static long rip_recv(struct in_data *, const struct iovec *, short, short, short, struct sockaddr_in *, short *);
static long rip_shutdown(struct in_data *, short);
static long rip_setsockopt(struct in_data *, short, short, char *, long);
static long rip_getsockopt(struct in_data *, short, short, char *, long *);

static long rip_error(short, short, BUF *, ulong, ulong);
static long rip_input(struct netif *, BUF *, ulong, ulong);

struct in_proto rip_proto = {
	IPPROTO_RAW,
	0,
	NULL,
	{
		rip_attach,
		rip_abort,
		rip_detach,
		rip_connect,
		NULL,
		rip_accept,
		rip_ioctl,
		rip_select,
		rip_send,
		rip_recv,
		rip_shutdown,
		rip_setsockopt,
		rip_getsockopt
	}, {
		IPPROTO_RAW,
		NULL,
		rip_error,
		rip_input
	},
	NULL
};

#ifdef __PUREC__
#pragma warn -rch /* for p_geteuid */
#endif


/*
 * For GNU-C, this function is usually inlined,
 * but does not have to be
 */
/* FIXME: duplicate */
#define iov_size rip_iov_size
#define IOV_MAX 16
static long iov_size(const struct iovec *iov, short n)
{
	long size;

	if (n <= 0 || n > IOV_MAX)
		return -1;
	
	for (size = 0; n; ++iov, --n)
	{
		if ((long)iov->iov_len < 0)
			return -1;
		
		size += iov->iov_len;
	}
	
	return size;
}


static long rip_attach(struct in_data *data)
{
	if (p_geteuid() != 0)
		return EACCES;

	data->pcb = 0;
	return 0;
}


static long rip_abort(struct in_data *data, short ostate)
{
	UNUSED(data);
	UNUSED(ostate);
	return 0;
}

static long rip_detach(struct in_data *data, short wait)
{
	in_data_destroy(data, wait);
	return 0;
}

static long rip_connect(struct in_data *data, const struct sockaddr_in *addr, short addrlen, short nonblock)
{
	UNUSED(addrlen);
	UNUSED(nonblock);
	data->dst.addr = ip_dst_addr(addr->sin_addr.s_addr);
	data->dst.port = 0;
	data->flags |= IN_ISCONNECTED;
	return 0;
}

static long rip_accept(struct in_data *data, struct in_data *newdata, short nonblock)
{
	UNUSED(data);
	UNUSED(newdata);
	UNUSED(nonblock);
	return EOPNOTSUPP;
}

static long rip_ioctl(struct in_data *data, short cmd, void *buf)
{
	BUF *b;

	switch (cmd)
	{
	case FIONREAD:
		{
			if ((data->sock->flags & SO_CANTRCVMORE) || data->err)
			{
				*(long *) buf = UNLIMITED;
				return 0;
			}

			if (!data->rcv.qfirst)
			{
				*(long *) buf = 0;
				return 0;
			}

			b = data->rcv.qfirst;
			*(long *) buf = b->dend - b->dstart;

			return 0;
		}
	case FIONWRITE:
		{
			*(long *) buf = UNLIMITED;
			return 0;
		}
	}

	return ENOSYS;
}

static long rip_select(struct in_data *data, short mode, long proc)
{
	switch (mode)
	{
	case O_WRONLY:
		return 1;

	case O_RDONLY:
		if ((data->sock->flags & SO_CANTRCVMORE) || data->err)
			return 1;

		return (data->rcv.qfirst ? 1 : so_rselect(data->sock, proc));
	}

	return 0;
}

static long rip_send(struct in_data *data, const struct iovec *iov, short niov, short nonblock, short flags,
	const struct sockaddr_in *addr, short addrlen)
{
	long size;
	long r;
	long copied;
	ulong dstaddr;
	short ipflags = 0;
	BUF *buf;

	UNUSED(nonblock);
	UNUSED(addrlen);
	if (flags & ~MSG_DONTROUTE)
	{
		DEBUG(("rip_send: invalid flags"));
		return EOPNOTSUPP;
	}

	size = iov_size(iov, niov);
	if (size == 0)
		return 0;

	if (size < 0)
	{
		DEBUG(("rip_send: Invalid iovec"));
		return EINVAL;
	}

	if (size > data->snd.maxdatalen)
	{
		DEBUG(("rip_send: Message too long"));
		return EMSGSIZE;
	}

	if (data->flags & IN_ISCONNECTED)
	{
		if (addr)
			return EISCONN;
		dstaddr = data->dst.addr;
	} else
	{
		if (!addr)
			return EDESTADDRREQ;
		dstaddr = ip_dst_addr(addr->sin_addr.s_addr);
	}

	buf = buf_alloc(size + RIP_RESERVE, RIP_RESERVE / 2, BUF_NORMAL);
	if (!buf)
	{
		DEBUG(("rip_send: Out of mem"));
		return ENOMEM;
	}

	copied = iov2buf_cpy(buf->dstart, size, iov, niov, 0);
	buf->dend += size;

	if (data->flags & IN_BROADCAST)
		ipflags |= IP_BROADCAST;

	if ((data->flags & IN_DONTROUTE) || flags & MSG_DONTROUTE)
		ipflags |= IP_DONTROUTE;

	if (data->opts.hdrincl || data->protonum == IPPROTO_RAW)
	{
		struct ip_dgram *iph = (struct ip_dgram *) buf->dstart;

		iph->version = IP_VERSION;
		iph->hdrlen = (unsigned int)(sizeof(*iph) / sizeof(long));
		iph->id = ip_dgramid++;
		buf->info = ip_priority(0, IPH_TOS(iph));
		r = ip_output(buf);
	} else
	{
		r = ip_send(data->src.addr, dstaddr, buf, data->protonum, ipflags, &data->opts);
	}

#ifdef NOTYET /* 3535e0d6a8f193c27ae189e5ca7eaf618e0641ba */
	if (r == 0)
		r = copied;

	return r;
#else
	return r ? r : copied;
#endif
}


static long rip_recv(struct in_data *data, const struct iovec *iov, short niov, short nonblock, short flags,
	 struct sockaddr_in *addr, short *addrlen)
{
	struct socket *so = data->sock;
	long size;
	long todo;
	long copied;
	BUF *buf;

	size = iov_size(iov, niov);
	if (size == 0)
		return 0;

	if (size < 0)
	{
		DEBUG(("rip_recv: invalid iovec"));
		return EINVAL;
	}

	if (addr && (!addrlen || *addrlen < 0))
	{
		DEBUG(("rip_recv: invalid address len"));
		return EINVAL;
	}

	while (!data->rcv.qfirst)
	{
		int i;

#ifdef NOTYET /* 3535e0d6a8f193c27ae189e5ca7eaf618e0641ba */
		if (nonblock)
		{
			DEBUG(("rip_recv: EAGAIN"));
			return EAGAIN;
		}

		if (so->flags & SO_CANTRCVMORE)
		{
			DEBUG(("rip_recv: shut down"));
			return 0;
		}
#else
		if (nonblock || (so->flags & SO_CANTRCVMORE))
		{
			DEBUG(("rip_recv: shut down"));
			return 0;
		}
#endif

		i = sleep(IO_Q, (long) so);
		if (i)
		{
			DEBUG(("rip_recv: interrupted (%i)", i));
			return EINTR;
		}

		if (so->state != SS_ISUNCONNECTED)
		{
			DEBUG(("rip_recv: Socket shut down while sleeping"));
			return 0;
		}

		if (data->err)
		{
			copied = data->err;
			data->err = 0;
			return copied;
		}
	}

	buf = data->rcv.qfirst;
	todo = buf->dend - buf->dstart;
	copied = buf2iov_cpy(buf->dstart, todo, iov, niov, 0);

	if (addr)
	{
		struct sockaddr_in in;

		*addrlen = MIN(*addrlen, (short)sizeof(struct sockaddr_in));
		in.sin_family = AF_INET;
		in.sin_addr.s_addr = IP_SADDR(buf);
		in.sin_port = 0;
		memcpy(addr, &in, *addrlen);
	}

	if (!(flags & MSG_PEEK))
	{
		if (!buf->next)
		{
			data->rcv.qfirst = data->rcv.qlast = 0;
			data->rcv.curdatalen = 0;
		} else
		{
			data->rcv.qfirst = buf->next;
			data->rcv.curdatalen -= todo;
			buf->next->prev = 0;
		}

		buf_deref(buf, BUF_NORMAL);
	}

	return copied;
}

static long rip_shutdown(struct in_data *data, short how)
{
	UNUSED(data);
	UNUSED(how);
	return 0;
}

static long rip_setsockopt(struct in_data *data, short level, short optname, char *optval, long optlen)
{
	UNUSED(data);
	UNUSED(level);
	UNUSED(optname);
	UNUSED(optval);
	UNUSED(optlen);
	return EOPNOTSUPP;
}

static long rip_getsockopt(struct in_data *data, short level, short optname, char *optval, long *optlen)
{
	UNUSED(data);
	UNUSED(level);
	UNUSED(optname);
	UNUSED(optval);
	UNUSED(optlen);
	return EOPNOTSUPP;
}

/*
 * RAW Input: Every matching socket gets a copy of buf. A socket `matches' if:
 * 1) Local address specified and local addresses match or no local address
 *    specified.
 * 2) Foreign address specified and foreign addresses match or no foreign
 *    address specified.
 * 3) IP protocol nonzero and packets protocol matches or protocol zero.
 *
 * NOTE that all raw sockets have their port numbers always set to zero. This
 * is special cased in inet.c.
 * Therefore in_data_lookup_next (data, saddr, 0, daddr, 0, 1).
 *					       ^	 ^
 * We assume that rip_input() is the last handler in the chain.
 */

static long rip_input(struct netif *iface, BUF *buf, ulong saddr, ulong daddr)
{
	short proto;
	short found = 0;
	short delivered = 0;
	struct in_data *d;
	long pktlen;
	BUF *nbuf;

	UNUSED(iface);
	pktlen = buf->dend - buf->dstart;
	d = rip_proto.datas;
	proto = IP_PROTO(buf);
	for (; (d = in_data_lookup_next(d, saddr, 0, daddr, 0, 1)) != NULL; d = d->next)
	{
		if (d->protonum != 0 && d->protonum != proto)
			continue;
		++found;
		if (pktlen + d->rcv.curdatalen > d->rcv.maxdatalen)
			continue;
		if (d->sock->flags & SO_CANTRCVMORE)
			continue;

		nbuf = (delivered == 0) ? buf : buf_clone(buf, BUF_NORMAL);
		if (nbuf == 0)
			break;
		nbuf->next = 0;
		if (d->rcv.qlast)
		{
			d->rcv.qlast->next = nbuf;
			nbuf->prev = d->rcv.qlast;
			d->rcv.qlast = nbuf;
		} else
		{
			nbuf->prev = 0;
			d->rcv.qfirst = d->rcv.qlast = nbuf;
		}
		d->rcv.curdatalen += pktlen;
		so_wakersel(d->sock);
		wake(IO_Q, (long) d->sock);
		++delivered;
	}

	if (found)
	{
		if (delivered == 0)
			buf_deref(buf, BUF_NORMAL);
		return 0;
	} else if (IS_INET_PROTO(proto))
	{
		buf_deref(buf, BUF_NORMAL);
		return 0;
	}

	TRACE(("rip_input: nobody wants it"));
	return -1;
}

/*
 * This is called from the icmp module on dst unreachable messages.
 * Note that `saddr' is our address, ie the source address of the packet
 * that caused the icmp reply.
 */
static long rip_error(short type, short code, BUF *buf, ulong saddr, ulong daddr)
{
	UNUSED(type);
	UNUSED(code);
	UNUSED(saddr);
	UNUSED(daddr);
	buf_deref(buf, BUF_NORMAL);
	return 0;
}

void rip_init(void)
{
	in_proto_register(IPPROTO_RAW, &rip_proto);
}
