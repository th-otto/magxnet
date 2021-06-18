/*
 * MagiCNet
 * (C) 2002 Vassilis Papathanassiou
 * (C) 2021 Thorsten Otto
 *
 * Assembler part of BPF driver
 *
 * based on
 *
 * MagiC Device Driver Development Kit
 * ===================================
 * (C) Andreas Kromke, 1994
 *
 */

     .globl bpf_dev
     .xref cdecl_bpf_dev

	.include "mgx_dfs.inc"
	.include "socket.inc"

    .text


**********************************************************************
*
* This is the device driver, which is called by the kernel. The
* functions write their arguments onto the stack and call the
* corresponding functions of "cdecl_dummydev", which are declared as "cdecl".
*

bpf_dev:
 .dc.l	bpf_dev_open
 .dc.l	bpf_dev_close
 .dc.l	bpf_dev_read
 .dc.l	bpf_dev_write
 .dc.l	0
 .dc.l	bpf_dev_seek
 .dc.l	bpf_dev_datime
 .dc.l	bpf_dev_ioctl
 .dc.l	bpf_dev_delete
 .dc.l	0 /* bpf_dev_getc */
 .dc.l	0 /* bpf_dev_getline */
 .dc.l	0 /* bpf_dev_putc */


**********************************************************************
*
* long bpf_dev_open( a0 = MX_DOSFD *f )
*

bpf_dev_open:
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_open,a0
 jsr		(a0)
 addq.l	#4,sp
 rts


**********************************************************************
*
* long bpf_dev_close( a0 = MX_DOSFD *f )
*

bpf_dev_close:
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_close,a0
 jsr		(a0)
 addq.l	#4,sp
 rts


**********************************************************************
*
* long bpf_dev_read( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

bpf_dev_read:
 .IFNE GNUC
/* BUG: arguments swapped */
 move.l	d0,-(sp)
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_read,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 move.l	cdecl_bpf_dev+ddev_read,a2 /* WTF; clobbers a2 */
 jsr (a2)
 .ENDC
 rts


**********************************************************************
*
* long bpf_dev_write( a0 = MX_DOSFD *f, a1 = char *buf,
*					d0 = LONG count )
*

bpf_dev_write:
 .IFNE GNUC
/* BUG: arguments swapped */
 move.l	d0,-(sp)
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_write,a0
 jsr		(a0)
 lea		12(sp),sp
 .ELSE
 move.l	cdecl_bpf_dev+ddev_write,a2 /* WTF; clobbers a2 */
 jsr (a2)
 .ENDC
 rts


**********************************************************************
*
* long bpf_dev_stat( a0 = MX_DOSFD *f, a1 = LONG *unselect,
*				 d0 = WORD rwflag, d1 = LONG apcode );
*

bpf_dev_stat:
 move.l	d1,-(sp)
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_stat,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		16(sp),sp
 .ELSE
 lea		14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* long bpf_dev_seek( a0 = MX_DOSFD *f, d0 = LONG where,
*					d1 = WORD mode )
*

bpf_dev_seek:
 .IFEQ MSHORT
 move.l	d1,-(sp)
 .ELSE
 move.w	d1,-(sp)
 .ENDC
 move.l	d0,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_seek,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG bpf_dev_datime( a0 = MX_DOSFD *f, a1 = WORD d[2],
*					d0 = WORD setflag )
*

bpf_dev_datime:
 .IFEQ MSHORT
 move.l	d0,-(sp)
 .ELSE
 move.w	d0,-(sp)
 .ENDC
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_datime,a0
 jsr		(a0)
 .IFEQ MSHORT
 lea		12(sp),sp
 .ELSE
 lea		10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG bpf_dev_ioctl(a0 = MX_DOSFD *f, d0 = WORD cmd,
*					a1 = void *buf)
*

bpf_dev_ioctl:
 move.l   a1,-(sp)                 /* buf */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* cmd */
 .ELSE
 move.w   d0,-(sp)                 /* cmd */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_bpf_dev+ddev_ioctl,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG bpf_dev_delete( a1 = DIR *dir )
*

bpf_dev_delete:
 move.l	a1,-(sp)
 move.l	a0,-(sp)
 move.l	cdecl_bpf_dev+ddev_delete,a0
 jsr		(a0)
 addq.l	#8,sp
 rts


**********************************************************************
*
* LONG bpf_dev_getc( a0 = MX_DOSFD *f, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: This is generally a longword for CON, else a byte
*         0x0000FF1A for EOF
*

bpf_dev_getc:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_bpf_dev+ddev_getc,a0
 jsr      (a0)
 .IFEQ MSHORT
 addq.l   #8,sp
 .ELSE
 addq.l   #6,sp
 .ENDC
 rts


**********************************************************************
*
* LONG bpf_dev_getline( a0 = MX_DOSFD *f, a1 = char *buf,
*					d1 = LONG size, d0 = WORD mode )
*
* mode & 0x0001:    cooked
* mode & 0x0002:    echo mode
*
* Return: Number of bytes read, or error code
*

bpf_dev_getline:
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   d1,-(sp)                 /* size */
 move.l   a1,-(sp)                 /* buf */
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_bpf_dev+ddev_getline,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  16(sp),sp
 .ELSE
 lea      14(sp),sp
 .ENDC
 rts


**********************************************************************
*
* LONG bpf_dev_putc( a0 = MX_DOSFD *f, d0 = WORD mode,
*					d1 = LONG value )
*
* mode & 0x0001:    cooked
*
* Return: Number of bytes written, 4 for a terminal
*

bpf_dev_putc:
 move.l   d1,-(sp)                 /* val */
 .IFEQ MSHORT
 move.l   d0,-(sp)                 /* mode */
 .ELSE
 move.w   d0,-(sp)                 /* mode */
 .ENDC
 move.l   a0,-(sp)                 /* MX_DOSFD */
 move.l	cdecl_bpf_dev+ddev_putc,a0
 jsr      (a0)
 .IFEQ MSHORT
 lea	  12(sp),sp
 .ELSE
 lea      10(sp),sp
 .ENDC
 rts
