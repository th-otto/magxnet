/*
 * MagiCNet
 * (C) 2002 Vassilis Papathanassiou
 * (C) 2021 Thorsten Otto
 *
 * Assembler part of info devices driver
 *
 * based on
 *
 * MagiC Device Driver Development Kit
 * ===================================
 * (C) Andreas Kromke, 1994
 *
 */

     .globl dummydev
     .xref cdecl_dummydev

	.include "mgx_dfs.inc"
	.include "socket.inc"

    .text


**********************************************************************
*
* This is the device driver, which is called by the kernel. The
* functions write their arguments onto the stack and call the
* corresponding functions of "cdecl_dummydev", which are declared as "cdecl".
*

dummydev:
 .dc.l	dummydev_open
 .dc.l	dummydev_close
 .dc.l	dummydev_read
 .dc.l	dummydev_write
 .dc.l	dummydev_stat
 .dc.l	dummydev_seek
 .dc.l	dummydev_datime
 .dc.l	dummydev_ioctl
 .dc.l	dummydev_delete
 .dc.l	0 /* dummydev_getc */
 .dc.l	0 /* dummydev_getline */
 .dc.l	0 /* dummydev_putc */


**********************************************************************
*
* long dummydev_open( a0 = MX_DOSFD *f )
*

dummydev_open:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_open,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long dummydev_close( a0 = MX_DOSFD *f )
*

dummydev_close:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_close,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long dummydev_read( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

dummydev_read:
 .IFNE GNUC
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_read,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 move.l	cdecl_dummydev+ddev_read,a2 /* WTF; clobbers a2 */
 jsr (a2)
 .ENDC
 rts


**********************************************************************
*
* long dummydev_write( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

dummydev_write:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_write,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 moveq #-36,d0 /* EACCESS */
 .ENDC
 rts


**********************************************************************
*
* long dummydev_stat( a0 = MX_DOSFD *f, a1 = LONG *unselect,
*				 d0 = WORD rwflag, d1 = LONG apcode );
*

dummydev_stat:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	d1,-(sp)
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_stat,a0
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
* long dummydev_seek( a0 = MX_DOSFD *f, d0 = LONG where,
*					d1 = WORD mode )
*

dummydev_seek:
 .IFNE 0 /* FIXME just a shortcut */
 .IFEQ MSHORT
 move.l	d1,-(sp)
 .ELSE
 move.w	d1,-(sp)
 .ENDC
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_seek,a0
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
* LONG dummydev_datime( a0 = MX_DOSFD *f, a1 = WORD d[2],
*					d0 = WORD setflag )
*

dummydev_datime:
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_datime,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG dummydev_ioctl(a0 = MX_DOSFD *f, d0 = WORD cmd,
*					a1 = void *buf)
*

dummydev_ioctl:
 move.l   a1,-(sp)                 /* buf */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* cmd */
 .ELSE
 move.w   d0,-(sp)                 /* cmd */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_dummydev+ddev_ioctl,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG dummydev_delete( a1 = DIR *dir )
*

dummydev_delete:
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_dummydev+ddev_delete,a0
 jsr		(a0)
 addq.l	#8,sp
 rts


**********************************************************************
*
* LONG dummydev_getc( a0 = MX_DOSFD *f, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: This is generally a longword for CON, else a byte
*         0x0000FF1A for EOF
*

dummydev_getc:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_dummydev+ddev_getc,a0
 jsr      (a0)
 .IFEQ MSHORT
 addq.l   #8,sp
 .ELSE
 addq.l   #6,sp
 .ENDC
 rts


**********************************************************************
*
* LONG dummydev_getline( a0 = MX_DOSFD *f, a1 = char *buf,
*					d1 = LONG size, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: Number of bytes read, or error code
*

dummydev_getline:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   d1,-(sp)                 /* size */
 move.l   a1,-(sp)                 /* buf */
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_dummydev+ddev_getline,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  16(sp),sp
 .ELSE
 lea      14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG dummydev_putc( a0 = MX_DOSFD *f, d0 = WORD mode,
*					d1 = LONG value )
*
* mode & 0x0001:    cooked
*
* Return: Number of bytes written, 4 for a terminal
*

dummydev_putc:
 move.l   d1,-(sp)                 /* val */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_dummydev+ddev_putc,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts
