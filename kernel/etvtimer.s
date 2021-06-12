	.globl new_etv_timer
	.globl old_etv_timer

	.globl uninstall_xbra
	.xref x1ef60

timer_c   = 0x114
etv_timer = 0x400

	.text
	
	.dc.l 0x58425241 /* 'XBRA' */
	.dc.l 0x53434b4d /* 'SCKM' */
old_etv_timer:
	.dc.l 0
new_etv_timer:
	move.l     x1ef60,d0
	beq.s      new_etv_timer1
	movea.l    d0,a0
	moveq.l    #20,d0
	sub.l      d0,8(a0)
new_etv_timer1:
	movea.l    new_etv_timer-4(pc),a0
	jmp        (a0)

/*
 * remove ourselves from XBRA chains.
 * return 0 on success
 */
uninstall_xbra:
	movem.l    a0-a1,-(a7)
	move.w     sr,-(a7)
	ori.w      #0x0700,sr
	lea.l      (timer_c).w,a0 /* timer c interrupt */
	bsr.w      check_xbra
	move.l     d0,-(a7)
	lea.l      (etv_timer).w,a0
	bsr.w      check_xbra
	or.l       (a7)+,d0
	move.w     (a7)+,sr
	movem.l    (a7)+,a0-a1
	rts

check_xbra:
	movea.l    (a0),a1
	moveq.l    #1,d0
	cmpi.l     #0x58425241,-12(a1) /* 'XBRA' */
	bne.s      check_xbra2 /* no xbra: we are done */
	cmpi.l     #0x53434B4D,-8(a1)  /* 'SCKM' */
	beq.s      check_xbra1 /* found our id */
	lea.l      -4(a1),a0   /* wrong id, keep searching */
	bra.s      check_xbra
check_xbra1:
	moveq.l    #0,d0
	move.l     -4(a1),(a0)
check_xbra2:
	rts
