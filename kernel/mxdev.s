/*
 * MagiCNet
 * (C) 2002 Vassilis Papathanassiou
 * (C) 2021 Thorsten Otto
 *
 * Assembler part of u:\dev\socket driver
 *
 * based on
 *
 * MagiC Device Driver Development Kit
 * ===================================
 * (C) Andreas Kromke, 1994
 *
 */

     .globl socket_dev
     .xref cdecl_socket_dev

	.include "mgx_dfs.inc"
	.include "socket.inc"

    .text


**********************************************************************
*
* This is the device driver, which is called by the kernel. The
* functions write their arguments onto the stack and call the
* corresponding functions of "cdecl_socket_dev", which are declared as "cdecl".
*

socket_dev:
 .dc.l	socket_open
 .dc.l	socket_close
 .dc.l	socket_read
 .dc.l	socket_write
 .dc.l	socket_stat
 .dc.l	socket_seek
 .dc.l	socket_datime
 .dc.l	socket_ioctl
 .dc.l	socket_delete
 .dc.l	0 /* socket_getc */
 .dc.l	0 /* socket_getline */
 .dc.l	0 /* socket_putc */


**********************************************************************
*
* long socket_open( a0 = MX_DOSFD *f )
*

socket_open:
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_open,a0
 jsr		(a0)
 addq.l	#4,sp
 rts


**********************************************************************
*
* long socket_close( a0 = MX_DOSFD *f )
*

socket_close:
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_close,a0
 jsr		(a0)
 addq.l	#4,sp
 rts


**********************************************************************
*
* long socket_read( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

socket_read:
 .IFNE GNUC
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_read,a0
 jsr		(a0)
 lea		12(sp),sp
 rts
 .ELSE
	movem.l    a2-a4,-(a7)
	subq.w     #8,a7			/* make room for iovec structure */
	movea.l    a0,a2
	movea.l    fd_user1(a2),a3
	move.w     so_state(a3),d1	/* state == SS_VIRGIN? */
	bne.s      socket_read1
	moveq.l    #-32,d0 /* ENOSYS BUG: should be EINVAL */
	bra.s      socket_read2
socket_read1:
	move.l     a1,(a7)			/* iov[0].iov_base = buf */
	move.l     d0,4(a7)			/* iov[0].iov_len = count */
	clr.l      -(a7)			/* addrlen */
	clr.l      -(a7)			/* addr */
	clr.w      d2				/* flags */
	move.w     fd_mode(a2),d1	/* fd_mode & O_NDELAY */
	andi.w     #0x0100,d1
	lea.l      8(a7),a1			/* iov */
	movea.l    a3,a0			/* so */
	movea.l    so_ops(a3),a4
	movea.l    dom_recv(a4),a4
	moveq.l    #1,d0			/* niov */
	jsr        (a4)
	addq.w     #8,a7
socket_read2:
	addq.w     #8,a7
	movem.l    (a7)+,a2-a4
	rts
 .ENDC


**********************************************************************
*
* long socket_write( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

socket_write:
 .IFNE GNUC
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_write,a0
 jsr		(a0)
 lea		12(sp),sp
 rts
 .ELSE
	movem.l    a2-a4,-(a7)
	subq.w     #8,a7			/* make room for iovec structure */
	movea.l    a0,a2
	movea.l    fd_user1(a2),a3
	move.w     so_state(a3),d1	/* state == SS_VIRGIN? */
	bne.s      socket_write1
	moveq.l    #-32,d0 /* ENOSYS BUG: should be EINVAL */
	bra.s      socket_write2
socket_write1:
	move.l     a1,(a7)
	move.l     d0,4(a7)
	clr.w      -(a7)			/* addrlen */
	clr.l      -(a7)			/* addr */
	clr.w      d2				/* flags */
	move.w     fd_mode(a2),d1	/* fd_mode & O_NDELAY */
	andi.w     #0x0100,d1
	lea.l      6(a7),a1			/* iov */
	movea.l    a3,a0			/* so */
	movea.l    so_ops(a3),a4
	movea.l    dom_send(a4),a4
	moveq.l    #1,d0			/* niov */
	jsr        (a4)
	addq.w     #6,a7
socket_write2:
	addq.w     #8,a7
	movem.l    (a7)+,a2-a4
	rts
 .ENDC


**********************************************************************
*
* long socket_stat( a0 = MX_DOSFD *f, a1 = LONG *unselect,
*				 d0 = WORD rwflag, d1 = LONG apcode );
*

socket_stat:
 move.l	d1,-(sp)
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_stat,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		16(sp),sp
 .ELSE
 lea		14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* long socket_seek( a0 = MX_DOSFD *f, d0 = LONG where,
*					d1 = WORD mode )
*

socket_seek:
 .IFEQ MSHORT
 move.l	d1,-(sp)
 .ELSE
 move.w	d1,-(sp)
 .ENDC
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_seek,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG socket_datime( a0 = MX_DOSFD *f, a1 = WORD d[2],
*					d0 = WORD setflag )
*

socket_datime:
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_datime,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG socket_ioctl(a0 = MX_DOSFD *f, d0 = WORD cmd,
*					a1 = void *buf)
*

socket_ioctl:
 move.l   a1,-(sp)                 /* buf */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* cmd */
 .ELSE
 move.w   d0,-(sp)                 /* cmd */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_socket_dev+ddev_ioctl,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG socket_delete( a1 = DIR *dir )
*

socket_delete:
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_socket_dev+ddev_delete,a0
 jsr		(a0)
 addq.l	#8,sp
 rts


**********************************************************************
*
* LONG socket_getc( a0 = MX_DOSFD *f, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: This is generally a longword for CON, else a byte
*         0x0000FF1A for EOF
*

socket_getc:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_socket_dev+ddev_getc,a0
 jsr      (a0)
 .IFEQ MSHORT
 addq.l   #8,sp
 .ELSE
 addq.l   #6,sp
 .ENDC
 rts


**********************************************************************
*
* LONG socket_getline( a0 = MX_DOSFD *f, a1 = char *buf,
*					d1 = LONG size, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: Number of bytes read, or error code
*

socket_getline:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   d1,-(sp)                 /* size */
 move.l   a1,-(sp)                 /* buf */
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_socket_dev+ddev_getline,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  16(sp),sp
 .ELSE
 lea      14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG socket_putc( a0 = MX_DOSFD *f, d0 = WORD mode,
*					d1 = LONG value )
*
* mode & 0x0001:    cooked
*
* Return: Number of bytes written, 4 for a terminal
*

socket_putc:
 move.l   d1,-(sp)                 /* val */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_socket_dev+ddev_putc,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts
