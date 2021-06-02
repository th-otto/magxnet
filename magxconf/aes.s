;
; sizes of the various arrays
; these MUST match the definitions in mt_gem.h/aes.h
;
AES_CTRLMAX		equ		15
AES_GLOBMAX		equ		15
AES_INTINMAX 	equ		132
AES_INTOUTMAX	equ		140
AES_ADDRINMAX	equ		16
AES_ADDROUTMAX	equ		16

				DATA
aespb:			dc.l	control
				dc.l	global
				dc.l	intin
				dc.l	intout
				dc.l	addrin
				dc.l	addrout

				BSS

control:		ds.w AES_CTRLMAX
global:			ds.w AES_GLOBMAX
intin:			ds.w AES_INTINMAX
intout:			ds.w AES_INTOUTMAX
addrin:			ds.l AES_ADDRINMAX
addrout:		ds.l AES_ADDROUTMAX

				TEXT

				MODULE	_aes
				lea.l	control,a1
				pea		(a2)
				moveq	#0,d2
				move.l	d2,(a1)+
				move.l	d2,(a1)+
				move.w	d2,(a1)		/* contrl[4] = naddrout */
				movep.l	d1,-7(a1)
				move.l	#aespb,d1
				move.w	#$00c8,d0
				trap	#2
				lea.l	intout,a0
				move.w	(a0)+,d0
				move.l	(a7)+,a2
				rts
				ENDMOD

				.globl appl_exit
				MODULE	appl_exit
				move.l  #$13000100,d1
				bra		_aes
				ENDMOD

				.globl appl_init
				MODULE	appl_init
				move.l	#$0a000100,d1
				bra		_aes
				ENDMOD

				GLOBL	evnt_timer_purec
				MODULE	evnt_timer_purec
				move.w	d0,intin
				move.w	d1,intin+2
				move.l  #$18020100,d1
				bra		_aes
				ENDMOD


				.globl appl_find
				MODULE	appl_find
				move.l	a0,addrin
				move.l  #$0d000101,d1
				bra		_aes
				ENDMOD

				
				.globl shel_write
				MODULE	shel_write
				movem.w	d0-d2,intin
				movem.l	a0-a1,addrin
				move.l  #$79030102,d1
				bra		_aes
				ENDMOD

