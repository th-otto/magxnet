/*
 * This file belongs to FreeMiNT. It's not in the original MiNT 1.12
 * distribution. See the file CHANGES for a detailed log of changes.
 * 
 * 
 * Copyright 2000 Frank Naumann <fnaumann@freemint.de>
 * Copyright 2000 Vassilis Papathanassiou
 * All rights reserved.
 * 
 * Driver for Realtek's RTL8012 pocket lan adaptor connected to the cartridge
 * port via my hardware (adapter hardware description, schematics, links
 * etc can be found at http://users.otenet.gr/~papval).
 * 
 * Based on DE600 sources and skeleton by Kay Roemer.
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * 
 * begin:	2000-07-24
 * last change:	2000-07-24
 * 
 * Author:	Frank Naumann <fnaumann@freemint.de>
 * 
 * Please send suggestions, patches or bug reports to me or
 * the MiNT mailing list.
 * 
 */

#define __KERNEL_XDD__

#include "sockets.h"
#include "buf.h"
#include "bpf.h"
#include "if.h"
#include "ifeth.h"
struct in_ip_ops { int dummy; };
#include "ip.h"
#include "arp.h"

#include "rtl8012.h"
#include "rtl8012v.h"
#include "timeout.h"
#include "kerinfo.h"
#include "netinfo.h"

#include "arch/timer.h"
#include "arch/delay.h"
#include "mint/mdelay.h"
#include "sockios.h"
#include "mint/ssystem.h"
#include "mint/cookie.h"
#include "asm_spl.h"
#include "mxkernel.h"

#include <mint/osbind.h>
#define uchar unsigned char


#ifdef NO_DELAY
#define udelay(x) ((void)0)						/* on 68000 we don't need to delay */
#define mdelay(x) ((void)0)						/* on 68000 we don't need to delay */
#endif

#ifdef __mc68060__
#define UDELAY_060(x)	udelay(x)
#else
#define UDELAY_060(x)
#endif

#define wait5ms	mdelay (5)

static struct netif if_RTL12;

struct rtl12
{
	ushort tx_tail;						/* queue tail */
	ushort tx_used;						/* no of used queue entries */
};

static struct rtl12 rtl8012_priv;

static int int_installed;

/* hardware NIC address */
static unsigned char address[6] /* = { 0x00, 0x00, 0xe8, 0xe5, 0xb6, 0xc2 } */;

/* Avoid input from interrupt while outputing */
volatile short in_use GNU_ASM_NAME("in_use");

char message[100];

/* fast conversion table for byte writing */
short Table[256];

unsigned char RxBuffer[16];



#undef INLINE
#define INLINE static

/*
 * version
 */

#define VER_MAJOR	1
#define VER_MINOR	4
#define VER_STATUS

#define MSG_VERSION __STRINGIFY(VER_MAJOR) "." __STRINGIFY(VER_MINOR) VER_STATUS


#ifdef __PUREC__
#pragma warn -par /* using UNUSED here generates slightly different code */
#endif

#define c_conws(str) printstr(str)

void printstr(const char *str)
{
	while (*str)
	{
		Bconout(2, (unsigned char)*str++);
	}
}


/*
 * debugging stuff
 */

#if 0
#define XIF_DEBUG
#endif


/*
 * messages
 */

static const char *MSG_BOOT = "MagiCNet - MiNTNet RTL8012 pocket driver\r\n";

#define MSG_GREET	\
	"\275 2000 by Vassilis Papathanassiou.\r\n" \
	"\275 2000-2010 by Frank Naumann.\r\n\r\n"

#define MSG_MINT	\
	"\033pMiNT too old!\033q\r\n"

#define MSG_CPU_000	\
	"\033pThis driver work only on 68000!\033q\r\n"

#define MSG_CPU_020	\
	"\033pThis driver require at least a 68020!\033q\r\n"

#define MSG_PROBE	\
	"RTL8012: no adapter detected.\r\n"

#define MSG_FAILURE	\
	"RTL8012: timeout, check network cable!\r\n"

#if 0
INLINE uchar RdNib(ushort rreg);
INLINE void WrNib(uchar wreg, uchar wdata);
INLINE void WrByte(uchar wreg, uchar wdata);
INLINE uchar RdBytEP(void);
#endif

static long rtl8012_reset(struct netif *nif);
void rtl8012_install_int(void) GNU_ASM_NAME("rtl8012_install_int");
void rtl8012_stop(void) GNU_ASM_NAME("rtl8012_stop");
int send_block(uchar *buf, struct netif *nif, ushort len);
void rtl8012_active(void) GNU_ASM_NAME("rtl8012_active");
static long rtl8012_probe(struct netif *nif);
int rtl8012_doreset(int flag) GNU_ASM_NAME("rtl8012_doreset");
void GetNodeID(uchar *buf) GNU_ASM_NAME("GetNodeID");
void rtl8012_sethw(const unsigned char *address);
static void rtl8012_recv_packet(struct netif *nif);
int buf_empty(void);
ushort get_paclen(void);
void read_block(uchar *buf, ushort len);


#ifdef __GNUC__
INLINE void NIC_make_table(void)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int x = i;

		if (x & 1)
			x |= 0x100;

		Table[i] = x & ~1;
	}
}
#else
/* FIXME: needlessly implemented in assembler for Pure-C */
INLINE void NIC_make_table(void);
#endif


static long cdecl rtl8012_open(struct netif *nif)
{
	rtl8012_reset(nif);
	if (!int_installed)
	{
		rtl8012_install_int();
		int_installed = 1;
	}
	return 0;
}

static long cdecl rtl8012_close(struct netif *nif)
{
#ifndef __PUREC__
	UNUSED(nif);
#endif
	/* block out interrupt */
	in_use = 1;

	rtl8012_stop();

	/* allow interrupt */
	/* in_use = 0; */

	return 0;
}


#undef eth_build_hdr
#define eth_build_hdr _eth_build_hdr
/*
 * from kernel/ifeth.c
 * ugly hack: addr declared as long,to get it in register
 */
static BUF *eth_build_hdr(BUF *buf, struct netif *nif, long _addr, short type)
{
	struct eth_dgram *ep;
	BUF *nbuf;
	long len;

	(void)&nif;
	DEBUG(("eth_build_hdr( buf=0x%lx, nif='%s', type=%d )", (unsigned long) buf, nif->name, type));

	len = buf->dend - buf->dstart;
	if (len > ETH_MAX_DLEN)
	{
		buf_deref(buf, BUF_NORMAL);
		return NULL;
	}

	nbuf = buf_reserve(buf, sizeof(*ep), BUF_RESERVE_START);
	if (!nbuf)
	{
		buf_deref(buf, BUF_NORMAL);
		return NULL;
	}

	nbuf->dstart -= sizeof(*ep);
	ep = (struct eth_dgram *) nbuf->dstart;
	{
	char *addr;
	short *ps;
	addr = (char *)_addr;
	(void)&addr;
#ifdef __GNUC__
	{
		long *pp = (long *)ep->saddr;
		long *pl = (long *)nif->hwlocal.adr.bytes;
		*pp = *pl;
	}
#else
	*((long *)ep->saddr) = *((long *)nif->hwlocal.adr.bytes);
#endif
	ps = (short *)(ep->saddr + 4);
	*ps = *((short *)(nif->hwlocal.adr.bytes + 4));

#ifdef __GNUC__
	{
		long *pp = (long *)ep->daddr;
		long *pl = ((long *)_addr);
		*pp = *pl;
	}
#else
	*((long *)ep->daddr) = *((long *)_addr);
#endif
	ps = (short *)(ep->daddr + 4);
	*ps = *((short *)((char *)_addr + 4));
	}

	ep->proto = type == ETHPROTO_8023 ? (short) len : type;

	return nbuf;
}

/* transmitter pages */
#define TX_PAGES		2

/*
 * Some macros for managing the output queue.
 */
#define Q_EMPTY(p)		((p)->tx_used <= 0)
#define Q_FULL(p)		((p)->tx_used >= TX_PAGES)
#define Q_DEL(p)		{ (p)->tx_used--; (p)->tx_tail ^= 1; }
#define Q_ADD(p)		((p)->tx_used++)

static long cdecl rtl8012_output(struct netif *nif, BUF *buf, const char *hwaddr, short hwlen, short pktype)
{
	struct rtl12 *pr = nif->data;
	ushort len;
	BUF *nbuf;

#ifndef __PUREC__
	UNUSED(hwlen);
#endif
	if (in_use)
		return ENOMEM;

	nbuf = eth_build_hdr(buf, nif, (long)hwaddr, pktype);
	if (!nbuf)
	{
		DEBUG(("rtl8012: eth_build_hdr failed, out of memory!"));

		nif->out_errors++;
		return ENOMEM;
	}

	/* pass to upper layer */
	if (nif->bpf)
		bpf_input(nif, nbuf);

#ifdef NOTYET
	if (Q_FULL(pr))
	{
		DEBUG(("rtl8012: output queue full!"));

		buf_deref(nbuf, BUF_NORMAL);
		return ENOMEM;
	}
#endif

	len = nbuf->dend - nbuf->dstart;

#ifdef XIF_DEBUG
	if (in_use)
		ALERT(("rtl8012: in_use already set!!!"));
#endif

	/* block out interrupt */
	in_use = 1;

#ifdef NOTYET
	/*
	 * Store packet in 1rst or 2nd buffer
	 * It is done automaticaly for rtl80xx
	 */
	send_block((unsigned char *) nbuf->dstart, nif, len);

	if (!Q_EMPTY(pr))
	{
		long ticks = jiffies + 2;		/* normal is 1 */

		do
		{
			uchar stat;

			stat = RdNib(ISR);
#if 0
			stat |= (RdNib(ISR + HNib) << 4);
#endif

			/* Clear ISR */
			WrNib(ISR, EPLC_ROK + EPLC_TER + EPLC_TOK);
			WrNib(ISR + HNib, HNib + EPLC_RBER);

#define RX_GOOD	EPLC_ROK
#define TX_FAILED16	EPLC_TER
#define TX_OK		EPLC_TOK

			if (stat & TX_OK)
			{
				DEBUG(("rtl8012: TX_OK ()!"));
				Q_DEL(pr);
			} else if (stat == TX_FAILED16)
			{
				DEBUG(("rtl8012: TX_FAILED16!"));
				nif->collisions++;
				ticks = jiffies + 1;
			}
		} while (!Q_EMPTY(pr) && jiffies - ticks <= 0);

		/*
		 * transmission timed out
		 */
		if (!Q_EMPTY(pr))
		{
			DEBUG(("rtl8012: timeout, check network cable!"));
			Q_DEL(pr);

			in_use = 0;

			buf_deref(nbuf, BUF_NORMAL);
			rtl8012_reset(nif);

			nif->out_errors++;
			return ETIMEDOUT;
		}
	}

	Q_ADD(pr);

	/* Enet packets must have at least 60 bytes */
	len = MAX(len, 60);

	WrByte(TBCR0, len & 0xff);
	WrNib(TBCR1, len >> 8);
	WrNib(CMR1, EPLC_TRA);				/* start transmission */

#else

	/*
	 * Store packet in 1rst or 2nd buffer
	 * It is done automaticaly for rtl80xx
	 */
	if (send_block((unsigned char *) nbuf->dstart, nif, len))
	{
		/*
		 * transmission timed out
		 */
			c_conws(MSG_FAILURE);

			buf_deref(nbuf, BUF_NORMAL);
			nif->out_errors++;
			rtl8012_active();

			pr->tx_used--;
			pr->tx_tail ^= 1;
			
			in_use = 0;

			return ETIMEDOUT;
	}
#endif

	in_use = 0;
	nif->out_packets++;

	buf_deref(nbuf, BUF_NORMAL);

	return 0;
}



static long cdecl rtl8012_ioctl(struct netif *nif, short cmd, long arg)
{
#ifndef __PUREC__
	UNUSED(arg);
#endif
	switch (cmd)
	{
	case SIOCSIFNETMASK:
	case SIOCSIFFLAGS:
	case SIOCSIFADDR:
		return 0;

	case SIOCSIFMTU:
		/*
		 * Limit MTU to 1500 bytes. MintNet has already set nif->mtu
		 * to the new value, we only limit it here.
		 */
		if (nif->mtu > ETH_MAX_DLEN)
			nif->mtu = ETH_MAX_DLEN;
		return 0;
	}

	return ENOSYS;
}


long driver_init(void);
long driver_init(void)
{
	/* build fast conversion table */
	NIC_make_table();

	c_conws(MSG_BOOT);

#if 0
	c_conws(MSG_GREET);

	if (MINT_MAJOR == 0 || (MINT_MAJOR == 1 && ((MINT_MINOR < 15) || (MINT_KVERSION < 2))))
	{
		c_conws(MSG_MINT);
		goto failure;
	}

	{
		long cpu;

		if ((s_system(S_GETCOOKIE, C__CPU, (long) &cpu) != 0)
#if defined(__mc68020__) || defined(__mc68030__) || defined(__mc68040__) || defined(__mc68060__)
			|| (cpu < 20))
		{
			c_conws(MSG_CPU_020);
			goto failure;
		}
#else
			|| (cpu > 0))
		{
			c_conws(MSG_CPU_000);
			goto failure;
		}
#endif
	}
#endif

	strcpy(if_RTL12.name, "en");
	/* allocate unit number */
	if_RTL12.unit = if_getfreeunit("en");
	if_RTL12.metric = 0;
	if_RTL12.flags = IFF_BROADCAST;
	if_RTL12.mtu = 1500;
	if_RTL12.timer = 0;
	if_RTL12.hwtype = HWTYPE_ETH;
	if_RTL12.hwlocal.len = if_RTL12.hwbrcst.len = ETH_ALEN;
	if_RTL12.rcv.maxqlen = IF_MAXQ;
	if_RTL12.snd.maxqlen = IF_MAXQ;

	if_RTL12.open = rtl8012_open;
	if_RTL12.close = rtl8012_close;
	if_RTL12.output = rtl8012_output;
	if_RTL12.ioctl = rtl8012_ioctl;
	if_RTL12.timeout = 0;
	if_RTL12.data = &rtl8012_priv;
	if_RTL12.maxpackets = 16;

	if (rtl8012_probe(&if_RTL12) < 0)
	{
		c_conws("RTL8012: driver not installed.\r\n");
		return 1;
	}

	rtl8012_stop();
	in_use = 1;
	
	/* register in upper layer */
	if_register(&if_RTL12);

	ksprintf(message, "version %s (en%d) (%x:%x:%x:%x:%x:%x)\r\n\r\n",
		MSG_VERSION,
		if_RTL12.unit,
		if_RTL12.hwlocal.adr.bytes[0],
		if_RTL12.hwlocal.adr.bytes[1],
		if_RTL12.hwlocal.adr.bytes[2],
		if_RTL12.hwlocal.adr.bytes[3],
		if_RTL12.hwlocal.adr.bytes[4],
		if_RTL12.hwlocal.adr.bytes[5]);

	c_conws(message);
	return 0;
}


static long rtl8012_reset(struct netif *nif)
{
#ifndef __PUREC__
	UNUSED(nif);
#endif
	/* block out interrupt */
	in_use = 1;

	/* reset adapter */
	rtl8012_doreset(0);

	/* set hw address */
	rtl8012_sethw(address);

#if 0
	/* enable Tx and Rx */
	rtl8012_active();
#endif

	/* allow interrupt */
	in_use = 0;

	return 0;
}


static long rtl8012_probe(struct netif *nif)
{
	int i;

	/* reset adapter */
	if (rtl8012_doreset(1) == 0)
	{
		c_conws(MSG_PROBE);
		return -1;
	}

	wait5ms;

#if 0
	/* reset adapter */
	rtl8012_doreset(0);

	wait5ms;

	/* now run the memory test */
	if (rtl8012_ramtest())
		return -1;

	wait5ms;

	/* reset adapter */
	rtl8012_doreset(0);

	wait5ms;

	RdNib(CMR1);
#endif

	GetNodeID(address);
	DEBUG(("rtl8012_probe: MAC: %2x:%2x:%2x:%2x:%2x:%2x", test[0], test[1], test[2], test[3], test[4], test[5]));

	/*
	 * Try to read magic code and last 3 bytes of hw
	 * address (Not possible with RTL8012 for the moment)
	 */
	for (i = 0; i < ETH_ALEN; i++)
	{
		nif->hwlocal.adr.bytes[i] = address[i];
		nif->hwbrcst.adr.bytes[i] = 0xff;
	}

	return 0;
}


/*
 * from kernel/ifeth.c
 */
#undef eth_remove_hdr
#define eth_remove_hdr _eth_remove_hdr
short eth_remove_hdr(BUF *buf)
{
	struct eth_dgram *ep = (struct eth_dgram *) buf->dstart;
	long len;
	long nlen;
	unsigned short type;

	type = ep->proto >= 1536 ? ep->proto : ETHPROTO_8023;
	buf->dstart += sizeof(*ep);

	DEBUG(("eth_remove_hdr( buf=0x%lx, type=%d )", (unsigned long) buf, type));

	/*
	 * Correct packet length for to short packets. (Ethernet
	 * requires all packets to be padded to at least 60 bytes)
	 */
	len = buf->dend - buf->dstart;
	switch (type)
	{
	case ETHPROTO_IP:
		{
			struct ip_dgram *i = (struct ip_dgram *) buf->dstart;

			nlen = i->length;
			break;
		}
	case ETHPROTO_ARP:
	case ETHPROTO_RARP:
		{
			struct arp_dgram *a = (struct arp_dgram *) buf->dstart;

			nlen = ARP_LEN(a);
			break;
		}
	default:
		{
			nlen = len;
			break;
		}
	}

	if (nlen < len)
		buf->dend = buf->dstart + nlen;

	return type;
}


void rtl8012_int(void)
{
	while (!buf_empty())
		rtl8012_recv_packet(&if_RTL12);
}


/*
 * Read a packet out of the adapter and pass it to the upper layers
 */
INLINE void rtl8012_recv_packet(struct netif *nif)
{
	ushort pktlen;
	BUF *b;

	/* read packet length (excluding 32 bit crc) */
	if ((pktlen = (short)get_paclen()) < 32)
	{
		nif->in_errors++;
#if 0
		if (pktlen > 10000)
			rtl8012_reset(nif);
#endif
		return;
	}

	b = buf_alloc(pktlen + 100, 50, BUF_ATOMIC);

	if (!b)
	{
		DEBUG(("rtl8012_recv_packet: out of mem (buf_alloc failed)"));
		nif->in_errors++;
		return;
	}
	b->dend += pktlen;

	read_block((unsigned char *) b->dstart, pktlen);

	/* Pass packet to upper layers */
	if (nif->bpf)
		bpf_input(nif, b);

	/* and enqueue packet */
	if (!if_input(nif, b, 0, eth_remove_hdr(b)))
		nif->in_packets++;
	else
		nif->in_errors++;
}




/***********************************************
 *
 * everything below was implemented in assembler
 * 
 ***********************************************/

#if 0

/*
 * general register read
 * - 'End Of Read' command is not issued
 */
INLINE uchar RdNib(ushort rreg)
{
	uchar c;
	char dummy;

	dummy = *(volatile uchar *) (0xfa0000UL + Table[EOC + rreg]);
	dummy = *(volatile uchar *) (0xfa0000UL + Table[RdAddr + rreg]);

	UDELAY_060(1);

	/* Read nib */
#if 0
	/* *(volatile ushort *) 0xfa0000UL; */
	c = *(volatile ushort *) 0xfa0000UL;
#else
	c = *(volatile uchar *) 0xfa0001UL;
#endif
	UDELAY_060(1);

	dummy = *(volatile uchar *) (0xfa0000UL + Table[EOC + rreg]);

	(void)dummy;

	return c & 0x0f;
}

/* 
 * general register write
 * - 'End Of Write' is issued
 */
INLINE void WrNib(uchar reg, uchar value)
{
	volatile uchar *cartbase = (volatile uchar *)0xfa0000uL;
	ulong x;
	uchar dummy;
	
	/* End Of Write */
	dummy = cartbase[Table[EOC | reg]];


	x = WrAddr | reg;

	/* Prepare address */
	dummy = cartbase[Table[x]];

	x &= 0xf0;
	x |= value;

	/* Write address */
	dummy = cartbase[Table[x]];

	x = value + 0x80;

	/* Write data */
	dummy = cartbase[Table[x]];


	/* End Of Write */
	dummy = cartbase[Table[EOW | x]];
	(void)dummy;
}

INLINE void WrByte(uchar wreg, uchar wdata)
{
	WrNib(wreg, wdata & 0x0f);
	WrNib(wreg + HNib, HNib + ((wdata >> 4) & 0x0f));
}

INLINE uchar RdBytEP(void)
{
	uchar x;
	uchar c;
	uchar dummy;

	/* get lo nib */
	/* x = *(volatile ushort *) 0xfa0000UL; */
	x = *(volatile uchar *) 0xfa0001UL;

	c = x & 0x0f;

	UDELAY_060(1);

	/* CLK down */
	dummy = *(volatile uchar *) 0xfb0000UL;

	/* nop */
	UDELAY_060(1);

	/* get hi nib */
	/* x = *(volatile ushort *) 0xfa0000UL; */
	x = *(volatile uchar *) 0xfa0001UL;

	c |= (x << 4) & 0xf0;

	/* nop */

	/* CLK up */
	dummy = *(volatile uchar *) (0xfb0000UL + 0x2000);

	(void) dummy;

	return c;
}

/* DMACSB down, INITB,CLK up */
#define CSB_DN_CLK_UP	dummy = ( *(volatile uchar *) (0xfb0000UL + 0x2000), udelay (1) )
#if 0
#define CSB_DN_CLK_UP	dummy = *(volatile uchar *) (0xfb0000UL + 0x2000)
#endif

/* DMACSB,INITB,CLK up ie P14,P16,P17 up */
#define P14_16_17_UP	( dummy = *(volatile uchar *) (0xfb0000UL + 0xe000), udelay (1) )
#if 0
#define P14_16_17_UP	dummy = *(volatile uchar *) (0xfb0000UL + 0xe000)
#endif


/*
 * stop the adapter from receiving packets
 */
#if 0
static void rtl8012_stop(void)
{
	/* Set 'No Acception' */
	WrNib(CMR2 + HNib, HNib);

	/*  disable Tx & Rx */
	WrNib(CMR1 + HNib, HNib);
}
#endif

static void rtl8012_active(void)
{
	/* acception mode broadcast and physical */
	WrNib(CMR2 + HNib, HNib + RxMode);

	/* enable Rx & Tx */
	WrNib(CMR1 + HNib, HNib + EPLC_RxE + EPLC_TxE);
}


#if 0
static void rtl8012_doreset(void)
{
	ushort sr = splhigh();
	uchar i;

	DEBUG(("rtl8012: reset"));

	P14_16_17_UP;

	WrNib(MODSEL, 0);
	WrNib(MODSEL + HNib, HNib + 2);

	udelay(10);

	i = RdNib(MODSEL + HNib);
	DEBUG(("rtl8012: reset -> 1: %i [expected 2]", i));
	i = RdNib(MODSEL + HNib);
	DEBUG(("rtl8012: reset -> 1: %i [expected 2]", i));
	i = RdNib(MODSEL + HNib);
	DEBUG(("rtl8012: reset -> 1: %i [expected 2]", i));
#if 0
	if (i != 2)
		return;
#endif

	WrNib(CMR1 + HNib, HNib + EPLC_RST);

	do
	{
		udelay(1);
		i = RdNib(CMR1 + HNib);
	} while (i & 0x4);

	/* Clear ISR */
	WrNib(ISR, EPLC_ROK + EPLC_TER + EPLC_TOK);
	WrNib(ISR + HNib, HNib + EPLC_RBER);

	UDELAY_060(10);

	i = RdNib(CMR2 + HNib);				/* must be 2 (EPLC_AM1) */
	DEBUG(("rtl8012: reset -> 2: %i [expected 2]", i));
	i = RdNib(CMR2 + HNib);				/* must be 2 (EPLC_AM1) */
	DEBUG(("rtl8012: reset -> 2: %i [expected 2]", i));
	i = RdNib(CMR2 + HNib);				/* must be 2 (EPLC_AM1) */
	DEBUG(("rtl8012: reset -> 2: %i [expected 2]", i));

#ifdef XIF_DEBUG
	i = RdNib(CMR1);
	i |= (RdNib(CMR1 + HNib) << 4);
	DEBUG(("rtl8012: reset CMR1 0x%x", i));

	i = RdNib(CMR2);
	i |= (RdNib(CMR2 + HNib) << 4);
	DEBUG(("rtl8012: reset CMR2 0x%x", i));
#endif

	spl(sr);
}
#endif


static int rtl8012_ramtest(void)
{
#define SIZE 256
	ushort test[SIZE];
	ushort verify[SIZE];
	uchar dummy;

	int i;
	int r;

	DEBUG(("rtl8012: ramtest"));

	for (i = 0; i < SIZE; i++)
		test[i] = i | (i << 8);

	P14_16_17_UP;

	/* Mode A */
	WrNib(MODSEL, 0);

	/* 0-450ns */
	WrNib(MODDEL, 0);

	/* 1.disable Tx & Rx */
	WrNib(CMR1 + HNib, HNib);

	/* 2.enter RAM test mode */
	WrNib(CMR2, EPLC_RAMTST);

	/* 3.disable Tx & Rx */
	WrNib(CMR1 + HNib, HNib);

	/* 4.enable Tx & Rx */
	WrNib(CMR1 + HNib, HNib + EPLC_RxE + EPLC_TxE);

#if 0
	udelay(10);
#endif

	i = RdNib(CMR1 + HNib);
	DEBUG(("rtl8012: ramtest -> 1: %i", i));
	i = RdNib(CMR1 + HNib);
	DEBUG(("rtl8012: ramtest -> 1: %i", i));
	i = RdNib(CMR1 + HNib);
	DEBUG(("rtl8012: ramtest -> 1: %i", i));

	/* Mode C to Write test pattern */
	WrNib(MODSEL + HNib, HNib + 3);

	CSB_DN_CLK_UP;

	for (i = 0; i < SIZE; i++)
	{
		ushort word = test[i];

		/* 1st byte */
		dummy = *(volatile uchar *) (0xfa0000UL + Table[((word >> 8) & 0xff)]);

		/* CLK down */
		dummy = *(volatile uchar *) (0xfb0000UL);

		/* 2nd byte */
		dummy = *(volatile uchar *) (0xfa0000UL + Table[(word & 0xff)]);

		/* CLK up */
		dummy = *(volatile uchar *) (0xfb0000UL + 0x2000);
		(void) dummy;
	}

	P14_16_17_UP;

	WrNib(CMR1, WRPAC);

	/* Mode A */
	WrNib(MODSEL, 0);

	/* 6.disable Tx & Rx */
	WrNib(CMR1 + HNib, HNib);

	/* 7.enable Tx & Rx */
	WrNib(CMR1 + HNib, HNib + EPLC_RxE + EPLC_TxE);

	i = RdNib(CMR1 + HNib);
	WrNib(CMR1, RDPAC);

	CSB_DN_CLK_UP;

	/* Mode C to Read test pattern */
	WrNib(MODSEL + HNib, HNib + 1);

	for (i = 0; i < SIZE; i++)
	{
		ushort x;

		x = RdBytEP();
		x <<= 8;
		x |= RdBytEP();

		verify[i] = x;
	}

#if 0
	nop
	nop
#endif
	P14_16_17_UP;

	/* 9.disable Tx & Rx before leaving RAM test mode */
	WrNib(CMR1 + HNib, HNib);

	/* 10.leave RAM test mode */
	WrNib(CMR2, EPLC_CMR2Null);

#if 0
	/* Accept physical and broadcast */
	WrNib(CMR2 + HNib, HNib + RxMode);

	/* Enable Rx TEST ONLY!!! */
	WrNib(CMR1 + HNib, HNib + EPLC_RxE + EPLC_TxE);
#endif

	r = 0;
	for (i = 0; i < SIZE; i++)
	{
		if (test[i] != verify[i])
		{
			DEBUG(("rtl8012: ramtest -> %i [%i, %i]", i, test[i], verify[i]));
			r = 1;
		}
	}

	return r;
}

#if 0
;-------------------------------------------------------------------------
;	To read Ethernet node id and store in buffer 'ID_addr'
;	Assume : 9346 is used
;
;	entry : nothing!
;
;	exit  : Hopefuly the 6 bytes MAC in the buffer :-)
;
;	Note: If ID= 12 34 56 78 9A BC,  then the read bit sequence will be
;		 0011-0100 0001-0010 0111-1000 0101-0110 .......
;		 ( 3 - 4     1 - 2     7 - 8	 5 - 6	 ..... )
;
;-------------------------------------------------------------------------
#endif

static void GetNodeID(uchar *buf)
{
	ushort d0, d4, d5, d6, d7;
	ushort _d4, _d6;
	uchar dummy;

	WrNib(CMR2, EPLC_PAGE);				/* point to page 1 */
	/* issue 3 read command to */
	/* read 6 bytes id */
	d6 = 0;
	d4 = 0x100 | 0x80;					/* start bit | READ command */

  read_next_id_word:
	_d4 = d4;
	_d6 = d6;

	d0 = RdNib(PDR);
	DEBUG(("GetNodeID [1]: d4 = %x, d6= %x (d0 = %x)", d4, d6, d0));

#if 0
	WrNib(PCMR + HNib, 0x2);
#endif

	d0 = 0x100;							/* mask */
	for (d6 = 0; d6 < 9; d6++)
	{
		/* 
		 * DI = Data Input of 9346
		 * 
		 *        ________________
		 * CS : __|
		 *            ___     ___
		 * SK : ______|  |___|   |
		 *        _______ _______
		 * DI :  X_______X_______X
		 */

#if 0
		d5 = %00010110;					/* d5 = 000 HNIB  0 SKB CS 1 */
#endif
		d5 = 0x4 | 0x2;					/* d5 = 000 HNIB  0 SKB CS 1 */

#if 0
		moveq #0,d0
		roxl.w #1,d4
		addx d0, d5
#endif
		if (d4 & d0)
			d5 |= 0x1;					/* put DI bit to lsb of d5 */

		d0 >>= 1;

		WrNib(PCMR + HNib, d5);			/* pulls SK low and outputs DI */
		udelay(1);
		DEBUG(("1: PCMR+HNib = %x", RdNib(PCMR + HNib)));
		DEBUG(("1: PCMR+HNib = %x", RdNib(PCMR + HNib)));
		DEBUG(("1: PCMR+HNib = %x", RdNib(PCMR + HNib)));
		wait5ms;						/* wait about 5ms (long but...) */
#if 0
		d5 &= ~0x4;						/* let SKB=0 */
#endif
		WrNib(PCMR + HNib, (d5 & ~0x4));	/* Pulls SK high to end this cycle */
		udelay(1);
		DEBUG(("2: PCMR+HNib = %x", RdNib(PCMR + HNib)));
		DEBUG(("2: PCMR+HNib = %x", RdNib(PCMR + HNib)));
		DEBUG(("2: PCMR+HNib = %x", RdNib(PCMR + HNib)));
#if 0
		wait5ms;
#endif

#if 0
		DEBUG(("GetNodeID [2]: d6 = %i, d5 = %x, d0 = %x", d6, d5, d0));
#endif
	}

	wait5ms;
	goto read_bit;

  read_bit:
	d5 = 0;

	for (d7 = 0; d7 < 17; d7++)
	{
		/* read 17-bit data,
		 * order : 0, D15, D14, --- ,D0
		 * the 1st bit is the dummy
		 * bit '0', and is discarded.
		 */

		WrNib(PCMR + HNib, 0x4 | 0x2);	/* pull SK low */
		wait5ms;
		WrNib(PCMR + HNib, 0x2);		/* pull SK high to latch DO */
#if 0
		wait5ms;
#endif

		d0 = RdNib(PDR);
#if 0
		d0 = RdNib(PDR);
		d0 = RdNib(PDR);

		nop
		nop
		lsl.l #1,d5
#endif
		d5 <<= 1;

		if (d0 & 0x1)
			d5++;

		dummy = *(volatile uchar *) 0xfa007fUL;	/* EORead reg */
#if 0
		wait5ms;
#endif

#if 0
		DEBUG(("GetNodeID [3]: d7 = %i, d0 = %x", d7, d0));
#endif
		(void) dummy;
	}

	DEBUG(("GetNodeID [4]: d5 = %x", d5));
	goto read_bit_return;

  read_bit_return:
	d0 = d5;

	*buf++ = d0;

#if 0
	asr.w #8,d5
#endif
	d5 >>= 8;

	*buf++ = d5;

#if 0
	nop
	nop
#endif
#if 0
	WrNib(PCMR + HNib, %00010100);	/* 000 HNib 0 SKB CS 0 */
#endif
	WrNib(PCMR + HNib, 0x4);			/* 000 HNib 0 SKB CS 0 */
	/* let CS low to end this instruction */

	/* 5ms for inter-instruction gap */
	wait5ms;

	d4 = _d4;
	d6 = _d6;

	d4 += 1;

	d6++;
	if (d6 < 3)
		goto read_next_id_word;

	WrNib(CMR2, EPLC_CMR2Null);			/* back to page 0 */

	return;
}

/*
 * Set an Ethernet Address to IDR0-5 registers and mask 0xFFFFFFFF multicast
 */
static void rtl8012_sethw(const char *address)
{
	WrByte(IDR0, address[0]);
	WrByte(IDR1, address[1]);
	WrByte(IDR2, address[2]);
	WrByte(IDR3, address[3]);
	WrByte(IDR4, address[4]);
	WrByte(IDR5, address[5]);

#ifdef XIF_DEBUG
	{
		uchar c0, c1, c2, c3, c4,c5;

		c0 = RdNib(IDR0);
		c0 |= (RdNib(IDR0 + HNib) << 4);

		c1 = RdNib(IDR1);
		c1 |= (RdNib(IDR1 + HNib) << 4);

		c2 = RdNib(IDR2);
		c2 |= (RdNib(IDR2 + HNib) << 4);

		c3 = RdNib(IDR3);
		c3 |= (RdNib(IDR3 + HNib) << 4);

		c4 = RdNib(IDR4);
		c4 |= (RdNib(IDR4 + HNib) << 4);

		c5 = RdNib(IDR5);
		c5 |= (RdNib(IDR5 + HNib) << 4);

		DEBUG(("expected: %2x:%2x:%2x:%2x:%2x:%2x", address[0], address[1], address[2], address[3], address[4],
				address[5]));
		DEBUG(("readen:   %2x:%2x:%2x:%2x:%2x:%2x", c0, c1, c2, c3, c4, c5));
	}
#endif

	/* Now set NIC to accept broadcast */

	WrNib(CMR2, EPLC_PAGE);				/* point to page 1 */

	WrByte(MAR0, 0xff);
	WrByte(MAR1, 0xff);
	WrByte(MAR2, 0xff);
	WrByte(MAR3, 0xff);
	WrByte(MAR4, 0xff);
	WrByte(MAR5, 0xff);
	WrByte(MAR6, 0xff);
	WrByte(MAR7, 0xff);

#ifdef XIF_DEBUG
	{
		uchar c0, c1, c2, c3, c4, c5, c6, c7;

		c0 = RdNib(MAR0);
		c0 |= (RdNib(MAR0 + HNib) << 4);

		c1 = RdNib(MAR1);
		c1 |= (RdNib(MAR1 + HNib) << 4);

		c2 = RdNib(MAR2);
		c2 |= (RdNib(MAR2 + HNib) << 4);

		c3 = RdNib(MAR3);
		c3 |= (RdNib(MAR3 + HNib) << 4);

		c4 = RdNib(MAR4);
		c4 |= (RdNib(MAR4 + HNib) << 4);

		c5 = RdNib(MAR5);
		c5 |= (RdNib(MAR5 + HNib) << 4);

		c6 = RdNib(MAR6);
		c6 |= (RdNib(MAR6 + HNib) << 4);

		c7 = RdNib(MAR7);
		c7 |= (RdNib(MAR7 + HNib) << 4);

		DEBUG(("broad:    %2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x", c0, c1, c2, c3, c4, c5, c6, c7));
	}
#endif

	WrNib(CMR2, EPLC_CMR2Null);			/* back to page 0 */

	WrNib(IMR, 0);						/* EPLC_ROK+EPLC_TER+EPLC_TOK */
}


INLINE int send_block(uchar *buf, struct netif *nif, ushort len)
{
	int flag;
	int i;
	uchar dummy;

	(void)nif;
	WrNib(MODSEL, 0);					/* Mode A */
	WrNib(MODSEL + HNib, HNib + 3);	/* Mode C to Write packet */

	/* start data transfer */
	CSB_DN_CLK_UP;

	if (len & 1)
	{
		flag = 1;
		len--;
	} else
	{
		flag = 0;
	}

	i = 0;
	while (i < len)
	{
		/* 1st byte */
		dummy = *(volatile uchar *) (0xfa0000UL + Table[buf[i++]]);

		/* CLK down */
		dummy = *(volatile uchar *) (0xfb0000UL);

		/* 2nd byte */
		dummy = *(volatile uchar *) (0xfa0000UL + Table[buf[i++]]);

		/* CLK up */
		dummy = *(volatile uchar *) (0xfb0000UL + 0x2000);
	}

	if (flag)
	{
		dummy = *(volatile uchar *) (0xfa0000UL + Table[buf[i]]);

		/* CLK down */
		dummy = *(volatile uchar *) (0xfb0000UL);
	}
	(void) dummy;

	udelay(1);

	/* end of data transfer */
	P14_16_17_UP;

	/* Tx jump packet command */
	WrNib(CMR1, WRPAC);

	udelay(1);
	
	return 0;
}


static void rtl8012_install_int(void)
{
#define vector(x)	(x / 4)
	old_vbl_int = Setexc(vector(0x70), vbl_interrupt);
}

ushort get_paclen(void)
{
	ushort i;

#if 0
	i = RdNib(CMR1 + HNib);
	i = (RdNib(CMR1 + HNib) << 4) | RdNib(CMR1);
#endif

	/* Rx jump packet command */
	WrNib(CMR1, RDPAC);

	udelay(1);

	/* start data transfer */
	CSB_DN_CLK_UP;

	/* Mode C to Read from adapter buffer */
	WrNib(MODSEL + HNib, HNib + 1);

	/* readin header */
	for (i = 0; i < 8; i++)
		RxBuffer[i] = RdBytEP();

	/* size of packet */
	i = RxBuffer[RxMSB] << 8;
	i |= RxBuffer[RxLSB];

	if (RxBuffer[RxStatus] == 0)
	{
		P14_16_17_UP;
		return 0;
	}

	if (i < 8)
	{
		P14_16_17_UP;

		/* Accept all packets */
#if 0
		WrNib(CMR2 + HNib, HNib + RxMode);
#endif

		return 0;
	}

	if (i >= 0x800)
		/* A full ethernet packet plus CRC */
		i = 1536 + 4;

	/* 4 CRC + 1 for dbf */
	i -= 4;

	return i;
}


void read_block(uchar *buf, ushort len)
{
	int i;

	for (i = 0; i < len; i++)
		*buf++ = RdBytEP();

	udelay(1);

	P14_16_17_UP;
}

int buf_empty(void)
{
	return (RdNib(CMR1) & 0x1);
}

#endif
