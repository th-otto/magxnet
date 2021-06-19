*********************************************************************************
*										*
*	Generic NEx000 driver for any Bus interface and STinG and MagicNet	*
*	(tested) and MINTNet (untestet)						*
*	Copyright 2001-2002 Dr. Thomas Redelberger				*
*	Use it under the terms of the GNU General Public License		*
*	(See file COPYING.TXT)							*
*										*
*										*
* Features:									*
*	Supports both (old) NE1000 and old and new NE2000 (clone) cards		*
*										*
* Limitations:									*
*	Can only handle one card per machine					*
*										*
* Credits:									*
* Although written in 68000 assembler this source code is based on the source	*
* modules NE.C and 8390.C from Linux originally due to the authors 		*
* Donald Becker and Paul Gortmaker.						*
*										*
* Tabsize 8, developed with DEVPAC assembler 2.0.				*
*										*
*********************************************************************************
*
* descriptions:
*
* All functions called from upper layer software request and relinquish Bus use
*
*
* Call Tree:
* 	ei_probe1
*		ethdev_init
*			ether_setup
*		NS8390_init
*			set_multicast_list
*
* 	ei_open
*		NS8390_init
*			set_multicast_list
*
* 	ei_close
*		NS8390_init
*
* 	ei_start_xmit
*		RAM2NE (macro)
*		ne_reset_8390
*		NS8390_init
*
* 	ei_interrupt
*		ei_rx_overrun
*			ei_receive
*				...
*		ei_receive
*			rtrvPckt (external)
* (StinG version)		rtrvStngDgram
* (StinG version)			NE2RAM (macro)
* (StinG version)		process_arp (external)
* (MagiCNet version)	rtrvPckt (external)
* (MagiCNet version)		NE2RAM (macro)
*		ei_tx_intr
*		ei_tx_err
*
*
*
* Handling of 8390 interrupt mask register; where are interupts enabled/disbled?
* This is not relevant here because this driver works with polling
*
* - ei_start_xmit
*	disable
*	enable
*
* - NS8390_init
*	disable			(called from ei_close)
*				(called from ei_probe1)
*	enable if "startp"	(called from ei_open)
*
* - ei_probe1
*	disable			(first inline, then by calling NS8390_init)
*
*
*********************************************************************************
*

*
* development switches
*

		.INCLUDE	"devswit.i"

*
* configuration switches
*


* entry points and references in this module
		XDEF	ei_probe1	; (); look for NEx000 hardware (super mode)
		XDEF	ei_open		; (); switch hardware on (super mode)
		XDEF	ei_close	; (); switch hardware off (super mode)
		XDEF	ei_start_xmit	; (); tx an ethernet packet (super mode)
		XDEF	ei_interrupt	; (); rx ethernet packets and housekeeping (super mode)
		XDEF	get_stats	; (); access to struct enet_statistics  (super mode)

		XDEF	DVS		; access to the device structure

* external references
		XREF	rtrvPckt	; (); call STinG or MagicNet or MINTNet specific code
					    ; to retrieve a packet from the card into RAM

*
* manifest constants
*
TRMAGIC		EQU	"TRNE"		; my XBRA ident (unused)

*
* addresses of system variables
*

_hz_200		EQU	$4ba		; (l) 200Hz system tick

HZ		EQU	200		; system timer ticks per second


		.INCLUDE	"uti.i"
		.INCLUDE	"bus.i"
		.INCLUDE	"netdev.i"

		.TEXT
**** Auxiliary Code *************************************************************


_appl_yield:
		move.l	a2,-(sp)	; not needed for GNU C, needed for Pure C/Turbo C
		move.w	#201,d0
		trap #2
		movea.l	(sp)+,a2
		rts



*********************************************************************************
******** taken from <linux/if.h> ************************************************

IFF_BROADCAST	EQU	$0002
IFF_PROMISC	EQU	$0100
IFF_ALLMULTI	EQU	$0200
IFF_MULTICAST	EQU	$1000


*********************************************************************************
******** taken from drivers/net/net_init.c **************************************

*** just what we need here
ether_setup:
		IFNE	1
		move	#IFF_BROADCAST+IFF_MULTICAST,DVS+dev_flags
		ELSE
		move	#IFF_BROADCAST+IFF_MULTICAST+IFF_PROMISC,DVS+dev_flags
		ENDC
		rts


*********************************************************************************
******** Start of NS8390 specific code ******************************************

		.INCLUDE	"8390.i"

		.TEXT
******** ei_open ****************************************************************
* in
*
* out
*	d0.l	0=OK
*
* changed
*	Turbo-C convention: d0-d2,a0-a1 may get changed
*********************************************************************************

Ron		REG	d3-d5/RxBUS/RyBUS/a2-a4/RcBUS/RdBUS


ei_open:
		movem.l	#Ron,-(sp)
		lockBUSWait.s			; aquire Bus
		ldBUSRegs			; load registers to access Bus

		moveq	#1,d0			; arg: startp
		bsr	NS8390_init
		st	DVS+dev_start

		deselBUS			; deselect Bus interface
		unlockBUSWait			; relinquish Bus
		moveq	#0,d0
		movem.l	(sp)+,#Ron
		rts



******** ei_close ***************************************************************
* in
*
* out
*	d0.l	0=OK
*
* changed
*	Turbo-C convention: d0-d2,a0-a1 may get changed
*********************************************************************************

Rcl		REG	d3-d5/RxBUS/RyBUS/a2-a4/RcBUS/RdBUS


ei_close:
		movem.l	#Rcl,-(sp)
		lockBUSWait.s			; aquire Bus
		ldBUSRegs			; load registers to access Bus

		moveq	#0,d0			; arg: startp
		bsr	NS8390_init
		sf	DVS+dev_start

		deselBUS			; deselect Bus interface
		unlockBUSWait			; relinquish Bus
		moveq	#0,d0
		movem.l	(sp)+,#Rcl
		rts



******** ei_start_xmit **********************************************************
* Transfer a raw ethernet packet from RAM into the NEx000 and start the transmitter
* To minimize packet double buffering this function allows to specify two non
* contiguous parts of the packet which get catenated here on the fly. This eases
* to assemble an enet packet from STinGs dgram representation.
*
* long ei_start_xmit (char* buff1, short len1, char* buff2, short len2);
*
* in
*	a0.l:	points to packet to send first  portion
*	d0.w:	length of packet to send first  portion
*	a1.l:	points to packet to send second portion
*	d1.w:	length of packet to send second portion
*
* out
*	d0.l	0=OK
*		1=transmitter busy or other errors
* changed
*	Turbo-C convention: d0-d2,a0-a1 may get changed
*
* N.b.: block_output and NS8390_trigger_send employed in the Linux code
* are inlined here to save cycles and to avoid the need for double buffering
*
* Only one tx buffer in use.
*
* Note that usually after calling ei_start_xmit (about 1.5 millisec) a Packet
* Transmitted interrupt would be fired ($02). Here this interrupt is dealt with
* by polling in ei_interrupt.
*
* Transmitting with NEx000 is a two step process:
* 1.	loading the RAM onboard the NE from the systems RAM by a remote
*	DMA write command
* 2.	kicking the NE so that it starts txing from its onboard RAM
*	out to the network
*
* There is a semaphore: dev_tbusy
* -	it is set at the beginning of ei_start_xmit (this function)
* -	it is queried before setting it
* -	it is reset in ei_tx_intr
* This avoids trying to transmit while transmit is still in progress
*
*********************************************************************************

* The maximum time waited (in jiffies) before assuming a Tx failed. (1000ms)
TX_TIMEOUT	EQU	((1000*HZ)/1000)

Rsx		REG	d3/RxBUS/RyBUS/a2/RcBUS/RdBUS


ei_start_xmit:
		move.w	d0,d2			; save arg
		lockBUS doNothing_xmit				; aquire Bus, jumps to .doNothing
						; on fail to lock with d0.l=-1
		movem.l	#Rsx,-(sp)
		move.l	a0,a2			; save arg
	IFGE	TXDEBPRT-3
		PrL	_hz_200.w
		PrA	" Tx l1: "
		PrW	d2
		PrA	" l2: "
		PrW	d1
	ENDC
		ldBUSRegs			; load registers to access Bus

		tas	DVS+dev_tbusy		; is NE transmitting?
		beq.b	c1_xmit			; no, go on

		move.l	_hz_200.w,d0		; yes, check how long ago
		sub.l	DVS+dev_trans_start,d0	; time since start of last xmit
		cmp.l	#TX_TIMEOUT,d0
		bls	xmit_busy			; not yet over, leave it alone

		addq.w	#1,DVS+lcl_es_tx_errors	; possibly died
	IFGE	TXDEBPRT-1
		PrA	"TX timed out",13,10
	ENDC
		movem.l	d1/d2/a1/a2,-(sp)	; save args
		bsr	ne_reset_8390		; hard reset 8390 chip
		moveq	#1,d0			; arg: startp=1
		bsr	NS8390_init		; init 8390 chip
		movem.l	(sp)+,d1/d2/a1/a2	; restore args used below
		st	DVS+dev_tbusy		; set semaphor again (NS8390_init clears it)
						; and fall thru to transmit

c1_xmit:
		move	d2,d3			; len1
		add	d1,d3			; + len2 = total length
		move	d3,d0			; need lenght below again
		putBUS	d0,EN0_RCNTLO		; DMA count
		lsr.w	#8,d0
		putBUS	d0,EN0_RCNTHI		; "
		putBUSi	0,EN0_RSARLO		; DMA destination
		move.b	DVS+lcl_tx_start_page,d0
		putBUS	d0,EN0_RSARHI		; 8390 page to tx from
* start remote DMA write
		putBUSi	E8390_RWRITE+E8390_START,E8390_CMD

		RAM2NE	a2,d2			; put ethernet packet first  portion
		RAM2NE	a1,d1			; put ethernet packet second portion

		putBUSi	E8390_NODMA+E8390_START,E8390_CMD	; complete remote DMA
		putBUSi	(1<<ENISR_RDC),EN0_ISR	; ack intr.

		putBUS	d3,EN0_TCNTLO		; transmit count
		lsr.w	#8,d3
		putBUS	d3,EN0_TCNTHI		; "
		; 8390 page to tx from was already set in NS8390_init
* start transmitter
		putBUSi	E8390_NODMA+E8390_TRANS+E8390_START,E8390_CMD
	IFGE	TXDEBPRT-3
		PrA	" start",13,10
	ENDC
		move.l	_hz_200.w,DVS+dev_trans_start	; save tx start time
		moveq	#0,d0			; rc=OK

quit_xmit:
		deselBUS			; deselect Bus interface
		movem.l	(sp)+,#Rsx		; restore used registers
		unlockBUS			; relinquish Bus
doNothing_xmit:
		rts


xmit_busy:
		moveq	#1,d0			; rc
		bra.b	quit_xmit




******** ei_interrupt ***********************************************************
* The typical workload of the driver:
* Handle the ether interface interrupts. This is done by polling here.
* Thus this function is *not* a proper interrupt handler (it does not save all
* regs. and ends with rts rather than rte).
*
* in:
*	nothing
*
* out:
*	nothing
*
* changed:
*	d0-d2,a0-a1 do change and are not saved to match Turbo-C 2.0 calling
*	convention we save only those regs. that get changed in ei_interrupt
*	to keep it fast. ei_receive saves the regs it uses
*
* uses
*	d5, RxBUS,RyBUS, a4, RcBUS,RdBUS
*
*********************************************************************************

* local variables in registers
RitInts		EQU	d5		; copy of interrupt register; to be conserved by lower levels!
RitDVS		EQU	a4		; pointer to global vars.

Rit		REG	RitInts/RxBUS/RyBUS/RitDVS/RcBUS/RdBUS	; saved registers on entry


ei_interrupt:
		lockBUS	doNothing_interrupt			; aquire Bus, jumps to .doNothing
						; on fail to lock with d0.l=-1
		movem.l	#Rit,-(sp)		; save all used registers
		lea	DVS,RitDVS		; allows faster access to global vars.
		ldBUSRegs			; load registers to access Bus

	IFNE	PARANOIA
t1_interrupt:
		move	sr,d1			; save int. level
		ori	#$700,sr		; disable all ints.
	ENDC
		getBUS	EN0_ISR,RitInts		; look what is up
	IFNE	PARANOIA
		getMore	EN0_ISR,d0		; again to minimize spike risk
		move	d1,sr			; reenable ints.
		cmp.b	d0,RitInts		; should always be the same
		bne.b	t1_interrupt
	ENDC
		tst.b	RitInts			; more interrupts to do?
		beq.w	exit_interrupt			; exit if no interrupts at all

* check for interrupt causes one by one
		btst	#ENISR_OVER,RitInts	; overrun on receive?
		beq.b	c1_interrupt

	IFGE	RXDEBPRT-4
		PrL	_hz_200.w
		PrA	" IRxOver ISR: "
		PrB	RitInts
		PrS	crlf(pc)
	ENDC
		bsr	ei_rx_overrun
		bra	c2_interrupt			 ; this did ei_receive already


c1_interrupt:
		btst	#ENISR_RX,RitInts	; got a good packet?
		beq	c11_interrupt

	IFGE	RXDEBPRT-4
		PrL	_hz_200.w
		PrA	" IRxRecv ISR: "
		PrB	RitInts
		PrA	" RSR: "
		getBUS	EN0_RSR,d0
		PrB	d0
		PrS	crlf(pc)
	ENDC
		bsr	ei_receive


c11_interrupt:
		btst	#ENISR_RX_ERR,RitInts	; RX with error?
		beq	c2_interrupt

	IFGE	RXDEBPRT-4
		PrL	_hz_200.w
		PrA	" IRxErr  ISR: "
		PrB	RitInts
		PrA	" RSR: "
		getBUS	EN0_RSR,d0
		PrB	d0
		PrS	crlf(pc)
	ENDC
		addq.w	#1,lcl_es_rx_errors(RitDVS)	; that is all we do
		putBUSi	(1<<ENISR_RX_ERR),EN0_ISR	; ack intr.


c2_interrupt:
		btst	#ENISR_TX,RitInts	; TX finished
		beq.b	c3_interrupt

	IFGE	TXDEBPRT-4
		PrL	_hz_200.w
		PrA	" ITxDone ISR: "
		PrB	RitInts
		PrS	crlf(pc)
	ENDC
		bsr	ei_tx_intr
***		bra.b	c4_interrupt			; TX and TX_ERR are mut. exclusive


c3_interrupt:
		btst	#ENISR_TX_ERR,RitInts
		beq.b	c4_interrupt

	IFGE	TXDEBPRT-4
		PrL	_hz_200.w
		PrA	" ITxErr  ISR: "
		PrB	RitInts
		PrS	crlf(pc)
	ENDC
		bsr	ei_tx_err


c4_interrupt:
		btst	#ENISR_COUNTERS,RitInts
		beq.b	c5_interrupt

		move	#$ff,d1
		getBUS	EN0_COUNTER0,d0
		and	d1,d0			; suppress junk bits
		add	d0,lcl_es_rx_frame_errors(RitDVS)
		getBUS	EN0_COUNTER1,d0
		and	d1,d0
		add	d0,lcl_es_rx_crc_errors(RitDVS)
		getBUS	EN0_COUNTER2,d0
		and	d1,d0
		add	d0,lcl_es_rx_missed_errors(RitDVS)

		putBUSi	(1<<ENISR_COUNTERS),EN0_ISR	; ack intr.


c5_interrupt:
		btst	#ENISR_RDC,RitInts
		beq.b	c6_interrupt

	IFGE	RXDEBPRT-4
		PrL	_hz_200.w
		PrA	" IRxRDC  ISR: "
		PrB	RitInts
		PrS	crlf(pc)
	ENDC
		putBUSi	(1<<ENISR_RDC),EN0_ISR	; ignore RDC interrupts that make it here

c6_interrupt:
exit_interrupt:
		deselBUS			; deselect Bus interface
		movem.l	(sp)+,#Rit		; restore used registers
		unlockBUS			; relinquish Bus
doNothing_interrupt:
	rts



******** ei_tx_err **************************************************************
* A transmitter error has happened. Most likely excess collisions (which
* is a fairly normal condition). If the error is one where the Tx will
* have been aborted, we do not try to send the packet sitting in the NE buffer
* again because tx would not be finished before the next tx is attempted by
* ei_start_xmit
*
* in
*	nothing
*
* out
*	nothing
*
* uses
*	RitDVS
*
* changed
*
*********************************************************************************

* local variables in registers
RteTxsr		EQU	d1		; temporary

ei_tx_err:
		getBUS	EN0_TSR,RteTxsr
		putBUSi	(1<<ENISR_TX_ERR),EN0_ISR		; Ack intr.
		sf	dev_tbusy(RitDVS)		; NE is available for next tx

		btst	#ENTSR_ABT,RteTxsr
		beq.b	c1_tx_err

		addq.w	#1,lcl_es_tx_aborted_errors(RitDVS)
		IFGE	TXDEBPRT-1
		PrS	m1_tx_err(pc)
		ENDC

c1_tx_err:
		btst	#ENTSR_ND,RteTxsr

		IFGE	TXDEBPRT-1
		beq.b	c2_tx_err
		PrS	m2_tx_err(pc)
		.ELSE
		nop
		ENDC

c2_tx_err:
		btst	#ENTSR_CRS,RteTxsr
		beq.b	c3_tx_err

		addq.w	#1,lcl_es_tx_carrier_errors(RitDVS)
		IFGE	TXDEBPRT-1
		PrS	m3_tx_err(pc)
		ENDC

c3_tx_err:
		btst	#ENTSR_FU,RteTxsr
		beq.b	c4_tx_err

		addq.w	#1,lcl_es_tx_fifo_errors(RitDVS)
		IFGE	TXDEBPRT-1
		PrS	m4_tx_err(pc)
		ENDC

c4_tx_err:
		btst	#ENTSR_CDH,RteTxsr
		beq.b	c5_tx_err

		addq.w	#1,lcl_es_tx_hartbeat_errors(RitDVS)
		IFGE	TXDEBPRT-1
		PrS	m5_tx_err(pc)
		ENDC

c5_tx_err:
		btst	#ENTSR_OWC,RteTxsr
		beq.b	c6_tx_err

		addq.w	#1,lcl_es_tx_window_errors(RitDVS)
		IFGE	TXDEBPRT-1
		PrS	m6_tx_err(pc)
		ENDC

c6_tx_err:
		IFGE	TXDEBPRT-1
		PrS	crlf(pc)
		ENDC
		rts


		IFGE	TXDEBPRT-1
m1_tx_err:		DC.B	"excess-collisions ",0
m2_tx_err:		DC.B	"non-deferral ",0
m3_tx_err:		DC.B	"lost-carrier ",0
m4_tx_err:		DC.B	"FIFO-underrun ",0
m5_tx_err:		DC.B	"lost-heartbeat ",0
m6_tx_err:		DC.B	"window ",0
		EVEN
		ENDC



******** ei_tx_intr *************************************************************
* We have finished a transmit: check for errors
* We can not trigger another send because we use only one tx buffer and tx would
* not be finished before next attempt by ei_start_xmit
*
* in
*	nothing
*
* out
*	nothing
*
* uses
*	RitDVS
*
* changed
*
*********************************************************************************

* local variables in registers
RtiStts		EQU	d1		; temporary


ei_tx_intr:
		getBUS	EN0_TSR,RtiStts		; first get status for this one
		putBUSi	(1<<ENISR_TX),EN0_ISR	; Ack intr.
		sf	dev_tbusy(RitDVS)	; NE is available for next tx

* Minimize Tx latency: update the statistics after we restart TXing

* do we have a collision?
		btst	#ENTSR_COL,RtiStts
		beq.b	c1_tx_intr
		addq.w	#1,lcl_es_collisions(RitDVS)	; just book keeping

* do we have a packet transmitted?
c1_tx_intr:
		btst	#ENTSR_PTX,RtiStts
		beq.b	c2_tx_intr

		addq.w	#1,lcl_es_tx_packets(RitDVS)	; the usual case
exit_tx_intr:
		rts


* this shall never happen
c2_tx_intr:
		IFGE	TXDEBPRT-1
		PrS	m1_tx_intr(pc)
		ENDC
		bra.b	exit_tx_intr

		IFGE	TXDEBPRT-1
m1_tx_intr:		DC.B	"TX Err in tx_intr?",13,10,0
		EVEN
		ENDC


******** ei_receive *************************************************************
* We have a good packet(s), get it/them out of the buffers.
*
* N.b.: get_8390_hdr and block_input employed in the Linux code are not used here.
* Rather an inlined remote DMA Read packet technique is used
*
* in
*	nothing
*
* out
*	nothing
*
* uses
*	RxBUS,RyBUS, RitDVS, RcBUS,RdBUS

*
* changed
*
*
********
* The 8390 header problem:
* Was observed with 1 NE1000 clone with Winbond W89C90 chip
* 1 NE2000 clone with Winbond W89C90 chip
* 1 NE2000 clone with UMC UM9090 chip
* Symptoms:
* The Status field (rxHdrSts) contains garbage
* The Next Frame field (RrxNextFrm) looks like it contains the Status value
* The Lo Byte Count field looks like it contains the Next Frame value
* The Hi Byte Count field looks like it contains the Lo Byte Count
* We recover accordingly
* This renders 8390 Remote DMA Send useless and necessitates Remote DMA Read
* This also defeats use of the error bits in the 8390 header
*
* This problem was not observed with a new single chip NE2000 clone with
* Realtek RTL8019AS chip
*********************************************************************************

N8390Hdr	EQU	4		; the chip saves a 4 bytes header preceeding the packet

* local variables in registers
RrxReadPg	EQU	d4			; page where the newly arrived packet shall be read from
RrxNextFrm	EQU	d3			; the next packet to be read
RrxPktLen	EQU	d2			; lenght of the newly arrived packet
RrxJnk8990	EQU	d1			; Header is junk flag (8990, 9090 symptom)

RrxHiLvl1	EQU	a2			; we save these here to avoid rtrvPkct needs to save
RrxHiLvl2	EQU	a3			; them at each call (assumes >1 packet to slurp)

* local variables in memory
		.OFFSET 0
rxHdrSts:	.ds.b	1			; 8390 status byte
rxDummy1:	.ds.b	1			; align
rxPktCnt:	.ds.w	1			; max. # of packets to slurp
rxNLcl:


Rrx		REG	RrxNextFrm/RrxReadPg/RrxHiLvl1/RrxHiLvl2	; saved registers on entry


		.TEXT

ei_receive:
		movem.l	#Rrx,-(sp)			; save registers
		Alloc	rxNLcl				; locals vars
		move	#20,rxPktCnt(sp)			; # packets maximum
	IFGE	RXDEBPRT-2
		addq.w	#1,eiCalls
	ENDC

		move.b	lcl_current_page(RitDVS),RrxReadPg	; start reading where writing should have commenced


t1_receive:
		putBUSi	E8390_NODMA+E8390_PAGE1+E8390_START,E8390_CMD	; switch to page 1
	IFNE	PARANOIA
		move	sr,d1					; save status register
		ori	#$700,sr				; disable all ints.
	ENDC
t0_receive:		getBUS	EN1_CURPAG,d0
		move.b	d0,lcl_current_page(RitDVS)		; get the rx page (incoming packet pointer)
	IFNE	PARANOIA
		getBUS	EN1_CURPAG,d0				; again
		cmp.b	lcl_current_page(RitDVS),d0		; should be equal
		bne.b	t0_receive
		move	d1,sr					; reenable ints.
	ENDC
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_START,E8390_CMD	; revert to page 0

* when we have read up to CurrPg we are done
		cmp.b	lcl_current_page(RitDVS),RrxReadPg	; Read all the frames?
		beq	exit_receive					; Done for now

* get the header the NIC stored at the beginning of the frame
		moveq	#0,d0
	IFNE	BUGGY_HW
		moveq	#0,RrxJnk8990			; assume no Hdr junk
	ENDC
		putBUSi	N8390Hdr,EN0_RCNTLO
		putBUS	d0,EN0_RCNTHI			; Hdr is only some bytes
		putBUS	d0,EN0_RSARLO			; Hdr is on page boundary
		putBUS	RrxReadPg,EN0_RSARHI
		putBUSi	E8390_RREAD+E8390_START,E8390_CMD ; start remote DMA Read

	IFNE	WORD_TRANSFER
		getBUSW	NE_DATAPORT,d0
		move.b	d0,RrxNextFrm			; status byte
		lsr.w	#8,d0				; next page
		move.b	d0,rxHdrSts(sp)
		getMoreW NE_DATAPORT,d0
		ror.w	#8,d0
		move.w	d0,RrxPktLen			; Count
	ELSE
		getBUS	NE_DATAPORT,d0
		move.b	d0,rxHdrSts(sp)			; first: status byte
		getMore	NE_DATAPORT,RrxNextFrm		; next:  next page
		getMore	NE_DATAPORT,d0			; next:  Count Lo
		getMore	NE_DATAPORT,RrxPktLen		; next:  Count Hi
		lsl.w	#8,RrxPktLen			; Count Hi in upper byte
		move.b	d0,RrxPktLen			; merge in Count Lo byte
	ENDC
* RrxPktLen is the length of the packet data to *follow* the header (incl. CRC)
		putBUSi	E8390_NODMA+E8390_START,E8390_CMD	; complete remote DMA
		putBUSi	(1<<ENISR_RDC),EN0_ISR		; reset remote DMA ready bit

* check status (only $01 and $21 is good)
		move.b	rxHdrSts(sp),d0
		andi.b	#$ff-ENRSR_PHY-ENRSR_DEF,d0	; do not care phys/multi., defer.
		cmp.b	#ENRSR_RXOK,d0			; only his should be set
		bne		err_receive

* check if next frame is within start and stop
		cmp.b	lcl_rx_start_page(RitDVS),RrxNextFrm
		bcs.s		err_receive
		cmp.b	lcl_stop_page(RitDVS),RrxNextFrm
		bhi.s		err_receive

* we should also check here if CountHi is consistent with NextFrm and ReadPg (sic!)

* check for good ethernet packet length
		cmp	#64,RrxPktLen			; check for bogus length
		bcs.s		err_receive
		cmp	#1518,RrxPktLen			; 6(eth)+6(eth)+2(type)+1500+4(crc)
		bhi.s		err_receive


getPkt:
		; here we assume things are OK
	IFGE	RXDEBPRT-3
		PrA	"\tRx "
		bsr	eirDumpState
	ENDC

		addq.w	#1,RrxPktLen			; round up to even
		andi.w	#$fffe,RrxPktLen

* implied arg: RrxPktLen (d2)
* implied arg: RrxReadPg (d4)
* implied arg: RrxJnk8990 (d1)
		bsr	rtrvPckt			; get packet out of the card
		tst.l	d0				; update driver statistics
		bne.b	c4_getpkt
		addq.w	#1,lcl_es_rx_packets(RitDVS)
		bra.b	c5_getpkt
c4_getpkt:
		addq.w	#1,lcl_es_rx_dropped(RitDVS)
c5_getpkt:
* set boundary one page behind up to including the page to be read next
skip_getpkt:
		move.b	RrxNextFrm,RrxReadPg			; start of next packet to be read
		cmp.b	lcl_rx_start_page(RitDVS),RrxNextFrm	; if Next is at the start
		bne.b	c6_getpkt
		move.b	lcl_stop_page(RitDVS),RrxNextFrm	; ...boundary must be stop-1
c6_getpkt:
		subq.b	#1,RrxNextFrm
		putBUS	RrxNextFrm,EN0_BOUNDARY			; update boundary

b2_getpkt:
		subq.w	#1,rxPktCnt(sp)			; make sure not tied up to much
		bne	t1_receive				; next packet


	IFGE	RXDEBPRT-2
		PrA	"\tToo much to receive",10,13
	ENDC


exit_receive:
		putBUSi	(1<<ENISR_RX)+(1<<ENISR_RX_ERR),EN0_ISR	; ack interrupt

		deAlloc	rxNLcl				; pop local vars
		movem.l	(sp)+,#Rrx			; restore registers
		rts


		MACRO debPrint msg,cr,lf
		IFGE	RXDEBPRT-2
		PrA	msg,cr,lf
		bsr	eirDumpState
		ENDC
		ENDM


* when we get here we likely have the 8390 header writing problem
	IFEQ	BUGGY_HW
m1_receive:		DC.B	"Funny Hdr in ei_receive",13,10,0
		EVEN
err_receive:
		PrS	m1_receive(pc)
		bra	skip_getpkt

	ELSE
err_receive:
		debPrint	9,"RxX"
* check status (only $01 and $21 is good) again
		move.b	RrxNextFrm,d0			; the misguided status
		andi.b	#$ff-ENRSR_PHY-ENRSR_DEF,d0	; do not care phys/multi., defer.
		cmp.b	#ENRSR_RXOK,d0			; only his should be set
		bne.b	e2_getpkt				; hopeless

		; here there is still a chance
		move.b	RrxPktLen,RrxNextFrm		; fix Next
		sub.b	RrxReadPg,RrxPktLen		; calculate Count Hi Byte
		subq.b	#1,RrxPktLen			; rounding ???
		bge.b	c1_receice

		add.b	lcl_stop_page(RitDVS),RrxPktLen		; adjust for wrap
		sub.b	lcl_rx_start_page(RitDVS),RrxPktLen

c1_receice:
		ror.w	#8,RrxPktLen			; swap bytes
		tst.b	RrxPktLen			; if Lo Count==0 ...
		bne.b	c2_receive
		addi.w	#$0100,RrxPktLen		; ...one more block
c2_receive:
		debPrint	9,"RxY"

* check if next frame is within start and stop again
		cmp.b	lcl_rx_start_page(RitDVS),RrxNextFrm
		bcs.b	e2_getpkt
		cmp.b	lcl_stop_page(RitDVS),RrxNextFrm
		bhi.b	e2_getpkt

* check for good ethernet packet length again
		cmp	#64,RrxPktLen			; check for bogus length
		bcs.b	e2_getpkt
		cmp	#1518,RrxPktLen			; 6+6+2(type)+1500+4(crc)
		bls.b	c3_receive


* this is the brute force method to skip the junk and try to synchronise again
e2_getpkt:
		debPrint	9,"RxZ"
		move.b	lcl_current_page(RitDVS),RrxNextFrm
		bra	skip_getpkt


c3_receive:
		; when we get here we have the 8390 header problem sucessfully fixed
		; and go for normal extraction of the packet
		st	RrxJnk8990			; take note of Hdr problem
		bra	getPkt				; and resume

	ENDC	; BUGGY_HW



		IFGE	RXDEBPRT-2
eirDumpState:
		PrA	" Cls: "
		PrW	eiCalls(pc)
		PrA	" Cnt: "
		PrB	rxPktCnt+1+4(sp)
		PrA	" Rea: "
		PrB	RrxReadPg
		PrA	" Nxt: "
		PrB	RrxNextFrm
		PrA	" Cur: "
		PrB	lcl_current_page(RitDVS)
		PrA	" Sts: "
		PrB	rxHdrSts+4(sp)
		PrA	" Siz: "
		PrW	RrxPktLen
		PrS	crlf(pc)
		rts

eiCalls		DC.W	0
		ENDC


******** ei_rx_overrun **********************************************************
* We have a receiver overrun: we have to kick the 8390 to get it started
* again. Problem is that you have to kick it exactly as NS prescribes in
* the updated datasheets, or "the NIC may act in an unpredictable manner."
* This includes causing "the NIC to defer indefinitely when it is stopped
* on a busy network."  Ugh.
*
* in
*	nothing
*
* out
*	nothing
*
* changed
*
*********************************************************************************

* local variables in memory; we must use them here instead of registers because
* ei_receive (called below) would destroy those registers
		.OFFSET 0
roWasTxing:	.ds.w	1
roMustResend:	.ds.w	1
roNLcl:

		.TEXT

ei_rx_overrun:
		Alloc	roNLcl				; locals vars
		clr	roMustResend(sp)

		getBUS	E8390_CMD,d0
		andi.w	#E8390_TRANS,d0
		move	d0,roWasTxing(sp)		; find out if transmit is in progress

		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_STOP,E8390_CMD	; stop

	IFGE	RXDEBPRT-1
		PrA	"Receiver overrun",13,10
	ENDC
		addq.w	#1,lcl_es_rx_over_errors(RitDVS)

* Wait a full Tx time (1.2ms) + some guard time, NS says 1.6ms total.
* Early datasheets said to poll the reset bit, but now they say that
* it "is not a reliable indicator and subsequently should be ignored."

		move.l	ticks2ms(RitDVS),d0	; wait 2ms
		moveq	#2,d1			; irrelevant here
		bsr	ADelay

* Reset RBCR[01] back to zero as per magic incantation.
		moveq	#0,d0
		putBUS	d0,EN0_RCNTLO
		putBUS	d0,EN0_RCNTHI

* See if any Tx was interrupted or not. According to NS, this
* step is vital, and skipping it will cause no end of havoc.
		tst	roWasTxing(sp)
		beq.b	c1_rx_overrun

		getBUS	EN0_ISR,d0
		andi.w	#((1<<ENISR_TX)+(1<<ENISR_TX_ERR)),d0	; completed if non zero
		seq	roMustResend(sp)		; if zero must resend

* Have to enter loopback mode and then restart the NIC before
* you are allowed to slurp packets up off the ring.
c1_rx_overrun:
		putBUSi	E8390_TXOFF,EN0_TXCR
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_START,E8390_CMD	; restart

* Clear the Rx ring of all the debris, and ack the interrupt.
		bsr	ei_receive
		putBUSi	(1<<ENISR_OVER),EN0_ISR

* Leave loopback mode, and resend any packet that got stopped.
		putBUSi	E8390_TXCONFIG,EN0_TXCR

		tst	roMustResend(sp)
		beq.b	c2_rx_overrun

		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_START+E8390_TRANS,E8390_CMD

c2_rx_overrun:
		deAlloc	roNLcl			; pop local vars
		rts




*********************************************************************************
* update statics counters and get access to them
*********************************************************************************


RgsDVS		EQU	a4

Rgs		REG	RxBUS/RyBUS/RgsDVS/RcBUS/RdBUS


get_stats:
		movem.l	#Rgs,-(sp)
		lea	DVS,RgsDVS
		lockBUS doNothing_stats					; aquire Bus
						; jumps to .doNothing on fail to lock
		ldBUSRegs			; load registers to access Bus

		tst.b	dev_start(RgsDVS)	; device active?
		beq.b	c1_get_stats			; if not, just return pointer

* if accessible and device is running, update statistics
		move	#$ff,d1
		getBUS	EN0_COUNTER0,d0
		and	d1,d0			; suppress junk bits
		add	d0,lcl_es_rx_frame_errors(RgsDVS)
		getBUS	EN0_COUNTER1,d0
		and	d1,d0
		add	d0,lcl_es_rx_crc_errors(RgsDVS)
		getBUS	EN0_COUNTER2,d0
		and	d1,d0
		add	d0,lcl_es_rx_missed_errors(RgsDVS)

c1_get_stats:
		deselBUS			; deselect Bus interface
		unlockBUS			; relinquish Bus
doNothing_stats:
		move.l	#DVS+lcl_stats,d0	; OK
		movem.l	(sp)+,#Rgs		; restore used registers
		rts


******** set_multicast_list *****************************************************
*	Set or clear the multicast filter for this adaptor.
*********************************************************************************

set_multicast_list:
		move	DVS+dev_flags,d0
		andi.w	#IFF_PROMISC,d0
		beq.b	c1_multicast
		putBUSi	E8390_RXCONFIG+$18,EN0_RXCR
		bra.b	c3_multicast

c1_multicast:
		move	DVS+dev_flags,d0
		andi.w	#IFF_ALLMULTI,d0
		or	DVS+dev_mc_list,d0
		beq.b	c2_multicast
* The multicast-accept list is initialized to accept-all, and we rely on
* higher-level filtering for now.
		putBUSi	E8390_RXCONFIG+$08,EN0_RXCR
		bra.b	c3_multicast

c2_multicast:
		putBUSi	E8390_RXCONFIG,EN0_RXCR

c3_multicast:
		rts



******** ethdev_init ************************************************************
* Initialize the rest of the 8390 device structure.
*********************************************************************************

ethdev_init:
		bsr	ether_setup
		rts



******** NS8390_init ************************************************************
* This page of functions should be 8390 generic
* Follow National Semi's recommendations for initializing the "NIC"
*
* in:
*	d0.w	if <> 0 initialize more
* out:
*	nothing
*
* changed:
*	d0, a0
*********************************************************************************

NS8390_init:
		move	d0,-(sp)		; save argument "startp"

* Follow National Semi's recommendations for initing the DP83902
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_STOP,E8390_CMD
	IFNE	WORD_TRANSFER
		putBUSi	$49,EN0_DCFG
	ELSE
		putBUSi	$48,EN0_DCFG
	ENDC
* Clear the remote byte count registers
		moveq	#0,d1
		putBUS	d1,EN0_RCNTLO
		putBUS	d1,EN0_RCNTHI
* Set to monitor and loopback mode -- this is vital!
		putBUSi	E8390_RXOFF,EN0_RXCR
		putBUSi	E8390_TXOFF,EN0_TXCR
* Set the transmit page and receive ring
		move.b	DVS+lcl_tx_start_page,d0
		putBUS	d0,EN0_TPSR
		move.b	DVS+lcl_rx_start_page,d0
		putBUS	d0,EN0_BOUNDARY
		putBUS	d0,EN0_STARTPG
		move.b	DVS+lcl_stop_page,d0
		putBUS	d0,EN0_STOPPG
* Clear the pending interrupts and mask
		putBUSi	$ff,EN0_ISR		; acknoledge all
		putBUS	d1,EN0_IMR		; and disable all interrupts

* Copy the station address into the DS8390 registers, and set the
* multicast hash bitmap to receive all multicasts
		putBUSi	E8390_NODMA+E8390_PAGE1+E8390_STOP,E8390_CMD
		lea	DVS+dev_dev_addr,a0
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+0
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+1
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+2
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+3
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+4
		move.b	(a0)+,d0
		putBUS	d0,EN1_PHYS+5

* Initialize the multicast list to accept-all. If we enable multicast the
* higher levels can do the filtering
		moveq	#-1,d0
		putBUS	d0,EN1_MULT+0
		putBUS	d0,EN1_MULT+1
		putBUS	d0,EN1_MULT+2
		putBUS	d0,EN1_MULT+3
		putBUS	d0,EN1_MULT+4
		putBUS	d0,EN1_MULT+5
		putBUS	d0,EN1_MULT+6
		putBUS	d0,EN1_MULT+7

		move.b	DVS+lcl_rx_start_page,d0
		addq.b	#1,d0				; boundary one behind current
		putBUS	d0,EN1_CURPAG
		move.b	d0,DVS+lcl_current_page		; mirror
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_STOP,E8390_CMD	; back to page 0

		sf	DVS+dev_tbusy		; initialize
		sf	DVS+dev_interrupt	; initialize
		sf	DVS+lcl_irqlock		; initialize

		move	(sp)+,d0		; restore argument "startp"
		beq.b	c1_init
		putBUSi	$ff,EN0_ISR		; clear all interrupts
* for this application (polling) we leave all intr masked.
*		putBUSi	ENISR_ALL,EN0_IMR	; and enable all
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_START,E8390_CMD
		putBUSi	E8390_TXCONFIG,EN0_TXCR	; xmit on
* 3c503 TechMan says rxconfig only after the NIC is started
		putBUSi	E8390_RXCONFIG,EN0_RXCR	; rx on
* Get the multicast status right if this was a reset
		bsr	set_multicast_list

c1_init:
		rts




******** End of the NS8390 chip specific code ***********************************
*********************************************************************************
******** Start of the NEx000 and clones board specific code *********************



**** Delay function specific to Ataris ******************************************
* in:
*	d0.l maximum delay time in processor specific units
*	d1.l maximum delay time in 200Hz timer ticks = 5ms
* out:
*	d0 time left in processor specific units

ADelay:
		lea	_hz_200.w,a0
		add.l	(a0),d1			; time to quit at

t1_delay:
		subq.l	#1,d0			; time has come?
		beq.b	exit_delay
		cmp.l	(a0),d1			; time has come?
		bhi.b	t1_delay

exit_delay:
		rts



******** ei_probe1 **************************************************************
* we do not have a ne_probe from which this gets called
*
* in:
*	nothing
* out:
*	d0.l	  0=OK
*		<>0=Error, no device found
*
* changed:
*	a lot
*********************************************************************************

* local variables in registers
RprDVS		EQU	a4		; pointer to global vars.

NBSA_prom	EQU	32

* local variables in memory
		.OFFSET 0
pbSA_prom:	.ds.b	NBSA_prom
pbWordLen:	.ds.w	1
pbNEx000:	.ds.b	1
pbFiller:	.ds.b	1
pbNLcl:

		.TEXT

Rpr		REG	RxBUS/RyBUS/RprDVS/RcBUS/RdBUS


ei_probe1:
		movem.l	#Rpr,-(sp)		; save used regs
		lea	DVS,RprDVS		; allows faster access to global vars.
		Alloc	pbNLcl			; allocate locals vars
		move	#2,pbWordLen(sp)
		lockBUSWait			; aquire Bus
		ldBUSRegs			; load registers to access Bus

* first we calibrate 2ms for this specific machine
		moveq.l	#0,d0			; indefinitely
		move.l	#26,d1			; 26*5ms=130ms according to _hz_200
		bsr.b	ADelay
		neg.l	d0			; so many machine ticks for 130ms
		asr.l	#6,d0			; 2ms = 130ms/64
		addq.l	#1,d0			; always round up
		move.l	d0,ticks2ms(RprDVS)	; store for use in the driver

		getBUS	NE_RESET,d1
		putBUS	d1,NE_RESET

*		move.l	ticks2ms(RprDVS),d0	; wait 2ms
		moveq	#2,d1			; wait 5-10ms maximum
		bsr.b	ADelay

		getBUS	EN0_ISR,d1		; read isr
		andi.b	#(1<<ENISR_RESET),d1		; test for reset bit
		bne.b	c1_probe1

		PrS	m1_probe1(pc)
		moveq	#-1,d0			; Error
		bra	quit_probe1

c1_probe1:
		putBUSi	$ff,EN0_ISR		; ack all interrupts

		moveq	#0,d0
		putBUSi	E8390_NODMA+E8390_PAGE0+E8390_STOP,E8390_CMD	; select page 0
		putBUSi	$48,EN0_DCFG		; set byte-wide access
		putBUS	d0,EN0_RCNTLO		; clear the count regs.
		putBUS	d0,EN0_RCNTHI
		putBUS	d0,EN0_IMR		; mask completion iqr
		putBUSi	$ff,EN0_ISR
		putBUSi	E8390_RXOFF,EN0_RXCR	; $20 set to monitor
		putBUSi	E8390_TXOFF,EN0_TXCR	; $02 and loopback mode
again_probe1:
		putBUSi	NBSA_prom&$ff,EN0_RCNTLO
		putBUSi	NBSA_prom>>8,EN0_RCNTHI
		putBUS	d0,EN0_RSARLO		; DMA starting at $0000
		putBUS	d0,EN0_RSARHI
		putBUSi	E8390_RREAD+E8390_START,E8390_CMD	; go

		move	#NBSA_prom/2-1,d2
		lea	pbSA_prom(sp),a0

t2_probe1:
		getBUS	NE_DATAPORT,d0
		move.b	d0,(a0)+
		getMore	NE_DATAPORT,d1
		move.b	d1,(a0)+
		cmp.b	d0,d1			; check for doubled up values
		beq.b	c2_probe1
		move	#1,pbWordLen(sp)
c2_probe1:
		dbra	d2,t2_probe1

		putBUSi	E8390_NODMA+E8390_START,E8390_CMD	; complete remote DMA
		putBUSi	(1<<ENISR_RDC),EN0_ISR	; ack intr.

	IFNE 0
		PollKey
		tst	d0
		beq	again_probe1

		WaitKey
	ENDC

	IFNE	MACAddDEBPRT
		lea	pbSA_prom(sp),a0
		moveq	#NBSA_prom/4-1,d2
***		PrA	33,"H"

t3_probe1:
		PrL	(a0)+
		PrS	crlf(pc)
		dbra	d2,t3_probe1

***		PrA	33,"H"
*		WaitKey
	ENDC	; MACAddDEBPRT

		cmp	#2,pbWordLen(sp)
		bne.b	c21_probe1

* we have a NE2000 or clone, reorder image of PROM contents
		moveq	#15,d2
		lea	pbSA_prom(sp),a0
		move.l	a0,a1

t31_probe1:
		move.b	(a0)+,(a1)+	; SA_prom[i]=SA_prom[i+i];
		addq.l	#1,a0
		dbra	d2,t31_probe1

		move.b	#NESM_START_PG,lcl_tx_start_page(RprDVS)
		move.b	#NESM_STOP_PG,lcl_stop_page(RprDVS)
		bra.b	c22_probe1

* we have a NE1000 or clone
c21_probe1:
		move.b	#NE1SM_START_PG,lcl_tx_start_page(RprDVS)
		move.b	#NE1SM_STOP_PG,lcl_stop_page(RprDVS)


c22_probe1:
		move.b	lcl_tx_start_page(RprDVS),lcl_rx_start_page(RprDVS)
		addi.b	#TX_PAGES,lcl_rx_start_page(RprDVS)
 
* check this
		cmp.b	#$57,pbSA_prom+14(sp)
		seq	d0
		cmp.b	#$57,pbSA_prom+15(sp)
		seq	pbNEx000(sp)
		and.b	d0,pbNEx000(sp)
		beq.b	c3_probe1

		lea	m2_probe1(pc),a0
		bra.b	c4_probe1

c3_probe1:
		lea	m3_probe1(pc),a0

c4_probe1:
		move.l	a0,lcl_name(RprDVS)

		bsr	ethdev_init

* store devices ethernet MAC address in struct device
		lea	pbSA_prom(sp),a0
		lea	dev_dev_addr(RprDVS),a1
		moveq	#ETHER_ADDR_LEN-1,d0

t4_probe1:
		move.b	(a0)+,(a1)+
		dbra	d0,t4_probe1

		moveq	#0,d0		; arg:
		bsr	NS8390_init

		moveq	#0,d0		; rc=OK


quit_probe1:
		deselBUS			; deselect Bus interface
		unlockBUSWait			; relinquish Bus
doNothing_probe1:
		deAlloc	pbNLcl			; pop local vars
		movem.l	(sp)+,#Rpr		; restore regs
		rts


m1_probe1:		DC.B	"NE Reset Bit not set. Fatal",13,10,0
m2_probe1:		DC.B	"NE1000",0
m3_probe1:		DC.B	"No NE1000",0
		EVEN



******** ne_reset_8390 **********************************************************
* Hard reset the card.  This used to pause for the same period that a
* 8390 reset command required, but that shouldn't be necessary
*********************************************************************************

ne_reset_8390:
* DON'T change these to inb_p/outb_p or reset will fail on clones
		getBUS	NE_RESET,d1
		putBUS	d1,NE_RESET

		move.l	DVS+ticks2ms,d0		; wait 2ms
		moveq	#2,d1			; wait 5-10ms maximum (irrelevant here)
		bsr	ADelay

* This check _should_not_ be necessary, omit eventually
		getBUS	EN0_ISR,d1		; read isr
		andi.b	#(1<<ENISR_RESET),d1		; test for reset bit
		bne.b	c1_reset

		PrS	m1_reset(pc)

c1_reset:
		putBUSi	(1<<ENISR_RESET),EN0_ISR	; ack interrupt

		rts


m1_reset:		DC.B	"ne_reset_8390 failed",13,10,0
		EVEN



******** Initialised data *******************************************************

crlf:	DC.B	13,10,0
		EVEN


******** data initialised to zero ***********************************************

		.BSS
DVS:	.ds.b	Ndevice		; allocate device structure

******** end of ne.s ************************************************************
