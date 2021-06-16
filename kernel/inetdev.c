/*
 *	This file implements /dev/inet. It is intended for controlling
 *	the behavior of the inet domain layer and getting information
 *	about it. netstat(8) is implemented using this device.
 *
 *	05/27/94, kay roemer.
 */

#include "sockets.h"
#include "inetdev.h"

#include <netinet/in.h>
#include "inet.h"
#include "tcp.h"
#include "udp.h"
#include "bpf.h"

#include "dummydev.h"
#include "mxkernel.h"


/*
 * read() obtains this structure for every inet domain socket
 */
struct inet_info
{
	short proto;						/* protocol, IPPROTO_* */
	long sendq;							/* bytes in send queue */
	long recvq;							/* bytes in recv queue */
	short state;						/* state */
	struct sockaddr_in laddr;			/* local address */
	struct sockaddr_in faddr;			/* foreign address */
};

struct _datas
{
	void (*getinfo)(struct inet_info *, struct in_data *);
	struct in_proto *proto;
};

static long inetdev_read(MX_DOSFD *, long, void *);

/*
 * Fill 'info' with information about TCP socket pointed at by 'data'
 */
static void tcp_getinfo(struct inet_info *info, struct in_data *data)
{
	struct tcb *tcb = data->pcb;

	info->state = tcb->state;
	info->sendq = data->snd.curdatalen;
	info->recvq = tcp_canread(data);
}


/*
 * Fill 'info' with information about UDP socket pointed at by 'data'
 */
static void udp_getinfo(struct inet_info *info, struct in_data *data)
{
	info->sendq = 0;
	info->recvq = data->rcv.curdatalen;
	info->state = data->flags & IN_ISCONNECTED ? TCBS_ESTABLISHED : TCBS_CLOSED;
}


static struct _datas allindatas[] = {
	{ tcp_getinfo, &tcp_proto },
	{ udp_getinfo, &udp_proto },
	{ 0, 0 }
};

extern MX_DDEV inetdev GNU_ASM_NAME("inetdev");

MX_DDEV cdecl_inetdev GNU_ASM_NAME("cdecl_inetdev") = {
	dummydev_open,
	dummydev_close,
	/* ugly hack here: function is not cdecl */
	(long cdecl (*)(MX_DOSFD *, long, void *))inetdev_read,
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

static char const inet_dev_name[] = "u:\\dev\\inet";
static char const cannot_install[] = "Cannot install device %S\r\n";

long inetdev_init(void)
{
	long r;
	const char *name = inet_dev_name;
	
	r = Dcntl(DEV_M_INSTALL, name, (long) &inetdev);
	if (r < 0)
	{
		char message[200];

		sprintf_params[0] = (long)name;
		p_kernel->_sprintf(message, cannot_install, sprintf_params);
		(void) Cconws(message);

		return -1;
	}

	return 0;
}


static long inetdev_read(MX_DOSFD *fp, long nbytes, void *buf)
{
	struct in_data *inp = NULL;
	struct inet_info info,
	*infop = (struct inet_info *) buf;
	struct _datas *datap;
	ulong space;
	int i;

	for (space = nbytes; space >= sizeof(info); fp->fd_fpos++)
	{
		datap = allindatas;

		for (i = (int)fp->fd_fpos; datap->proto; datap++)
		{
			inp = datap->proto->datas;
			for (; inp && --i >= 0; inp = inp->next)
				;

			if (i < 0)
				break;
		}

		if (datap->proto == 0)
			break;

		info.proto = inp->protonum;

		info.laddr.sin_family = AF_INET;
		info.laddr.sin_addr.s_addr = inp->src.addr;
		info.laddr.sin_port = inp->src.port;

		info.faddr.sin_family = AF_INET;
		info.faddr.sin_addr.s_addr = inp->dst.addr;
		info.faddr.sin_port = inp->dst.port;

		(*datap->getinfo) (&info, inp);

		*infop++ = info;
		space -= sizeof(info);
	}

	return (nbytes - space);
}
