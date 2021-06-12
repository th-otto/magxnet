/*
 * This file belongs to FreeMiNT. It's not in the original MiNT 1.12
 * distribution. See the file CHANGES for a detailed log of changes.
 * 
 * 
 * Copyright 2000 Frank Naumann <fnaumann@freemint.de>
 * All rights reserved.
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * 
 * begin:	2000-06-28
 * last change:	2000-06-28
 * 
 * Author:	Frank Naumann <fnaumann@freemint.de>
 * 
 * Please send suggestions, patches or bug reports to me or
 * the MiNT mailing list.
 * 
 */

# ifndef _dummydev_h
# define _dummydev_h

/* FIXME: unneeded for MagiC */
struct dev_descr
{
	MX_DDEV	*driver;
	short	dinfo;
	short	flags;
	void *tty;
	long	drvsize;		/* size of DEVDRV struct */
	long	fmode;
	void	*bdevmap;
	short	bdev;
	short	reserved;
};

extern long sprintf_params[];

long	dummydev_init		(const char *name, const struct dev_descr *);

long	cdecl dummydev_open		(MX_DOSFD *f);
long	cdecl dummydev_close	(MX_DOSFD *f);
long	cdecl dummydev_read		(MX_DOSFD *f, long len, void *buf);
long	cdecl dummydev_write	(MX_DOSFD *f, long len, void *buf);
long    cdecl dummydev_stat     (MX_DOSFD *f, MAGX_UNSEL *unsel, short rwflag, long /* APPL * */ appl);
long	cdecl dummydev_lseek	(MX_DOSFD *f, long where, short whence);
long	cdecl dummydev_datime	(MX_DOSFD *f, short *, short rwflag);
long	cdecl dummydev_ioctl	(MX_DOSFD *f, short, void *);
long	cdecl dummydev_delete	(MX_DOSFD *f, MX_DOSDIR *dir);



# endif /* _dummydev_h */
