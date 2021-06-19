*********************************************************************************
* Bus access macros for ST/TT ACSI bus for register base hardware               *
* Version for my ACSI-NE2000 interface hardware for NE2000 (clone) cards        *
*                                                                               *
*       Copyright 2002 Dr. Thomas Redelberger                                   *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* processor registers are used as follows:                                      *
*       d0:     as ACSI is w. and you mostly need b. d0 stores intermediate     *
*       d6.w:   will contain $88;       do not change it!                       *
*       d7.w:   will contain $8a;       do not change it!                       *
*                                                                               *
*       a5:     will point to dmamodus; do not change it!                       *
*       a6:     will point to diskctl;  do not change it!                       *
*                                                                               *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*                                                                               *
*********************************************************************************

*
* manifest constants
*

TheACh		EQU	$60		/* ACSI channel of NE hardware (=3) */
NilACh		EQU	0		/* inactive ACSI channel used to deselect the */
					/* ACSI/ISA hardware */
BUGGY_HW	EQU	1		/* if defined enables code to handle buggy hardware */

WORD_TRANSFER	EQU	0		/* if set enables 16-bit DMA */

*
* hardware addresses
*

gpip		EQU	$fffffa01	/* (b) 68901 input register (unused) */

diskctl		EQU	$ffff8604	/* (w) disk controller data access */
dmamodus	EQU	$ffff8606	/* (w) DMA mode control */
dmahigh		EQU	$ffff8609	/* (b) DMA base address high (unused) */
dmamid		EQU	$ffff860b	/* (b) DMA base address medium (unused) */
dmalow		EQU	$ffff860d	/* (b) DMA base address low (unused) */


*
* addresses of system variables
*

flock		EQU	$43e		/* (w) semaphor to lock floppy usage of DMA */


*
* macros
*

		MACRO lockBUS doNothing
		moveq	#-1,d0			/* preset error code */
		tas	flock.w			/* check for race about ACSI bus and */
		bne	doNothing		/* if somebody owns the bus we quit */
		tas	flock+1.w		/* Holzauge: we do *both* bytes */
		bne	doNothing
		ENDM

		MACRO lockBUSWait.size
		.LOCAL lt1
		.LOCAL lwait
		.LOCAL lc1
* more elegant would be to wait for the bus (e.g. dma_begin in MagiC)
* there should be a timeout based on _hz_200 (and then branch to .doNothing)

lt1:		tas	flock.w			/* check for race about ACSI bus and */
		bne.b	lwait			/* if somebody owns the bus we *wait* */
		tas	flock+1.w		/* Holzauge: we do *both* bytes */
		beq.b	lc1

lwait:		bsr.size _appl_yield		/* wait */
		bra.b	lt1

lc1:						/* proceed */
		ENDM


		MACRO unlockBUS
		clr.w	flock.w			/* let other tasks access ACSI bus */
		ENDM

		MACRO unlockBUSWait
* we should relinquish the bus (e.g. dma_end in MagiC)
		clr.w	flock.w			/* let other tasks access ACSI bus */
		ENDM


RcBUS		EQU	a5
RdBUS		EQU	a6
Ra1l		EQU	d6
RxBUS		EQU	d6			/* synonym for Ra1l in NE.S */
Ra1h		EQU	d7
RyBUS		EQU	d7			/* synonym for Ra1h in NE.S */


		MACRO ldBUSRegs			/* for faster access to hardware */
		lea	dmamodus.w,RcBUS
		lea	diskctl.w,RdBUS
* what has to be written to dmamodus before and after to write a byte with A1=low
                move    #%010001000,Ra1l        /* =$088 A1=0 R_W=1 */
                move    #%010001010,Ra1h        /* =$08a A1=1 R_W=1 */
*                         ||  || |
*                         ||  || A1
*                         ||  |0=flop, 1=DMA
*                         ||  0=flop,DMA, 1=SecCntReg
*                         |0=DMA enable, 1=DMA disable
*                         0=R_W=1 (read), 1=R_W=0 (write)
* R_W is only effective in DMA mode. Otherwise state of R_W line reflects direction of move.w
		ENDM


		MACRO putBUS val,offset
		move	Ra1l,(RcBUS)		/* A1 gets low */
		move	#TheACh+offset,(RdBUS)	/* set register selection via ACSI channel */
		move	Ra1h,(RcBUS)		/* A1 gets high again */
		.IFNE ACSI_SLOW_ACCESS
		move	Ra1h,(RdBUS)		/* dummy write */
		.ENDC
		move	val,(RdBUS)		/* write to register */
		ENDM


		MACRO putMore val,offset
		.IFNE ACSI_SLOW_ACCESS
		move	Ra1h,(RdBUS)		/* dummy write */
		.ENDC
		move	val,(RdBUS)		/* write to register */
		ENDM


		MACRO putBUSi val,offset
		putBUS	#val,offset
		ENDM


		MACRO getBUS offset,val
		move	Ra1l,(RcBUS)		/* A1 gets low */
		move	#TheACh+offset,(RdBUS)	/* set register selection via ACSI channel */
		move	Ra1h,(RcBUS)		/* A1 gets high again */
		.IFNE ACSI_SLOW_ACCESS
		tst	(RdBUS)			/* dummy read */
		.ENDC
		move	(RdBUS),val		/* read from register */
		ENDM


		MACRO getMore offset,val	/* arg 1 is not used! */
		.IFNE ACSI_SLOW_ACCESS
		tst	(RdBUS)			/* dummy read */
		.ENDC
		move	(RdBUS),val		/* read from register */
		ENDM

*
* macro to deselect an interface by selecting ACSI device 0 
*
		MACRO deselBUS
		move	Ra1l,(RcBUS)		/* A1 gets low */
		move	#NilACh,(RdBUS)		/* select unused ACSI channel to deselect The device */
		move	Ra1h,(RcBUS)		/* A1 gets hi again */
		move	#$80,(RcBUS)		/* prepare ACSI bus for floppy */
		ENDM



******** RAM2NE *****************************************************************
* This is a macro for speed
* put # of bytes in register arg2 to NE from location pointed to by arg1.
*
* in:	arg1	address register source address
*	arg2	data register (w) # of bytes to put; must not be d0!
*
* both registers plus d0 get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeate explicitly
	
		MACRO RAM2NE addr,count
		.LOCAL Rt1
		.LOCAL Rb1
		.LOCAL Rt2
		.LOCAL Rb2
		.LOCAL doNothing_ram2ne
		ext.l	count			/* clear upper word */
		subq.w	#2,count		/* first two puts are outside loop */
		ble.b	doNothing_ram2ne	/* nothing to do? */
		move.b	(addr)+,d0
		putBUS	d0,NE_DATAPORT		/* this sets the NE IO address */
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT

* put the packet; optimized for speed, we do 8 bytes at once
		ror.l	#3,count		/* store remainder in upper word */
		bra.b	Rb1

Rt1:		move.b	(addr)+,d0		/* a fool designed word access to ACSI port */
		putMore	d0,NE_DATAPORT		/* that prevents that we do the faster */
		move.b	(addr)+,d0		/* putMore (addr)+,NE_DATAPORT */
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT
Rb1:		dbra	count,Rt1

		clr.w	count			/* prepare for remainder */
		rol.l	#3,count		/* restore remainder bits */
		bra.b	Rb2

Rt2:		move.b	(addr)+,d0
		putMore	d0,NE_DATAPORT		/* put the remaining bytes */
Rb2:		dbra	count,Rt2

doNothing_ram2ne:
		ENDM



******** NE2RAM *****************************************************************
* This is a macro for speed
* get # of bytes in register arg2 to RAM location pointed to by arg1.
*
* in:	arg1	address register destination address
*	arg2	data register (w) # of bytes to get; must not be d0!
*
* both registers plus d0 get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeate explicitly

		MACRO NE2RAM addr,count
		.LOCAL Nt1
		.LOCAL Nb1
		.LOCAL Nt2
		.LOCAL Nb2
		.LOCAL doNothing_ne2ram
		ext.l	count			/* clear upper word */
		subq.w	#2,count		/* first two gets are outside loop */
		ble.b	doNothing_ne2ram	/* nothing to do? */
		getBUS	NE_DATAPORT,d0		/* this sets the NE IO address */
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+

* get the Packet; optimized for speed, we do 8 bytes at once.
		ror.l	#3,count		/* store remainder in upper word */
		bra.b	Nb1

Nt1:		getMore	NE_DATAPORT,d0		/* a fool designed word access to ACSI port */
		move.b	d0,(addr)+		/* that prevents that we do a faster */
		getMore	NE_DATAPORT,d0		/* getMore NE_DATAPORT,(addr)+ */
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
		getMore	NE_DATAPORT,d0
		move.b	d0,(addr)+
Nb1:		dbra	count,Nt1

		clr.w	count			/* prepare for remainder */
		rol.l	#3,count		/* restore remainder bits */
		bra.b	Nb2

Nt2:		getMore	NE_DATAPORT,d0		/* get the remaining bytes */
		move.b	d0,(addr)+
Nb2:		dbra	count,Nt2

doNothing_ne2ram:
		ENDM



*********************************************************************************
