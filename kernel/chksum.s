/*
 * chksum functions.
 * Versions are fore Pure-C only,
 * GNU-C uses inline-asm
 */

	.text

/* ushort udp_checksum(struct udp_dgram *dgram, in_addr_t srcadr, in_addr_t dstadr) */
	.globl udp_checksum
udp_checksum:
	move.l     #0x00110000,d2 /* IPPROTO_UDP */
	move.w     4(a0),d2 /* get length from dgram */
	bra.s      tcp_chksum1

/* ushort tcp_checksum(struct tcp_dgram *dgram, ulong srcadr, ulong dstadr, ushort len) */
	.globl tcp_checksum
tcp_checksum:
	swap       d2
	move.w     #6,d2 /* IPPROTO_TCP */
	swap       d2

tcp_chksum1:
	add.w      d0,d1
	clr.w      d0
	swap       d0
	addx.w     d1,d0
	clr.w      d1
	swap       d1
	addx.l     d1,d0
	moveq.l    #0,d1
	move.w     d2,d1
	add.l      d1,d0
	clr.w      d2
	swap       d2
	add.l      d2,d0
	clr.l      d2
	ror.l      #1,d1
	beq.s      tcp_chksum3
	subq.w     #1,d1
tcp_chksum2:
	move.w     (a0)+,d2
	add.l      d2,d0
	dbf        d1,tcp_chksum2
tcp_chksum3:
	tst.l      d1
	bpl.s      tcp_chksum4
	move.b     (a0)+,d2
	asl.w      #8,d2
	add.l      d2,d0
tcp_chksum4:
	move.w     d0,d2
	clr.w      d0
	swap       d0
	add.w      d2,d0
	clr.w      d2
	addx.w     d2,d0
	not.w      d0
	rts

/* short chksum(void *buf, short nwords) */
* BUG: is exported via netinfo and must be cdecl */
	.globl chksum
chksum:
	move.w     d0,d1
	moveq.l    #0,d2
	moveq.l    #0,d0
	lsr.w      #1,d1
	bcc.s      chksum1
	add.w      (a0)+,d2
	addx.w     d0,d2
chksum1:
	subq.w     #1,d1
	bmi.s      chksum3
chksum2:
	add.l      (a0)+,d2
	addx.l     d0,d2
	dbf        d1,chksum2
chksum3:
	move.l     d2,d1
	swap       d1
	add.w      d1,d2
	addx.w     d0,d2
	move.w     d2,d0
	not.w      d0
	rts

/* void small_memcpy(void *dst, const void *src, size_t len); */
	.globl small_memcpy
small_memcpy:
	subq.l     #1,d0
	bne.s      small_memcpy1
	rts
small_memcpy1:
	move.b     (a1)+,(a0)+
	dbf        d0,small_memcpy /* BUG: wrong label */
	rts
