/*
 *	Device independent network interface routines.
 *
 *	02/24/94, kay roemer.
 */

#include "sockets.h"
#include "if.h"
#include "bpf.h"
#include <stdlib.h>

#include "arp.h"
#include "inetutil.h"
#include "ip.h"
#include "loopback.h"
#include "route.h"
#include "igmp.h"
#include "timeout.h"
#include "asm_spl.h"
#include "mxkernel.h"

#ifdef __PUREC__
#pragma warn -rch /* for p_geteuid */
#endif

#define GET_QUEUE(q)    (((q)->curr >> IF_PRIORITY_BITS) & (IF_PRIORITIES-1))
#define SET_QUEUE(q, n) ((q)->curr = (n) << IF_PRIORITY_BITS)
#define INC_QUEUE(q, n) ((q)->curr += (IF_PRIORITIES - (n)))


/*
 * Pending timeout
 */
TIMEOUT *tmout = 0;

/*
 * List of all registered interfaces, loopback and primary interface.
 */
struct netif *allinterfaces;
struct netif *if_lo;
#ifndef NOTYET
struct netif *if_primary;
#endif

#ifdef __GNUC__
/*
 * Stack used while processing incoming packets
 * do we really need this?
 */
static char stack[8192];

static void *setstack(void *sp)
{
	register void *osp __asm__("d0") = 0;

	__asm__ volatile (
		"\tmovel sp,%0\n"
		"\tmovel %2,sp\n"
		: "=a" (osp)
		: "0"(osp), "a"(sp));

	return osp;
}
#else
#define setstack(sp) sp
#endif


short cdecl if_enqueue(struct ifq *q, BUF *buf, short pri)
{
#ifdef __PUREC__
	/* dangerous; better use splhigh() */
	pushsr();
#else
	ushort sr;

	sr = splhigh();
#endif

	if (q->qlen >= q->maxqlen)
	{
		/*
		 * queue full, dropping packet
		 */
		buf_deref(buf, BUF_ATOMIC);

#ifdef __PUREC__
		popsr();
#else
		spl(sr);
#endif
		return ENOMEM;
	} else
	{
		if ((ushort) pri >= IF_PRIORITIES)
			pri = IF_PRIORITIES - 1;

		buf->link3 = NULL;
		if (q->qlast[pri])
		{
			q->qlast[pri]->link3 = buf;
			q->qlast[pri] = buf;
		} else
			q->qlast[pri] = q->qfirst[pri] = buf;

		q->qlen++;
	}

#ifdef __PUREC__
	popsr();
#else
	spl(sr);
#endif

	return 0;
}

BUF *cdecl if_dequeue(struct ifq *q)
{
	BUF *buf = NULL;
	short i;

#ifdef __PUREC__
	/* dangerous; better use splhigh() */
	pushsr();
#else
	ushort sr;
	sr = splhigh();
#endif

	if (q->qlen > 0)
	{
		short que;

		que = GET_QUEUE(q);
		for (i = IF_PRIORITIES; i > 0; --i)
		{
			if ((buf = q->qfirst[que]) != NULL)
			{
				q->qfirst[que] = buf->link3;
				if (!buf->link3)
					q->qlast[que] = NULL;

				if (i < IF_PRIORITIES)
					SET_QUEUE(q, que);

				INC_QUEUE(q, que);
				q->qlen--;

				break;
			}
			que = (que + 1) & (IF_PRIORITIES - 1);
		}
	}

#ifdef __PUREC__
	popsr();
#else
	spl(sr);
#endif

	return buf;
}

short cdecl if_putback(struct ifq *q, BUF *buf, short pri)
{
#ifdef __GNUC__
	ushort sr;

	sr = splhigh();
#else
	pushsr();
#endif

	if (q->qlen >= q->maxqlen)
	{
		/*
		 * queue full, dropping packet
		 */
		buf_deref(buf, BUF_ATOMIC);

#ifdef __GNUC__
		spl(sr);
#endif
		return ENOMEM;
	} else
	{
		if ((ushort) pri >= IF_PRIORITIES)
			pri = IF_PRIORITIES - 1;

		buf->link3 = q->qfirst[pri];
		q->qfirst[pri] = buf;
		if (!q->qlast[pri])
			q->qlast[pri] = buf;

		q->qlen++;
	}

#ifdef __GNUC__
	spl(sr);
#else
	popsr();
#endif

	return 0;
}

void cdecl if_flushq(struct ifq *q)
{
	short i;

#ifdef __PUREC__
	/* dangerous; better use splhigh() */
	pushsr();
#else
	ushort sr;
	sr = splhigh();
#endif

	for (i = 0; i < IF_PRIORITIES; ++i)
	{
		register BUF *buf;
		register BUF *next;

		for (buf = q->qfirst[i]; buf; buf = next)
		{
			next = buf->link3;
			buf_deref(buf, BUF_NORMAL);
		}
		q->qfirst[i] = q->qlast[i] = 0;
	}
	q->qlen = 0;

#ifdef __PUREC__
	popsr();
#else
	spl(sr);
#endif
}


/*
 * BUG: not declared cdecl
 */
void if_doinput(PROC *proc, long arg)
{
	struct netif *nif;
#ifdef __GNUC__
	char *sp;
#endif
	short comeagain = 0;

	UNUSED(proc);
	UNUSED(arg);
	tmout = 0;
#ifdef __GNUC__
	sp = setstack(stack + sizeof(stack));
#endif

	for (nif = allinterfaces; nif; nif = nif->next)
	{
		short todo;

		if ((nif->flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
			continue;

		for (todo = nif->rcv.maxqlen; todo; --todo)
		{
			BUF *buf;

			buf = if_dequeue(&nif->rcv);
			if (!buf)
				break;

			switch ((unsigned short) buf->info)
			{
			case PKTYPE_IP:
				ip_input(nif, buf);
				break;

			case PKTYPE_ARP:
				arp_input(nif, buf);
				break;

			case PKTYPE_RARP:
				rarp_input(nif, buf);
				break;

			default:
				DEBUG(("if_input: unknown pktype 0x%x", (short) buf->info));
				buf_deref(buf, BUF_NORMAL);
				break;
			}
		}

		if (!todo)
			comeagain = 1;
	}

	if (comeagain)
	{
		/*
		 * Come again at next context switch, since we did
		 * not check all interfaces, there might be packets
		 * waiting for us.
		 */
#ifdef NOTYET
		if_input(0, 0, 100, 0);
#else
		tmout = addroottimeout(60, if_doinput, 1);
#endif
	}

#ifdef __GNUC__
	setstack(sp);
#endif
}


#if 0  /* for MagiCNet, moved to kerinfo; FIXME */
short cdecl if_input(struct netif *nif, BUF *buf, long delay, short type)
{
	short r = 0;
	ushort sr;

	sr = splhigh();

	if (buf)
	{
		buf->info = type;
		r = if_enqueue(&nif->rcv, buf, IF_PRIORITIES - 1);
	}

	if (tmout == 0)
		tmout = addroottimeout(delay, if_doinput, 1);

	spl(sr);

	return r;
}
#endif


/*
 * BUG: not declared cdecl
 */
static void if_slowtimeout(PROC *proc, long arg)
{
	struct netif *nif;

	UNUSED(proc);
	UNUSED(arg);
	for (nif = allinterfaces; nif; nif = nif->next)
	{
		if ((nif->flags & IFF_UP) && nif->timeout)
			(*nif->timeout) (nif);
	}

	addroottimeout(IF_SLOWTIMEOUT, if_slowtimeout, 0);
}


long cdecl if_deregister(struct netif *nif)
{
	struct netif *ifp;
	struct netif *ifpb = NULL;

	for (ifp = allinterfaces; ifp; ifp = ifp->next)
	{
		if (ifp == nif)
		{
			/* HEAD REMOVAL */
			if (ifpb == NULL)
			{
				allinterfaces = ifp->next;
			} else
			{
				ifpb->next = ifp->next;
			}
			return 1;					/* indicating removed */
		}
		ifpb = ifp;
	}

	return 0;							/* not removed */
}


long cdecl if_register(struct netif *nif)
{
	static short have_timeout = 0;
	short i;

	nif->addrlist = 0;
	nif->snd.qlen = 0;
	nif->rcv.qlen = 0;
	nif->snd.curr = 0;
	nif->rcv.curr = 0;

	for (i = 0; i < IF_PRIORITIES; ++i)
	{
		nif->snd.qfirst[i] = nif->snd.qlast[i] = 0;
		nif->rcv.qfirst[i] = nif->rcv.qlast[i] = 0;
	}

	if (nif->hwtype >= HWTYPE_NONE)
	{
		nif->hwlocal.len = 0;
		nif->hwbrcst.len = 0;
	}

	nif->in_packets = 0;
	nif->in_errors = 0;
	nif->out_packets = 0;
	nif->out_errors = 0;
	nif->collisions = 0;

	nif->next = allinterfaces;
	allinterfaces = nif;
	if (nif->timeout && !have_timeout)
	{
		addroottimeout(IF_SLOWTIMEOUT, if_slowtimeout, 0);
		have_timeout = 1;
	}

	return 0;
}


/*
 * Get an unused unit number for interface name 'name'
 */
short cdecl if_getfreeunit(char *name)
{
	struct netif *ifp;
	short max = -1;

	for (ifp = allinterfaces; ifp; ifp = ifp->next)
	{
		if (!strncmp(ifp->name, name, IF_NAMSIZ) && ifp->unit > max)
			max = ifp->unit;
	}

	return max + 1;
}

long if_open(struct netif *nif)
{
	struct kernel_ifaddr *ifa;
	long error;

	error = (*nif->open) (nif);
	if (error)
	{
		DEBUG(("if_open: cannot open interface %s%d", nif->name, nif->unit));
		return error;
	}
	nif->flags |= IFF_UP | IFF_RUNNING;

	/*
	 * Make sure lo0 is always reachable as 127.0.0.1
	 */
	if (nif->flags & IFF_LOOPBACK)
	{
		route_add(nif, 0x7f000000L, IN_CLASSA_NET, INADDR_ANY, RTF_STATIC | RTF_UP | RTF_LOCAL, 999, 0);
	}

	ifa = if_af2ifaddr(nif, AF_INET);
	if (!ifa)
	{
		DEBUG(("if_open: warning: iface %s%d has no inet addr", nif->name, nif->unit));
		return 0;
	}

	/*
	 * This route is necessary for IP to deliver incoming packets
	 * to the local software. It routes the incoming packet and
	 * then compares the packets destination address and the
	 * interfaces' local address. If they match, the packet is
	 * delivered to the local software.
	 */
#ifdef NOTYET
	if (ifa->adr.in.sin_addr.s_addr != INADDR_ANY)
#else
	if (!(if_primary->flags & IFF_UP))
		if_primary = nif;
#endif
	{
		route_add(if_lo, ifa->adr.in.sin_addr.s_addr, 0xffffffffL,
				  INADDR_ANY, RTF_STATIC | RTF_UP | RTF_HOST | RTF_LOCAL, 999, 0);
	}

	/*
	 * Want the new interface to become primary one.
	 *
	 * This is usefull to broadcast packets.
	 */
#ifdef NOTYET
	rt_primary.nif = nif;
#endif

#ifdef IGMP_SUPPORT
	igmp_start(nif);
	igmp_report_groups(nif);
	nif->flags |= IFF_IGMP;
#endif

	DEBUG(("if_open: primary_nif = %s", rt_primary.nif->name));

	return 0;
}


long if_close(struct netif *nif)
{
	struct kernel_ifaddr *ifa;
	long error;

	error = (*nif->close) (nif);
	if (error)
	{
		DEBUG(("if_close: cannot close if %s%d", nif->name, nif->unit));
		return error;
	}

	if_flushq(&nif->snd);
	if_flushq(&nif->rcv);

#ifdef IGMP_SUPPORT
	igmp_stop(nif);
#endif
	route_flush(nif);
	arp_flush(nif);

	ifa = if_af2ifaddr(nif, AF_INET);
	if (ifa)
		route_del(ifa->adr.in.sin_addr.s_addr, 0xffffffffUL);

	if (nif->flags & IFF_LOOPBACK)
		route_del(INADDR_LOOPBACK & IN_CLASSA_NET, IN_CLASSA_NET);

	nif->flags &= ~(IFF_UP |
#ifdef IGMP_SUPPORT
		IFF_IGMP |
#endif
		IFF_RUNNING);

	/*
	 * Want a running primary interface
	 */
#ifdef NOTYET
	if (nif == rt_primary.nif)
#else
	if (nif == if_primary)
#endif
	{
		for (nif = allinterfaces; nif; nif = nif->next)
		{
			if (nif->flags & IFF_UP)
			{
#ifdef NOTYET
				rt_primary.nif = nif;
#else
				if_primary = nif;
#endif
				break;
			}
		}
	}

	DEBUG(("if_close: primary_nif = %s", rt_primary.nif->name));

	return 0;
}


#ifdef IGMP_SUPPORT
long if_send(struct netif *nif, BUF *buf, ulong nexthop, short addrtype)
#else
long if_send(struct netif *nif, BUF *buf, ulong nexthop, short isbrcst)
#endif
{
	struct arp_entry *are;
	long ret;

	if ((nif->flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
	{
		DEBUG(("if_send: interface %s%d is not UP and RUNNING", nif->name, nif->unit));

		buf_deref(buf, BUF_NORMAL);
		return ENETUNREACH;
	}

	if (nif->hwtype >= HWTYPE_NONE)
	{
		DEBUG(("if_send(%s): >= HWTYPE_NONE", nif->name));

		/*
		 * This pseudo hardware type needs no ARP resolving
		 * of the next hop IP address into a hardware address.
		 * We pass the IP address of the next destination instead.
		 */
		return (*nif->output) (nif, buf, (char *) &nexthop, sizeof(nexthop), PKTYPE_IP);
	} else
	{
		switch (nif->hwtype)
		{
		case HWTYPE_ETH:
			DEBUG(("if_send(%s): HWTYPE_ETH (addrtype=%d)", nif->name, addrtype));

			/*
			 * When broadcast then use interface's broadcast address
			 */
#ifdef IGMP_SUPPORT
			if (addrtype == IPADDR_BRDCST)
			{
				return (*nif->output) (nif, buf, (char *) nif->hwbrcst.adr.bytes, nif->hwbrcst.len, PKTYPE_IP);
			} else if (addrtype == IPADDR_MULTICST)
			{
				struct ip_dgram *iph = (struct ip_dgram *) buf->dstart;
				struct hwaddr hwmcast;

				hwmcast.adr.bytes[0] = 0x01;
				hwmcast.adr.bytes[1] = 0x00;
				hwmcast.adr.bytes[2] = 0x5e;
				hwmcast.adr.bytes[3] = (iph->daddr & 0xFF0000UL) >> 16;
				hwmcast.adr.bytes[4] = (iph->daddr & 0x00FF00UL) >> 8;
				hwmcast.adr.bytes[5] = (iph->daddr & 0x0000FFUL) >> 0;
				hwmcast.len = ETH_ALEN;

				return (*nif->output) (nif, buf, (char *) hwmcast.adr.bytes, hwmcast.len, PKTYPE_IP);
			}
#else
			if (isbrcst)
				return (*nif->output) (nif, buf, (char *) nif->hwbrcst.adr.bytes, nif->hwbrcst.len, PKTYPE_IP);
#endif

			/*
			 * Here we must first resolve the IP address into a hardware
			 * address using ARP.
			 */
			are = arp_lookup(0, nif, ARPRTYPE_IP, 4, (char *) &nexthop);
			if (are == 0)
			{
				buf_deref(buf, BUF_NORMAL);
				return ENOMEM;
			}

			if (ATF_ISCOM(are))
			{
				ret = (*nif->output) (nif, buf, (char *) are->hwaddr.adr.bytes, are->hwaddr.len, PKTYPE_IP);
			} else
				ret = if_enqueue(&are->outq, buf, IF_PRIORITIES - 1);

#ifdef NOTYET
			arp_deref(are);
#else
			if (--(are)->links == 0)
				arp_free(are);
#endif
			return ret;

		default:
			DEBUG(("if_send: %d: Invalid hardware type", nif->hwtype));
			buf_deref(buf, BUF_NORMAL);
			return EINVAL;
		}
	}
}

long if_ioctl(short cmd, long arg)
{
	struct kernel_ifreq *ifr;
	struct netif *nif;
	long error;
	struct kernel_ifaddr *ifa;

	switch (cmd)
	{
	case SIOCSIFLINK:
		{
			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: SIFLINK: permission denied"));
				return EACCES;
			}
		}
		/* fallback ?????? */
	case SIOCGIFNAME:
		{
			struct iflink *ifl;

			ifl = (struct iflink *) arg;
			nif = if_name2if(ifl->ifname);
			if (!nif)
			{
				DEBUG(("if_ioctl: %s: no such if", ifl->ifname));
				return ENOENT;
			}
			return (*nif->ioctl) (nif, cmd, arg);
		}
	case SIOCGIFCONF:
		{
			return if_config((struct ifconf *) arg);
		}
	}

	ifr = (struct kernel_ifreq *) arg;
	nif = if_name2if(ifr->ifr_name);
	if (!nif)
	{
		DEBUG(("if_ioctl: %s: no such interface", ifr->ifr_name));
		return ENOENT;
	}

	switch (cmd)
	{
#ifdef NOTYET
	case SIOCSIFHWADDR:
		{
			struct sockaddr_hw *shw = &ifr->ifru.adr.hw;

			memcpy(nif->hwlocal.adr.bytes, shw->shw_addr, MIN(shw->shw_len, sizeof(shw->shw_addr)));
		}
		return 0;
#endif

	case SIOCGIFHWADDR:
		{
			struct sockaddr_hw *shw = &ifr->ifru.adr.hw;

			shw->shw_family = AF_LINK;
			shw->shw_type = nif->hwtype;
			shw->shw_len = nif->hwlocal.len;
			memcpy(shw->shw_addr, nif->hwlocal.adr.bytes, MIN(shw->shw_len, sizeof(shw->shw_addr)));
		}
		return 0;

	case SIOCSLNKFLAGS:
	case SIOCSIFOPT:
		if (p_geteuid() != 0)
			return EACCES;
		/* fall through */
	case SIOCGLNKFLAGS:
	case SIOCGLNKSTATS:
		return (*nif->ioctl) (nif, cmd, arg);

	case SIOCGIFSTATS:
		ifr->ifru.stats.in_packets = nif->in_packets;
		ifr->ifru.stats.out_packets = nif->out_packets;
		ifr->ifru.stats.in_errors = nif->in_errors;
		ifr->ifru.stats.out_errors = nif->out_errors;
		ifr->ifru.stats.collisions = nif->collisions;
		return 0;

	case SIOCGIFFLAGS:
		ifr->ifru.flags = nif->flags;
		return 0;

	case SIOCSIFFLAGS:
		{
			short nflags = ifr->ifru.flags & IFF_MASK;

			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: permission denied"));
				return EACCES;
			}

			if ((nif->flags & IFF_UP) && !(nflags & IFF_UP))
			{
				error = if_close(nif);
				if (error)
					return error;
			} else if (!(nif->flags & IFF_UP) && nflags & IFF_UP)
			{
				error = if_open(nif);
				if (error)
					return error;
			}

			nif->flags &= ~IFF_MASK;
			nif->flags |= nflags;

			return (*nif->ioctl) (nif, cmd, arg);
		}

	case SIOCGIFMETRIC:
		ifr->ifru.metric = nif->metric;
		return 0;

	case SIOCSIFMETRIC:
		if (p_geteuid() != 0)
		{
			DEBUG(("if_ioctl: permission denied"));
			return EACCES;
		}
		nif->metric = ifr->ifru.metric;
		return 0;

	case SIOCSIFMTU:
		if (p_geteuid() != 0)
		{
			DEBUG(("if_ioctl: permission denied"));
			return EACCES;
		}
		nif->mtu = ifr->ifru.mtu;
		(*nif->ioctl) (nif, cmd, 0);
		return 0;

	case SIOCGIFMTU:
		ifr->ifru.mtu = nif->mtu;
		return 0;

	case SIOCSIFDSTADDR:
		{
			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: permission denied"));
				return EACCES;
			}

			if (!(nif->flags & IFF_POINTOPOINT))
			{
				DEBUG(("if_ioctl: nif is not p2p"));
				return EACCES;
			}

			ifa = if_af2ifaddr(nif, ifr->ifru.dstadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr in this AF", ifr->ifru.dstadr.sa.sa_family));
				return EINVAL;
			}

			sa_copy(&ifa->ifu.dstadr.sa, &ifr->ifru.dstadr.sa);
		}
		return 0;

	case SIOCGIFDSTADDR:
		{
			if (!(nif->flags & IFF_POINTOPOINT))
			{
				DEBUG(("if_ioctl: nif is not p2p"));
				return EACCES;
			}

			ifa = if_af2ifaddr(nif, ifr->ifru.dstadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr in this AF", ifr->ifru.dstadr.sa.sa_family));
				return EINVAL;
			}

			sa_copy(&ifr->ifru.dstadr.sa, &ifa->ifu.dstadr.sa);
		}
		return 0;

	case SIOCSIFADDR:
		{
			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: permission denied"));
				return EACCES;
			}

			error = if_setifaddr(nif, &ifr->ifru.adr.sa);
			if (error)
				return error;

			return (*nif->ioctl) (nif, cmd, arg);
		}

	case SIOCGIFADDR:
		{
			ifa = if_af2ifaddr(nif, ifr->ifru.dstadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr " "in this AF", ifr->ifru.dstadr.sa.sa_family));
				return EINVAL;
			}

			sa_copy(&ifr->ifru.adr.sa, &ifa->adr.sa);
		}
		return 0;

	case SIOCSIFBRDADDR:
		{
			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: permission denied"));
				return EACCES;
			}

			if (!(nif->flags & IFF_BROADCAST))
			{
				DEBUG(("if_ioctl: nif is not broadcast"));
				return EACCES;
			}

			ifa = if_af2ifaddr(nif, ifr->ifru.broadadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr in this AF", ifr->ifru.broadadr.sa.sa_family));
				return EINVAL;
			}

			sa_copy(&ifa->ifu.broadadr.sa, &ifr->ifru.broadadr.sa);
		}
		return 0;

	case SIOCGIFBRDADDR:
		{
			if (!(nif->flags & IFF_BROADCAST))
			{
				DEBUG(("if_ioctl: nif is not broadcast"));
				return EACCES;
			}

			ifa = if_af2ifaddr(nif, ifr->ifru.broadadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr in this AF", ifr->ifru.broadadr.sa.sa_family));
				return EINVAL;
			}

			sa_copy(&ifr->ifru.broadadr.sa, &ifa->ifu.broadadr.sa);
		}
		return 0;

	case SIOCSIFNETMASK:
		{
			if (p_geteuid() != 0)
			{
				DEBUG(("if_ioctl: permission denied"));
				return EACCES;
			}

			if (ifr->ifru.broadadr.sa.sa_family != AF_INET)
			{
				DEBUG(("if_ioctl: address family != AF_INET"));
				return EAFNOSUPPORT;
			}

			ifa = if_af2ifaddr(nif, AF_INET);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr " "in this AF", ifr->ifru.netmsk.sa.sa_family));
				return EINVAL;
			}

			ifa->subnetmask = ifr->ifru.netmsk.in.sin_addr.s_addr;
			ifa->subnet = ifa->subnetmask & ifa->adr.in.sin_addr.s_addr;

			return (*nif->ioctl) (nif, cmd, arg);
		}

	case SIOCGIFNETMASK:
		{
			struct sockaddr_in in;

			ifa = if_af2ifaddr(nif, ifr->ifru.broadadr.sa.sa_family);
			if (!ifa)
			{
				DEBUG(("if_ioctl: %d: interface has no addr " "in this AF", ifr->ifru.netmsk.sa.sa_family));
				return EINVAL;
			}

			in.sin_family = AF_INET;
			in.sin_addr.s_addr = ifa->subnetmask;
			in.sin_port = 0;

			sa_copy(&ifr->ifru.netmsk.sa, (struct sockaddr *) &in);
		}
		return 0;
	}

	return ENOSYS;
}

struct netif *if_name2if(char *ifname)
{
	char name[IF_NAMSIZ + 1];
	char *cp;
	short i;
	long unit = 0;
	struct netif *nif;

	for (i = 0, cp = ifname; i < IF_NAMSIZ && *cp; ++cp, ++i)
	{
		if (*cp >= '0' && *cp <= '9')
		{
			unit = atol(cp);
			break;
		}
		name[i] = *cp;
	}

	name[i] = '\0';
	for (nif = allinterfaces; nif; nif = nif->next)
	{
		if (!stricmp(nif->name, name) && nif->unit == unit)
			return nif;
	}

	return NULL;
}

/* find an interface which is on the same network as the address `addr' */
struct netif *if_net2if(ulong addr)
{
	struct netif *nif;
	struct kernel_ifaddr *ifa;

	for (nif = allinterfaces; nif; nif = nif->next)
	{
		if (!(nif->flags & IFF_UP))
			continue;

		ifa = if_af2ifaddr(nif, AF_INET);
		if (!ifa)
			continue;

		if (nif->flags & IFF_POINTOPOINT && addr == ifa->ifu.dstadr.in.sin_addr.s_addr)
		{
			DEBUG(("if_net2if: nif '%s' p2p", nif->name));
			return nif;
		}

		if (nif->flags & IFF_BROADCAST && addr == ifa->ifu.broadadr.in.sin_addr.s_addr)
		{
			DEBUG(("if_net2if: nif '%s' broadcast", nif->name));
			return nif;
		}
	}

	for (nif = allinterfaces; nif; nif = nif->next)
	{
		if (!(nif->flags & IFF_UP))
			continue;

		ifa = if_af2ifaddr(nif, AF_INET);
		if (!ifa)
			continue;

		if ((addr & ifa->netmask) == ifa->net)
		{
			DEBUG(("if_net2if: nif '%s' netmask match", nif->name));
			return nif;
		}
	}

	DEBUG(("if_net2if: no match"));
	return NULL;
}

long if_setifaddr(struct netif *nif, struct sockaddr *sa)
{
	struct kernel_ifaddr *ifa;
	struct kernel_ifaddr *ifa2;
	short error = 0;

	ifa = kmalloc(sizeof(*ifa));
	if (!ifa)
	{
		DEBUG(("if_newaddr: out of memory"));
		return ENOMEM;
	}

	sa_copy(&ifa->adr.sa, sa);
	ifa->family = sa->sa_family;
	ifa->ifp = nif;
	ifa->flags = 0;

	switch (sa->sa_family)
	{
	case AF_INET:
		{
			struct sockaddr_in *in = (struct sockaddr_in *) sa;
			ulong netmask;

			netmask = ip_netmask(in->sin_addr.s_addr);
			if (netmask == 0
#ifdef NOTYET /* 1008abcb2b91f42c96f505d4370bbc8ae439b7ad */
				&& in->sin_addr.s_addr != INADDR_ANY
#endif
				)
			{
				DEBUG(("if_setaddr: Addr not in class A/B/C"));
				error = EADDRNOTAVAIL;
				break;
			}
			ifa->net = ifa->subnet = in->sin_addr.s_addr & netmask;
			ifa->netmask = ifa->subnetmask = netmask;
			ifa->net_broadaddr = ifa->net | ~netmask;

			in = &ifa->ifu.broadadr.in;
			in->sin_family = AF_INET;
			in->sin_port = 0;
			in->sin_addr.s_addr = (nif->flags & IFF_BROADCAST) ? ifa->net_broadaddr : INADDR_ANY;

			route_flush(nif);
			ifa2 = if_af2ifaddr(nif, AF_INET);
			if (ifa2)
				route_del(ifa2->adr.in.sin_addr.s_addr, 0xffffffffUL);

			/*
			 * Make sure lo0 is always reachable as 127.0.0.1
			 */
			if (nif->flags & IFF_LOOPBACK)
				route_add(nif, 0x7f000000L, IN_CLASSA_NET, INADDR_ANY, RTF_STATIC | RTF_UP | RTF_LOCAL, 999, 0);

			/*
			 * This route is necessary for IP to deliver incoming packets
			 * to the local software. It routes the incoming packet and
			 * then compares the packets destination address and the
			 * interfaces' local address. If they match, the packet is
			 * delivered to the local software.
			 */
#ifdef NOTYET
			if (ifa->adr.in.sin_addr.s_addr != INADDR_ANY)
			{
				route_add(if_lo, ifa->adr.in.sin_addr.s_addr,
						  0xffffffffUL, INADDR_ANY, RTF_STATIC | RTF_UP | RTF_HOST | RTF_LOCAL, 999, 0);
			} else
			{
				/* primary broadcast interface */
				rt_primary.nif = nif;
				DEBUG(("if_setifaddr: primary_nif = %s", rt_primary.nif->name));
			}
#else
			route_add(if_lo, ifa->adr.in.sin_addr.s_addr,
					  0xffffffffUL, INADDR_ANY, RTF_STATIC | RTF_UP | RTF_HOST | RTF_LOCAL, 999, 0);
#endif
			break;
		}
	default:
		{
			DEBUG(("if_setifaddr: %d: address family not supported", sa->sa_family));
			error = EAFNOSUPPORT;
			break;
		}
	}

	if (error)
	{
		kfree(ifa);
		return error;
	}

	ifa2 = if_af2ifaddr(nif, ifa->family);
	if (ifa2)
	{
		ifa->next = ifa2->next;
		*ifa2 = *ifa;
		kfree(ifa);
	} else
	{
		ifa->next = nif->addrlist;
		nif->addrlist = ifa;
	}

	return 0;
}

long if_config(struct ifconf *ifconf)
{
	struct netif *nif;
	struct kernel_ifreq *ifr;
	struct kernel_ifaddr *ifa;
	char name[64]; /* FIXME: 100 in MiNT */
	ulong len;

#ifdef NOTYET /* 2d3421f8220e8b49f4eadd22e86f21f0ec86b759 */
	if (!ifconf->ifc_ifcu.ifcu_buf)
	{
		/* count interfaces when no buffer! */
		for (nif = allinterfaces; nif; nif = nif->next)
		{
			ifconf->ifc_len += sizeof(*ifr);	/* AF_INET */
			ifconf->ifc_len += sizeof(*ifr);	/* AF_LINK */
		}
		return 0;
	}
#endif

	len = ifconf->ifc_len;
	ifr = (struct kernel_ifreq *)ifconf->ifc_ifcu.ifcu_req;
	for (nif = allinterfaces; len >= sizeof(*ifr) && nif; nif = nif->next)
	{
#ifdef NOTYET
		ksprintf(name, "%s%d", nif->name, nif->unit);
#else
		sprintf_params[0] = (long)nif->name;
		sprintf_params[1] = nif->unit;
		p_kernel->_sprintf(name, "%S%L", sprintf_params);
#endif
		ifa = nif->addrlist;
		if (!ifa)
		{
			struct sockaddr_in in;

			in.sin_family = AF_INET;
			in.sin_addr.s_addr = INADDR_ANY;
			in.sin_port = 0;
			strncpy(ifr->ifr_name, name, IF_NAMSIZ);
			sa_copy(&ifr->ifru.adr.sa, (struct sockaddr *) &in);
			len -= sizeof(*ifr);
			ifr++;
		} else
		{
			for (; len >= sizeof(*ifr) && ifa; ifa = ifa->next)
			{
				strncpy(ifr->ifr_name, name, IF_NAMSIZ);
				sa_copy(&ifr->ifru.adr.sa, &ifa->adr.sa);
				len -= sizeof(*ifr);
				ifr++;
			}
		}
#ifdef NOTYET
		{
			struct sockaddr_hw shw;

			shw.shw_family = AF_LINK;
			shw.shw_type = nif->hwtype;
			shw.shw_len = nif->hwlocal.len;
			memcpy(shw.shw_addr, nif->hwlocal.adr.bytes, MIN(shw.shw_len, sizeof(shw.shw_addr)));
			strncpy(ifr->ifr_name, name, IF_NAMSIZ);
			sa_copy((struct sockaddr *) &ifr->ifru.adr.hw, (struct sockaddr *) &shw);
			len -= sizeof(*ifr);
			ifr++;
		}
#endif
	}
	ifconf->ifc_len -= len;
	return 0;
}

struct kernel_ifaddr *if_af2ifaddr(struct netif *nif, short family)
{
#ifdef NOTYET /* 97bfb56b5758d3ed8e1c702377ee6597d255be2a */
	struct kernel_ifaddr *ifa = NULL;

	if (nif)
	{
		for (ifa = nif->addrlist; ifa; ifa = ifa->next)
		{
			if (ifa->family == family)
				break;
		}
	}
#else
	struct kernel_ifaddr *ifa;

	for (ifa = nif->addrlist; ifa; ifa = ifa->next)
		if (ifa->family == family)
			break;
#endif
	return ifa;
}

long if_init(void)
{
	struct netif *nif;

	if_load();
	loopback_init();					/* must be last */
	arp_init();

	/*
	 * Look for primary and loopback interface
	 */
	for (nif = allinterfaces; nif; nif = nif->next)
	{
		if (nif->flags & IFF_LOOPBACK)
		{
#ifdef NOTYET
			if_lo = rt_primary.nif = nif;
#else
			if_lo = if_primary = nif;
#endif
			break;
		}
#ifdef IGMP_SUPPORT
		nif->igmp_mac_filter = NULL;
#endif
	}

	if (!if_lo)
	{
		FATAL("if_init: no loopback interface");
		return -1;
	}

	return 0;
}
