/*
 * MagiCNet
 * (C) 2002 Vassilis Papathanassiou
 * (C) 2021 Thorsten Otto
 *
 * Assembler part of u:\dev\masquerade driver
 *
 * based on
 *
 * MagiC Device Driver Development Kit
 * ===================================
 * (C) Andreas Kromke, 1994
 *
 */

     .globl masqdev
     .xref cdecl_masqdev

	.include "mgx_dfs.inc"
	.include "socket.inc"

    .text


**********************************************************************
*
* This is the device driver, which is called by the kernel. The
* functions write their arguments onto the stack and call the
* corresponding functions of "cdecl_dummydev", which are declared as "cdecl".
*

masqdev:
 .dc.l	masqdev_open
 .dc.l	masqdev_close
 .dc.l	masqdev_read
 .dc.l	masqdev_write
 .dc.l	0 /* masqdev_stat */
 .dc.l	masqdev_seek
 .dc.l	masqdev_datime
 .dc.l	masqdev_ioctl
 .dc.l	masqdev_delete
 .dc.l	0 /* masqdev_getc */
 .dc.l	0 /* masqdev_getline */
 .dc.l	0 /* masqdev_putc */


**********************************************************************
*
* long masqdev_open( a0 = MX_DOSFD *f )
*

masqdev_open:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_open,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long masqdev_close( a0 = MX_DOSFD *f )
*

masqdev_close:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_close,a0
 jsr		(a0)
 addq.l	#4,sp
 .ELSE
 moveq #0,d0
 .ENDC
 rts


**********************************************************************
*
* long masqdev_read( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

masqdev_read:
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_read,a0
 jsr		(a0)
 lea		12(sp),sp
 rts


**********************************************************************
*
* long masqdev_write( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

masqdev_write:
 move.l	a1,-(sp)
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_write,a0
 jsr		(a0)
 lea		12(sp),sp
 rts


**********************************************************************
*
* long masqdev_stat( a0 = MX_DOSFD *f, a1 = LONG *unselect,
*				 d0 = WORD rwflag, d1 = LONG apcode );
*

masqdev_stat:
 .IFNE 0 /* FIXME just a shortcut */
 move.l	d1,-(sp)
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_stat,a0
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
* long masqdev_seek( a0 = MX_DOSFD *f, d0 = LONG where,
*					d1 = WORD mode )
*

masqdev_seek:
 .IFNE 0 /* FIXME just a shortcut */
 .IFEQ MSHORT
 move.l	d1,-(sp)
 .ELSE
 move.w	d1,-(sp)
 .ENDC
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_seek,a0
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
* LONG masqdev_datime( a0 = MX_DOSFD *f, a1 = WORD d[2],
*					d0 = WORD setflag )
*

masqdev_datime:
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_datime,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG masqdev_ioctl(a0 = MX_DOSFD *f, d0 = WORD cmd,
*					a1 = void *buf)
*

masqdev_ioctl:
 move.l   a1,-(sp)                 /* buf */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* cmd */
 .ELSE
 move.w   d0,-(sp)                 /* cmd */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_masqdev+ddev_ioctl,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG masqdev_delete( a1 = DIR *dir )
*

masqdev_delete:
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_masqdev+ddev_delete,a0
 jsr		(a0)
 addq.l	#8,sp
 rts


**********************************************************************
*
* LONG masqdev_getc( a0 = MX_DOSFD *f, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: This is generally a longword for CON, else a byte
*         0x0000FF1A for EOF
*

masqdev_getc:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_masqdev+ddev_getc,a0
 jsr      (a0)
 .IFEQ MSHORT
 addq.l   #8,sp
 .ELSE
 addq.l   #6,sp
 .ENDC
 rts


**********************************************************************
*
* LONG masqdev_getline( a0 = MX_DOSFD *f, a1 = char *buf,
*					d1 = LONG size, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: Number of bytes read, or error code
*

masqdev_getline:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   d1,-(sp)                 /* size */
 move.l   a1,-(sp)                 /* buf */
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_masqdev+ddev_getline,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  16(sp),sp
 .ELSE
 lea      14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG masqdev_putc( a0 = MX_DOSFD *f, d0 = WORD mode,
*					d1 = LONG value )
*
* mode & 0x0001:    cooked
*
* Return: Number of bytes written, 4 for a terminal
*

masqdev_putc:
 move.l   d1,-(sp)                 /* val */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_masqdev+ddev_putc,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts
