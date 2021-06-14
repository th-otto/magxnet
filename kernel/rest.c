#include "sockets.h"
#include "iov.h"
#include "dummydev.h"
#include "timeout.h"
#include "iov.h"
#include "bpf.h"
#include "rawip.h"
#include "route.h"
#include "icmp.h"
#include "igmp.h"
#include "tcp.h"
#include "udp.h"
#include "masquera.h"

void x1c39c(void)
{
}


void x1c022(void)
{
	so_connect(0, 0, 0, 0, 0); /* XXX */
	so_register(0, 0);
	so_rselect(0, 0);
	so_wselect(0, 0);
	so_xselect(0, 0);
	iov2buf_cpy(0, 0, 0, 0, 0);
	buf2iov_cpy(0, 0, 0, 0, 0);
	dummydev_init(0, 0);
	/* install packetfilter */
	bpf_init();
}


int sleep(int queue, long cond)
{
	UNUSED(queue);
	UNUSED(cond);
	return 0;
}


void wake(int queue, long cond)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(queue);
	UNUSED(cond);
}

void wakeselect(long proc)
{
	/* NOT IMPLEMENTED YET */
	UNUSED(proc);
}


TIMEOUT *cdecl addroottimeout(long delta, void cdecl (*func)(struct proc *, long), ushort flags)
{
	(void)delta;
	(void)func;
	(void)flags;
	return 0;
}


void cdecl cancelroottimeout(TIMEOUT *which)
{
	(void)which;
}


long cdecl unixtime(unsigned short time, unsigned short date)
{
	(void)time;
	(void)date;
	return 0;
}



#ifndef __GNUC__
short cdecl if_input(struct netif *nif, BUF *buf, long delay, short type)
{
	(void)nif;
	(void)buf;
	(void)delay;
	(void)type;
	ip_input(nif,buf);
	return 0;
}
#endif

#ifndef __GNUC__
ushort udp_checksum(struct udp_dgram *dgram, in_addr_t srcadr, in_addr_t dstadr)
{
	/* TODO */
	(void)dgram;
	(void)srcadr;
	(void)dstadr;
	return 0;
}
#endif

#ifndef __GNUC__
/* BUG: is exported via netinfo and must be cdecl */
short chksum(void *buf, short nwords)
{
	/* TODO */
	(void)buf;
	(void)nwords;
	return 0;
}
#endif
