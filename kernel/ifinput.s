/* short cdecl if_input(struct netif *nif, BUF *buf, long delay, short type) */
/* FIXME: for some unknown reason, implemented in asm */
	.xref addroottimeout
	.xref if_enqueue
	.xref if_doinput
	.xref tmout

	.text
	.globl if_input
if_input:
	movem.l    d2-d3/a2,-(a7)
	moveq.l    #0,d1
	move.w     sr,d3
	ori.w      #0x0700,sr
	move.l     20(a7),d1 /* BUG: clobbers return value */
	beq.s      if_input1
	move.w     28(a7),d2
	ext.l      d2
	movea.l    d1,a2
	move.l     d2,26(a2)
	move.w     #1,-(a7)
	move.l     d1,-(a7)
	movea.l    22(a7),a0
	pea.l      84(a0)
	bsr        if_enqueue
	lea.l      10(a7),a7
	move.w     d0,d1
if_input1:
	move.l     tmout,d0
	bne.s      if_input2
	move.w     #1,-(a7)
	pea.l      if_doinput(pc)
	move.l     30(a7),-(a7)
	bsr        addroottimeout
	lea.l      10(a7),a7
	move.l     d0,tmout
if_input2:
	move.w     d3,sr
	move.w     d1,d0
	movem.l    (a7)+,d2-d3/a2
	rts
