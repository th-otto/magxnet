/*
 *	This file implements /dev/masquerade, a device for controlling the IP
 *	masquerading features.
 *
 *	Started 10/5/1999 Mario Becroft.
 */

#include "sockets.h"
#include "masqdev.h"

#include "if.h"
#include "bpf.h"
#include <netinet/in.h>
#include "masquera.h"
#include "route.h"

#include "dummydev.h"
#include "mxkernel.h"

#ifdef USE_MASQUERADE

static long cdecl masqdev_read(MX_DOSFD *, long, void *);
static long cdecl masqdev_write(MX_DOSFD *, long, void *);

extern MX_DDEV masqdev GNU_ASM_NAME("masqdev");

MX_DDEV cdecl_masqdev GNU_ASM_NAME("cdecl_masqdev") = {
	dummydev_open,
	dummydev_close,
	masqdev_read,
	masqdev_write,
	dummydev_stat,
	dummydev_lseek,
	dummydev_datime,
	dummydev_ioctl,
	dummydev_delete,
	0, /* getc */
	0, /* getline */
	0, /* putc */
};

static char const masquerade_dev_name[] = "u:\\dev\\inet";
static const char *cannot_install = "Cannot install device %S\r\n";

long masqdev_init(void)
{
	long r;

	r = Dcntl(DEV_M_INSTALL, masquerade_dev_name, (long) &masqdev);
	if (r < 0)
	{
		char message[200];

		sprintf_params[0] = (long)masquerade_dev_name;
		p_kernel->_sprintf(message, cannot_install, sprintf_params);
		(void) Cconws(message);

		return -1;
	}

	return 0;
}

static int record = 0;
static PORT_DB_RECORD *redirection = NULL;
static PORT_DB_RECORD *prev_redir = NULL;

static long cdecl masqdev_read(MX_DOSFD *fp, long nbytes, void *buf)
{
	switch (fp->fd_fpos)
	{
	case 0:
		if ((ulong) nbytes >= sizeof(masq.magic))
		{
			memcpy(buf, &masq.magic, sizeof(masq.magic));
			return sizeof(masq.magic);
		}
		break;
	case 1:
		if ((ulong) nbytes >= sizeof(masq.version))
		{
			memcpy(buf, &masq.version, sizeof(masq.version));
			return sizeof(masq.version);
		}
		break;
	case 2:
		if ((ulong) nbytes >= sizeof(masq.addr))
		{
			memcpy(buf, &masq.addr, sizeof(masq.addr));
			return sizeof(masq.addr);
		}
		break;
	case 3:
		if ((ulong) nbytes >= sizeof(masq.mask))
		{
			memcpy(buf, &masq.mask, sizeof(masq.mask));
			return sizeof(masq.mask);
		}
		break;
	case 4:
		if ((ulong) nbytes >= sizeof(masq.flags))
		{
			memcpy(buf, &masq.flags, sizeof(masq.flags));
			return sizeof(masq.flags);
		}
		break;
	case 5:
		if ((ulong) nbytes >= sizeof(masq.tcp_first_timeout))
		{
			memcpy(buf, &masq.tcp_first_timeout, sizeof(masq.tcp_first_timeout));
			return sizeof(masq.tcp_first_timeout);
		}
		break;
	case 6:
		if ((ulong) nbytes >= sizeof(masq.tcp_ack_timeout))
		{
			memcpy(buf, &masq.tcp_ack_timeout, sizeof(masq.tcp_ack_timeout));
			return sizeof(masq.tcp_ack_timeout);
		}
		break;
	case 7:
		if ((ulong) nbytes >= sizeof(masq.tcp_fin_timeout))
		{
			memcpy(buf, &masq.tcp_fin_timeout, sizeof(masq.tcp_fin_timeout));
			return sizeof(masq.tcp_fin_timeout);
		}
		break;
	case 8:
		if ((ulong) nbytes >= sizeof(masq.udp_timeout))
		{
			memcpy(buf, &masq.udp_timeout, sizeof(masq.udp_timeout));
			return sizeof(masq.udp_timeout);
		}
		break;
	case 9:
		if ((ulong) nbytes >= sizeof(masq.icmp_timeout))
		{
			memcpy(buf, &masq.icmp_timeout, sizeof(masq.icmp_timeout));
			return sizeof(masq.icmp_timeout);
		}
		break;
	case 50:
		if ((ulong) nbytes >= sizeof(ulong))
		{
			ulong time;

			time = MASQ_TIME;
			memcpy(buf, &time, sizeof(ulong));
			return sizeof(ulong);
		}
		/* fall through */
	case 100:
		record = 0;
		fp->fd_fpos += 1;
		/* fall through */
	case 101:
		if ((ulong) nbytes >= sizeof(PORT_DB_RECORD))
		{
			while (record < MASQ_NUM_PORTS && !masq.port_db[record])
				record += 1;
			if (record >= MASQ_NUM_PORTS)
				return 0;
			memcpy(buf, masq.port_db[record], sizeof(PORT_DB_RECORD));
			record += 1;
			return sizeof(PORT_DB_RECORD);
		}
		break;
	case 200:
		redirection = masq.redirection_db;
		fp->fd_fpos += 1;
		/* fall through */
	case 201:
		if ((ulong) nbytes >= sizeof(PORT_DB_RECORD))
		{
			if (!redirection)
				return 0;
			memcpy(buf, redirection, sizeof(PORT_DB_RECORD));
			prev_redir = redirection;
			redirection = redirection->next_port;
			return sizeof(PORT_DB_RECORD);
		}
		break;
	}

	return 0;
}

static long masqdev_write(MX_DOSFD *fp, long nbytes, void *buf)
{
	switch (fp->fd_fpos)
	{
	case 2:
		if (nbytes == sizeof(masq.addr))
		{
			memcpy(&masq.addr, buf, sizeof(masq.addr));
			return sizeof(masq.addr);
		}
		break;
	case 3:
		if (nbytes == sizeof(masq.mask))
		{
			memcpy(&masq.mask, buf, sizeof(masq.mask));
			return sizeof(masq.mask);
		}
		break;
	case 4:
		if (nbytes == sizeof(masq.flags))
		{
			memcpy(&masq.flags, buf, sizeof(masq.flags));
			return sizeof(masq.flags);
		}
		break;
	case 5:
		if (nbytes == sizeof(masq.tcp_first_timeout))
		{
			memcpy(&masq.tcp_first_timeout, buf, sizeof(masq.tcp_first_timeout));
			return sizeof(masq.tcp_first_timeout);
		}
		break;
	case 6:
		if (nbytes == sizeof(masq.tcp_ack_timeout))
		{
			memcpy(&masq.tcp_ack_timeout, buf, sizeof(masq.tcp_ack_timeout));
			return sizeof(masq.tcp_ack_timeout);
		}
		break;
	case 7:
		if (nbytes == sizeof(masq.tcp_fin_timeout))
		{
			memcpy(&masq.tcp_fin_timeout, buf, sizeof(masq.tcp_fin_timeout));
			return sizeof(masq.tcp_fin_timeout);
		}
		break;
	case 8:
		if (nbytes == sizeof(masq.udp_timeout))
		{
			memcpy(&masq.udp_timeout, buf, sizeof(masq.udp_timeout));
			return sizeof(masq.udp_timeout);
		}
		break;
	case 9:
		if (nbytes == sizeof(masq.icmp_timeout))
		{
			memcpy(&masq.icmp_timeout, buf, sizeof(masq.icmp_timeout));
			return sizeof(masq.icmp_timeout);
		}
		break;
	case 102:
		if (nbytes == 5 && strncmp("purge", buf, 5) == 0)
		{
			purge_port_records();
			return 5;
		}
	case 200:
		if (nbytes == sizeof(PORT_DB_RECORD) && (redirection = new_redirection()))
		{
			PORT_DB_RECORD *next;

			next = redirection->next_port;
			memcpy(redirection, buf, sizeof(PORT_DB_RECORD));
			redirection->next_port = next;
			return sizeof(PORT_DB_RECORD);
		}
		break;
	case 201:
		if (nbytes == sizeof(prev_redir->num) && prev_redir)
		{
			ushort port;

			memcpy(&port, buf, sizeof(port));

			if (port == prev_redir->num)
			{
				delete_redirection(prev_redir);
				return sizeof(port);
			}
		}
		break;
	}

	return 0;
}

#endif /* USE_MASQUERADE */
