; setjmp.s - from the mintlib
; Pure C version 21-11-92 um

; FPU=0

	.text
	.even
	.globl setjmp
setjmp:
	move.l	(sp),(a0)			; save return address
	movem.l	d3-d7/a2-a7,4(a0)	; save registers d3-d7/a2-a7
.if FPU
	.mc68881
	fmovem.x fp3-fp7,48(a0)		; save FPU registers
.endif
	clr.l	d0					; return value is 0
	rts

	.globl longjmp
	.globl __mint

longjmp:
	tst.w	__mint				; see if MiNT is active
	beq	NOMINT					; no -- do not call sigreturn
	move.l	a0,-(sp)
	move.w	d0,-(sp)			; return value is in d0
	move.w	#$11a,-(sp)			; Psigreturn() system call
	trap	#1					; (ignored if not in a sig handler)
	addq.w	#2,sp
	move.w	(sp)+,d0
	move.l	(sp)+,a0
NOMINT:
	tst.w	d0
	bne	L1						; may not be 0
	move.l	#1,d0
L1:
	movem.l	4(a0),d3-d7/a2-a7	; restore saved reggies
.if FPU
	fmovem.x 48(a0),fp3-fp7
.endif
	move.l	(a0),(sp)			; and the saved return address
	rts
