*********************************************************************************
* Register definitions for the 8390 chip central to a NE2000 device		*
*										*
*	Copyright 2001 Dr. Thomas Redelberger					*
*	Use it under the terms of the GNU General Public License		*
*	(See file COPYING.TXT)							*
*										*
* Credits:									*
* Although written in 68000 assembler this source code is based on the source	*
* module 8390.h of Linux originally due to the authors ... 			*
*										*
* Tabsize 8, developed with DEVPAC assembler 2.0.				*
*********************************************************************************


**** 8390 definitions taken from Linux 8390.h ***********************************

* Generic NS8390 register definitions.

* We only use one Tx slot. Should always use two Tx slots to get back-to-back transmits.

TX_PAGES	=	6
ETHER_ADDR_LEN	=	6


* Some generic ethernet register configurations.
E8390_TX_IRQ_MASK	=	$0a		/* For register EN0_ISR */
E8390_RX_IRQ_MASK	=	$05
E8390_RXCONFIG		=	$04		/* EN0_RXCR: broadcasts:yes, multicast:no, errors:no */
E8390_RXOFF		=	$20		/* EN0_RXCR: Accept no packets */
E8390_TXCONFIG		=	$00		/* EN0_TXCR: Normal transmit mode */
E8390_TXOFF		=	$02		/* EN0_TXCR: Transmitter off */

*  Register accessed at EN_CMD, the 8390 base addr.
E8390_STOP	=	$01		/* Stop and reset the chip */
E8390_START	=	$02		/* Start the chip, clear reset */
E8390_TRANS	=	$04		/* Transmit a frame */
E8390_RREAD	=	$08		/* Remote read  DMA on */
E8390_RWRITE	=	$10		/* Remote write DMA on */
E8390_RSEND	=	$18		/* Remote send  DMA on */
E8390_NODMA	=	$20		/* Remote DMA off */
E8390_PAGE0	=	$00		/* Select page chip registers */
E8390_PAGE1	=	$40		/* using the two high-order bits */
E8390_PAGE2	=	$80		/* Page 3 is invalid. */

E8390_CMD	=	$00		/* The command register (for all pages) */
* Page 0 register offsets.
EN0_CLDALO	=	$01		/* Low byte of current local dma addr  RD */
EN0_STARTPG	=	$01		/* Starting page of ring bfr WR */
EN0_CLDAHI	=	$02		/* High byte of current local dma addr  RD */
EN0_STOPPG	=	$02		/* Ending page +1 of ring bfr WR */
EN0_BOUNDARY	=	$03		/* Boundary page of ring bfr RD WR */
EN0_TSR		=	$04		/* Transmit status reg RD */
EN0_TPSR	=	$04		/* Transmit starting page WR */
EN0_NCR		=	$05		/* Number of collision reg RD */
EN0_TCNTLO	=	$05		/* Low  byte of tx byte count WR */
EN0_FIFO	=	$06		/* FIFO RD */
EN0_TCNTHI	=	$06		/* High byte of tx byte count WR */
EN0_ISR		=	$07		/* Interrupt status reg RD WR */
EN0_CRDALO	=	$08		/* low byte of current remote dma address RD */
EN0_RSARLO	=	$08		/* Remote start address reg 0 */
EN0_CRDAHI	=	$09		/* high byte, current remote dma address RD */
EN0_RSARHI	=	$09		/* Remote start address reg 1 */
EN0_RCNTLO	=	$0a		/* Remote byte count reg WR */
EN0_RCNTHI	=	$0b		/* Remote byte count reg WR */
EN0_RSR		=	$0c		/* rx status reg RD */
EN0_RXCR	=	$0c		/* RX configuration reg WR */
EN0_TXCR	=	$0d		/* TX configuration reg WR */
EN0_COUNTER0	=	$0d		/* Rcv alignment error counter RD */
EN0_DCFG	=	$0e		/* Data configuration reg WR */
EN0_COUNTER1	=	$0e		/* Rcv CRC error counter RD */
EN0_IMR		=	$0f		/* Interrupt mask reg WR */
EN0_COUNTER2	=	$0f		/* Rcv missed frame error counter RD */

* Bits in EN0_ISR - Interrupt status register
ENISR_RX	=	0		/* Receiver, no error */
ENISR_TX	=	1		/* Transmitter, no error */
ENISR_RX_ERR	=	2		/* Receiver, with error */
ENISR_TX_ERR	=	3		/* Transmitter, with error */
ENISR_OVER	=	4		/* Receiver overwrote the ring */
ENISR_COUNTERS	=	5		/* Counters need emptying */
ENISR_RDC	=	6		/* remote dma complete */
ENISR_RESET	=	7		/* Reset completed (this does never fire an int) */
ENISR_ALL	=	$00		/* no Interrupts ($3f would be normal) */

* Bits in EN0_DCFG - Data config register
ENDCFG_WTS	=	$01		/* word transfer mode selection */

* Page 1 register offsets.
EN1_PHYS	=	$01		/* This board's physical enet addr RD WR */
EN1_CURPAG	=	$07		/* Current memory page RD WR */
EN1_MULT	=	$08		/* Multicast filter mask array (8 bytes) RD WR */

* Bits in received packet status byte and EN0_RSR
ENRSR_RXOK	=	$01		/* Received a good packet */
ENRSR_CRC	=	$02		/* CRC error */
ENRSR_FAE	=	$04		/* frame alignment error */
ENRSR_FO	=	$08		/* FIFO overrun */
ENRSR_MPA	=	$10		/* missed pkt */
ENRSR_PHY	=	$20		/* physical/multicase address */
ENRSR_DIS	=	$40		/* receiver disable. set in monitor mode */
ENRSR_DEF	=	$80		/* deferring */

* Transmitted packet status, EN0_TSR
ENTSR_PTX	=	0		/* Packet transmitted without error */
ENTSR_ND	=	1		/* The transmit wasn't deferred. */
ENTSR_COL	=	2		/* The transmit collided at least once. */
ENTSR_ABT	=	3		/* The transmit collided 16 times, and was deferred. */
ENTSR_CRS	=	4		/* The carrier sense was lost. */
ENTSR_FU	=	5		/* A "FIFO underrun" occurred during transmit. */
ENTSR_CDH	=	6		/* The collision detect "heartbeat" signal was lost. */
ENTSR_OWC	=	7		/* There was an out-of-window collision. */


******** Start of the NEx000 and clones board specific code *********************

*NE_BASE	=	 (dev->base_addr)	/* that is fixed here */
NE_CMD		=	$00
NE_DATAPORT	=	$10		/* NatSemi-defined port window offset */
NE_RESET	=	$1f		/* Issue a read to reset, a write to clear */
NE_IO_EXTENT	=	$20

NE1SM_START_PG	=	$20		/* First page of TX buffer */
	.IFNE	0
NE1SM_STOP_PG	=	$40		/* Last page +1 of RX ring (NE1000 with 8K) */
	.ELSE
NE1SM_STOP_PG	=	$60		/* Last page +1 of RX ring (NE1000 with 16K) */
	.ENDC

NESM_START_PG	=	$40		/* First page of TX buffer */
	.IFNE	WORD_TRANSFER
NESM_STOP_PG	=	$80		/* Last page +1 of RX ring */
	.ELSE
* NE2000 cards in 8bit mode seem to use only half of memory (use only 8 from 16 bits internally?)
NESM_STOP_PG	=	$60		/* Last page +1 of RX ring */
	.ENDC



*********************************************************************************
