/*
 * MagiCNet
 * (C) 2002 Vassilis Papathanassiou
 * (C) 2021 Thorsten Otto
 *
 * Assembler part of u:\dev\inet driver
 *
 * based on
 *
 * MagiC Device Driver Development Kit
 * ===================================
 * (C) Andreas Kromke, 1994
 *
 */

     .globl inetdev
     .xref cdecl_inetdev

	.include "mgx_dfs.inc"
	.include "socket.inc"

    .text


**********************************************************************
*
* This is the device driver, which is called by the kernel. The
* functions write their arguments onto the stack and call the
* corresponding functions of "cdecl_dummydev", which are declared as "cdecl".
*

inetdev:
 .dc.l	inetdev_open
 .dc.l	inetdev_close
 .dc.l	inetdev_read
 .dc.l	inetdev_write
 .dc.l	0 /* inetdev_stat */
 .dc.l	inetdev_seek
 .dc.l	inetdev_datime
 .dc.l	inetdev_ioctl
 .dc.l	inetdev_delete
 .dc.l	0 /* inetdev_getc */
 .dc.l	0 /* inetdev_getline */
 .dc.l	0 /* inetdev_putc */


**********************************************************************
*
* long inetdev_open( a0 = MX_DOSFD *f )
*

inetdev_open:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_open,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long inetdev_close( a0 = MX_DOSFD *f )
*

inetdev_close:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_close,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long inetdev_read( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

inetdev_read:
 .IFNE GNUC
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_read,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 move.l	cdecl_inetdev+ddev_read,a2 /* WTF; clobbers a2 */
 jsr (a2)
 .ENDC
 rts


**********************************************************************
*
* long inetdev_write( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

inetdev_write:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_write,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 moveq #-36,d0 /* EACCESS */
 .ENDC
 rts


**********************************************************************
*
* long inetdev_stat( a0 = MX_DOSFD *f, a1 = LONG *unselect,
*				 d0 = WORD rwflag, d1 = LONG apcode );
*

inetdev_stat:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	d1,-(sp)
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_stat,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		16(sp),sp
 .ELSE
 lea		14(sp),sp
 .ENDC
 .ELSE
 moveq #1,d0
 .ENDC
 rts


**********************************************************************
*
* long inetdev_seek( a0 = MX_DOSFD *f, d0 = LONG where,
*					d1 = WORD mode )
*

inetdev_seek:
 .IFNE 0 /* FIXME just a shortcut */
 .IFEQ MSHORT
 move.l	d1,-(sp)
 .ELSE
 move.w	d1,-(sp)
 .ENDC
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_seek,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 .ELSE
 moveq #-36,d0 /* EACCESS */
 .ENDC
 rts


**********************************************************************
*
* LONG inetdev_datime( a0 = MX_DOSFD *f, a1 = WORD d[2],
*					d0 = WORD setflag )
*

inetdev_datime:
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_datime,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG inetdev_ioctl(a0 = MX_DOSFD *f, d0 = WORD cmd,
*					a1 = void *buf)
*

inetdev_ioctl:
 move.l   a1,-(sp)                 /* buf */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* cmd */
 .ELSE
 move.w   d0,-(sp)                 /* cmd */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_inetdev+ddev_ioctl,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG inetdev_delete( a1 = DIR *dir )
*

inetdev_delete:
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_inetdev+ddev_delete,a0
 jsr		(a0)
 addq.l	#8,sp
 rts


**********************************************************************
*
* LONG inetdev_getc( a0 = MX_DOSFD *f, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: This is generally a longword for CON, else a byte
*         0x0000FF1A for EOF
*

inetdev_getc:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_inetdev+ddev_getc,a0
 jsr      (a0)
 .IFEQ MSHORT
 addq.l   #8,sp
 .ELSE
 addq.l   #6,sp
 .ENDC
 rts


**********************************************************************
*
* LONG inetdev_getline( a0 = MX_DOSFD *f, a1 = char *buf,
*					d1 = LONG size, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: Number of bytes read, or error code
*

inetdev_getline:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   d1,-(sp)                 /* size */
 move.l   a1,-(sp)                 /* buf */
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_inetdev+ddev_getline,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  16(sp),sp
 .ELSE
 lea      14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG inetdev_putc( a0 = MX_DOSFD *f, d0 = WORD mode,
*					d1 = LONG value )
*
* mode & 0x0001:    cooked
*
* Return: Number of bytes written, 4 for a terminal
*

inetdev_putc:
 move.l   d1,-(sp)                 /* val */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_inetdev+ddev_putc,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts
