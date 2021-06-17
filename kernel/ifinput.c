#include "sockets.h"
#include "timeout.h"
#include "bpf.h"
#include "asm_spl.h"

#ifdef __GNUC__
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
#endif
