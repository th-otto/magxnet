/*
 *	BSD compatible packet filter.
 *
 *	10/22/95, Kay Roemer.
 */

#include "sockets.h"
#define __BPF_IMPLEMENTATION__
#include "bpf.h"

#include <netinet/in.h>
#include <fcntl.h>
#include "ifeth.h"
#include "ip.h"

#include "buf.h"
#include "timer.h"
#include "inet.h"
#include "mxkernel.h"
#include "asm_spl.h"
#include "timeout.h"

#define BPF_MAXPACKETS	60
#define BPF_READBUF	(8*1024)
#define BPF_RESERVE	100
#define BPF_HDRLEN	(BPF_WORDALIGN (SIZEOF_BPF_HDR))

struct bpf
{
	struct bpf *link;					/* next desc in global list */
	struct bpf *next;					/* next desc for this if */
	struct ifq recvq;					/* packet input queue */
	struct netif *nif;					/* the if we are listening to */
	struct bpf_insn *prog;				/* filter program */
	short proglen;						/* # of insns in the filter program */
	long tmout;							/* read timeout */
	struct event evt;					/* timeout event */
	long rsel;							/* read-selecting process */
	volatile short flags;				/* what do you think? */
#define BPF_OPEN 0x0001
#define BPF_WAKE 0x0002
	long hdrlen;						/* MAC header length */
	long hwtype;						/* hardware type */

	long in_pkts;						/* # of packets received */
	long in_drop;						/* # of packets dropped */

	long s_time;						/* GMT at open() */
	long s_ticks;						/* HZ200 ticks at open() */
};

static long cdecl bpf_open(MX_DOSFD *);
static long cdecl bpf_close(MX_DOSFD *);
/* ugly hack here: function is not cdecl */
static long bpf_read(MX_DOSFD *, long, void *);
static long bpf_write(MX_DOSFD *, long, void *);
static long cdecl bpf_stat(MX_DOSFD *, MAGX_UNSEL *unsel, short rwflag, long /* APPL * */ appl);
static long cdecl bpf_lseek(MX_DOSFD *, long, short);
static long cdecl bpf_datime(MX_DOSFD *, short *, short);
static long cdecl bpf_ioctl(MX_DOSFD *, short, void *);
static long cdecl bpf_delete(MX_DOSFD *, MX_DOSDIR *dir);

static struct bpf *bpf_create(void);
static void bpf_release(struct bpf *);
static long bpf_attach(struct bpf *, struct ifreq *);
static void bpf_reset(struct bpf *);
static long bpf_sfilter(struct bpf *, struct bpf_program *);

extern MX_DDEV bpf_dev GNU_ASM_NAME("bpf_dev");

MX_DDEV cdecl_bpf_dev GNU_ASM_NAME("cdecl_bpf_dev") = {
	bpf_open,
	bpf_close,
	/* ugly hack here: function is not cdecl */
	(long cdecl (*)(MX_DOSFD *, long, void *))bpf_read,
	(long cdecl (*)(MX_DOSFD *, long, void *))bpf_write,
	bpf_stat,
	bpf_lseek,
	bpf_datime,
	bpf_ioctl,
	bpf_delete,
	0,
	0,
	0
};

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * BPF initialization
 */
void bpf_init(void)
{
	long r;

	r = Dcntl(DEV_M_INSTALL, "u:\\dev\\bpf0", (long) &bpf_dev);
	if (!r || r == ENOSYS)
		(void) Cconws("Cannot install BPF device\r\n");
}

/*** ---------------------------------------------------------------------- ***/

static struct bpf *allbpfs = 0;

/*
 * create a new packet filter
 */
static struct bpf *bpf_create(void)
{
	struct bpf *bpf;

	bpf = kmalloc(sizeof(*bpf));
	if (!bpf)
		return NULL;

	mint_bzero(bpf, sizeof(*bpf));

	bpf->recvq.maxqlen = BPF_MAXPACKETS;
	bpf->link = allbpfs;
	allbpfs = bpf;

	return bpf;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * destroy a packet filter
 */
static void bpf_release(struct bpf *bpf)
{
	struct bpf *bpf2;
	struct bpf **prev;
	ushort sr;

	sr = spl7();
	if_flushq(&bpf->recvq);
	if (bpf->flags & BPF_OPEN)
	{
		prev = &bpf->nif->bpf;
		bpf2 = *prev;
		for (; bpf2; prev = &bpf2->next, bpf2 = *prev)
		{
			if (bpf2 == bpf)
			{
				/*
				 * found
				 */
				*prev = bpf->next;
				break;
			}
		}
		if (!bpf2)
		{
			spl(sr);
			FATAL("bpf_release: bpf not in list!");
		}
	}
	spl(sr);

	prev = &allbpfs;
	for (bpf2 = *prev; bpf2; prev = &bpf->link, bpf2 = *prev)
	{
		if (bpf2 == bpf)
		{
			/*
			 * found
			 */
			*prev = bpf->link;
			break;
		}
	}

	if (!bpf2)
	{
		FATAL("bpf_release: bpf not in global list!");
	}

	if (bpf->prog)
		kfree(bpf->prog);

	kfree(bpf);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * attach an interface to the packet filter
 */
static long bpf_attach(struct bpf *bpf, struct ifreq *ifr)
{
	ushort sr;

	sr = spl7();
	if (bpf->flags & BPF_OPEN)
	{
		struct bpf *bpf2;
		struct bpf **prev;

		prev = &bpf->nif->bpf;
		bpf2 = *prev;
		for (; bpf2; prev = &bpf2->next, bpf2 = *prev)
		{
			if (bpf2 == bpf)
			{
				/*
				 * found
				 */
				*prev = bpf->next;
				break;
			}
		}
		bpf->next = 0;
		bpf->flags &= ~BPF_OPEN;

		if (!bpf2)
		{
			spl(sr);
			FATAL("bpf_attach: bpf not in list!");
		}
	}
	spl(sr);

	bpf->nif = if_name2if(ifr->ifr_name);
	if (!bpf->nif)
		return ENOENT;

	if (!(bpf->nif->flags & IFF_UP))
		return ENETDOWN;

	bpf->next = bpf->nif->bpf;
	bpf->nif->bpf = bpf;

	switch (bpf->nif->hwtype)
	{
	case HWTYPE_NONE:
		bpf->hwtype = DLT_NULL;
		bpf->hdrlen = 0;
		break;

	case HWTYPE_SLIP:
	case HWTYPE_PLIP:
		bpf->hwtype = DLT_SLIP;
		bpf->hdrlen = 0;
		break;

	case HWTYPE_PPP:
		bpf->hwtype = DLT_PPP;
		bpf->hdrlen = 4;
		break;

	case HWTYPE_ETH:
		bpf->hwtype = DLT_EN10MB;
		bpf->hdrlen = sizeof(struct eth_dgram);
		break;

	default:
		FATAL(("bpf_attach: unknown hardware type for if %s", bpf->nif->name));
		break;
	}

	bpf->flags |= BPF_OPEN;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * flush queue and reset counters
 */
static void bpf_reset(struct bpf *bpf)
{
	if_flushq(&bpf->recvq);
	bpf->in_pkts = 0;
	bpf->in_drop = 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * set new filer program
 */
static long bpf_sfilter(struct bpf *bpf, struct bpf_program *prog)
{
	struct bpf_insn *oprog;
	struct bpf_insn *nprog;
	long size;
	ushort sr;

	oprog = bpf->prog;
	if (prog->bf_insns == 0)
	{
		if (prog->bf_len != 0)
			return EINVAL;
		sr = spl7();
		bpf->prog = 0;
		bpf->proglen = 0;
		bpf_reset(bpf);
		spl(sr);
		if (oprog)
			kfree(oprog);
		return 0;
	}

	if (prog->bf_len > BPF_MAXINSNS)
		return EINVAL;

	size = prog->bf_len * sizeof(struct bpf_insn);
	nprog = kmalloc(size);
	if (!nprog)
		return ENOMEM;
	memcpy(nprog, prog->bf_insns, size);

	if (bpf_validate(nprog, prog->bf_len))
	{
		sr = spl7();
		bpf->prog = nprog;
		bpf->proglen = prog->bf_len;
		bpf_reset(bpf);
		spl(sr);
		if (oprog)
			kfree(oprog);
		return 0;
	}

	kfree(nprog);
	return EINVAL;
}

/*** ---------------------------------------------------------------------- ***/

static volatile short have_tmout;

/*
 * top half input handler.
 */
static void cdecl bpf_handler(PROC *proc, long arg)
{
	struct bpf *bpf;

	UNUSED(proc);
	UNUSED(arg);
	have_tmout = 0;
	for (bpf = allbpfs; bpf; bpf = bpf->link)
	{
		ushort sr = spl7();

		if (bpf->flags & BPF_WAKE)
		{
			bpf->flags &= ~BPF_WAKE;
			spl(sr);
			wake(IO_Q, (long) bpf);
			if (bpf->rsel)
				wakeselect(bpf->rsel);
		} else
			spl(sr);
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * bottom half input handler, called from interface input routine.
 * packet contains MAC header.
 */
long cdecl bpf_input(struct netif *nif, BUF *buf)
{
	struct bpf *bpf;
	ulong caplen;
	ulong pktlen;
	ulong snaplen;
	ulong ticks;
	ulong align;
	struct bpf_hdr *hp;
	BUF *buf2;
	ushort sr;

	for (bpf = nif->bpf; bpf; bpf = bpf->next)
	{
		if (!(bpf->flags & BPF_OPEN))
			continue;
		bpf->in_pkts++;
		pktlen = buf->dend - buf->dstart;
		snaplen = bpf_filter(bpf->prog, (unsigned char *) buf->dstart, pktlen, pktlen);
		if (snaplen == 0)
			continue;

		caplen = BPF_HDRLEN + MIN(snaplen, pktlen);
		if (caplen > BPF_READBUF - BPF_ALIGNMENT)
			caplen = BPF_READBUF - BPF_ALIGNMENT;

		/*
		 * We have to be careful. A driver with higher interrupt
		 * priority may grap the buffer and manipulate the dend
		 * and dstart pointer while we are on them.
		 */
		sr = spl7();
		if ((buf2 = bpf->recvq.qlast[IF_PRIORITIES - 1]) != NULL &&
			BUF_TRAIL_SPACE(buf2) >= caplen + BPF_ALIGNMENT + BPF_RESERVE / 2)
		{
			/*
			 * We can prepend the packet to a previously
			 * allocated buf. We leave BPF_RESERVE/2 bytes
			 * of space at the end of the buf.
			 */
			align = BPF_WORDALIGN((long) buf2->dend - (long) buf2->dstart);
			buf2->dend = buf2->dstart + align;
		} else
		{
			if (bpf->recvq.qlen >= BPF_MAXPACKETS)
			{
				spl(sr);
				++bpf->in_drop;
				continue;
			}
			spl(sr);
			buf2 = buf_alloc(caplen + BPF_RESERVE, BPF_RESERVE / 2, BUF_ATOMIC);
			if (!buf2)
			{
				++bpf->in_drop;
				continue;
			}
			sr = spl7();
			if (if_enqueue(&bpf->recvq, buf2, IF_PRIORITIES - 1))
			{
				spl(sr);
				++bpf->in_drop;
				continue;
			}
		}
		hp = (struct bpf_hdr *) buf2->dend;
		buf2->dend += caplen;
		/*
		 * All manipulations on buf pointers done. So we can switch
		 * interrupts back on.
		 */
		bpf->flags |= BPF_WAKE;
		if (!have_tmout)
		{
			have_tmout = 1;
			spl(sr);
			addroottimeout(0, bpf_handler, 1);
		} else
			spl(sr);

		ticks = (GETTIME() - bpf->s_ticks) * 5;
		hp->bh_tstamp.tv_sec = bpf->s_time + ticks / 1000;
		hp->bh_tstamp.tv_usec = (ticks % 1000) * 1000;

		hp->bh_datalen = pktlen;
		hp->bh_hdrlen = BPF_HDRLEN;
		hp->bh_caplen = (caplen -= BPF_HDRLEN);

		memcpy((char *) hp + BPF_HDRLEN, buf->dstart, caplen);
	}

	return 0;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#ifdef __PUREC__
#pragma warn -rch /* for p_geteuid */
#endif

/*
 * /dev/bpf device driver
 */
static long cdecl bpf_open(MX_DOSFD *fp)
{
	struct bpf *bpf;

	if (p_geteuid() != 0)
		return EACCES;

	bpf = bpf_create();
	if (!bpf)
		return ENOMEM;

	bpf->s_time = unixtime(Tgettime(), Tgetdate());
	bpf->s_ticks = GETTIME();

	fp->fd_user1 = (long) bpf;
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Passed data contains the MAC header!
 */
static long bpf_write(MX_DOSFD *fp, long nbytes, void *buf)
{
	struct bpf *bpf = (struct bpf *) fp->fd_user1;
	short daddrlen;
	short pktype;
	const void *daddr;
	BUF *b;

	if (!(bpf->flags & BPF_OPEN))
		/*
		 * this maps to ENXIO in the lib
		 */
		return ENXIO;

	b = buf_alloc(nbytes + BPF_RESERVE, BPF_RESERVE / 2, BUF_NORMAL);
	if (!b)
		return ENOMEM;

	switch (bpf->nif->hwtype)
	{
	case HWTYPE_NONE:
	case HWTYPE_SLIP:
	case HWTYPE_PLIP:
		/*
		 * Assume IP... and pass IP dst as next hop.
		 */
		daddr = &((const struct ip_dgram *) buf)->daddr;
		daddrlen = 4;
		pktype = PKTYPE_IP;
		break;

	case HWTYPE_PPP:
		/*
		 * may not be IP...
		 */
		daddr = &((const struct ip_dgram *) ((char *)buf + bpf->hdrlen))->daddr;
		daddrlen = 4;
		pktype = PKTYPE_IP;
		break;

	case HWTYPE_ETH:
		daddr = &((const struct eth_dgram *) buf)->daddr;
		daddrlen = ETH_ALEN;
		memcpy(&pktype, &((const struct eth_dgram *) buf)->proto, 2);
		if (pktype >= 1536)
			pktype = ETHPROTO_8023;
		break;

	default:
		FATAL(("bpf_write: unknown hardware type for if %s", bpf->nif->name));
		return EINTERNAL;
	}

	if (nbytes < bpf->hdrlen)
		return EINVAL;

	memcpy(b->dstart, (char *)buf + bpf->hdrlen, nbytes - bpf->hdrlen);
	b->dend += nbytes - bpf->hdrlen;

	return (*bpf->nif->output) (bpf->nif, b, daddr, daddrlen, pktype);
}

/*** ---------------------------------------------------------------------- ***/

static void wakemeup(long arg)
{
	wake(IO_Q, arg);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * the following data ist returned by read():
 *
 * N					: struct bpf_hdr  #1
 * N + bh_hdrlen			: packet snapshot #1
 * N + bh_hdrlen + ALIGN4(bh_caplen)	: struct bpf_hdr  #2
 * ...
 */
static long bpf_read(MX_DOSFD *fp, long nbytes, void *buf)
{
	struct bpf *bpf = (struct bpf *) fp->fd_user1;
	BUF *b;

	if (!(bpf->flags & BPF_OPEN))
		return ENXIO;

	if (nbytes < BPF_READBUF)
		return EINVAL;

	if ((b = if_dequeue(&bpf->recvq)) == NULL)
	{
		if (fp->fd_mode & O_NDELAY)
			return EWOULDBLOCK;
		if (bpf->tmout)
			event_add(&bpf->evt, bpf->tmout, wakemeup, (long) bpf);
		if (sleep(IO_Q, (long) bpf))
			return EINTR;
		if (bpf->tmout)
			event_del(&bpf->evt);
		if ((b = if_dequeue(&bpf->recvq)) == NULL)
			return EWOULDBLOCK;
	}

	nbytes = b->dend - b->dstart;
	memcpy(buf, b->dstart, nbytes);
	buf_deref(b, BUF_NORMAL);

	return nbytes;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_lseek(MX_DOSFD *fp, long where, short whence)
{
	UNUSED(fp);
	UNUSED(where);
	UNUSED(whence);
	return EACCES;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_ioctl(MX_DOSFD *fp, short cmd, void *arg)
{
	struct bpf *bpf = (struct bpf *) fp->fd_user1;
	BUF *b;
	ushort sr;

	switch (cmd)
	{
	case FIONREAD:
		b = bpf->recvq.qfirst[IF_PRIORITIES - 1];
		*(long *) arg = b ? b->dend - b->dstart : 0;
		return 0;

	case FIONWRITE:
		*(long *) arg = 1;
		return 0;

	case FIOEXCEPT:
		*(long *) arg = 0;
		return 0;

	case SIOCGIFADDR:
		return if_ioctl(SIOCGIFADDR, (long) arg);

	case BIOCGFLEN:
		/*
		 * max # of insns in a filter program
		 */
		*(long *) arg = BPF_MAXINSNS;
		return 0;

	case BIOCGBLEN:
		/*
		 * read() buffer length
		 */
		*(long *) arg = BPF_READBUF;
		return 0;

	case BIOCSETF:
		/*
		 * set filter program
		 */
		return bpf_sfilter(bpf, (struct bpf_program *) arg);

	case BIOCFLUSH:
		/*
		 * flush in-queue
		 */
		sr = spl7();
		bpf_reset(bpf);
		spl(sr);
		return 0;

	case BIOCPROMISC:
		/*
		 * set if to promisc mode (not yet)
		 */
		if (!(bpf->flags & BPF_OPEN))
			return ENXIO;
		return 0;

	case BIOCGDLT:
		/*
		 * Get link level hardware type
		 */
		if (!(bpf->flags & BPF_OPEN))
			return ENXIO;
		*(long *) arg = bpf->hwtype;
		return 0;

	case BIOCGETIF:
		/*
		 * get interface name
		 */
		if (bpf->flags & BPF_OPEN)
		{
#if 0
			struct ifreq *ifr = (struct ifreq *) arg;

			ksprintf(ifr->ifr_name, "%s%d", bpf->nif->name, bpf->nif->unit);
#endif
			return 0;
		}
		return ENXIO;

	case BIOCSETIF:
		/*
		 * attach to interface
		 */
		return bpf_attach(bpf, (struct ifreq *) arg);

	case BIOCSRTIMEOUT:
		/*
		 * set read timeout
		 */
		bpf->tmout = *(long *) arg / EVTGRAN;
		return 0;

	case BIOCGRTIMEOUT:
		/*
		 * get read timeout
		 */
		*(long *) arg = bpf->tmout * EVTGRAN;

	case BIOCGSTATS:
		((struct bpf_stat *) arg)->bs_recv = bpf->in_pkts;
		((struct bpf_stat *) arg)->bs_drop = bpf->in_drop;
		return 0;

	case BIOCIMMEDIATE:
		/*
		 * set immediate mode
		 */
		return 0;

	case BIOCVERSION:
		/*
		 * get verion info
		 */
		((struct bpf_version *) arg)->bv_major = BPF_MAJOR_VERSION;
		((struct bpf_version *) arg)->bv_minor = BPF_MINOR_VERSION;
		return 0;
	}

	return ENOSYS;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_datime(MX_DOSFD *fp, short *timeptr, short mode)
{
	UNUSED(fp);
	if (mode == 0)
	{
		timeptr[0] = Tgettime();
		timeptr[1] = Tgetdate();

		return 0;
	}

	return EACCES;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_close(MX_DOSFD *fp)
{
	struct bpf *bpf = (struct bpf *) fp->fd_user1;

	if (bpf->rsel)
		wakeselect(bpf->rsel);

	wake(IO_Q, (long) bpf);

	if (fp->fd_refcnt <= 0)
		bpf_release(bpf);

	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_stat(MX_DOSFD *fp, MAGX_UNSEL *unsel, short rwflag, long appl)
{
	struct bpf *bpf = (struct bpf *) fp->fd_user1;

	UNUSED(unsel);
	switch (rwflag)
	{
	case O_RDONLY:
		if (bpf->recvq.qfirst[IF_PRIORITIES - 1])
			return 1;
		if (bpf->rsel == 0)
		{
			bpf->rsel = appl;
			return 0;
		}
		return 2;

	case O_WRONLY:
		return 1;

	case O_RDWR:
		return 0;
	}

	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static long cdecl bpf_delete(MX_DOSFD *f, MX_DOSDIR *dir)
{
	/* Nothing to do */
	UNUSED(f);
	UNUSED(dir);
	return 0;
}
