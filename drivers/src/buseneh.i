*********************************************************************************
* Bus access macros for Hades for register base hardware on the ISA bus         *
* This hardware supports all(?) NE2000 (clone) cards                            *
*                                                                               *
*       Copyright 2002 Dr. Thomas Redelberger                                   *
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

* From the Hades Manual:
*
* Der Hades hat ja neben 4 PCI auch 2 ISA Schnittstellen.
* Die sind folgendermassen dokumentiert:
* 
* ISA-Bus
* *********************************************************************
* DMA wird auf dem ISA-Bus nicht untersuetzt. Saemtliche Interruptsignale
* eines Slots sind zusammengefasst und werden der STMFP zugefuehrt.
* 
* Slot        STMFP-Anschluss
* -------------------------------
* ISA1        IO3 Pin 28
* ISA2        IO7 Pin 32
* 
* Bei Byt-Zugriffen im I/O-Bereich $FFF3'0000 - $FFF3'FFFF werden die
* Byts beim Lesen und beim Schreiben von und nach SD0-7 transferiert,
* sowohl bei high wie auch by low Byt Zugriffen. Dies ist z.B. fuer
* ET4000 Grafikkarten mit NVDI noetig.
* Im Rest dieses Bereichs wird beim Schreiben das Byt auf der anderen
* Wordhaelfte ebenfalls ausgegeben.
*  
* Adressen I/O    : $FFF0'0000-$FFF7'FFFF
* Adressen Mem    : $FF00'0000-$FF7F'FFFF
* Bus-Breite    : 16 Bit
* Groesse I/O    : 8x64 KiloByt gespiegelt
* Groesse Mem    : 8 MegaByt
* Burstmode    : Nein
* Transferrate    : 5-?
* Cachable    : Normal Nein, aber moeglich
* 


*
* manifest constants
*

BUGGY_HW	EQU	1		/* if defined enables code to handle buggy hardware */

WORD_TRANSFER	EQU	0		/* if set enables 16-bit DMA */

*
* hardware addresses
*

*ISA_BASE	EQU	$FFF30000	/* ISA base address for Hades */
*ISA_BASE	EQU	$FED00000	/* ISA base address for TsengET4000(Panther/2) */
*ISA_BASE	EQU	$FEC00000	/* ISA base address for ATIMach64(Panther/2) */
*ISA_BASE	EQU	$FE900000	/* ISA base address for ATIMach32(Panther/2) */
NE_IO_BASE	EQU	$300		/* if your card is somewhere else change it. */

	IFEQ BUS-BUS_ISA_HADES_ET4000
ISA_BASE EQU 0xFFF30000
	ENDC
	IFEQ BUS-BUS_ISA_HADES_TSENG
ISA_BASE EQU 0xFED00000
	ENDC
	IFEQ BUS-BUS_ISA_HADES_MACH64
ISA_BASE EQU 0xFEC00000
	ENDC
	IFEQ BUS-BUS_ISA_HADES_MACH32
ISA_BASE EQU 0xFE900000
	ENDC

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
		move.b	val,(offset)(RcBUS)
		ENDM


		MACRO putMore val,offset
		move.b	val,(offset)(RcBUS)
		ENDM


		MACRO putBUSi val,offset
		putBUS	#val,offset
		ENDM


		MACRO getBUS offset,val
		move.b	(offset)(RcBUS),val
		ENDM


		MACRO getMore offset,val
		getBUS offset,val
		ENDM

*
* macro to deselect an interface
*
		MACRO deselBUS
* empty as the Hades ISA bus does not need deselecting
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

		lea	(NE_DATAPORT)(RcBUS),RdBUS

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
		lea	(NE_DATAPORT)(RcBUS),RdBUS	/* register to point to data bus */
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

