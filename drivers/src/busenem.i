*********************************************************************************
* Bus access macros for Milan for register base hardware on the ISA bus         *
* This hardware supports all(?) NE2000 (clone) cards                            *
*                                                                               *
*       Copyright 2002 Mariusz Buras, Dr. Thomas Redelberger                    *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* processor registers are used as follows:                                      *
*                                                                               *
*       a5:     will point to ISA_BASE+NE_IO_BASE       ; do not change it!     *
*       a6:     will be used temporarily                                        *
*                                                                               *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*                                                                               *
*********************************************************************************

*
* manifest constants
*

BUGGY_HW	EQU	1		/* if defined enables code to handle buggy hardware */

WORD_TRANSFER	EQU	0		/* if set enables 16-bit DMA */

*
* hardware addresses
*

ISA_BASE	EQU	$80000000	/* ISA base address for Milan */
NE_IO_BASE	EQU	$300		/* if your card is somewhere else change it. */

*
* macros
*

		MACRO lockBUS doNothing
		moveq	#-1,d0			/* preset error code */
		tas	DVS+lcl_irqlock		/* check for race about Cartrige Port and */
		bne	doNothing               /* if somebody owns the bus we quit */
		ENDM


		MACRO lockBUSWait.size
		.LOCAL lt1
		.LOCAL lc1
* there should be a timeout based on _hz_200 (and then branch to .doNothing)

lt1:		tas	DVS+lcl_irqlock		/* check for race about Cartrige Port and */
		beq.b	lc1			/* if somebody owns the bus we *wait* */

		bsr.size	_appl_yield	/* wait */
		bra.b	lt1

lc1:						/* proceed */
		ENDM


		MACRO unlockBUS
		sf	DVS+lcl_irqlock		/* let other tasks access this device */
		ENDM


		MACRO unlockBUSWait
		sf	DVS+lcl_irqlock		/* let other tasks access this device */
		ENDM


RxBUS		EQU	d6			/* unused here */
RyBUS		EQU	d7			/* unused here */
RcBUS		EQU	a5
RdBUS		EQU	a6			/* used temporarily */


		MACRO ldBUSRegs			/* for faster access to hardware */
		lea	ISA_BASE+NE_IO_BASE,RcBUS
		ENDM


		MACRO putBUS val,offset
		move.b	val,(offset)^3(RcBUS)
		ENDM


		MACRO putMore val,offset
		move.b	val,(offset)^3(RcBUS)
		ENDM


		MACRO putBUSi val,offset
		putBUS	#val,offset
		ENDM


		MACRO getBUS offset,val
		move.b	(offset)^3(RcBUS),val
		ENDM


		MACRO getMore offset,val
		getBUS offset,val
		ENDM

*
* macro to deselect an interface
*
		MACRO deselBUS
* empty as the Milan ISA bus does not need deselecting
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
		ble	doNothing_ram2ne	/* nothing to do? */

		lea	(NE_DATAPORT)^3(RcBUS),RdBUS

* put the packet; optimized for speed, we do 8 bytes at once
		ror.l	#3,count		/* store remainder in upper word */
		bra.b	Rb1

Rt1:		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
		move.b	(addr)+,(RdBUS)
Rb1:		dbra	count,Rt1

		clr.w	count			/* prepare for remainder */
		rol.l	#3,count		/* restore remainder bits */
		bra.b	Rb2

Rt2:
		move.b	(addr)+,(RdBUS)		/* put the remaining bytes */
Rb2:
		dbra	count,Rt2

doNothing_ram2ne:
		ENDM



******** NE2RAM *****************************************************************
* This is a macro for speed
* get # of bytes in register arg2 to RAM location pointed to by arg1.
*
* in:	arg1	address register destination address
*	arg2	data register (w) # of bytes to get
*
* both registers get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeate explicitly

		MACRO NE2RAM addr,count
		.LOCAL Nt1
		.LOCAL Nb1
		.LOCAL Nt2
		.LOCAL Nb2
		.LOCAL doNothing_ne2ram
		lea	(NE_DATAPORT)^3(RcBUS),RdBUS	/* register to point to data bus */
		ext.l	count			/* clear upper word */
		ble.b	doNothing_ne2ram	/* nothing to do? */

* get the Packet; optimized for speed, we do 8 bytes at once.
		ror.l	#3,count		/* store remainder in upper word */
		bra.b	Nb1

Nt1:
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
Nb1:
		dbra	count,Nt1

		clr.w	count			/* prepare for remainder */
* rol.l #2 instead of #3 because we write TWO bytes in a loop iteration not a byte
* thus the lowest significant bit (which shall be 0 anyway) is not used
		rol.l	#2,count		/* restore remainder bits */
		bra.b	Nb2

Nt2:
		move.b	(RdBUS),(addr)+
		move.b	(RdBUS),(addr)+
Nb2:
		dbra	count,Nt2

doNothing_ne2ram:
		ENDM



*********************************************************************************

