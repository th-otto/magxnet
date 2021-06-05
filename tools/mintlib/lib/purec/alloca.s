; alloca(nbytes) allocate junk in stack frame
; Pure C version 19-6-92 bm
;
; Poor C doesn't handle stack frames correctly. It tries to restore
; registers sp-relative *before* unlk, so if we change the stack
; pointer, we must copy up to 36 bytes :-(
;
;  void *alloca(size_t size)
;

; the caller MUST be compiled with option -S !!!

.globl alloca

; STACKCH = 1
; FPU = 0

.text
alloca:
	tst.l d0			;alloca(0)?
	beq ret

	movea.l (sp)+,a0	;get return address
	movea.l sp,a1		;old stack pointer
	addq.l #1,d0		;ensure address in d0 even
	bclr.l #0,d0		;lop off extra bits

	suba.l d0,sp		;increase stack frame

.if STACKCH
	cmpa.l _StkLim, sp	;enough stack space?
	bcs no_space
.endif
	
	move.l a6,d1		;calculate amount of bytes to be copied
	sub.l a1,d1
.iff FPU
	cmpi.l #37,d1		;maximum 36 bytes (a2-a5/d3-d7)
.else
	cmpi.l #97,d1		;unless FPU is used
.endif
	bcs alloca1
	moveq.l #36,d1
	
alloca1:
	suba.l d1,sp		;space for the copy
	move.l a0,-(sp)		;push return address
	lea 4(sp),a0		;a0 <- sp + 4
	
	bra alloca3			;start copying:
						;d1 bytes from (a1) to (a0)
alloca2:
	move.w (a1)+,(a0)+
alloca3:
	subq.l #2,d1
	bne alloca2
ret:
	rts					;a0 is start of free block

.if STACKCH
.globl _StkLim
.globl _StkOver
no_space:
	adda.l	d0,sp
	jmp _StkOver		;und tschž
.endif
