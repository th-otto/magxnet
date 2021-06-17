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
#include "asm_spl.h"

void dispose_old_timeouts(void);


short cdecl if_input(struct netif *nif, BUF *buf, long delay, short type)
{
	short r = 0;
	ushort sr;

#ifdef __PUREC__
	/* only for binary equivalence; might as well use splhigh() */
	sr = getsr();
	setipl7();
#else
	sr = splhigh();
#endif

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
ushort tcp_checksum(struct tcp_dgram *dgram, ulong srcadr, ulong dstadr, ushort len)
{
	/* TODO */
	(void)dgram;
	(void)len;
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
	so_connect(0, 0, 0, 0, 0); /* XXX */
	so_rselect(0, 0);
	so_wselect(0, 0);
	so_xselect(0, 0);
	iov2buf_cpy(0, 0, 0, 0, 0);
	buf2iov_cpy(0, 0, 0, 0, 0);
	dummydev_init(0, 0);
	/* install packetfilter */
	bpf_init();
	dispose_old_timeouts();
	return 0;
}
#endif
