*********************************************************************************
* Bus access macros for ST/TT/Falcon Cartridge Port for register base hardware  *
* Version for Lyndon Amsdon's and mine NE2000 interface hardware for the        *
* Cartridge Port. This hardware supports only fast "newer" NE2000 (clone) cards *
*                                                                               *
* Version to use only CP address line and avoids /UDS   (slower, arrgh!)        *
*                                                                               *
*       Copyright 2002 Dr. Thomas Redelberger                                   *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* processor registers are used as follows:                                      *
*       d6.w:   will be used temporarily;  do not use it!                       *
*       d7.w:   will be used temporarily;  do not use it!                       *
*                                                                               *
*       a5:     will point to ROM3 at $FB0000;  do not change it!               *
*       a6:     will point to ROM4 at $FA0000;  do not change it!               *
*                                                                               *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*                                                                               *
*********************************************************************************

*
* manifest constants
*

BUGGY_HW	=	1		/* if defined enables code to handle buggy hardware */

WORD_TRANSFER	=	0		/* if set enables 16-bit DMA */

*
* hardware addresses
*

rom4		=	$00fa0000	/* ROM4 base address */
rom3		=	$00fb0000	/* ROM3 base address */

*
* macros
*

		.MACRO lockBUS.size doNothing
		moveq	#-1,d0			/* preset error code */
		tas	DVS+lcl_irqlock		/* check for race about Cartrige Port and */
		bne.size	doNothing	/* if somebody owns the bus we quit */
		.ENDM


		.MACRO lockBUSWait.size
		.LOCAL lt1
		.LOCAL lc1
* there should be a timeout based on _hz_200 (and then branch to .doNothing)

lt1:		tas	DVS+lcl_irqlock		/* check for race about Cartrige Port and */
		beq.b	lc1			/* if somebody owns the bus we *wait* */

		bsr.size	_appl_yield	/* wait */
		bra.b	lt1

lc1:						/* proceed */
		.ENDM


		.MACRO unlockBUS
		sf	DVS+lcl_irqlock		/* let other tasks access Cartridge Port */
		.ENDM


		.MACRO unlockBUSWait
		sf	DVS+lcl_irqlock		/* let other tasks access Cartridge Port */
		.ENDM


RxBUS		=	d6
RyBUS		=	d7
RcBUS		=	a5
RdBUS		=	a6


		.MACRO ldBUSRegs			/* for faster access to hardware */
		lea	rom3,RcBUS
		lea	rom4,RdBUS
		.ENDM


		.MACRO putBUS val,offset
		move.w	#(offset)<<8,RyBUS	/* move ISA address to bits 8-15 */
		.IFNE CPU020
		move.b	val,RyBUS		/* merge in data */
		.DC.W	$4A35,$7200		/* tst.b	0(RcBUS,RyBUS.w*2); machine code as Genst2 cannot do 68030 */
		.IFNE CPU060
		nop
		.ENDC
		.ELSE
		move.w	RyBUS,RxBUS		/* get a copy */
		move.b	val,RyBUS		/* merge in data */
		add.w	RyBUS,RyBUS		/* shift up one bit to avoid UDS/LDS */
		tst.b	0(RcBUS,RyBUS.w)	/* write by reading */
		.ENDC
		.ENDM


		.MACRO putMore val,offset
		.IFNE CPU020
		move.b	val,RyBUS		/* merge in data */
		.DC.W	$4A35,$7200		/* 0(RcBUS,RyBUS.w*2); machine code as Genst2 cannot do 68030 */
		.IFNE CPU060
		nop
		.ENDC
		.ELSE
		move.w	RxBUS,RyBUS		/* move ISA address to bits 8-15 */
		move.b	val,RyBUS		/* merge in data */
		add.w	RyBUS,RyBUS		/* shift up one bit to avoid UDS/LDS */
		tst.b	0(RcBUS,RyBUS.w)	/* write by reading */
		.ENDC
		.ENDM


		.MACRO putBUSi val,offset
		tst.b	((offset<<8)+(val))<<1(RcBUS)	/* write by reading */
		.IFNE CPU060
		nop
		.ENDC
		.ENDM


		.MACRO getBUS offset,val
		move.b	(offset)<<9(RdBUS),val	/* read from CP */
		.ENDM


		.MACRO getMore offset,val
		move.b	(offset)<<9(RdBUS),val	/* read from CP */
		.ENDM

*
* macro to deselect an interface
*
		.MACRO deselBUS
* empty as the cartridge port does not need deselecting
		.ENDM



******** RAM2NE *****************************************************************
* This is a macro for speed
* put # of bytes in register arg2 to NE from location pointed to by arg1.
*
* in:	arg1	address register source address
*	arg2	data register (w) # of bytes to put/* must not be d0! */
*
* both registers plus d0 get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeate explicitly
	
		.IFEQ CPU060

		.MACRO RAM2NE addr,count
		.LOCAL Rt1
		.LOCAL Rb1
		.LOCAL Rt2
		.LOCAL Rb2
		.LOCAL doNothing_ram2ne
		ext.l	count			/* clear upper word */
		subq.w	#2,count		/* first two puts are outside loop */
		ble	doNothing_ram2ne	/* nothing to do? */
		putBUS	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT

* put the packet; optimized for speed, we do 8 bytes at once
		ror.l	#3,count		/* store remainder in upper word */
		bra.b	Rb1

Rt1:		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
		putMore	(addr)+,NE_DATAPORT
Rb1:		dbra	count,Rt1

		clr.w	count			/* prepare for remainder */
		rol.l	#3,count		/* restore remainder bits */
		bra.b	Rb2

Rt2:		putMore	(addr)+,NE_DATAPORT	/* put the remaining bytes */
Rb2:		dbra	count,Rt2

doNothing_ram2ne:
		.ENDM

		.ELSE

		.MACRO RAM2NE addr,count
		.LOCAL Rt1
		.LOCAL Rb1
		.LOCAL Rt2
		.LOCAL Rb2
		.LOCAL doNothing_ram2ne
		ext.l	count			/* clear upper word */
		subq.w	#1,count		/* first two puts are outside loop */
		ble.w	doNothing_ram2ne	/* nothing to do? */
		putBUS	(addr)+,NE_DATAPORT

* put the packet; optimized for speed
		bra.w	Rb1

Rt1:		putMore	(addr)+,NE_DATAPORT
Rb1:		dbra	count,Rt1

doNothing_ram2ne:
		.ENDM

		.ENDC


******** NE2RAM *****************************************************************
* This is a macro for speed
* get # of bytes in register arg2 to RAM location pointed to by arg1.
*
* in:	arg1	address register destination address; MUST be even
*	arg2	data register (w) # of bytes to get; must not be d0!
*		MUST be even!
*
* both registers plus d0 get destroyed.
* Assembler inst. REPT does not work inside a macro, we repeate explicitly

		.IFEQ CPU060

		.MACRO NE2RAM addr,count
		.LOCAL Nt1
		.LOCAL Nb1
		.LOCAL Nt2
		.LOCAL Nb2
		.LOCAL doNothing_ne2ram
		ext.l	count			/* clear upper word */
		ble.b	doNothing_ne2ram	/* nothing to do? */

* get the Packet; optimized for speed, we do 16 bytes at once.
		ror.l	#4,count			/* store remainder in upper word */
		bra.b	Nb1

Nt1:		movep.l	NE_DATAPORT<<9(RdBUS),d0	/* four bytes at once */
		move.l	d0,(addr)+
		movep.l	NE_DATAPORT<<9(RdBUS),d0
		move.l	d0,(addr)+
		movep.l	NE_DATAPORT<<9(RdBUS),d0	/* four bytes at once */
		move.l	d0,(addr)+
		movep.l	NE_DATAPORT<<9(RdBUS),d0
		move.l	d0,(addr)+
Nb1:		dbra	count,Nt1

		clr.w	count				/* prepare for remainder */
* rol.l #3 instead of #4 because we write a WORD at once not a byte
* thus the lowest significant bit (which shall be 0 anyway) is not used
		rol.l	#3,count			/* restore remainder bits */
		bra.b	Nb2

Nt2:		movep.w	NE_DATAPORT<<9(RdBUS),d0
		move.w	d0,(addr)+
Nb2:		dbra	count,Nt2

doNothing_ne2ram:
		.ENDM

		.ELSE

		.MACRO NE2RAM addr,count
		.LOCAL Nt1
		.LOCAL Nb1
		.LOCAL Nt2
		.LOCAL Nb2
		.LOCAL doNothing_ne2ram
		ext.l	count			/* clear upper word */
		subq.w #1,count
		ble.w	doNothing_ne2ram	/* nothing to do? */

* get the Packet; optimized for speed

Nt1:	
		getBUS	NE_DATAPORT,(addr)+
		bra.w   Nb2

Nt2:	getMore	NE_DATAPORT,(addr)+
Nb2:		dbra	count,Nt2

doNothing_ne2ram:
		.ENDM

		.ENDC


*********************************************************************************

