*********************************************************************************
* Helper Macros for ST assembler programming					*
*	Copyright 2001 Dr. Thomas Redelberger					*
*	Use it under the terms of the GNU General Public License		*
*	(See file COPYING.TXT)							*
*										*
*										*
* Tabsize 8, developed with DEVPAC assembler 2.0.				*
*										*
*********************************************************************************

*
* external references
*
		.xref	prntStr		/* CDECL (); debugging */
		.xref	prntLong	/* CDECL (); " */
		.xref	prntWord	/* CDECL (); " */
		.xref	prntByte	/* CDECL (); " */
		.xref	prntSR		/* CDECL (); " */

*
* macros
*

		.MACRO	Alloc size		/* allocate a number of bytes on stack */
		.IFLE	size-8
		subq.l	#size,sp
		.ELSE
		lea	-size(sp),sp
		.ENDC
		.ENDM

		.MACRO	deAlloc size		/* pop a number of bytes from stack */
		.IFLE	size-8
		addq.l	#size,sp
		.ELSE
		lea	size(sp),sp
		.ENDC
		.ENDM


**** debugging macros ***********************************************************

* sounds the bell

		.MACRO myPling
		move	#$0700,-(sp)		/* string BELL */
		pea	(sp)			/* arg: address to this string */
		bsr	prntStr
		addq.l	#6,sp			/* pop arg and string */
		.ENDM

* polls if a key was pressed
*	d0 =  0 no key pressed
*	d0 = -1 key pressed
*	
		.MACRO PollKey
		movem.l	d1-d2/a0-a2,-(sp)
		move.l	#$00010002,-(sp)	/* bconstat (1) con (2) */
		trap	#13
		addq.l	#4,sp
		movem.l	(sp)+,d1-d2/a0-a2
		.ENDM

* waits for a key pressed
*	bits 0-7 ACSII
*	bits 16-23 scan code
*	bits 24-31 value of Kbshift()
		.MACRO WaitKey
		movem.l	d1-d2/a0-a2,-(sp)
		move.l	#$00020002,-(sp)	/* bconin (2) con (2) */
		trap	#13
		addq.l	#4,sp
		movem.l	(sp)+,d1-d2/a0-a2
		.ENDM


		.MACRO PrA msg,cr,lf
		.LOCAL mess
		.LOCAL cont
		pea	mess(pc)
		bsr	prntStr
		addq.l	#4,sp
		bra.b	cont
mess:
		DC.B	msg
		DC.B    cr
		DC.B    lf
		DC.B    0
		EVEN
cont:
		.ENDM


		.MACRO PrS msg
		pea	msg
		bsr	prntStr
		addq.l	#4,sp
		.ENDM


		.MACRO PrL val
		move.l	val,-(sp)
		bsr	prntLong
		addq.l	#4,sp
		.ENDM


		.MACRO PrW val
		move.w	val,-(sp)
		bsr	prntWord
		addq.l	#2,sp
		.ENDM


		.MACRO PrB val
		move.b	val,-(sp)	/* the 68K pushes an extra align byte */
		bsr	prntByte
		addq.l	#2,sp		/* the 68K pushes an extra align byte */
		.ENDM


*********************************************************************************
