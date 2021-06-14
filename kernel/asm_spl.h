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
 * Author: Frank Naumann <fnaumann@freemint.de>
 * Started: 2000-04-18
 * 
 * Please send suggestions, patches or bug reports to me or
 * the MiNT mailing list.
 * 
 */

#ifndef _mint_m68k_asm_spl_h
#define _mint_m68k_asm_spl_h

/*
 * Normally we'd include global.h, but there are multiple global.h files.
 * This needs cleaning up. So just define directly.
 *
 * #include "global.h"
 */

/* Called inside init.c */

#ifdef __PUREC__
static short __stop1(void) 0x4e72;
static void __stop2(short) 0x2000;
static short __lpstop1(void) 0xf800;
static short __lpstop2(short) 0x01c0;
static void __lpstop3(short) 0x2000;
static unsigned short __splhigh1(void) 0x40c0; /* move.w sr,d0 */
static unsigned short __splhigh2(unsigned short) 0x007c; /* ori.w #0x700,sr */
static unsigned short __splhigh3(unsigned short) 0x0700;
static unsigned short __splhigh4(void) 0x007c; /* ori.w #0x700,sr */
static void nop(void) 0x4e71;

/* dangerous code: modifies stack without compilers knowledge */
static unsigned short __pushsr1(void) 0x40e7; /* move.w sr,-(a7) */
#define pushsr() __splhigh3(__splhigh2(__pushsr1()))
static void popsr(void) 0x46df; /* move.w (a7)+,sr */

#define cpu_stop() __stop2(__stop1());
#define cpu_lpstop() __lpstop3(__lpstop2(__lpstop1()))
#define splhigh() __splhigh3(__splhigh2(__splhigh1()))
static void spl(unsigned short sr) 0x46c0;

#define getsr() __splhigh1()
#define setipl7() __splhigh3(__splhigh4())

#endif /* __PUREC___ */


#ifdef __GNUC__
extern int coldfire_68k_emulation;

static inline void cpu_stop(void)
{
#ifdef __mcoldfire__
	if (coldfire_68k_emulation)
	{
		/* The stop instruction is currently buggy with FireTOS */
		return;
	}
#endif

	__asm__ volatile("stop  #0x2000");
}


static inline void cpu_lpstop(void)
{
#ifndef __mcoldfire__
	/* 68060's lpstop #$2000 instruction */
	__asm__ volatile ("dc.w	0xf800,0x01c0,0x2000");
#endif
}

static inline unsigned short splhigh(void)
{
	unsigned short sr;

#ifdef __mcoldfire__
	unsigned short tempo;
#endif

	__asm__ volatile (
#ifdef __mcoldfire__
		"\tmovew   sr,%0\n"
		"\tmovew   %0,%1\n"
		"\toril    #0x0700,%1\n"
		"\tmovew   %1,sr\n"
		: "=d" (sr), "=d"(tempo)
#else
		"\tmovew   %%sr,%0\n"
		"\toriw    #0x0700,%%sr"
		: "=d" (sr)
#endif
		);

	return sr;
}

static inline void spl(unsigned short sr)
{
	__asm__ volatile ("movew   %0,sr"::"d" (sr));
}
#endif /* __GNUC__ */


#endif /* _mint_m68k_asm_spl_h */
