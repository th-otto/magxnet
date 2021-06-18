/*
 *	This file implements /dev/route. It is intended for controlling
 *	the behavior of the IP router and getting information
 *	about it.
 *
 *	02/28/94, kay roemer.
 */

#include "sockets.h"
#include "routedev.h"

#include "if.h"
#include "bpf.h"
#include <netinet/in.h>
#include "route.h"

#include "dummydev.h"
#include "mxkernel.h"

/*
 * read() obtains this structure for every route
 */
struct route_info
{
	char nif[IF_NAMSIZ];				/* name of the interface */
	struct rtentry rt;					/* route info */
};

/* ugly hack here: function is not cdecl */
static long routedev_read(MX_DOSFD *, long, void *);

extern MX_DDEV routedev GNU_ASM_NAME("routedev");

MX_DDEV cdecl_routedev GNU_ASM_NAME("cdecl_routedev") = {
	dummydev_open,
	dummydev_close,
	(long cdecl (*)(MX_DOSFD *, long, void *))routedev_read,
	dummydev_write,
	dummydev_stat,
	dummydev_lseek,
	dummydev_datime,
	dummydev_ioctl,
	dummydev_delete,
	0, /* getc */
	0, /* getline */
	0, /* putc */
};

static char const routedev_name[] = "u:\\dev\\route";

long routedev_init(void)
{
	long r;

	r = Dcntl(DEV_M_INSTALL, routedev_name, (long) &routedev);
	if (r < 0)
	{
		char message[200];

		sprintf_params[0] = (long)routedev_name;
		p_kernel->_sprintf(message, "Cannot install device %S\r\n", sprintf_params);
		(void) Cconws(message);

		return -1;
	}

	return 0;
}


/* BUG: not declared cdecl */
static long routedev_read(MX_DOSFD *f, long nbytes, void *buf)
{
	struct route *rt = NULL;
	struct route_info info;
	struct route_info *infop = (struct route_info *) buf;
	int i, j;
	ulong space;

	for (space = nbytes; space >= sizeof(info); f->fd_fpos++)
	{
		rt = defroute;
		i = (int)(rt ? f->fd_fpos - 1 : f->fd_fpos);
		for (j = 0; j < RT_HASH_SIZE && i >= 0; j++)
		{
			rt = allroutes[j];
			for (; rt && --i >= 0; rt = rt->next)
				;
		}

#ifdef NOTYET /* commented out?? */
		if (j >= RT_HASH_SIZE)
			break;
#endif

		if (rt == NULL)
			break;

		mint_bzero(&info, sizeof(info));

		info.rt.dst.in.sin_family = AF_INET;
		info.rt.dst.in.sin_addr.s_addr = rt->net;

		if (rt->flags & RTF_GATEWAY)
		{
			info.rt.gateway.in.sin_family = AF_INET;
			info.rt.gateway.in.sin_addr.s_addr = rt->gway;
		}

		info.rt.rt_flags = rt->flags;
		info.rt.rt_metric = rt->metric;
		info.rt.rt_refcnt = rt->refcnt;
		info.rt.rt_use = rt->usecnt;

		sprintf_params[0] = (long)rt->nif->name;
		sprintf_params[1] = rt->nif->unit;
		p_kernel->_sprintf(info.nif, "%S%L", sprintf_params);
		DEBUG(("routedev_read: %s", info.nif));

		*infop++ = info;
		space -= sizeof(info);
	}

	return nbytes - space;
}
