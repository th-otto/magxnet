*************************************************************************
*                                                                       *
* MagiCNet driver for EtherNE NE2000 adapter                            *
* Works also with MintNet.                                              *
* Generic for ACSI and Cartridge Port interface hardware                *
* Copyright 2001-2002 Dr. Thomas Redelberger                            *
* Use it under the terms of the GNU General Public License              *
* (See file COPYING.TXT)                                                *
*                                                                       *
* Module to install and activate the port and to interface transmit     *
* to MagiCNet                                                           *
* Receive (done from HZ200 interrupts) is also installed and initiated  *
* from here                                                             *
*                                                                       *
* Limitations:                                                          *
*       Can handle only one interface                                   *
*                                                                       *
* Credits:                                                              *
*       Based on dummyeth sceleton, Kay Roemer, 1994-12-14.             *
*                                                                       *
* Tab size 8, developed with DEVPAC ST 2.0                              *
*************************************************************************

*
* development switches
*

MajVersion	=	1
MinVersion	=	14
		.include	"devswit.i"

*
* configuration switches
*

USEKERNEL	EQU	0		/* we do not need to access the KERNEL via a pointer */


* entry points and references in this module
	.IFNE	USEKERNEL
		.globl	KERNEL
	.ENDC
		.globl	if_ENE				/* structure for interface en0 */
		.globl	netinfo				/* pointer to structure of MNet functions */

* references from NE.S
		.xref	ei_probe1			/* (void); */
		.xref	ei_open				/* (void); */
		.xref	ei_close			/* (void); */
		.xref	ei_start_xmit			/* (); */
		.xref	ei_interrupt			/* (void); */
		.xref	DVS				/* from that we just need... */
dev_dev_addr	EQU	10				/* ...this */

*
* manifest constants
*
NERMAXPCKT	EQU	4			/* max #packets hardware can receive rapidly */

*
* includes
*

* my stuff
		.include	"uti.i"		/* debugging and stack handling macros */

* MNet stuff, offsets in netinfo and if
		.include	"buf.i"
		.include	"if.i"
		.include	"netinfo.i"



		.TEXT


*
* init MUST be the first executable code in the TEXT segment.
* There is no other startup code.


*********************************************************************************
*
* Initialization. This is called when the driver is loaded.
*
* You should probe for your hardware here, setup the interface
* structure and register your interface.
*
* This function should return 0 on success and != 0 if initialization
* fails.
*

RinNif		EQU	a4

Rin		REG	d3/RinNif

init:
		movem.l	#Rin,-(sp)
		move.l	16(sp),netinfo			/* get pointer to netinfo */
	IFNE	USEKERNEL
		move.l	12(sp),KERNEL
	ENDC
		jsr	ei_probe1
		tst.l	d0				/* found hardware? */
		beq.b	c1_init
		moveq	#1,d0
		bra	quit_init				/* no quit */

c1_init:
		lea	if_ENE,RinNif			/* this interface */

* Set interface unit. if_getfreeunit("name") returns a yet
* unused unit number for the interface type "name".
; 		If_getfreeunit	if_name(RinNif)
                dc.w 0x486c,0x0000
                move.l  netinfo,a0
                move.l  if_getfreeunit(a0),a0
                jsr     (a0)
                addq    #4,sp                   /* pop arg */
	
		move	d0,if_unit(RinNif)		/* store unit */
		lea	mess3_init(pc),a0
		add.b	d0,(a0)				/* update interface name in message */

		lea	DVS+dev_dev_addr,a0		/* copy MAC ei_probe1 got... */
		lea	if_hwlocalAddr(RinNif),a1	/* ...to MNet interface structure */
		move.w	(a0)+,(a1)+
		move.w	(a0)+,(a1)+
		move.w	(a0)+,(a1)+

* register the interface
		If_register	(RinNif)

* say we are alive...
		move.l	netinfo,a0
		move.l	ni_fname(a0),d0			/* print file name if existing */
		beq.b	c2_init

		move.l	d0,a0
		PrS	(a0)

c2_init:
		PrS	mess1_init(pc)			/* print message */
		lea	if_hwlocalAddr(RinNif),a0	/* MAC */
		moveq	#5,d3

t1_init:
		PrB	(a0)+				/* print a MAC byte */
		PrS	colon(pc)			/* print colon */
		dbra	d3,t1_init

		PrS	bscrlf(pc)			/* eat the last colon and CRLF */

		bsr.b	NEInstInt			/* install our HZ200 interrupt handler */
		moveq	#0,d0				/* OK */

quit_init:
		movem.l	(sp)+,#Rin
		rts

mess1_init:
		DC.B	' EtherNE driver v'
		.dc.b  MajVersion+'0'
		.dc.b '.'
		.dc.b (MinVersion/10)+'0'
		.dc.b (MinVersion-((MinVersion/10)*10))+'0'
mess2_init:
		IFNE (RXDEBPRT+TXDEBPRT+MACAddDEBPRT)
		DC.B    ' dev'
		ELSE
		DC.B    ' prd'
		ENDC
		DC.B	' (C) 2002 Dr. Thomas Redelberger',13,10
		DC.B	'Device (en'
mess3_init:
		DC.B	'0) MAC: ',0

bscrlf:		DC.B	8,32
crlf:		DC.B	13,10,0
colon:		DC.B	':',0
		EVEN

*********************************************************************************
* NEInstInt installs our interrupt handler in the HZ200 interrupt.
* As the old vector gets called in a chain like manner, I have to disable all ints
* until the chain is ready
*********************************************************************************

HZ200TrapNo	EQU	$114/4 

NEInstInt:
		move	sr,-(sp)		/* save int. level */
		ori	#$700,sr		/* disable all ints. */

		pea	myHZ200(pc)		/* new vector */
		move.w	#HZ200TrapNo,-(sp)
		move.w	#5,-(sp)		/* Setexc */
		trap	#13			/* BIOS */
		addq.l	#8,sp
		lea	oldHZ200(pc),a0
		move.l	d0,(a0)			/* save old vector */

		move	(sp)+,sr		/* restore ints. */
		rts


		DC.B	"XBRA"
		DC.L	"TREN"
oldHZ200:
		DC.L	0


*********************************************************************************
* myHZ200 is just a wrapper around ei_interrupt as ei_interrupt does not
* preserve d0-d2 and a0-a1
*********************************************************************************

RmH		REG	d0-d2/a0-a2

myHZ200:
		movem.l	#RmH,-(sp)

		jsr	ei_interrupt

		bsr	ENE_re_xmit

		movem.l	(sp)+,#RmH

* _branch_ to the old vector without using a register;
* the old vector will do the rte finally
		move.l	oldHZ200(pc),-(sp)
		rts



*********************************************************************************
*
* This gets called when someone makes an 'ifconfig up' on this interface
* and the interface was down before.
*

ENE_open:
		jsr	ei_open
		rts


*********************************************************************************
*
* Opposite of ENE_open(), is called when 'ifconfig down' on this interface
* is done and the interface was up before.
*

ENE_close:
		jsr	ei_close
		rts



*********************************************************************************
*
* we need to put back into the queue, but if_putback is missing from struct netinfo
*
RipBuf		EQU	a3
RipQSnd		EQU	a4

Rip		REG	d3-d4/RipBuf/RipQSnd

if_putback:
		movem.l	#Rip,-(sp)
		move.l	a0,RipQSnd			/* save arg: sndqueue */
		move.l	a1,RipBuf			/* save arg: buf */
		move	d0,d4				/* save arg: pri */

		move	sr,d3				/* save int. level */
		ori	#$700,sr			/* disable all ints. */

		move	q_qlen(RipQSnd),d1
;		cmp	q_maxqlen(RipQSnd),d1
	.dc.w 0xb26c,0
		bge.b	err_putback

		cmp	#IF_PRIORITIES,d4		/* pri >= Priorities ? */
		bcs.b	c2_putback			/* fix to maximum */
		moveq	#IF_PRIORITIES-1,d4
c2_putback:
		lsl	#2,d4				/* ->longword index */
		move.l	q_qfirst(RipQSnd,d4),bf_link3(RipBuf)	/* buf->link3 = q->qfirst[pri] */
		move.l	RipBuf,q_qfirst(RipQSnd,d4)	/* q->qfirst[pri] = buf */
		tst.l	q_qlast(RipQSnd,d4)		/* if (!q->qlast[pri]) */
		bne.b	c3_putback
		move.l	RipBuf,q_qlast(RipQSnd,d4)	/* q->qlast[pri] = buf */

c3_putback:
		addq	#1,q_qlen(RipQSnd)		/* q->qlen++ */
		move	d3,sr				/* restore ints. */
		moveq	#0,d0				/* OK */

quit_putback:
		movem.l	(sp)+,#Rip
		rts


* queue full, dropping packet
err_putback:
		Buf_deref	(RipBuf),#BUF_ATOMIC
		move	d3,sr				/* restore ints. */
		moveq	#-3,d0				/* error */
		bra.b	quit_putback



*********************************************************************************
*
* this is called from our HZ200 interrupt handler.
* It checks for packets that have been queued and tries to send *one*.
* As this may fail (e.g. bus in use or send in progress) it may put the packet
* back in the queue
*

RrxBuf		EQU	a3
RrxNif		EQU	a4

Rrx		REG	RrxBuf/RrxNif

ENE_re_xmit:
		movem.l	#Rrx,-(sp)
		lea	if_ENE,RrxNif
		move	if_snd+q_qlen(RrxNif),d0	/* something to send? */
		ble.b	quit_xmit

		If_dequeue	if_snd(RrxNif)
		tst.l	d0
		beq.b	quit_xmit

		move.l	d0,RrxBuf			/* buf */
		moveq	#0,d1				/* arg4: length second portion=0 */
		suba.l	a1,a1				/* arg3: NULL */
		move.l	bf_dend(RrxBuf),d0
		move.l	bf_dstart(RrxBuf),a0		/* arg1: address first portion */
		sub.l	a0,d0				/* arg2: length  first portion */
		jsr	ei_start_xmit
		tst.l	d0				/* successfully sent? */
		bne.b	sendlater

		addq.l	#1,if_out_packets(RrxNif)	/* yes */
		Buf_deref	(RrxBuf),#BUF_NORMAL

quit_xmit:
		movem.l	(sp)+,#Rrx
		rts


sendlater:
		move.l	bf_info(RrxBuf),d0		/* arg3: info */
		move.l	RrxBuf,a1			/* arg2: buf */
		lea	if_snd(RrxNif),a0		/* arg1: if_snd */
		bsr	if_putback
		bra.b	quit_xmit




*********************************************************************************
*
* This routine is responsible for enqueing a packet for later sending.
* The packet it passed in `buf', the destination hardware address and
* length in `hwaddr' and `hwlen' and the type of the packet is passed
* in `pktype'.
*
* `hwaddr' is guaranteed to be of type nif->hwtype and `hwlen' is
* garuanteed to be equal to nif->hwlocal.len.
*
* `pktype' is currently one of (definitions in if.h):
*	PKTYPE_IP for IP packets,
*	PKTYPE_ARP for ARP packets,
*	PKTYPE_RARP for reverse ARP packets.
*
* These constants are equal to the ethernet protocol types, ie. an
* Ethernet driver may use them directly without prior conversion to
* write them into the `proto' field of the ethernet header.
*
* If the hardware is currently busy, then you can use the interface
* output queue (nif->snd) to store the packet for later transmission:
*	if_enqueue (&nif->snd, buf, buf->info).
*
* `buf->info' specifies the packet's delivering priority. if_enqueue()
* uses it to do some priority queuing on the packets, ie. if you enqueue
* a high priority packet it may jump over some lower priority packets
* that were already in the queue (ie that is *no* FIFO queue).
*
* You can dequeue a packet later by doing:
*	buf = if_dequeue (&nif->snd);
*
* This will return NULL is no more packets are left in the queue.
*
* The buffer handling uses the structure BUF that is defined in buf.h.
* Basically a BUF looks like this:
*
* typedef struct {
*	long buflen;
*	char *dstart;
*	char *dend;
*	...
*	char data[0];
* } BUF;
*
* The structure consists of BUF.buflen bytes. Up until BUF.data there are
* some header fields as shown above. Beginning at BUF.data there are
* BUF.buflen - sizeof (BUF) bytes (called userspace) used for storing the
* packet.
*
* BUF.dstart must always point to the first byte of the packet contained
* within the BUF, BUF.dend points to the first byte after the packet.
*
* BUF.dstart should be word aligned if you pass the BUF to any MintNet
* functions! (except for the buf_* functions itself).
*
* BUF's are allocated by
*	nbuf = buf_alloc (space, reserve, mode);
*
* where `space' is the size of the userspace of the BUF you need, `reserve'
* is used to set BUF.dstart = BUF.dend = BUF.data + `reserve' and mode is
* one of
*	BUF_NORMAL for calls from kernel space,
*	BUF_ATOMIC for calls from interrupt handlers.
*
* buf_alloc() returns NULL on failure.
*
* Usually you need to pre- or postpend some headers to the packet contained
* in the passed BUF. To make sure there is enough space in the BUF for this
* use
*	nbuf = buf_reserve (obuf, reserve, where);
*
* where `obuf' is the BUF where you want to reserve some space, `reserve'
* is the amount of space to reserve and `where' is one of
*	BUF_RESERVE_START for reserving space before BUF.dstart
*	BUF_RESERVE_END for reserving space after BUF.dend
*
* Note that buf_reserve() returns pointer to a new buffer `nbuf' (possibly
* != obuf) that is a clone of `obuf' with enough space allocated. `obuf'
* is no longer existant afterwards.
*
* However, if buf_reserve() returns NULL for failure then `obuf' is
* untouched.
*
* buf_reserve() does not modify the BUF.dstart or BUF.dend pointers, it
* only makes sure you have the space to do so.
*
* In the worst case (if the BUF is to small), buf_reserve() allocates a new
* BUF and copies the old one to the new one (this is when `nbuf' != `obuf').
*
* To avoid this you should reserve enough space when calling buf_alloc(), so
* buf_reserve() does not need to copy. This is what MintNet does with the BUFs
* passed to the output function, so that copying is never needed. You should
* do the same for input BUFs, ie allocate the packet as eg.
*	buf = buf_alloc (nif->mtu+sizeof (eth_hdr)+100, 50, BUF_ATOMIC);
*
* Then up to nif->mtu plus the legth of the ethernet header bytes long
* frames may ne received and there are still 50 bytes after and before
* the packet.
*
* If you have sent the contents of the BUF you should free it by calling
*	buf_deref (`buf', `mode');
*
* where `buf' should be freed and `mode' is one of the modes described for
* buf_alloc().
*
* Functions that can be called from interrupt:
*	buf_alloc (..., ..., BUF_ATOMIC);
*	buf_deref (..., BUF_ATOMIC);
*	if_enqueue ();
*	if_dequeue ();
*	if_input ();
*	eth_remove_hdr ();
*	addroottimeout (..., ..., 1);
*

ReoNBuf		EQU	a3
ReoNif		EQU	a4

Reo		REG	ReoNBuf/ReoNif

ENE_output:
		movem.l	#Reo,-(sp)
		move.l	12(sp),ReoNif		/* save arg1: *nif */
		move.l	16(sp),a0		/* arg2: *buf */
		move.l	20(sp),a1		/* arg3: *hwaddr */
**		move	24(sp),d0		/* arg4: hwlen */
**		move	26(sp),d1		/* arg5: pktype */

* Attach eth header. MNet provides you with the eth_build_hdr function that attaches
* an ethernet header to the packet in buf. It takes the BUF (buf), the interface (nif),
* the hardware address (hwaddr) and the packet type (pktype).
*
* Returns NULL if the header could not be attached (the passed buf is thrown away 
* in this case).
*
* Otherwise a pointer to a new BUF with the packet and attached header is returned 
* and the old buf pointer is no longer valid.

		Eth_build_hdr	(a0),(ReoNif),(a1),26(sp)
		tst.l	d0
		beq.b	err1_output

		move.l	d0,ReoNBuf			/* nbuf */

* Before sending it pass it to the packet filter.
		tst.l	if_bpf(ReoNif)			/* packet filter present? */
		beq.b	c2_output

		Bpf_input	(ReoNif),(ReoNBuf)

* Here you should either send the packet to the hardware or enqueue the packet and 
* send the next packet as soon as the hardware is finished.
*
* If you are done sending the packet free it with buf_deref().
c2_output:
		move.l	bf_dend(ReoNBuf),d0
		move.l	bf_dstart(ReoNBuf),a0
		sub.l	a0,d0
		cmp	#ETH_HLEN+ETH_MIN_DLEN,d0
		bge.b	c3_output

		moveq	#ETH_HLEN+ETH_MIN_DLEN,d0	/* must at least be this long */

c3_output:
		moveq	#0,d1				/* arg4: length second portion=0 */
		suba.l	a1,a1				/* arg3: NULL */
**		move	d0,d0				/* arg2: length  first portion */
**		move.l	a0,a0				/* arg1: address first portion */
		jsr	ei_start_xmit			/* send the packet */
		tst.l	d0				/* succesful? */
		bne.b	c4_output

* success, do not need it any more
		addq.l	#1,if_out_packets(ReoNif)

		Buf_deref	(ReoNBuf),#BUF_NORMAL
		bra.b	exit_output

* no success, queue it
c4_output:
		If_enqueue	if_snd(ReoNif),(ReoNBuf),bf_info(ReoNBuf)

exit_output:
		moveq	#0,d0				/* OK */

quit_output:
		movem.l	(sp)+,#Reo
		rts


err1_output:
		addq.l	#1,if_out_errors(ReoNif)
		moveq	#-3,d0
		bra.b	quit_output




*********************************************************************************
*
* MintNet notifies you of some noteable IOCLT's. Usually you don't
* need to act on them because MintNet already has done so and only
* tells you that an ioctl happened.
*
* One useful thing might be SIOCGLNKFLAGS and SIOCSLNKFLAGS for setting
* and getting flags specific to your driver. For an example how to use
* them look at slip.c
*

SIOCSIFNETMASK	EQU	('S'<<8)+22		/* set network PA mask */ */
SIOCSIFFLAGS	EQU	('S'<<8)+14		/* set flags */ */
SIOCSIFADDR	EQU	('S'<<8)+16		/* set PA address */ */
	
SIOCSIFMTU	EQU	('S'<<8)+28		/* set MTU size */

SIOCSIFOPT	EQU	('S'<<8)+52		/* set interface option */

ifru_data	EQU	16

RioNif		EQU	a4


ENE_ioctl:
	move.l	RioNif,-(sp)		/* save used Regs */
		move.l	8(sp),RioNif		/* arg1: nif */
		move	12(sp),d0		/* arg2: cmd */

		cmp	#SIOCSIFNETMASK,d0
		beq.b	exit_ioctl
		cmp	#SIOCSIFFLAGS,d0
		beq.b	exit_ioctl
		cmp	#SIOCSIFADDR,d0
		beq.b	exit_ioctl


		cmp	#SIOCSIFMTU,d0
		bne.b	c1_ioctl

* Limit MTU to 1500 bytes. MintNet has already set nif->mtu to the new value,
* we only limit it here.
		moveq.l	#0,d1			/* unsigned extend */
		move	#ETH_MAX_DLEN,d1
		cmp.l	if_mtu(RioNif),d1
		bhi.b	exit_ioctl
		move.l	d1,if_mtu(RioNif)

exit_ioctl:
		moveq	#0,d0
		bra.b	quit_ioctl


c1_ioctl:
		cmp	#SIOCSIFOPT,d0
		bne.b	c2_ioctl

* Interface configuration, handled by ENE_config()
		move.l	14(sp),a0		/* this function arg3: arg */

		move.l	ifru_data(a0),a1	/* arg2: ifo */
		move.l	RioNif,a0		/* arg1: nif */
		jsr	ENE_config
		bra.b	quit_ioctl			/* return RC of ENE_config */

c2_ioctl:
		moveq	#-1,d0

quit_ioctl:
		move.l	(sp)+,RioNif
		rts


*
* Interface configuration via SIOCSIFOPT. The ioctl is passed a
* struct ifreq *ifr. ifr->ifru.data points to a struct ifopt, which
* we get as the second argument here.
*
* If the user MUST configure some parameters before the interface
* can run make sure that ENE_open() fails unless all the necessary
* parameters are set.
*
* Return values	meaning
* ENOSYS		option not supported
* ENOENT		invalid option value
* 0			Ok
*

ENE_config:
		moveq	#0,d0
		rts


*********************************************************************************

* do what you can do at compile time...
		.DATA

if_ENE:
		DC.B	'e','n',0,0,0,0,0,0,0,0,0,0,0,0,0,0	/* char name[IF_NAMSIZ] */
		DC.W	0					/* short unit */
		DC.W	2					/* ushort flags */
		DC.L	0					/* ulong metric */
		DC.L	ETH_MAX_DLEN				/* ulong mtu */
		DC.L	0					/* ulong timer */
		DC.W	HWTYPE_ETH				/* short hwtype */
		DC.W	ETH_ALEN				/* hwlocal.len */
		DC.B	0,0,0,0,0,0,0,0,0,0			/* hwlocal.addr */
		DC.W	ETH_ALEN				/* hwbrcst.len */
		DC.B	255,255,255,255,255,255,0,0,0,0		/* hwbrcst.addr */
		DC.L	0					/* struct ifaddr* addrlist */
		DC.W	IF_MAXQ					/* snd.maxqlen */
		DC.W	0					/* snq.qlen */
		DC.W	0					/* snd.curr */
		DC.L	0,0					/* *qfirst[2] */
		DC.L	0,0					/* *qlast[2] */
		DC.W	IF_MAXQ					/* rcv.maxqlen */
		DC.W	0					/* rcv.qlen */
		DC.W	0					/* rcv.curr */
		DC.L	0,0					/* *qfirst[2] */
		DC.L	0,0					/* *qlast[2] */
		DC.L	ENE_open				/* *open */
		DC.L	ENE_close				/* *close */
		DC.L	ENE_output				/* *output */
		DC.L	ENE_ioctl				/* *ioctl */
		DC.L	0					/* *timeout */
		DC.L	0					/* void* data */
		DC.L	0					/* ulong in_packets */
		DC.L	0					/* ulong in_errors */
		DC.L	0					/* ulong out_packets */
		DC.L	0					/* ulong out_errors */
		DC.L	0					/* ulong collisions */
		DC.L	0					/* struct netif* next */
		DC.W	NERMAXPCKT				/* short maxpackets */
		DC.L	0					/* struct bpf *bpf */
		DC.L	0,0,0,0					/* long reserved[4] */
NDif		EQU	*-if_ENE

* check whether definition above and declaration in IF.I agree
		IFNE	Nif-NDif
		FAIL	Inconsistent declaration and definition of struct if
		ENDC


		.BSS

	IFNE	USEKERNEL
KERNEL:		.ds.l	1
	ENDC

netinfo:
		.ds.l	1
		END

******** end of enemnet.s *******************************************************

