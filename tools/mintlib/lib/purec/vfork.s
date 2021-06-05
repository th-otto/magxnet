; From the mintlib.
; Pure C version 21-6-92 bm
;
; vfork for MiNT. Note that the return address must be popped off the stack,
; or else it could be clobbered by the child and the parent would be left
; returning to la-la land. Also note that MiNT guarantees that register a1
; will be preserved across a vfork() system call.
;
	.globl	vfork
	.globl	__mint		; MiNT version kept here
	.comm	L_vfsav,128
	.globl errno
	.globl tfork		; thread.c
	.text
	.even
vfork:
	move.l	(sp)+,a1	; save return address; this is important!
	tst.w	__mint
	beq	L_TOS			; go do the TOS thing
	move.w	#$113,-(sp)	; push MiNT Pvfork() parameter
	trap	#1			; Vfork
	addq.l	#2,sp
	tst.l	d0			; error??
	bmi	L_err
	jmp	(a1)			; return
L_TOS:
	movem.l	d2-d7/a1-a6,L_vfsav	; save registers
	move.l #L_vfsav,d0	; arguments are passed in a0 (func) and d0 (arg)
	lea.l L_newprog,a0
	jsr	tfork			; tfork(L_newprog, L_vfsav)
	movem.l	L_vfsav,d2-d7/a1-a6	; restore reggies
	tst.l	d0			; fork went OK??
	bmi	L_err			; no -- error
	jmp	(a1)			; return to caller
L_err:
	neg.l	d0
	move.w	d0,errno	; save error code in errno
	moveq.l	#-1,d0		; return -1
	jmp	(a1)			; return

;
; L_newprog: here is where the child starts executing, with argument
; L_vfsav. We restore registers, zero d0, and jump back to parent
;

L_newprog:
	addq.l	#4,sp		; pop useless return address
	move.l	d0,a0		; get address of save area
	movem.l	(a0),d2-d7/a1-a6	; restore reggies
	clr.l	d0			; child always returns 0 from vfork
	jmp	(a1)			; back to caller, as child process
