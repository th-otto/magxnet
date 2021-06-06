/* More subroutines needed by GCC output code on some machines.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1989, 1992, 1993, 1994 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#ifndef atarist

/* It is incorrect to include config.h here, because this file is being
   compiled for the target, and hence definitions concerning only the host
   do not apply.  */

#include "tconfig.h"
#include "machmode.h"
#ifndef L_trampoline
#include <stddef.h>
#endif

/* Don't use `fancy_abort' here even if config.h says to use it.  */
#ifdef abort
#undef abort
#endif

#else

/* For the atari, include the relevant parts of config/m68k.h directly. */

#ifndef __mc68000__
#define __mc68000__ 1
#ifdef __M68020__
#define __mc68020__ 1
#endif
#endif

#if __GNUC__ < 2
#error This file is for GCC version 2 only.
#endif

#ifndef L_trampoline
#include <stddef.h>
#endif

#if defined(XFLOAT_ENABLE) && (__GNUC__ > 2 || __GNUC_MINOR__ >= 4)
/* Define for XFmode extended real floating point support.  */
#define LONG_DOUBLE_TYPE_SIZE 96
#else
/* Don't try using XFmode.  */
#define LONG_DOUBLE_TYPE_SIZE 64
#endif

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.
   This is true for 68020 insns such as bfins and bfexts.  */
#define BITS_BIG_ENDIAN 1

/* Define this if most significant word of a multiword number is the lowest
   numbered.  */
/* For 68000 we can decide arbitrarily
   since there are no machine instructions for them.
   So let's be consistent.  */
#define WORDS_BIG_ENDIAN 1

/* number of bits in an addressable storage unit */
#define BITS_PER_UNIT 8

/* Width in bits of a "word", which is the contents of a machine register.
   Note that this is not necessarily the width of data type `int';
   if using 16-bit ints on a 68000, this would still be 32.
   But on a machine with 16-bit registers, this would be 16.  */
#define BITS_PER_WORD 32

/* This is the library routine that is used
   to transfer control from the trampoline
   to the actual nested function.  */

/* A colon is used with no explicit operands
   to cause the template string to be scanned for %-constructs.  */
/* The function name __transfer_from_trampoline is not actually used.
   The function definition just permits use of "asm with operands"
   (though the operand list is empty).  */
#define TRANSFER_FROM_TRAMPOLINE				\
void								\
__transfer_from_trampoline ()					\
{								\
  register char *a0 asm ("%a0");				\
  asm (".globl ___trampoline");					\
  asm ("___trampoline:");					\
  asm volatile ("move%.l %0,%@" : : "m" (a0[22]));		\
  asm volatile ("move%.l %1,%0" : "=a" (a0) : "m" (a0[18]));	\
  asm ("rts":);							\
}

#endif /* atarist */

/* In the first part of this file, we are interfacing to calls generated
   by the compiler itself.  These calls pass values into these routines
   which have very specific modes (rather than very specific types), and
   these compiler-generated calls also expect any return values to have
   very specific modes (rather than very specific types).  Thus, we need
   to avoid using regular C language type names in this part of the file
   because the sizes for those types can be configured to be anything.
   Instead we use the following special type names.  */

typedef unsigned int UQItype __attribute__((mode(QI)));
typedef int SItype __attribute__((mode(SI)));
typedef unsigned int USItype __attribute__((mode(SI)));
typedef int DItype __attribute__((mode(DI)));
typedef unsigned int UDItype __attribute__((mode(DI)));
typedef float SFtype __attribute__((mode(SF)));
typedef float DFtype __attribute__((mode(DF)));

#if LONG_DOUBLE_TYPE_SIZE == 96
typedef float XFtype __attribute__((mode(XF)));
#endif
#if LONG_DOUBLE_TYPE_SIZE == 128
typedef float TFtype __attribute__((mode(TF)));
#endif

#if BITS_PER_WORD==16
typedef int word_type __attribute__((mode(HI)));
#endif
#if BITS_PER_WORD==32
typedef int word_type __attribute__((mode(SI)));
#endif
#if BITS_PER_WORD==64
typedef int word_type __attribute__((mode(DI)));
#endif

/* Make sure that we don't accidentally use any normal C language built-in
   type names in the first part of this file.  Instead we want to use *only*
   the type names defined above.  The following macro definitions insure
   that if we *do* accidentally use some normal C language built-in type name,
   we will get a syntax error.  */

#define char bogus_type
#define short bogus_type
#define int bogus_type
#define long bogus_type
#define unsigned bogus_type
#define float bogus_type
#define double bogus_type

#define SI_TYPE_SIZE (sizeof (SItype) * BITS_PER_UNIT)

/* DIstructs are pairs of SItype values in the order determined by
   WORDS_BIG_ENDIAN.  */

#if WORDS_BIG_ENDIAN
struct DIstruct
{
	SItype high,
	 low;
};
#else
struct DIstruct
{
	SItype low,
	 high;
};
#endif

/* We need this union to unpack/pack DImode values, since we don't have
   any arithmetic yet.  Incoming DImode parameters are stored into the
   `ll' field, and the unpacked result is read from the struct `s'.  */

typedef union
{
	struct DIstruct s;
	DItype ll;
} DIunion;

#if defined (L_udivmoddi4) || defined (L_muldi3) || defined (L_udiv_w_sdiv)

#include "longlong.h"

#endif /* udiv or mul */

extern DItype __fixunssfdi(SFtype a);
extern DItype __fixunsdfdi(DFtype a);

#if LONG_DOUBLE_TYPE_SIZE == 96
extern DItype __fixunsxfdi(XFtype a);
#endif
#if LONG_DOUBLE_TYPE_SIZE == 128
extern DItype __fixunstfdi(TFtype a);
#endif

#if defined (L_negdi2) || defined (L_divdi3) || defined (L_moddi3)
#if defined (L_divdi3) || defined (L_moddi3)
static inline
#endif
 DItype __negdi2(u)
DItype u;
{
	DIunion w;
	DIunion uu;

	uu.ll = u;

	w.s.low = -uu.s.low;
	w.s.high = -uu.s.high - ((USItype) w.s.low > 0);

	return w.ll;
}
#endif

#ifdef L_lshldi3
DItype __lshldi3(u, b)
DItype u;
SItype b;
{
	DIunion w;
	SItype bm;
	DIunion uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.low = 0;
		w.s.high = (USItype) uu.s.low << -bm;
	} else
	{
		USItype carries = (USItype) uu.s.low >> bm;

		w.s.low = (USItype) uu.s.low << b;
		w.s.high = ((USItype) uu.s.high << b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_lshrdi3
DItype __lshrdi3(u, b)
DItype u;
SItype b;
{
	DIunion w;
	SItype bm;
	DIunion uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.high = 0;
		w.s.low = (USItype) uu.s.high >> -bm;
	} else
	{
		USItype carries = (USItype) uu.s.high << bm;

		w.s.high = (USItype) uu.s.high >> b;
		w.s.low = ((USItype) uu.s.low >> b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_ashldi3
DItype __ashldi3(u, b)
DItype u;
SItype b;
{
	DIunion w;
	SItype bm;
	DIunion uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.low = 0;
		w.s.high = (USItype) uu.s.low << -bm;
	} else
	{
		USItype carries = (USItype) uu.s.low >> bm;

		w.s.low = (USItype) uu.s.low << b;
		w.s.high = ((USItype) uu.s.high << b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_ashrdi3
DItype __ashrdi3(u, b)
DItype u;
SItype b;
{
	DIunion w;
	SItype bm;
	DIunion uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		/* w.s.high = 1..1 or 0..0 */
		w.s.high = uu.s.high >> (sizeof(SItype) * BITS_PER_UNIT - 1);
		w.s.low = uu.s.high >> -bm;
	} else
	{
		USItype carries = (USItype) uu.s.high << bm;

		w.s.high = uu.s.high >> b;
		w.s.low = ((USItype) uu.s.low >> b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_ffsdi2
DItype __ffsdi2(u)
DItype u;
{
	DIunion uu,
	 w;

	uu.ll = u;
	w.s.high = 0;
	w.s.low = ffs(uu.s.low);
	if (w.s.low != 0)
		return w.ll;
	w.s.low = ffs(uu.s.high);
	if (w.s.low != 0)
	{
		w.s.low += BITS_PER_UNIT * sizeof(SItype);
		return w.ll;
	}
	return w.ll;
}
#endif

#ifdef L_muldi3
DItype __muldi3(u, v)
DItype u,
	v;
{
	DIunion w;
	DIunion uu,
	 vv;

	uu.ll = u, vv.ll = v;

	w.ll = __umulsidi3(uu.s.low, vv.s.low);
	w.s.high += ((USItype) uu.s.low * (USItype) vv.s.high + (USItype) uu.s.high * (USItype) vv.s.low);

	return w.ll;
}
#endif

#ifdef L_udiv_w_sdiv
USItype __udiv_w_sdiv(rp, a1, a0, d)
USItype *rp,
	a1,
	a0,
	d;
{
	USItype q,
	 r;
	USItype c0,
	 c1,
	 b1;

	if ((SItype) d >= 0)
	{
		if (a1 < d - a1 - (a0 >> (SI_TYPE_SIZE - 1)))
		{
			/* dividend, divisor, and quotient are nonnegative */
			sdiv_qrnnd(q, r, a1, a0, d);
		} else
		{
			/* Compute c1*2^32 + c0 = a1*2^32 + a0 - 2^31*d */
			sub_ddmmss(c1, c0, a1, a0, d >> 1, d << (SI_TYPE_SIZE - 1));
			/* Divide (c1*2^32 + c0) by d */
			sdiv_qrnnd(q, r, c1, c0, d);
			/* Add 2^31 to quotient */
			q += (USItype) 1 << (SI_TYPE_SIZE - 1);
		}
	} else
	{
		b1 = d >> 1;					/* d/2, between 2^30 and 2^31 - 1 */
		c1 = a1 >> 1;					/* A/2 */
		c0 = (a1 << (SI_TYPE_SIZE - 1)) + (a0 >> 1);

		if (a1 < b1)					/* A < 2^32*b1, so A/2 < 2^31*b1 */
		{
			sdiv_qrnnd(q, r, c1, c0, b1);	/* (A/2) / (d/2) */

			r = 2 * r + (a0 & 1);		/* Remainder from A/(2*b1) */
			if ((d & 1) != 0)
			{
				if (r >= q)
					r = r - q;
				else if (q - r <= d)
				{
					r = r - q + d;
					q--;
				} else
				{
					r = r - q + 2 * d;
					q -= 2;
				}
			}
		} else if (c1 < b1)				/* So 2^31 <= (A/2)/b1 < 2^32 */
		{
			c1 = (b1 - 1) - c1;
			c0 = ~c0;					/* logical NOT */

			sdiv_qrnnd(q, r, c1, c0, b1);	/* (A/2) / (d/2) */

			q = ~q;						/* (A/2)/b1 */
			r = (b1 - 1) - r;

			r = 2 * r + (a0 & 1);		/* A/(2*b1) */

			if ((d & 1) != 0)
			{
				if (r >= q)
					r = r - q;
				else if (q - r <= d)
				{
					r = r - q + d;
					q--;
				} else
				{
					r = r - q + 2 * d;
					q -= 2;
				}
			}
		} else							/* Implies c1 = b1 */
		{								/* Hence a1 = d - 1 = 2*b1 - 1 */
			if (a0 >= -d)
			{
				q = -1;
				r = a0 + d;
			} else
			{
				q = -2;
				r = a0 + 2 * d;
			}
		}
	}

	*rp = r;
	return q;
}
#endif

#ifdef L_udivmoddi4
static const UQItype __clz_tab[] = {
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

UDItype __udivmoddi4(n, d, rp)
UDItype n,
	d;
UDItype *rp;
{
	DIunion ww;
	DIunion nn,
	 dd;
	DIunion rr;
	USItype d0,
	 d1,
	 n0,
	 n1,
	 n2;
	USItype q0,
	 q1;
	USItype b,
	 bm;

	nn.ll = n;
	dd.ll = d;

	d0 = dd.s.low;
	d1 = dd.s.high;
	n0 = nn.s.low;
	n1 = nn.s.high;

#if !UDIV_NEEDS_NORMALIZATION
	if (d1 == 0)
	{
		if (d0 > n1)
		{
			/* 0q = nn / 0D */

			udiv_qrnnd(q0, n0, n1, n0, d0);
			q1 = 0;

			/* Remainder in n0.  */
		} else
		{
			/* qq = NN / 0d */

			if (d0 == 0)
				d0 = 1 / d0;			/* Divide intentionally by zero.  */

			udiv_qrnnd(q1, n1, 0, n1, d0);
			udiv_qrnnd(q0, n0, n1, n0, d0);

			/* Remainder in n0.  */
		}

		if (rp != 0)
		{
			rr.s.low = n0;
			rr.s.high = 0;
			*rp = rr.ll;
		}
	}

#else /* UDIV_NEEDS_NORMALIZATION */

	if (d1 == 0)
	{
		if (d0 > n1)
		{
			/* 0q = nn / 0D */

			count_leading_zeros(bm, d0);

			if (bm != 0)
			{
				/* Normalize, i.e. make the most significant bit of the
				   denominator set.  */

				d0 = d0 << bm;
				n1 = (n1 << bm) | (n0 >> (SI_TYPE_SIZE - bm));
				n0 = n0 << bm;
			}

			udiv_qrnnd(q0, n0, n1, n0, d0);
			q1 = 0;

			/* Remainder in n0 >> bm.  */
		} else
		{
			/* qq = NN / 0d */

			if (d0 == 0)
				d0 = 1 / d0;			/* Divide intentionally by zero.  */

			count_leading_zeros(bm, d0);

			if (bm == 0)
			{
				/* From (n1 >= d0) /\ (the most significant bit of d0 is set),
				   conclude (the most significant bit of n1 is set) /\ (the
				   leading quotient digit q1 = 1).

				   This special case is necessary, not an optimization.
				   (Shifts counts of SI_TYPE_SIZE are undefined.)  */

				n1 -= d0;
				q1 = 1;
			} else
			{
				/* Normalize.  */

				b = SI_TYPE_SIZE - bm;

				d0 = d0 << bm;
				n2 = n1 >> b;
				n1 = (n1 << bm) | (n0 >> b);
				n0 = n0 << bm;

				udiv_qrnnd(q1, n1, n2, n1, d0);
			}

			/* n1 != d0... */

			udiv_qrnnd(q0, n0, n1, n0, d0);

			/* Remainder in n0 >> bm.  */
		}

		if (rp != 0)
		{
			rr.s.low = n0 >> bm;
			rr.s.high = 0;
			*rp = rr.ll;
		}
	}
#endif /* UDIV_NEEDS_NORMALIZATION */

	else
	{
		if (d1 > n1)
		{
			/* 00 = nn / DD */

			q0 = 0;
			q1 = 0;

			/* Remainder in n1n0.  */
			if (rp != 0)
			{
				rr.s.low = n0;
				rr.s.high = n1;
				*rp = rr.ll;
			}
		} else
		{
			/* 0q = NN / dd */

			count_leading_zeros(bm, d1);
			if (bm == 0)
			{
				/* From (n1 >= d1) /\ (the most significant bit of d1 is set),
				   conclude (the most significant bit of n1 is set) /\ (the
				   quotient digit q0 = 0 or 1).

				   This special case is necessary, not an optimization.  */

				/* The condition on the next line takes advantage of that
				   n1 >= d1 (true due to program flow).  */
				if (n1 > d1 || n0 >= d0)
				{
					q0 = 1;
					sub_ddmmss(n1, n0, n1, n0, d1, d0);
				} else
					q0 = 0;

				q1 = 0;

				if (rp != 0)
				{
					rr.s.low = n0;
					rr.s.high = n1;
					*rp = rr.ll;
				}
			} else
			{
				USItype m1,
				 m0;

				/* Normalize.  */

				b = SI_TYPE_SIZE - bm;

				d1 = (d1 << bm) | (d0 >> b);
				d0 = d0 << bm;
				n2 = n1 >> b;
				n1 = (n1 << bm) | (n0 >> b);
				n0 = n0 << bm;

				udiv_qrnnd(q0, n1, n2, n1, d1);
				umul_ppmm(m1, m0, q0, d0);

				if (m1 > n1 || (m1 == n1 && m0 > n0))
				{
					q0--;
					sub_ddmmss(m1, m0, m1, m0, d1, d0);
				}

				q1 = 0;

				/* Remainder in (n1n0 - m1m0) >> bm.  */
				if (rp != 0)
				{
					sub_ddmmss(n1, n0, n1, n0, m1, m0);
					rr.s.low = (n1 << b) | (n0 >> bm);
					rr.s.high = n1 >> bm;
					*rp = rr.ll;
				}
			}
		}
	}

	ww.s.low = q0;
	ww.s.high = q1;
	return ww.ll;
}
#endif

#ifdef L_divdi3
UDItype __udivmoddi4();

DItype __divdi3(u, v)
DItype u,
	v;
{
	SItype c = 0;
	DIunion uu,
	 vv;
	DItype w;

	uu.ll = u;
	vv.ll = v;

	if (uu.s.high < 0)
		c = ~c, uu.ll = __negdi2(uu.ll);
	if (vv.s.high < 0)
		c = ~c, vv.ll = __negdi2(vv.ll);

	w = __udivmoddi4(uu.ll, vv.ll, (UDItype *) 0);
	if (c)
		w = __negdi2(w);

	return w;
}
#endif

#ifdef L_moddi3
UDItype __udivmoddi4();

DItype __moddi3(u, v)
DItype u,
	v;
{
	SItype c = 0;
	DIunion uu,
	 vv;
	DItype w;

	uu.ll = u;
	vv.ll = v;

	if (uu.s.high < 0)
		c = ~c, uu.ll = __negdi2(uu.ll);
	if (vv.s.high < 0)
		vv.ll = __negdi2(vv.ll);

	(void) __udivmoddi4(uu.ll, vv.ll, &w);
	if (c)
		w = __negdi2(w);

	return w;
}
#endif

#ifdef L_umoddi3
UDItype __udivmoddi4();

UDItype __umoddi3(u, v)
UDItype u,
	v;
{
	UDItype w;

	(void) __udivmoddi4(u, v, &w);

	return w;
}
#endif

#ifdef L_udivdi3
UDItype __udivmoddi4();

UDItype __udivdi3(n, d)
UDItype n,
	d;
{
	return __udivmoddi4(n, d, (UDItype *) 0);
}
#endif

#ifdef L_cmpdi2
word_type __cmpdi2(a, b)
DItype a,
	b;
{
	DIunion au,
	 bu;

	au.ll = a, bu.ll = b;

	if (au.s.high < bu.s.high)
		return 0;
	else if (au.s.high > bu.s.high)
		return 2;
	if ((USItype) au.s.low < (USItype) bu.s.low)
		return 0;
	else if ((USItype) au.s.low > (USItype) bu.s.low)
		return 2;
	return 1;
}
#endif

#ifdef L_ucmpdi2
word_type __ucmpdi2(a, b)
DItype a,
	b;
{
	DIunion au,
	 bu;

	au.ll = a, bu.ll = b;

	if ((USItype) au.s.high < (USItype) bu.s.high)
		return 0;
	else if ((USItype) au.s.high > (USItype) bu.s.high)
		return 2;
	if ((USItype) au.s.low < (USItype) bu.s.low)
		return 0;
	else if ((USItype) au.s.low > (USItype) bu.s.low)
		return 2;
	return 1;
}
#endif

#if defined(L_fixunstfdi) && (LONG_DOUBLE_TYPE_SIZE == 128)
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

DItype __fixunstfdi(a)
TFtype a;
{
	TFtype b;
	UDItype v;

	if (a < 0)
		return 0;

	/* Compute high word of result, as a flonum.  */
	b = (a / HIGH_WORD_COEFF);
	/* Convert that to fixed (but not to DItype!),
	   and shift it into the high word.  */
	v = (USItype) b;
	v <<= WORD_SIZE;
	/* Remove high part from the TFtype, leaving the low part as flonum.  */
	a -= (TFtype) v;
	/* Convert that to fixed (but not to DItype!) and add it in.
	   Sometimes A comes out negative.  This is significant, since
	   A has more bits than a long int does.  */
	if (a < 0)
		v -= (USItype) (-a);
	else
		v += (USItype) a;
	return v;
}
#endif

#if defined(L_fixtfdi) && (LONG_DOUBLE_TYPE_SIZE == 128)
DItype __fixtfdi(a)
TFtype a;
{
	if (a < 0)
		return -__fixunstfdi(-a);
	return __fixunstfdi(a);
}
#endif

#if defined(L_fixunsxfdi) && (LONG_DOUBLE_TYPE_SIZE == 96)
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

DItype __fixunsxfdi(a)
XFtype a;
{
	XFtype b;
	UDItype v;

	if (a < 0)
		return 0;

	/* Compute high word of result, as a flonum.  */
	b = (a / HIGH_WORD_COEFF);
	/* Convert that to fixed (but not to DItype!),
	   and shift it into the high word.  */
	v = (USItype) b;
	v <<= WORD_SIZE;
	/* Remove high part from the XFtype, leaving the low part as flonum.  */
	a -= (XFtype) v;
	/* Convert that to fixed (but not to DItype!) and add it in.
	   Sometimes A comes out negative.  This is significant, since
	   A has more bits than a long int does.  */
	if (a < 0)
		v -= (USItype) (-a);
	else
		v += (USItype) a;
	return v;
}
#endif

#if defined(L_fixxfdi) && (LONG_DOUBLE_TYPE_SIZE == 96)
DItype __fixxfdi(a)
XFtype a;
{
	if (a < 0)
		return -__fixunsxfdi(-a);
	return __fixunsxfdi(a);
}
#endif

#ifdef L_fixunsdfdi
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

DItype __fixunsdfdi(a)
DFtype a;
{
	DFtype b;
	UDItype v;

	if (a < 0)
		return 0;

	/* Compute high word of result, as a flonum.  */
	b = (a / HIGH_WORD_COEFF);
	/* Convert that to fixed (but not to DItype!),
	   and shift it into the high word.  */
	v = (USItype) b;
	v <<= WORD_SIZE;
	/* Remove high part from the DFtype, leaving the low part as flonum.  */
	a -= (DFtype) v;
	/* Convert that to fixed (but not to DItype!) and add it in.
	   Sometimes A comes out negative.  This is significant, since
	   A has more bits than a long int does.  */
	if (a < 0)
		v -= (USItype) (-a);
	else
		v += (USItype) a;
	return v;
}
#endif

#ifdef L_fixdfdi
DItype __fixdfdi(a)
DFtype a;
{
	if (a < 0)
		return -__fixunsdfdi(-a);
	return __fixunsdfdi(a);
}
#endif

#ifdef L_fixunssfdi
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

DItype __fixunssfdi(SFtype original_a)
{
	/* Convert the SFtype to a DFtype, because that is surely not going
	   to lose any bits.  Some day someone else can write a faster version
	   that avoids converting to DFtype, and verify it really works right.  */
	DFtype a = original_a;
	DFtype b;
	UDItype v;

	if (a < 0)
		return 0;

	/* Compute high word of result, as a flonum.  */
	b = (a / HIGH_WORD_COEFF);
	/* Convert that to fixed (but not to DItype!),
	   and shift it into the high word.  */
	v = (USItype) b;
	v <<= WORD_SIZE;
	/* Remove high part from the DFtype, leaving the low part as flonum.  */
	a -= (DFtype) v;
	/* Convert that to fixed (but not to DItype!) and add it in.
	   Sometimes A comes out negative.  This is significant, since
	   A has more bits than a long int does.  */
	if (a < 0)
		v -= (USItype) (-a);
	else
		v += (USItype) a;
	return v;
}
#endif

#ifdef L_fixsfdi
DItype __fixsfdi(SFtype a)
{
	if (a < 0)
		return -__fixunssfdi(-a);
	return __fixunssfdi(a);
}
#endif

#if defined(L_floatdixf) && (LONG_DOUBLE_TYPE_SIZE == 96)
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_HALFWORD_COEFF (((UDItype) 1) << (WORD_SIZE / 2))
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

XFtype __floatdixf(u)
DItype u;
{
	XFtype d;
	SItype negate = 0;

	if (u < 0)
		u = -u, negate = 1;

	d = (USItype) (u >> WORD_SIZE);
	d *= HIGH_HALFWORD_COEFF;
	d *= HIGH_HALFWORD_COEFF;
	d += (USItype) (u & (HIGH_WORD_COEFF - 1));

	return (negate ? -d : d);
}
#endif

#if defined(L_floatditf) && (LONG_DOUBLE_TYPE_SIZE == 128)
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_HALFWORD_COEFF (((UDItype) 1) << (WORD_SIZE / 2))
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

TFtype __floatditf(u)
DItype u;
{
	TFtype d;
	SItype negate = 0;

	if (u < 0)
		u = -u, negate = 1;

	d = (USItype) (u >> WORD_SIZE);
	d *= HIGH_HALFWORD_COEFF;
	d *= HIGH_HALFWORD_COEFF;
	d += (USItype) (u & (HIGH_WORD_COEFF - 1));

	return (negate ? -d : d);
}
#endif

#ifdef L_floatdidf
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_HALFWORD_COEFF (((UDItype) 1) << (WORD_SIZE / 2))
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

DFtype __floatdidf(u)
DItype u;
{
	DFtype d;
	SItype negate = 0;

	if (u < 0)
		u = -u, negate = 1;

	d = (USItype) (u >> WORD_SIZE);
	d *= HIGH_HALFWORD_COEFF;
	d *= HIGH_HALFWORD_COEFF;
	d += (USItype) (u & (HIGH_WORD_COEFF - 1));

	return (negate ? -d : d);
}
#endif

#ifdef L_floatdisf
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_HALFWORD_COEFF (((UDItype) 1) << (WORD_SIZE / 2))
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)
#define DI_SIZE (sizeof (DItype) * BITS_PER_UNIT)
#define DF_SIZE 53
#define SF_SIZE 24

SFtype __floatdisf(u)
DItype u;
{
	/* Do the calculation in DFmode
	   so that we don't lose any of the precision of the high word
	   while multiplying it.  */
	DFtype f;
	SItype negate = 0;

	if (u < 0)
		u = -u, negate = 1;

	/* Protect against double-rounding error.
	   Represent any low-order bits, that might be truncated in DFmode,
	   by a bit that won't be lost.  The bit can go in anywhere below the
	   rounding position of the SFmode.  A fixed mask and bit position
	   handles all usual configurations.  It doesn't handle the case
	   of 128-bit DImode, however.  */
	if (DF_SIZE < DI_SIZE && DF_SIZE > (DI_SIZE - DF_SIZE + SF_SIZE))
	{
#define REP_BIT ((USItype) 1 << (DI_SIZE - DF_SIZE))
		if (u >= ((UDItype) 1 << DF_SIZE))
		{
			if ((USItype) u & (REP_BIT - 1))
				u |= REP_BIT;
		}
	}
	f = (USItype) (u >> WORD_SIZE);
	f *= HIGH_HALFWORD_COEFF;
	f *= HIGH_HALFWORD_COEFF;
	f += (USItype) (u & (HIGH_WORD_COEFF - 1));

	return (SFtype) (negate ? -f : f);
}
#endif

#if defined(L_fixunsxfsi) && LONG_DOUBLE_TYPE_SIZE == 96
#include <limits.h>

USItype __fixunsxfsi(a)
XFtype a;
{
	if (a >= -(DFtype) LONG_MIN)
		return (SItype) (a + LONG_MIN) - LONG_MIN;
	return (SItype) a;
}
#endif

#ifdef L_fixunsdfsi
#include <limits.h>

USItype __fixunsdfsi(a)
DFtype a;
{
	if (a >= -(DFtype) LONG_MIN)
		return (SItype) (a + LONG_MIN) - LONG_MIN;
	return (SItype) a;
}
#endif

#ifdef L_fixunssfsi
#include <limits.h>

USItype __fixunssfsi(SFtype a)
{
	if (a >= -(SFtype) LONG_MIN)
		return (SItype) (a + LONG_MIN) - LONG_MIN;
	return (SItype) a;
}
#endif

/* From here on down, the routines use normal data types.  */

#define SItype bogus_type
#define USItype bogus_type
#define DItype bogus_type
#define UDItype bogus_type
#define SFtype bogus_type
#define DFtype bogus_type

#undef char
#undef short
#undef int
#undef long
#undef unsigned
#undef float
#undef double

/* Default free-store management functions for C++, per sections 12.5 and
   17.3.3 of the Working Paper. */

#ifdef L_op_new
/* operator new (size_t), described in 17.3.3.5.  This function is used by
   C++ programs to allocate a block of memory to hold a single object. */

#include <memory.h>

typedef void (*vfp)(void);
extern vfp __new_handler;

void *__builtin_new(size_t sz)
{
	void *p;

	/* malloc (0) is unpredictable; avoid it.  */
	if (sz == 0)
		sz = 1;
	p = (void *) malloc(sz);
	while (p == 0)
	{
		(*__new_handler) ();
		p = (void *) malloc(sz);
	}

	return p;
}
#endif /* L_op_new */

#ifdef L_op_vnew
/* void * operator new [] (size_t), described in 17.3.3.6.  This function
   is used by C++ programs to allocate a block of memory for an array.  */

extern void *__builtin_new(size_t);

void *__builtin_vec_new(size_t sz)
{
	return __builtin_new(sz);
}
#endif /* L_op_vnew */

#ifdef L_new_handler
/* set_new_handler (fvoid_t *) and the default new handler, described in
   17.3.3.2 and 17.3.3.5.  These functions define the result of a failure
   to allocate the amount of memory requested from operator new or new []. */

  /* Avoid forcing the library's meaning of `write' on the user program
     by using the "internal" name (for use within the library)  */
#define write(fd, buf, n)	_write((fd), (buf), (n))

typedef void (*vfp)(void);
void __default_new_handler(void);

vfp __new_handler = __default_new_handler;

vfp set_new_handler(vfp handler)
{
	vfp prev_handler;

	prev_handler = __new_handler;
	if (handler == 0)
		handler = __default_new_handler;
	__new_handler = handler;
	return prev_handler;
}

#define MESSAGE "Virtual memory exceeded in `new'\n"

void __default_new_handler()
{
	/* don't use fprintf (stderr, ...) because it may need to call malloc.  */
	/* This should really print the name of the program, but that is hard to
	   do.  We need a standard, clean way to get at the name.  */
	write(2, MESSAGE, sizeof(MESSAGE) - 1);
	/* don't call exit () because that may call global destructors which
	   may cause a loop.  */
	_exit(-1);
}
#endif

#ifdef L_op_delete
/* operator delete (void *), described in 17.3.3.3.  This function is used
   by C++ programs to return to the free store a block of memory allocated
   as a single object. */

void __builtin_delete(void *ptr)
{
	if (ptr)
		free(ptr);
}
#endif

#ifdef L_op_vdel
/* operator delete [] (void *), described in 17.3.3.4.  This function is
   used by C++ programs to return to the free store a block of memory
   allocated as an array. */

extern void __builtin_delete(void *);

void __builtin_vec_delete(void *ptr)
{
	__builtin_delete(ptr);
}
#endif

/* End of C++ free-store management functions */

#ifdef L_trampoline

/* Jump to a trampoline, loading the static chain address.  */

#ifdef TRANSFER_FROM_TRAMPOLINE
TRANSFER_FROM_TRAMPOLINE
#endif
#endif /* L_trampoline */
#ifdef L__main
#include "gbl-ctors.h"
/* Run all the global destructors on exit from the program.  */
void __do_global_dtors()
{
#ifdef DO_GLOBAL_DTORS_BODY
	DO_GLOBAL_DTORS_BODY;
#else
	func_ptr *p;

	for (p = __DTOR_LIST__ + 1; *p;)
		(*p++) ();
#endif
}

/* Run all the global constructors on entry to the program.  */

void __do_global_ctors()
{
	DO_GLOBAL_CTORS_BODY;
}

/* Subroutine called automatically by `main'.
   Compiling a global function named `main'
   produces an automatic call to this function at the beginning.

   For many systems, this routine calls __do_global_ctors.
   For systems which support a .init section we use the .init section
   to run __do_global_ctors, so we need not do anything here.  */

void __main()
{
	/* Support recursive calls to `main': run initializers just once.  */
	static int initialized = 0;

	if (!initialized)
	{
		initialized = 1;
		__do_global_ctors();
	}
}

#endif /* L__main */

#ifdef L_ctor_list
#include "gbl-ctors.h"
const func_ptr __CTOR_LIST__[2] = { 0, 0 };
#endif /* L_ctor_list */

#ifdef L_dtor_list
#include "gbl-ctors.h"
const func_ptr __DTOR_LIST__[2] = { 0, 0 };
#endif /* L_ctor_list */

#ifdef L_eh
#include <stdlib.h>
#include <string.h>

typedef struct
{
	void *start;
	void *end;
	void *exception_handler;
} exception_table;

struct exception_table_node
{
	exception_table *table;
	void *start;
	void *end;
	struct exception_table_node *next;
};

static int except_table_pos = 0;
static void *except_pc = (void *) 0;
static struct exception_table_node *exception_table_list = 0;

static exception_table *find_exception_table(pc)
void *pc;
{
	register struct exception_table_node *table = exception_table_list;

	for (; table != 0; table = table->next)
	{
		if (table->start <= pc && table->end > pc)
			return table->table;
	}
	return 0;
}

/* this routine takes a pc, and the address of the exception handler associated
   with the closest exception table handler entry associated with that PC,
   or 0 if there are no table entries the PC fits in.  The algorithm works
   something like this:

    while(current_entry exists) {
        if(current_entry.start < pc )
            current_entry = next_entry;
        else {
            if(prev_entry.start <= pc && prev_entry.end > pc) {
                save pointer to prev_entry;
                return prev_entry.exception_handler;
             }
            else return 0;
         }
     }
    return 0;

   Assuming a correctly sorted table (ascending order) this routine should
   return the tighest match...

   In the advent of a tie, we have to give the last entry, as it represents
   an inner block.
 */


void *__find_first_exception_table_match(pc)
void *pc;
{
	exception_table *table = find_exception_table(pc);
	int pos = 0;
	int best = 0;

	if (table == 0)
		return (void *) 0;
#if 0
	printf("find_first_exception_table_match(): pc = %x!\n", pc);
#endif

	except_pc = pc;

#if 0
	/* We can't do this yet, as we don't know that the table is sorted.  */
	do
	{
		++pos;
		if (table[pos].start > except_pc)
			/* found the first table[pos].start > except_pc, so the previous
			   entry better be the one we want! */
			break;
	} while (table[pos].exception_handler != (void *) -1);

	--pos;
	if (table[pos].start <= except_pc && table[pos].end > except_pc)
	{
		except_table_pos = pos;
#if 0
		printf("find_first_eh_table_match(): found match: %x\n", table[pos].exception_handler);
#endif
		return table[pos].exception_handler;
	}
#else
	while (table[++pos].exception_handler != (void *) -1)
	{
		if (table[pos].start <= except_pc && table[pos].end > except_pc)
		{
			/* This can apply.  Make sure it is better or as good as the previous
			   best.  */
			/* The best one ends first. */
			if (best == 0 || (table[pos].end <= table[best].end
							  /* The best one starts last.  */
							  && table[pos].start >= table[best].start))
				best = pos;
		}
	}
	if (best != 0)
		return table[best].exception_handler;
#endif

#if 0
	printf("find_first_eh_table_match(): else: returning NULL!\n");
#endif
	return (void *) 0;
}

int __throw_type_match(const char *catch_type, const char *throw_type)
{
#if 0
	printf("__throw_type_match (): catch_type = %s, throw_type = %s\n", catch_type, throw_type);
#endif
	return strcmp(catch_type, throw_type);
}

void __register_exceptions(exception_table * table)
{
	struct exception_table_node *node = (struct exception_table_node *) malloc(sizeof(struct exception_table_node));
	exception_table *range = table + 1;

	node->table = table;

	/* This look can be optimized away either if the table
	   is sorted, or if we pass in extra parameters. */
	node->start = range->start;
	node->end = range->end;
	for (range++; range->start != (void *) (-1); range++)
	{
		if (range->start < node->start)
			node->start = range->start;
		if (range->end < node->end)
			node->end = range->end;
	}

	node->next = exception_table_list;
	exception_table_list = node;
}
#endif /* L_eh */

#ifdef L_pure
#include <unistd.h>
#define MESSAGE "pure virtual method called\n"
#define write(fd, buf, n)	_write((fd), (buf), (n))
void __pure_virtual()
{
	write(2, MESSAGE, sizeof(MESSAGE) - 1);
	_exit(-1);
}
#endif