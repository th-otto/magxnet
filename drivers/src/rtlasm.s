/*
 * Low level and interrupt routines for the RTL8012 driver
 *
 * 2000-08-02 Frank Naumann
 * 2000-03-24 Vassilis Papathanassiou
 *
 */
	.globl old_vbl_int
	.globl vbl_interrupt
	.globl rtl8012_int

	.xref in_use
	.xref Setexc
	.xref Mfpint

/* PAGE 0 of EPLC registers */
IDR0		= 0x0
IDR1		= 0x1
IDR2		= 0x2
IDR3		= 0x3
IDR4		= 0x4
IDR5		= 0x5
TBCR0		= 0x6
TBCR1		= 0x7
TSR		= 0x8
RSR		= 0x9
ISR		= 0xA
IMR		= 0xB
CMR1		= 0xC
CMR2		= 0xD
MODSEL		= 0xE
MODDEL		= 0xF

PNR		= TBCR0
COLR		= TBCR1

/* PAGE 1 of EPLC registers */
MAR0		= 0x0
MAR1		= 0x1
MAR2		= 0x2
MAR3		= 0x3
MAR4		= 0x4
MAR5		= 0x5
MAR6		= 0x6
MAR7		= 0x7
PCMR		= 0x8
PDR		= 0x9


/* The followings define remote DMA command through LptCtrl */
ATFD		= 3		/* ATFD bit in Lpt's Control register */
					/* -> ATFD bit is added for Xircom's MUX */
Ctrl_LNibRead	= 0x8+ATFD	/* specify low  nibble */
Ctrl_HNibRead	= 0x0+ATFD	/* specify high nibble */
Ctrl_SelData	= 0x4+ATFD	/* not through LptCtrl but through LptData */
Ctrl_IRQEN	= 0x10		/* set IRQEN of lpt control register */

/* Here define constants to construct the required read/write commands */
WrAddr		= 0xC0		/* set address of EPLC write register */
RdAddr		= 0x40		/* set address of EPLC read register */

EOR		= 0x20		/* ORed to make 'end of read',set CSB=1 */
EOW		= 0xE0		/* end of write, R/WB=A/DB=CSB=1 */
EOC		= 0x60		/* End Of r/w Command, R/WB=A/DB=CSB=1 */
HNib	= 0x10

/* EPLC register contents */
EPLC_RBER	= 0x1		/* ISR, HNib */
EPLC_ROK	= 0x4		/* ISR */
EPLC_TER	= 0x2		/* ISR */
EPLC_TOK	= 0x1		/* ISR */

EPLC_POV	= 0x4		/* RSR, HNib */
EPLC_BUFO	= 0x1		/* RSR, HNib */

EPLC_RST	= 0x4		/* CMR1, HNib */
EPLC_RxE	= 0x2		/* CMR1, HNib */
EPLC_TxE	= 0x1		/* CMR1, HNib */
EPLC_RETX	= 0x8		/* CMR1 */
EPLC_TRA	= 0x4		/* CMR1 */
EPLC_IRQ	= 0x2		/* CMR1 */
WRPAC		= 0x2		/* CMR1	*/
RDPAC		= 0x1		/* CMR1	*/
EPLC_BUFE	= 0x1		/* CMR1 */
EPLC_JmpPac	= EPLC_BUFE

EPLC_AM1	= 0x2		/* CMR2, HNib */
EPLC_AM0	= 0x1		/* CMR2, HNib */
EPLC_PAGE	= 0x4		/* CMR2 */
EPLC_RAMTST	= 0x2		/* CMR2 */
EPLC_LOOPBK	= 0x4		/* CMR2 */
EPLC_IRQOUT	= 0x1		/* CMR2 */
EPLC_CMR2Null	= 0x0		/* CMR2 */

EPLC_AddrMode	= 0x2		/* accept physical & broadcast packets */
					/* reject error packets */
RxMode		= EPLC_AddrMode

RxCount	= 2
RxLSB		= 3
RxMSB		= 4
RxStatus	= 5

	.xref Table
	.xref in_use
	.xref Setexc
	.xref Mfpint
	.xref rtl8012_int
	.xref RxBuffer

_hz_200 = 0x4ba



	.MACRO WrNib reg,value
	/* x = reg */
	move.w     reg,d2

	/* End Of Write */
	/* *(volatile uchar *) (0xfa0000 + Table[EOC | x]); */
	move.w     d2,d3
	ori.w      #EOC,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	
	/* x = WrAddr | reg; */
	move.w     d3,d2
	ori.w      #WrAddr,d2
	
	/* Prepare address */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	move.w     d2,d3
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)

	/* x &= 0xf0; */
	move.w     d3,d2
	and.w      #0x00F0,d2
	/* x |= value; */
	or.w       value,d2
	
	/* Write address */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	
	/* x = value + 0x80; */
	move.w     value,d2
	add.w      #0x0080,d2
	
	/* Write data */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	move.w     d2,d3
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)

	/* End Of Write */
	/* *(volatile uchar *) (0xfa0000 + Table[EOW|x]);  */
	move.w     d3,d2
	or.b       #EOW,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	.ENDM


	.MACRO WrByte reg,value
	/* x = reg */
	move.w     reg,d2

	/* End Of Write */
	/* *(volatile uchar *) (0xfa0000 + Table[EOC | x]); */
	ori.w      #EOC,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	
	/* x = WrAddr | reg; */
	move.w     reg,d2
	ori.w      #WrAddr,d2
	
	/* Prepare address */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	move.w     d2,d3
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)

	/* x &= 0xf0; */
	move.w     d3,d2
	and.w      #0x00F0,d2
	/* x |= value; */
	move.w     value,d3
	andi.w     #0x0F,d3
	or.w       d3,d2
	
	/* Write address */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	
	/* x = value + 0x80; */
	move.w     d3,d2
	add.w      #0x0080,d2
	
	/* Write data */
	/* *(volatile uchar *) (0xfa0000 + Table[x]); */
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)

	/* End Of Write */
	/* *(volatile uchar *) (0xfa0000 + Table[EOW|x]);  */
	move.w     reg,d2
	ori.w      #EOW-HNib,d2
	move.w     d2,d3
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)

	move.w     d3,d2
	andi.w     #0xF0,d2
	move.w     value,d3
	lsr.w      #4,d3
	or.w       d3,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	move.w     d3,d2
	ori.w      #0x0090,d2
	move.w     d2,d3
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	move.w     d3,d2
	ori.w      #EOW,d2
	andi.w     #0x007F,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	.ENDM


	.MACRO RdNib reg
	/* *(volatile uchar *) (0xfa0000UL + Table[EOC + rreg]); */
	move.w     #EOC+reg,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	/* *(volatile uchar *) (0xfa0000UL + Table[EOC + rreg]); */
	move.w     #RdAddr+reg,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	/* Read nib */
	move.w     (a3),d0
	andi.w     #0x000F,d0
	.ENDM


	.MACRO udelay ticks
	.LOCAL loop
	move.w     sr,d1
	andi.w     #0xF3FF,sr
	move.l     (_hz_200).w,d0
	addq.w     #ticks,d0
loop:
	cmp.l      (_hz_200).w,d0
	bpl.s      loop
	move.w     d1,sr
	.ENDM

	
	.MACRO RdBytEP
	/* get lo nib */
	move.w     (a3),d0
	andi.w     #0x000F,d0
	/* CLK down */
	tst.b      (a4)
	nop
	/* get hi nib */
	move.w     (a3),d1
	lsl.b      #4,d1
	or.b       d1,d0
	/* CLK up */
	lea.l      0x00FB2000,a1
	tst.b      (a1)
	nop
	.ENDM


	.MACRO P14_16_17_UP
	lea.l      0x00FBE000,a0
	tst.b      (a0)
	.ENDM
	

	.MACRO CSB_DN_CLK_UP
	lea.l      0x00FB2000,a0
	tst.b      (a0)
	.ENDM




	.globl vbl_interrupt
	.globl old_vbl_int
	.globl rtl8012_install_int
	.globl rtl8012_doreset
	.globl GetNodeID
	.globl rtl8012_sethw
	.globl rtl8012_stop
	.globl send_block
	.globl rtl8012_active
	.globl get_paclen
	.globl read_block
	.globl buf_empty

	.text

/* unused */
	move.b     #0xFE,(0xFFFFFA0D).w
	rts
	bclr       #0,(0xFFFFFA15).w
	rts
	bset       #0,(0xFFFFFA15).w
	rts

	dc.l	0x58425241		/* XBRA */
	dc.l	0x52544c70		/* RTLp */
old_vbl_int:
	ds.l	1
vbl_interrupt:
	tst.w in_use
	bne.s vbl_interrupt1
	movem.l	a0-a2/d0-d2,-(sp)
	lea in_use,a2
	move.w #1,(a2)
	bsr	rtl8012_int
	lea in_use,a2
	clr.w (a2)
	movem.l	(sp)+,a0-a2/d0-d2
vbl_interrupt1:
	move.l	old_vbl_int(PC),-(sp)
	rts

inst_vbl_int:
	move.l     (0x70).w,old_vbl_int
	move.l     #vbl_interrupt,(0x70).w
	rts
rtl8012_install_int:
	pea.l      (a2)
	pea.l      inst_vbl_int
	move.w     #38,-(a7) /* Supexec */
	trap       #14
	addq.l     #6,a7
	movea.l    (a7)+,a2
	rts

/* unused */
inst_busy_int:
	movea.w    #-1,a0
	moveq.l    #256/4,d0
	bsr        Setexc
	move.l     a0,busy_int-4
	clr.l      d0
	lea.l      busy_int,a0
	bsr        Mfpint /* BUG? should be Setexc */
	bset       #0,(0xFFFFFA09).w /* enable interrupt */
	bset       #0,(0xFFFFFA03).w /* set active edge */
	rts


rtl8012_doreset:
	movem.l    d3-d7/a2-a6,-(a7)
	move.w     d0,d7
	move.w     sr,d0
	ori.w      #0x0700,sr
	move.w     d0,save_sr
	jsr        resethw
	tst.w      d0
	beq.s      doreset1
	tst.w      d7
	beq.s      doreset1
	jsr        rtl8012_ramtest
doreset1:
	move.w     save_sr,sr
	movem.l    (a7)+,d3-d7/a2-a6
	rts
save_sr:
	dc.w       0


	dc.l	0x58425241		/* XBRA */
	dc.l	0x72746c61		/* rtla */
	dc.l	0xdeadface
busy_int:
	movem.l    d0-d2/a0-a2,-(a7)
	move.w     sr,d0
	ori.w      #0x0700,sr
	move.w     16(a7),d1
	and.w      #0x0700,d1
	and.w      #0xF8FF,d0
	or.w       d1,d0
	move.w     d0,sr
	jsr        rtl8012_int
	ori.w      #0x0700,sr
	movem.l    (a7)+,d0-d2/a0-a2
	move.b     #0xFE,(0xFFFFFA11).w
	rte


resethw:
	lea.l      0xFFFA0000,a3
	lea.l      0xFFFB0000,a4
	lea.l      Table,a2

	P14_16_17_UP

	WrNib #MODSEL,#0
	WrNib #MODSEL+HNib,#HNib+2

	RdNib MODSEL+HNib
	/* if (i != 2) return 0; */
	cmpi.b     #2,d0
	beq.s      resethw1
	moveq.l    #0,d0
	rts
resethw1:
	WrNib #CMR1+HNib,#HNib+EPLC_RST

	udelay 4
resethw2:
	RdNib CMR1+HNib
	btst       #2,d0
	bne.s      resethw2
	nop

	/* Clear ISR */
	WrNib #ISR,#EPLC_ROK+EPLC_TER+EPLC_TOK
	WrNib #ISR+HNib,#HNib+EPLC_RBER

	RdNib CMR2+HNib
	nop
	nop
	
	WrNib #MODDEL,#0
	/* Accept physical and broadcast */
	WrNib #CMR2+HNib,#HNib+RxMode
	/* Enable Rx TEST ONLY!!! */
	WrNib #CMR1+HNib,#HNib+EPLC_RxE+EPLC_TxE
	moveq.l    #1,d0
	rts


GetNodeID:
	movem.l    d3-d7/a2-a5,-(a7)
	lea.l      0x00FA0000,a3
	lea.l      Table,a2
	WrNib #CMR2,#EPLC_PAGE

	moveq.l    #0,d6
	move.l     #0x00006000,d4
read_next_id_word:
	movem.l    d4/d6,-(a7)
	moveq.l    #9,d6
	bsr        readbits
	bra        read_bit
read_bit_return:
	move.b     d5,d0
	move.b     d0,(a0)+
	asr.w      #8,d5
	move.b     d5,(a0)+
	nop
	nop
	WrNib #PCMR+HNib,#0x04+HNib
	udelay 1 
	movem.l    (a7)+,d4/d6
	addi.w     #0x0040,d4
	addq.w     #1,d6
	cmpi.w     #3,d6
	blt        read_next_id_word

	WrNib #CMR2,#EPLC_CMR2Null			/* back to page 0 */
	movem.l    (a7)+,d3-d7/a2-a5
	rts


read_bit:
	moveq.l    #0,d5
	moveq.l    #16,d7
read_bit1:
	WrNib #PCMR+HNib,#0x4+0x2+HNib		/* pull SK low */
	udelay 1
	WrNib #PCMR+HNib,#0x2+HNib		/* pull SK high to latch DO */
	RdNib PDR
	nop
	nop
	lsl.l      #1,d5
	btst       #0,d0
	beq.s      read_bit2
	addq.w     #1,d5
read_bit2:
	tst.b      0xFFFA007F
	dbf        d7,read_bit1
	bra        read_bit_return


readbits:
	moveq.l    #0,d0
	roxl.w     #1,d4
	move.w     #0x04+0x02+HNib,d5   /* d5 = 000 HNIB  0 SKB CS 1 */
	addx.w     d0,d5
	WrNib #PCMR+HNib,d5			/* pulls SK low and outputs DI */
	udelay 1
	andi.b     #0xFB,d5			/* let SKB=0 */
	WrNib #PCMR+HNib,d5			/* Pulls SK high to end this cycle */
	dbf        d6,readbits
	rts


rtl8012_sethw:
	movem.l    d3-d7/a2-a5,-(a7)
	lea.l      0x00FA0000,a3
	lea.l      Table,a2
	move.w     #IDR0,d0
	moveq.l    #0,d5
	moveq.l    #5,d7
rtl8012_sethw1:
	move.b     (a0)+,d5
	WrByte d0,d5
	addq.w     #1,d0
	dbf        d7,rtl8012_sethw1

	/* Now set NIC to accept broadcast */
	WrNib #CMR2,#EPLC_PAGE				/* point to page 1 */
	move.w     #MAR0,d0
	move.l     #0xFF,d5
	moveq.l    #7,d7
rtl8012_sethw2:
	WrByte d0,d5
	addq.w     #1,d0
	dbf        d7,rtl8012_sethw2

	WrNib #CMR2,#EPLC_CMR2Null			/* back to page 0 */
	WrNib #IMR,#0						/* EPLC_ROK+EPLC_TER+EPLC_TOK */
	
	movem.l    (a7)+,d3-d7/a2-a5
	rts


rtl8012_stop:
	movem.l    d3-d7/a2-a5,-(a7)
	lea.l      0x00FA0000,a3
	lea.l      Table,a2

	/* Set 'No Acception' */
	WrNib #CMR2+HNib,#HNib

	/*  disable Tx & Rx */
	WrNib #CMR1+HNib,#HNib
	
	movem.l    (a7)+,d3-d7/a2-a5
	rts


rtl8012_ready:
	movem.l    a2-a3,-(a7)
	lea.l      0x00FA0000,a3
	lea.l      Table,a2
	RdNib CMR1
	movem.l    (a7)+,a2-a3
	andi.w     #EPLC_TRA,d0
	beq.s      rtl8012_ready1
	clr.l      d0
	rts
rtl8012_ready1:
	moveq.l    #1,d0
	rts

	.include "if.i"
	
	/* rtl12 private structure */
	.offset 0
tx_tail: ds.w 1
tx_used: ds.w 1

	.text

send_block:
	movem.l    d3-d7/a2-a4,-(a7)
	lea.l      0xFFFA0000,a3
	lea.l      0xFFFB0000,a4
	lea.l      Table,a2

	move.w     d0,d7
	move.w     d0,d6
	subq.w     #1,d7

	WrNib #MODSEL+HNib,#HNib+3	/* Mode C to Write packet */
	
	/* start data transfer */
	move.l     a0,d1
	CSB_DN_CLK_UP
	nop
	nop
	nop
	nop
	movea.l    d1,a0

	move.w     #0x00FF,d0
	move.w     #0x2000,d1
send_block1:
	/* 1st byte */
	move.b     (a0)+,d2
	and.w      d0,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	/* CLK down on first byte, ip on 2nd */
	move.w     #0x2000,d2
	eor.w      d2,d1
	tst.b      0(a4,d1.w)
	dbf        d7,send_block1
	nop
	nop
	/* end of data transfer */
	P14_16_17_UP
	/* Tx jump packet command */
	WrNib #CMR1,#WRPAC

	/* if (!Q_EMPTY(pr)) */
	movea.l    if_data(a1),a0
	move.w     tx_used(a0),d0
	beq        send_block5
	move.l     (_hz_200).w,d7
	addq.l     #4,d7
send_block2:
	RdNib CMR1
	andi.w     #EPLC_TRA,d0
	bne.s      send_block3
	subq.w     #1,tx_used(a0)
	bra        send_block5
send_block3:
	RdNib ISR
	/* Clear ISR */
	WrNib #ISR,#EPLC_ROK+EPLC_TER+EPLC_TOK
	moveq.l    #EPLC_TER,d1
	and.w      d0,d1
	beq.s      send_block4
	addq.l     #1,if_collisions(a1)
send_block4:
	move.l     (_hz_200).w,d0
	cmp.l      d7,d0
	ble        send_block2
	moveq.l    #1,d0             /* transmission timed out */
	bra        send_block7
send_block5:
	addq.w     #1,tx_used(a0)
	/* Enet packets must have at least 60 bytes */
	cmpi.w     #60,d6
	bge.s      send_block6
	moveq.l    #60,d6
send_block6:
	move.w     d6,d1
	andi.w     #0xFF,d1
	WrByte #TBCR0,d1
	lsr.w      #8,d6
	WrNib #TBCR1,d6
	WrNib #CMR1,#EPLC_TRA				/* start transmission */
	moveq.l    #0,d0
send_block7:
	movem.l    (a7)+,d3-d7/a2-a4
	rts


rtl8012_active:
	movem.l    d3/a2-a3,-(a7)
	lea.l      0xFFFA0000,a3
	lea.l      Table,a2

	WrNib #CMR1+HNib,#HNib+EPLC_RxE
	REPT 40
	nop
	ENDM
	WrNib #CMR1+HNib,#HNib+EPLC_RxE+EPLC_TxE
	movem.l    (a7)+,d3/a2-a3
	rts


/* ??? unused */
	WrNib #CMR2+HNib,#HNib+EPLC_CMR2Null
	WrNib #ISR+HNib,#HNib+EPLC_RBER
	move.w     #1,x12180
	bra.s      get_paclen1


buf_empty:
	movem.l    a2-a3,-(a7)
	lea.l      0x00FA0000,a3
	lea.l      Table,a2
	RdNib CMR1
	movem.l    (a7)+,a2-a3
	andi.w     #1,d0
	rts


get_paclen:
	movem.l    d3-d7/a2-a4,-(a7)
	lea.l      0xFFFA0000,a3
	lea.l      0xFFFB0000,a4
	lea.l      Table,a2
	move.w     sr,d5
	ori.w      #0x0700,sr
get_paclen1:
	/* Rx jump packet command */
	WrNib #CMR1,#RDPAC

	/* Mode C to Read from adapter buffer */
	WrNib #MODSEL,#0
	WrNib #MODSEL+HNib,#HNib+1
	
	/* start data transfer */
	CSB_DN_CLK_UP
	nop
	nop
	nop
	nop

	moveq.l    #7,d7
	lea.l      RxBuffer,a0
	clr.l      d6
get_paclen2:
	RdBytEP
	move.b     d0,(a0)+
	dbf        d7,get_paclen2
	move.w     d5,sr

	lea.l      RxBuffer,a0
	moveq.l    #0,d7
	move.b     RxStatus(a0),d0
	beq.s      get_paclen5
	move.b     RxMSB(a0),d0
	lsl.w      #8,d0
	move.b     RxLSB(a0),d0
	move.w     d0,d7
	cmpi.w     #8,d7
	blt.s      get_paclen5
	cmpi.w     #2048,d7
	blt.s      get_paclen3
	move.w     #1536+4,d7    /* A full ethernet packet plus CRC */
get_paclen3:
	/* 4 CRC + 1 for dbf */
	subq.w     #4,d7
get_paclen4:
	move.w     d7,d0
	movem.l    (a7)+,d3-d7/a2-a4
	rts
	
get_paclen5:
	moveq.l    #0,d7
	tst.w      x12180
	beq.s      get_paclen4
	clr.w      x12180
	/* Accept all packets */
	WrNib #CMR2+HNib,#HNib+RxMode
	bra.s      get_paclen4


read_block:
	movem.l    a2-a4,-(a7)
	lea.l      0xFFFA0000,a3
	lea.l      0xFFFB0000,a4
	lea.l      Table,a2
	move.w     d0,d2
	subq.w     #1,d2
read_block1:
	RdBytEP
	move.b     d0,(a0)+
	dbf        d2,read_block1
	P14_16_17_UP
	movem.l    (a7)+,a2-a4
	rts
	


rtl8012_ramtest:
	lea.l      0xFFFA0000,a3
	lea.l      0xFFFB0000,a4
	lea.l      Table,a2

	P14_16_17_UP
	/* Mode A */
	WrNib #MODSEL,#0
	
	/* 0-450ns */
	WrNib #MODDEL,#0
	
	/* 1.disable Tx & Rx */
	WrNib #CMR1+HNib,#HNib
	
	/* 2.enter RAM test mode */
	WrNib #CMR2,#EPLC_RAMTST
	
	/* 3.disable Tx & Rx */
	WrNib #CMR1+HNib,#HNib
	
	/* 4.enable Tx & Rx */
	WrNib #CMR1+HNib,#HNib+EPLC_RxE+EPLC_TxE
	
	RdNib CMR1+HNib
	
	/* Mode C to Write test pattern */
	WrNib #MODSEL+HNib,#HNib+3
	
	CSB_DN_CLK_UP
	udelay 1

	move.w     #28-1,d7
	lea.l      pattern,a0
ramtest1:
	/* 1st byte */
	move.w     (a0)+,d1
	divs.w     d0,d0
	move.w     d1,d2
	lsr.w      #8,d2
	and.w      #0x00FF,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	/* CLK down */
	tst.b      (a4)
	/* 2nd byte */
	move.w     d1,d2
	and.w      #0x00FF,d2
	add.w      d2,d2
	move.w     0(a2,d2.w),d2
	tst.b      0(a3,d2.w)
	/* CLK up */
	move.w     #0x2000,d0
	tst.b      0(a4,d0.w)
	dbf        d7,ramtest1
	
	P14_16_17_UP
	udelay 1
	
	WrNib #CMR1,#WRPAC
	udelay 1

	/* Mode A */
	WrNib #MODSEL,#0

	/* 6.disable Tx & Rx */
	WrNib #CMR1+HNib,#HNib
	
	/* 7.enable Tx & Rx */
	WrNib #CMR1+HNib,#HNib+EPLC_RxE+EPLC_TxE
	
	RdNib CMR1+HNib
	WrNib #CMR1,#RDPAC
	
	CSB_DN_CLK_UP
	
	/* Mode C to Read test pattern */
	WrNib #MODSEL+HNib,#HNib+1

	move.w     #28-1,d7
	lea.l      verify,a0
	moveq.l    #0,d6
ramtest2:
	RdBytEP
	lsl.w      #8,d0
	move.w     d0,d6
	RdBytEP
	or.w       d0,d6
	move.w     d6,(a0)+
	dbf        d7,ramtest2
	nop
	nop
	P14_16_17_UP
	
	/* 9.disable Tx & Rx before leaving RAM test mode */
	WrNib #CMR1+HNib,#HNib
	
	/* 10.leave RAM test mode */
	WrNib #CMR2,#EPLC_CMR2Null
	
	/* Enable Rx TEST ONLY!!! */
	WrNib #CMR1+HNib,#HNib+EPLC_RxE+EPLC_TxE
	
	/* Accept physical and broadcast */
	WrNib #CMR2+HNib,#HNib+RxMode

	lea.l      verify,a3
	lea.l      pattern,a4
	cmpm.w     (a4)+,(a3)+
	beq.s      ramtest3
	subq.w     #1,a3
ramtest3:
	move.w     #20-1,d7
ramtest4:
	cmpm.w     (a4)+,(a3)+
	dbne       d7,ramtest4
	cmpi.w     #-1,d7
	beq.s      ramtest5
	clr.l      d0
	rts
ramtest5:
	moveq.l    #1,d0
	rts


	.globl NIC_make_table
NIC_make_table:
	lea.l      Table,a1
	clr.w      d0
	move.w     #255,d2
NIC_make_table1:
	moveq.l    #0,d1
	btst       #0,d0
	beq.s      NIC_make_table2
	bset       #8,d0
NIC_make_table2:
	add.w      d0,d1
	bclr       #0,d1
	move.w     d1,(a1)+
	bclr       #8,d0
	addq.w     #1,d0
	dbf        d2,NIC_make_table1
	rts

	.data
x12180:
	.dc.w 0
	.dc.w 2,0,0,0

pattern:
	.ascii "RTL8012 REALTEK""s Ethernet Pocket LAN Controller"
	.dc.w 0x5a3c,0x78f0,0xa5c3,0x870f

	.dc.w 0,0,0,0,0
	.ascii "CMR1=%d"
	.dc.b 10,0
	.ascii " TSR=%d"
	.dc.b 10,0

	.bss
verify: .ds.w 28

	.ds.b 72
