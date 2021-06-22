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
 * begin:	2000-08-02
 * last change:	2000-08-02
 * 
 * Author:	Frank Naumann <fnaumann@freemint.de>
 * 
 * Please send suggestions, patches or bug reports to me or
 * the MiNT mailing list.
 * 
 */

# ifndef _rtl8012_vblint_h
# define _rtl8012_vblint_h


/* old handler */
extern void (*old_vbl_int)(void) GNU_ASM_NAME("old_vbl_int");

/* interrupt wrapper routine */
void vbl_interrupt (void) GNU_ASM_NAME("vbl_interrupt");

/* C handler */
void rtl8012_int (void) GNU_ASM_NAME("rtl8012_int");


# endif /* _rtl8012_vblint_h */
