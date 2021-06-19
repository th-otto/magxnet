*********************************************************************************
* Bus access macros for Falcon Expansion Port for register base hardware        *
* Version for Martin Hodge's interface hardware for the Falcon Expansion port   *
* This hardware supports only fast "newer" RTL8019AS controller chip.           *
*                                                                               *
*       Copyright 2002 Dr. Thomas Redelberger                                   *
*       Copyright 2015 Martin Hodge                                             *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* processor registers are used as follows:                                      *
*       a5:     will point to EXPANSION at $F10600;     do not change it!       *
*       a6:     will point to EXPANSION at $F10600;     do not change it!       *
*                                                                               *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*                                                                               *
*********************************************************************************

*
* manifest constants
*

BUGGY_HW	EQU	0		/* if set enables code to handle buggy hardware */

WORD_TRANSFER	EQU	1		/* if set enables 16-bit DMA */

*
* hardware addresses
*

rom4		EQU	$00f10600	/* EXP base address */
rom3		EQU	$00f10600	/* EXP base address */

*
* macros
*

		MACRO lockBUS doNothing
		moveq	#-1,d0			/* preset error code */
		tas	DVS+lcl_irqlock		/* check for race about EXP Port and */
		bne	doNothing		/* if somebody owns the bus we quit */
		ENDM


		MACRO lockBUSWait.size
		.LOCAL lt1
		.LOCAL lc1
* there should be a timeout based on _hz_200 (and then branch to .doNothing)

lt1:		tas	DVS+lcl_irqlock		/* check for race about EXP Port and */
		beq.b	lc1			/* if somebody owns the bus we *wait* */

		bsr.size	_appl_yield	/* wait */
		bra.b	lt1

lc1:						/* proceed */
		ENDM


		MACRO unlockBUS
		sf	DVS+lcl_irqlock		/* let other tasks access EXP Port */
		ENDM


		MACRO unlockBUSWait
		sf	DVS+lcl_irqlock		/* let other tasks access Cartridge Port */
		ENDM


RxBUS		EQU	d7
RyBUS		EQU	d6
RcBUS		EQU	a5
RdBUS		EQU	a6


		MACRO ldBUSRegs			/* for faster access to hardware */
		lea	rom3,RcBUS
		lea	rom4,RdBUS
		ENDM


		MACRO putBUS val,offset
		move.b	val,(offset)<<1(RcBUS)
		ENDM


		MACRO putMore val,offset
		putBUS val,offset
		ENDM

		MACRO putBUSW val,offset
                move.w	val,(offset)<<1(RcBUS)
		ENDM


		MACRO putMoreW val,offset
                putBUSW val,offset
		ENDM


		MACRO putBUSi val,offset
		putBUS	#val,offset
		ENDM


		MACRO getBUS offset,val
		move.b	(offset)<<1(RdBUS),val
		ENDM


		MACRO getMore offset,val
		getBUS	offset,val
		ENDM


		MACRO getBUSW offset,val
                move.w	(offset)<<1(RdBUS),val
		ENDM


		MACRO getMoreW offset,val
                getBUSW	offset,val
		ENDM

*
* macro to deselect an interface
*
		MACRO deselBUS
* empty as the EXP port does not need deselecting
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
		.LOCAL doNothing_ram2ne
		ext.l	count			/* clear upper word */
		ble	doNothing_ram2ne	/* nothing to do? */
	IFNE	WORD_TRANSFER
		move.l	count,d0
		lsr.l	#1,count
	ENDC
		bra.b	Rb1

Rt1:
	IFNE	WORD_TRANSFER
		putMoreW (addr)+,NE_DATAPORT
	ELSE
		putMore (addr)+,NE_DATAPORT
	ENDC
Rb1:		dbra	count,Rt1

	IFNE	WORD_TRANSFER
		lsr.l	#1,d0
		bcc.b	doNothing_ram2ne
		putMore	(addr)+,NE_DATAPORT
	ENDC

doNothing_ram2ne:
		ENDM



******** NE2RAM *****************************************************************
* This is a macro for speed
* get # of bytes in register arg2 to RAM location pointed to by arg1.
*
* in:	arg1	address register destination address; MUST be even
*	arg2	data register (w) # of bytes to get; must not be d0!
*		MUST be even!
*
* both registers plus d0 get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeat explicitly

		MACRO NE2RAM addr,count
		.LOCAL Nt1
		.LOCAL Nb1
		.LOCAL doNothing_ne2ram
		ext.l	count			/* clear upper word */
		ble.b	doNothing_ne2ram	/* nothing to do? */
	IFNE	WORD_TRANSFER
		lsr.l	#1,count
	ENDC
		bra.b	Nb1

Nt1:
	IFNE	WORD_TRANSFER
		getMoreW NE_DATAPORT,(addr)+
	ELSE
		getMore NE_DATAPORT,(addr)+
	ENDC
Nb1:		dbra	count,Nt1

doNothing_ne2ram:
		ENDM



*********************************************************************************

