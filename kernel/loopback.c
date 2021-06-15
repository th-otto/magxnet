#include "sockets.h"
#include "loopback.h"

#include "if.h"
#include "bpf.h"

#include "buf.h"


static long cdecl loop_open(struct netif *);
static long cdecl loop_close(struct netif *);
static long cdecl loop_output(struct netif *, BUF *, const char *, short, short);
static long cdecl loop_ioctl(struct netif *, short, long);

static struct netif if_loopback
#ifdef NOTYET
	= {
	"lo",
	0,
	IFF_LOOPBACK | IFF_BROADCAST
#ifdef IGMP_SUPPORT
		| IFF_IGMP
#endif
		,
	0,
	2 * 8192,
	0,
	HWTYPE_NONE,

	{ 0, { { 0 } } }, /* hwlocal */
	{ 0, { { 0 } } }, /* hwbrcst */
	
	NULL,
	
	{ IF_MAXQ, 0, 0, { 0 }, { 0 } },
	{ IF_MAXQ, 0, 0, { 0 }, { 0 } },

	loop_open,
	loop_close,
	loop_output,
	loop_ioctl,
	NULL,

	NULL,
	0,
	0,
	0,
	0,
	0,
	NULL,
	0,
	NULL,
	NULL,
	0,
	{ 0, 0 }
}
#endif
;


static long cdecl loop_open(struct netif *nif)
{
	UNUSED(nif);
	return 0;
}

static long cdecl loop_close(struct netif *nif)
{
	UNUSED(nif);
	return 0;
}

static long cdecl loop_output(struct netif *nif, BUF *buf, const char *hwaddr, short hwlen, short pktype)
{
	long r;

	UNUSED(hwaddr);
	UNUSED(hwlen);
	nif->out_packets++;
	if (nif->bpf && BUF_LEAD_SPACE(buf) >= 4)
	{
		/*
		 * Add 4 byte dummy header containing the address
		 * family for the packet type.
		 */
		long af = pktype == PKTYPE_IP ? AF_INET : AF_UNSPEC;

		buf->dstart -= 4;
		*(long *) buf->dstart = af;
		bpf_input(nif, buf);
		buf->dstart += 4;
	}

	if ((r = if_input(&if_loopback, buf, 0, pktype)) != 0)
	{
		nif->in_errors++;
		return r;
	}
	nif->in_packets++;
	return 0;
}

static long cdecl loop_ioctl(struct netif *nif, short cmd, long arg)
{
	UNUSED(arg);
	switch (cmd)
	{
	case SIOCSIFFLAGS:
		return 0;

	case SIOCSIFADDR:
		nif->flags |= IFF_UP | IFF_RUNNING
#ifdef IGMP_SUPPORT
			| IFF_IGMP
#endif
			;
		return 0;

	case SIOCSIFNETMASK:
		return 0;
	}

	return ENOSYS;
}

void loopback_init(void)
{
	/* FIXME: runtime init completely unneeded */
	strcpy(if_loopback.name, "lo");
	if_loopback.unit = 0;
	if_loopback.metric = 0;
	if_loopback.flags = IFF_LOOPBACK | IFF_BROADCAST;
	if_loopback.mtu = 2 * 8192;
	if_loopback.timer = 0;
	if_loopback.hwtype = HWTYPE_NONE;
	if_loopback.rcv.maxqlen = IF_MAXQ;
	if_loopback.snd.maxqlen = IF_MAXQ;
	if_loopback.open = loop_open;
	if_loopback.close = loop_close;
	if_loopback.output = loop_output;
	if_loopback.ioctl = loop_ioctl;
	if_loopback.timeout = 0;
	if_loopback.data = NULL;
	
	if_register(&if_loopback);
}
