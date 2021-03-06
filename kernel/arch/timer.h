/*
 * This file belongs to FreeMiNT. It's not in the original MiNT 1.12
 * distribution.
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
 * Author: Frank Naumann <fnaumann@freemint.de>
 * Started: 2000-07-17
 * 
 * please send suggestions, patches or bug reports to me or
 * the MiNT mailing list
 * 
 */

# ifndef _m68k_timer_h
# define _m68k_timer_h


# define _hz_200		((long *) 0x4baL)
# define HZ			200L

extern volatile long *hz_200;
# define jiffies    		(*hz_200)

unsigned long get_hz_200(void);
void delay_seconds(unsigned short seconds);

# endif /* _m68k_timer_h */
