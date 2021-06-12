#include "sockets.h"
#include "loopback.h"

#include "if.h"
#include "bpf.h"

#include "buf.h"


static long loop_open(struct netif *);
static long loop_close(struct netif *);
static long loop_output(struct netif *, BUF *, const char *, short, short);
static long loop_ioctl(struct netif *, short, long);

static struct netif if_loopback = {
	"lo",
	0,
	IFF_LOOPBACK | IFF_BROADCAST | IFF_IGMP,
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
};


static long loop_open(struct netif *nif)
{
	UNUSED(nif);
	return 0;
}

static long loop_close(struct netif *nif)
{
	UNUSED(nif);
	return 0;
}

static long loop_output(struct netif *nif, BUF * buf, const char *hwaddr, short hwlen, short pktype)
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
		long af = (pktype == PKTYPE_IP) ? AF_INET : AF_UNSPEC;

		buf->dstart -= 4;
		*(long *) buf->dstart = af;
		bpf_input(nif, buf);
		buf->dstart += 4;
	}

	r = if_input(&if_loopback, buf, 0, pktype);
	if (r)
		nif->in_errors++;
	else
		nif->in_packets++;

	return r;
}

static long loop_ioctl(struct netif *nif, short cmd, long arg)
{
	UNUSED(arg);
	switch (cmd)
	{
	case SIOCSIFFLAGS:
		return 0;

	case SIOCSIFADDR:
		nif->flags |= (IFF_UP | IFF_RUNNING | IFF_IGMP);
		return 0;

	case SIOCSIFNETMASK:
		return 0;
	}

	return ENOSYS;
}

void loopback_init(void)
{
	if_register(&if_loopback);
}
