; Line A bindings, Pure C version; adapted from jrb's
; why is this part of the MiNT library?

	.bss

	.globl __aline
__aline:
	.ds.l	1
	.globl __fonts
__fonts:
	.ds.l	1
	.globl __funcs
__funcs:
	.ds.l	1

	.text

	.globl linea0
	.IFNE 0
linea0:
	move.l	a2,-(sp)
	.dc.w   $A000
	move.l	a0,__aline	
	move.l	a1,__fonts
	move.l	a2,__funcs
	move.l	(sp)+,a2
	rts
	.else
	.offset 0
linea0:
	 .text
	.ENDC

	.globl linea1
linea1: 							
	.dc.w   $A001
	rts

	.globl linea2
linea2:
	.dc.w   $A002
	rts

	.globl linea3
linea3: 							
	.dc.w   $A003
	rts

	.globl linea4
linea4: 							
	.dc.w   $A004
	rts

	.globl linea5
linea5: 							
	.dc.w   $A005
	rts

	.globl linea6
linea6: 							
	.dc.w   $A006
	rts

	.globl linea7
linea7:
	move.l	a6,-(sp)
	move.l	a0,a6
	.dc.w   $A007
	move.l	(sp)+,a6
	rts

	.globl linea8
linea8: 							
	.dc.w   $A008
	rts

	.globl linea9
linea9: 							
	.dc.w   $A009
	rts

	.globl lineaa
lineaa: 							
	.dc.w   $A00A
	rts

	.globl lineab
lineab: 							
	.dc.w   $A00B
	rts

	.globl lineac
lineac:
	move.l	a6,-(sp)
	move.l	a2,-(sp)
	move.l	a0,a2
	.dc.w   $A00C
	move.l	(sp)+,a2
	move.l	(sp)+,a6
	rts

	.globl linead
linead:
	move.l	a6,-(sp)
	move.l	a2,-(sp)
	move.l	a1,a2
	.dc.w   $A00D
	move.l	(sp)+,a2
	move.l	(sp)+,a6
	rts

	.globl lineae
lineae: 							
	.dc.w   $A00E
	rts

	.globl lineaf
lineaf: 							
	.dc.w   $A00F
	rts
