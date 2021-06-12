/*
 *	This file implementes /dev/arp, from which the arp table can be
 *	read.
 *
 *	12/15/94, Kay Roemer.
 */

#include "sockets.h"
#include "dummydev.h"
#include "if.h"
#include "bpf.h"
#include "arp.h"
#include "arpdev.h"
#include "mxkernel.h"

/*
 * structure that can be read from /dev/arp
 */
struct arp_info
{
	struct sockaddr_hw praddr;
	struct sockaddr_hw hwaddr;
	ushort flags;
	ushort links;
	ulong tmout;
};

/*
 * /dev/arp device driver
 */
extern MX_DDEV arpdev GNU_ASM_NAME("arpdev");

MX_DDEV cdecl_arpdev GNU_ASM_NAME("cdecl_arpdev") = {
	dummydev_open,
	dummydev_close,
	/* ugly hack here: function is not cdecl */
	(long cdecl (*)(MX_DOSFD *, long, void *))arpdev_read,
	dummydev_write,
	dummydev_stat,
	dummydev_lseek,
	dummydev_datime,
	dummydev_ioctl,
	dummydev_delete,
	0, /* getc */
	0, /* getline */
	0  /* putc */
};

static struct dev_descr arpdev_descr =
{
	&cdecl_arpdev,
	0,
	0,
	NULL,
	0,
	0,
	NULL,
	0,
	0
};

static char arpdev_name[] = "u:\\dev\\arp";

long arpdev_init(void)
{
	return dummydev_init(arpdev_name, &arpdev_descr);
}


long arpdev_read(MX_DOSFD *fp, long nbytes, void *buf)
{
	struct arp_info info;
	struct arp_info *infop = (struct arp_info *) buf;
	struct arp_entry *are = 0;
	long space;
	long t;
	int i;
	int j;

	for (space = nbytes; (unsigned long)space >= sizeof(info); fp->fd_fpos++)
	{
		i = (int)fp->fd_fpos;
		for (j = 0; j < ARP_HASHSIZE && i >= 0; j++)
		{
			are = arptab[j];
			for (; are && --i >= 0; are = are->prnext)
				;
		}

		if (j >= ARP_HASHSIZE)
			break;

		bzero(&info, sizeof(info));

		/*
		 * Protocoll address
		 */
		info.praddr.shw_family = AF_LINK;
		info.praddr.shw_len = are->praddr.len;
		info.praddr.shw_type = are->prtype;
		if (are->flags & ATF_PRCOM)
			memcpy(info.praddr.shw_addr, are->praddr.adr.bytes, are->praddr.len);

		/*
		 * Hardware address
		 */
		info.hwaddr.shw_family = AF_LINK;
		info.hwaddr.shw_len = are->hwaddr.len;
		info.hwaddr.shw_type = are->hwtype;
		if (are->flags & ATF_HWCOM)
			memcpy(info.hwaddr.shw_addr, are->hwaddr.adr.bytes, are->hwaddr.len);

		info.flags = are->flags;
		info.links = are->links;
		t = event_delta(&are->tmout);
		info.tmout = (t < 0) ? 0 : t * EVTGRAN;

		*infop++ = info;
		space -= sizeof(info);
	}

	return nbytes - space;
}
