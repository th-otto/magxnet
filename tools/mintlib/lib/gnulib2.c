/* WARNING: compile this in 32 bit int mode even for short library */
#include <string.h>
#include <memory.h>
#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __DEF_ALL__						/* this def'ed when making on the ST */

#define L_adddi3
#define L_subdi3
#define L_muldi3
#define L_divdi3
#define L_moddi3
#define L_udivdi3
#define L_umoddi3
#define L_negdi2
#define L_anddi3
#define L_iordi3
#define L_xordi3
#define L_lshrdi3
#define L_lshldi3
#define L_ashldi3
#define L_ashrdi3
#define L_one_cmpldi2
#define L_bdiv
#define L_cmpdi2
#define L_ucmpdi2
#define L_fixunsdfdi
#define L_fixdfdi
#define L_floatdidf

/* gcc-2.0 stuff */
#if 0									/* NOTE: these are now covered, and should not be generated here */
#define L_lshrsi3
#define L_lshlsi3
#define L_ashrsi3
#define L_ashlsi3
#define L_eqdf2
#define L_nedf2
#define L_gtdf2
#define L_gedf2
#define L_ltdf2
#define L_ledf2
#define L_fixsfsi
#define L_floatsisf
#define L_eqsf2
#define L_nesf2
#define L_gtsf2
#define L_gesf2
#define L_ltsf2
#define L_lesf2
#endif
#define L_fxussfsi
#define L_gccbcmp

#endif /* __DEF_ALL__ */

/* More subroutines needed by GCC output code on some machines.  */
/* Compile this one with gcc.  */

#if 0
#include "config.h"						/* dont drag this in, just define relevant
										   stuff from xm/tm-atari.h & xm/tm-m68k.h here */
#else

/* #defines that need visibility everywhere.  */
#define FALSE 0
#define TRUE 1

/* This describes the machine the compiler is hosted on.  */
#define HOST_BITS_PER_CHAR 8
#define HOST_BITS_PER_SHORT 16
#define HOST_BITS_PER_INT 32
#define HOST_BITS_PER_LONG 32

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.
   This is true for 68020 insns such as bfins and bfexts.
   We make it true always by avoiding using the single-bit insns
   except in special cases with constant bit numbers.  */
#define BITS_BIG_ENDIAN

/* Define this if most significant byte of a word is the lowest numbered.  */
/* That is true on the 68000.  */
#define BYTES_BIG_ENDIAN

/* Define this if most significant word of a multiword number is numbered.  */
/* For 68000 we can decide arbitrarily
   since there are no machine instructions for them.  */
#define WORDS_BIG_ENDIAN

/* number of bits in an addressible storage unit */
#define BITS_PER_UNIT 8

/* Width in bits of a "word", which is the contents of a machine register.
   Note that this is not necessarily the width of data type `int';
   if using 16-bit ints on a 68000, this would still be 32.
   But on a machine with 16-bit registers, this would be 16.  */
#define BITS_PER_WORD 32

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD 4

/* Width in bits of a pointer.
   See also the macro `Pmode' defined below.  */
#define POINTER_SIZE 32

/* Allocation boundary (in *bits*) for storing pointers in memory.  */
#define POINTER_BOUNDARY 16

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY (TARGET_SHORT ? 16 : 32)

/* Boundary (in *bits*) on which stack pointer should be aligned.  */
#define STACK_BOUNDARY 16

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY 16

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY 16

/* No data type wants to be aligned rounder than this.  */
#define BIGGEST_ALIGNMENT 16

/* Define this if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT

/* Define number of bits in most basic integer type.
   (If undefined, default is BITS_PER_WORD).  */
#ifdef __MSHORT__
#define INT_TYPE_SIZE 16
#else
#define INT_TYPE_SIZE 32
#endif

#endif

#ifndef minix
#include <stddef.h>
#else
typedef unsigned long size_t;

#include "lib.h"
#endif

#ifndef SItype
#define SItype long int
#endif

/* long long ints are pairs of long ints in the order determined by
   WORDS_BIG_ENDIAN.  */

#ifdef WORDS_BIG_ENDIAN
struct longlong
{
	long high,
	 low;
};
#else
struct longlong
{
	long low,
	 high;
};
#endif

/* We need this union to unpack/pack longlongs, since we don't have
   any arithmetic yet.  Incoming long long parameters are stored
   into the `ll' field, and the unpacked result is read from the struct
   longlong.  */

typedef union
{
	struct longlong s;
	long long ll;
	SItype i[2];
	unsigned SItype ui[2];
} long_long;

/* Internally, long long ints are strings of unsigned shorts in the
   order determined by BYTES_BIG_ENDIAN.  */

#define B 0x10000
#define low16 (B - 1)

#ifdef BYTES_BIG_ENDIAN

/* Note that HIGH and LOW do not describe the order
   of words in a long long.  They describe the order of words
   in vectors ordered according to the byte order.  */

#define HIGH 0
#define LOW 1

#define big_end(n)	0
#define little_end(n)	((n) - 1)
#define next_msd(i)	((i) - 1)
#define next_lsd(i)	((i) + 1)
#define is_not_msd(i,n)	((i) >= 0)
#define is_not_lsd(i,n)	((i) < (n))

#else

#define LOW 0
#define HIGH 1

#define big_end(n)	((n) - 1)
#define little_end(n)	0
#define next_msd(i)	((i) + 1)
#define next_lsd(i)	((i) - 1)
#define is_not_msd(i,n)	((i) < (n))
#define is_not_lsd(i,n)	((i) >= 0)

#endif

/* These algorithms are all straight out of Knuth, vol. 2, sec. 4.3.1. */

__EXTERN long long __adddi3 __PROTO((long long u, long long v));
__EXTERN long long __anddi3 __PROTO((long long u, long long v));
__EXTERN long long __iordi3 __PROTO((long long u, long long v));
__EXTERN long long __xordi3 __PROTO((long long u, long long v));
__EXTERN long long __one_cmpldi2 __PROTO((long long u));
__EXTERN long long __lshldi3 __PROTO((long long u, long int b1));
__EXTERN long long __lshrdi3 __PROTO((long long u, long int b1));
__EXTERN long long __ashldi3 __PROTO((long long u, long int b1));
__EXTERN long long __ashrdi3 __PROTO((long long u, long int b1));
__EXTERN long long __subdi3 __PROTO((long long u, long long v));
__EXTERN long long __muldi3 __PROTO((long long u, long long v));
__EXTERN long long __divdi3 __PROTO((long long u, long long v));
__EXTERN long long __moddi3 __PROTO((long long u, long long v));
__EXTERN long long __udivdi3 __PROTO((long long u, long long v));
__EXTERN long long __umoddi3 __PROTO((long long u, long long v));
__EXTERN long long __negdi2 __PROTO((long long u));
__EXTERN void __bdiv
__PROTO((unsigned short *a, unsigned short *b, unsigned short *q, unsigned short *r, size_t m, size_t n));
__EXTERN SItype __cmpdi2 __PROTO((long long a, long long b));
__EXTERN SItype __ucmpdi2 __PROTO((long long a, long long b));
__EXTERN long long __fixunsdfdi __PROTO((double a));
__EXTERN long long __fixdfdi __PROTO((double a));
__EXTERN double __floatdidf __PROTO((long long u));
__EXTERN int __builtin_saveregs __PROTO((void));
__EXTERN unsigned SItype __fixunssfsi __PROTO((float a));

#ifdef L_adddi3
static int badd __PROTO((unsigned short *a, unsigned short *b, unsigned short *c, size_t n));

long long __adddi3(u, v)
long long u,
	v;
{
	long a[2],
	 b[2],
	 c[2];
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	a[HIGH] = uu.s.high;
	a[LOW] = uu.s.low;
	b[HIGH] = vv.s.high;
	b[LOW] = vv.s.low;

	badd((unsigned short *) a, (unsigned short *) b, (unsigned short *) c, sizeof c);

	w.s.high = c[HIGH];
	w.s.low = c[LOW];
	return w.ll;
}

static int badd(a, b, c, n)
unsigned short *a,
*b,
*c;
size_t n;
{
	unsigned long acc;
	int i;

	n /= sizeof *c;

	acc = 0;
	for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
	{
		/* Widen before adding to avoid loss of high bits.  */
		acc += (unsigned long) a[i] + b[i];
		c[i] = acc & low16;
		acc = acc >> 16;
	}
	return acc;
}
#endif

#ifdef L_anddi3
long long __anddi3(u, v)
long long u,
	v;
{
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	w.s.high = uu.s.high & vv.s.high;
	w.s.low = uu.s.low & vv.s.low;

	return w.ll;
}
#endif

#ifdef L_iordi3
long long __iordi3(u, v)
long long u,
	v;
{
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	w.s.high = uu.s.high | vv.s.high;
	w.s.low = uu.s.low | vv.s.low;

	return w.ll;
}
#endif

#ifdef L_xordi3
long long __xordi3(u, v)
long long u,
	v;
{
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	w.s.high = uu.s.high ^ vv.s.high;
	w.s.low = uu.s.low ^ vv.s.low;

	return w.ll;
}
#endif

#ifdef L_one_cmpldi2
long long __one_cmpldi2(u)
long long u;
{
	long_long w;
	long_long uu;

	uu.ll = u;

	w.s.high = ~uu.s.high;
	w.s.low = ~uu.s.low;

	return w.ll;
}
#endif

#ifdef L_lshldi3
long long __lshldi3(u, b1)
long long u;
long int b1;
{
	long_long w;
	unsigned long carries;
	int bm;
	long_long uu;
	int b = b1;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(int) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.low = 0;
		w.s.high = (unsigned long) uu.s.low << -bm;
	} else
	{
		carries = (unsigned long) uu.s.low >> bm;
		w.s.low = (unsigned long) uu.s.low << b;
		w.s.high = ((unsigned long) uu.s.high << b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_lshrdi3
long long __lshrdi3(u, b1)
long long u;
long int b1;
{
	long_long w;
	unsigned long carries;
	int bm;
	long_long uu;
	int b = b1;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(int) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.high = 0;
		w.s.low = (unsigned long) uu.s.high >> -bm;
	} else
	{
		carries = (unsigned long) uu.s.high << bm;
		w.s.high = (unsigned long) uu.s.high >> b;
		w.s.low = ((unsigned long) uu.s.low >> b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_ashldi3
long long __ashldi3(u, b1)
long long u;
long int b1;
{
	long_long w;
	unsigned long carries;
	int bm;
	long_long uu;
	int b = b1;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(int) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.low = 0;
		w.s.high = (unsigned long) uu.s.low << -bm;
	} else
	{
		carries = (unsigned long) uu.s.low >> bm;
		w.s.low = (unsigned long) uu.s.low << b;
		w.s.high = ((unsigned long) uu.s.high << b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_ashrdi3
long long __ashrdi3(u, b1)
long long u;
long int b1;
{
	long_long w;
	unsigned long carries;
	int bm;
	long_long uu;
	int b = b1;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(int) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.high = uu.s.high >> 31;		/* just to make w.s.high 1..1 or 0..0 */
		w.s.low = uu.s.high >> -bm;
	} else
	{
		carries = (unsigned long) uu.s.high << bm;
		w.s.high = uu.s.high >> b;
		w.s.low = ((unsigned long) uu.s.low >> b) | carries;
	}

	return w.ll;
}
#endif

#ifdef L_subdi3

static int bsub __PROTO((unsigned short *a, unsigned short *b, unsigned short *c, size_t n));

long long __subdi3(u, v)
long long u,
	v;
{
	long a[2],
	 b[2],
	 c[2];
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	a[HIGH] = uu.s.high;
	a[LOW] = uu.s.low;
	b[HIGH] = vv.s.high;
	b[LOW] = vv.s.low;

	bsub((unsigned short *) a, (unsigned short *) b, (unsigned short *) c, sizeof c);

	w.s.high = c[HIGH];
	w.s.low = c[LOW];
	return w.ll;
}

static int bsub(a, b, c, n)
unsigned short *a,
*b,
*c;
size_t n;
{
	signed long acc;
	int i;

	n /= sizeof *c;

	acc = 0;
	for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
	{
		/* Widen before subtracting to avoid loss of high bits.  */
		acc += (long) a[i] - b[i];
		c[i] = acc & low16;
		acc = acc >> 16;
	}
	return acc;
}
#endif

#ifdef L_muldi3

static void bmul __PROTO((unsigned short *a, unsigned short *b, unsigned short *c, size_t m, size_t n));

long long __muldi3(u, v)
long long u,
	v;
{
	long a[2],
	 b[2],
	 c[2][2];
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	a[HIGH] = uu.s.high;
	a[LOW] = uu.s.low;
	b[HIGH] = vv.s.high;
	b[LOW] = vv.s.low;

	bmul((unsigned short *) a, (unsigned short *) b, (unsigned short *) c, sizeof a, sizeof b);

	w.s.high = c[LOW][HIGH];
	w.s.low = c[LOW][LOW];
	return w.ll;
}

static void bmul(a, b, c, m, n)
unsigned short *a,
*b,
*c;
size_t m,
	n;
{
	int i,
	 j;
	unsigned long acc;

	(void) bzero(c, m + n);

	m /= sizeof *a;
	n /= sizeof *b;

	for (j = little_end(n); is_not_msd(j, n); j = next_msd(j))
	{
		unsigned short *c1 = c + j + little_end(2);

		acc = 0;
		for (i = little_end(m); is_not_msd(i, m); i = next_msd(i))
		{
			/* Widen before arithmetic to avoid loss of high bits.  */
			acc += (unsigned long) a[i] * b[j] + c1[i];
			c1[i] = acc & low16;
			acc = acc >> 16;
		}
		c1[i] = acc;
	}
}
#endif

#ifdef L_divdi3
long long __divdi3(u, v)
long long u,
	v;
{
	if (u < 0)
		if (v < 0)
			return (unsigned long long) -u / (unsigned long long) -v;
		else
			return -((unsigned long long) -u / (unsigned long long) v);
	else if (v < 0)
		return -((unsigned long long) u / (unsigned long long) -v);
	else
		return (unsigned long long) u / (unsigned long long) v;
}
#endif

#ifdef L_moddi3
long long __moddi3(u, v)
long long u,
	v;
{
	if (u < 0)
		if (v < 0)
			return -((unsigned long long) -u % (unsigned long long) -v);
		else
			return -((unsigned long long) -u % (unsigned long long) v);
	else if (v < 0)
		return (unsigned long long) u % (unsigned long long) -v;
	else
		return (unsigned long long) u % (unsigned long long) v;
}
#endif

#ifdef L_udivdi3
long long __udivdi3(u, v)
long long u,
	v;
{
	unsigned long a[2][2],
	 b[2],
	 q[2],
	 r[2];
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	a[HIGH][HIGH] = 0;
	a[HIGH][LOW] = 0;
	a[LOW][HIGH] = uu.s.high;
	a[LOW][LOW] = uu.s.low;
	b[HIGH] = vv.s.high;
	b[LOW] = vv.s.low;

	__bdiv((unsigned short *) a, (unsigned short *) b, (unsigned short *) q, (unsigned short *) r, sizeof a, sizeof b);

	w.s.high = q[HIGH];
	w.s.low = q[LOW];
	return w.ll;
}
#endif

#ifdef L_umoddi3
long long __umoddi3(u, v)
long long u,
	v;
{
	unsigned long a[2][2],
	 b[2],
	 q[2],
	 r[2];
	long_long w;
	long_long uu,
	 vv;

	uu.ll = u;
	vv.ll = v;

	a[HIGH][HIGH] = 0;
	a[HIGH][LOW] = 0;
	a[LOW][HIGH] = uu.s.high;
	a[LOW][LOW] = uu.s.low;
	b[HIGH] = vv.s.high;
	b[LOW] = vv.s.low;

	__bdiv((unsigned short *) a, (unsigned short *) b, (unsigned short *) q, (unsigned short *) r, sizeof a, sizeof b);

	w.s.high = r[HIGH];
	w.s.low = r[LOW];
	return w.ll;
}
#endif

#ifdef L_negdi2

static int bneg __PROTO((unsigned short *a, unsigned short *b, size_t n));

long long __negdi2(u)
long long u;
{
	unsigned long a[2],
	 b[2];
	long_long w;
	long_long uu;

	uu.ll = u;

	a[HIGH] = uu.s.high;
	a[LOW] = uu.s.low;

	bneg((unsigned short *) a, (unsigned short *) b, sizeof b);

	w.s.high = b[HIGH];
	w.s.low = b[LOW];
	return w.ll;
}

static int bneg(a, b, n)
unsigned short *a,
*b;
size_t n;
{
	signed long acc;
	int i;

	n /= sizeof(short);

	acc = 0;
	for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
	{
		acc -= a[i];
		b[i] = acc & low16;
		acc = acc >> 16;
	}
	return acc;
}
#endif

/* Divide a by b, producing quotient q and remainder r.

       sizeof a is m
       sizeof b is n
       sizeof q is m - n
       sizeof r is n

   The quotient must fit in m - n bytes, i.e., the most significant
   n digits of a must be less than b, and m must be greater than n.  */

/* The name of this used to be __div_internal,
   but that is too long for SYSV.  */

#ifdef L_bdiv
static int bshift __PROTO((unsigned short *u, int k, unsigned short *w, unsigned int carry_in, int n));

void __bdiv(a, b, q, r, m, n)
unsigned short *a,
*b,
*q,
*r;
size_t m,
	n;
{
	void abort(void);
	unsigned long qhat,
	 rhat;
	unsigned long acc;
	unsigned short *u = (unsigned short *) alloca(m);
	unsigned short *v = (unsigned short *) alloca(n);
	unsigned short *u0,
	*u1,
	*u2;
	unsigned short *v0;
	int d,
	 qn;
	int i,
	 j;

	m /= sizeof *a;
	n /= sizeof *b;
	qn = m - n;

	/* Remove leading zero digits from divisor, and the same number of
	   digits (which must be zero) from dividend.  */

	while (b[big_end(n)] == 0)
	{
		r[big_end(n)] = 0;

		a += little_end(2);
		b += little_end(2);
		r += little_end(2);
		m--;
		n--;

		/* Check for zero divisor.  */
		if (n == 0)
			abort();
	}

	/* If divisor is a single digit, do short division.  */

	if (n == 1)
	{
		acc = a[big_end(m)];
		a += little_end(2);
		for (j = big_end(qn); is_not_lsd(j, qn); j = next_lsd(j))
		{
			acc = (acc << 16) | a[j];
			q[j] = acc / *b;
			acc = acc % *b;
		}
		*r = acc;
		return;
	}

	/* No such luck, must do long division. Shift divisor and dividend
	   left until the high bit of the divisor is 1.  */

	for (d = 0; d < 16; d++)
		if (b[big_end(n)] & (1 << (16 - 1 - d)))
			break;

	bshift(a, d, u, 0, m);
	bshift(b, d, v, 0, n);

	/* Get pointers to the high dividend and divisor digits.  */

	u0 = u + big_end(m) - big_end(qn);
	u1 = next_lsd(u0);
	u2 = next_lsd(u1);
	u += little_end(2);

	v0 = v + big_end(n);

	/* Main loop: find a quotient digit, multiply it by the divisor,
	   and subtract that from the dividend, shifted over the right amount. */

	for (j = big_end(qn); is_not_lsd(j, qn); j = next_lsd(j))
	{
		/* Quotient digit initial guess: high 2 dividend digits over high
		   divisor digit.  */

		if (u0[j] == *v0)
		{
			qhat = B - 1;
			rhat = (unsigned long) *v0 + u1[j];
		} else
		{
			unsigned long numerator = ((unsigned long) u0[j] << 16) | u1[j];

			qhat = numerator / *v0;
			rhat = numerator % *v0;
		}

		/* Now get the quotient right for high 3 dividend digits over
		   high 2 divisor digits.  */

		while (rhat < B && qhat * *next_lsd(v0) > ((rhat << 16) | u2[j]))
		{
			qhat -= 1;
			rhat += *v0;
		}

		/* Multiply quotient by divisor, subtract from dividend.  */

		acc = 0;
		for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
		{
			acc += (unsigned long) (u + j)[i] - v[i] * qhat;
			(u + j)[i] = acc & low16;
			if (acc < B)
				acc = 0;
			else
				acc = (acc >> 16) | -B;
		}

		q[j] = qhat;

		/* Quotient may have been too high by 1.  If dividend went negative,
		   decrement the quotient by 1 and add the divisor back.  */

		if ((signed long) (acc + u0[j]) < 0)
		{
			q[j] -= 1;
			acc = 0;
			for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
			{
				acc += (unsigned long) (u + j)[i] + v[i];
				(u + j)[i] = acc & low16;
				acc = acc >> 16;
			}
		}
	}

	/* Now the remainder is what's left of the dividend, shifted right
	   by the amount of the normalizing left shift at the top.  */

	r[big_end(n)] = bshift(u + 1 + little_end(j - 1), 16 - d, r + little_end(2), u[little_end(m - 1)] >> d, n - 1);
}

/* Left shift U by K giving W; fill the introduced low-order bits with
   CARRY_IN.  Length of U and W is N.  Return carry out.  K must be
   in 0 .. 16.  */

static int bshift(u, k, w, carry_in, n)
unsigned short *u,
*w;
unsigned int carry_in;
int k,
	n;
{
	unsigned long acc;
	int i;

	if (k == 0)
	{
		bcopy(u, w, n * sizeof *u);
		return 0;
	}

	acc = carry_in;
	for (i = little_end(n); is_not_msd(i, n); i = next_msd(i))
	{
		acc |= (unsigned long) u[i] << k;
		w[i] = acc & low16;
		acc = acc >> 16;
	}
	return acc;
}
#endif

#ifdef L_cmpdi2
SItype __cmpdi2(a, b)
long long a,
	b;
{
	long_long au,
	 bu;

	au.ll = a, bu.ll = b;

	if (au.s.high < bu.s.high)
		return 0;
	else if (au.s.high > bu.s.high)
		return 2;
	if ((unsigned) au.s.low < (unsigned) bu.s.low)
		return 0;
	else if ((unsigned) au.s.low > (unsigned) bu.s.low)
		return 2;
	return 1;
}
#endif

#ifdef L_ucmpdi2
SItype __ucmpdi2(a, b)
long long a,
	b;
{
	long_long au,
	 bu;

	au.ll = a, bu.ll = b;

	if ((unsigned) au.s.high < (unsigned) bu.s.high)
		return 0;
	else if ((unsigned) au.s.high > (unsigned) bu.s.high)
		return 2;
	if ((unsigned) au.s.low < (unsigned) bu.s.low)
		return 0;
	else if ((unsigned) au.s.low > (unsigned) bu.s.low)
		return 2;
	return 1;
}
#endif

#ifdef L_fixunsdfdi
#define HIGH_WORD_COEFF (((long long) 1) << BITS_PER_WORD)

long long __fixunsdfdi(a)
double a;
{
	double b;
	unsigned long long v;

	if (a < 0)
		return 0;

	/* Compute high word of result, as a flonum.  */
	b = (a / HIGH_WORD_COEFF);
	/* Convert that to fixed (but not to long long!),
	   and shift it into the high word.  */
	v = (unsigned long int) b;
	v <<= BITS_PER_WORD;
	/* Remove high part from the double, leaving the low part as flonum.  */
	a -= (double) v;
	/* Convert that to fixed (but not to long long!) and add it in.
	   Sometimes A comes out negative.  This is significant, since
	   A has more bits than a long int does.  */
	if (a < 0)
		v -= (unsigned long int) (-a);
	else
		v += (unsigned long int) a;
	return v;
}
#endif

#ifdef L_fixdfdi
long long __fixdfdi(a)
double a;
{
	long long __fixunsdfdi(double a);

	if (a < 0)
		return -__fixunsdfdi(-a);
	return __fixunsdfdi(a);
}
#endif

#ifdef L_floatdidf
#define HIGH_HALFWORD_COEFF (((long long) 1) << (BITS_PER_WORD / 2))
#define HIGH_WORD_COEFF (((long long) 1) << BITS_PER_WORD)

double __floatdidf(u)
long long u;
{
	double d;
	int negate = 0;

	if (u < 0)							/* was : if (d < 0)     */
		u = -u, negate = 1;

	d = (unsigned int) (u >> BITS_PER_WORD);
	d *= HIGH_HALFWORD_COEFF;
	d *= HIGH_HALFWORD_COEFF;
	d += (unsigned int) (u & (HIGH_WORD_COEFF - 1));

	return (negate ? -d : d);
}
#endif

#ifdef L_varargs
#ifdef sparc
asm(".global ___builtin_saveregs");
asm("___builtin_saveregs:");
asm("st %i0,[%fp+68]");
asm("st %i1,[%fp+72]");
asm("st %i2,[%fp+76]");
asm("st %i3,[%fp+80]");
asm("st %i4,[%fp+84]");
asm("retl");
asm("st %i5,[%fp+88]");
#else /* not sparc */
#if defined(MIPSEL) | defined(R3000) | defined(R2000) | defined(mips)

asm("	.ent __builtin_saveregs");
asm("	.globl __builtin_saveregs");
asm("__builtin_saveregs:");
asm("	sw	$4,0($30)");
asm("	sw	$5,4($30)");
asm("	sw	$6,8($30)");
asm("	sw	$7,12($30)");
asm("	j	$31");
asm("	.end __builtin_saveregs");
#else /* not mips */
__builtin_saveregs()
{
	abort();
}
#endif /* not mips */
#endif /* not sparc */
#endif

/* stuff for gcc-2.0, note that you cannot compile the corresponding
   C code from libgcc1.c for these functions with gcc-2.0!
   This stuff should eventually be hand optimized as we have done
   with the other stuff,
 */

#if 0									/* these were the decls used to compile the asm below */

#ifndef SItype
#define SItype long int
#endif

#ifndef FLOAT_VALUE_TYPE
#define FLOAT_VALUE_TYPE long int
#endif

#ifndef INTIFY
#define INTIFY(FLOATVAL)  (intify.f = (FLOATVAL), intify.i)
#endif

#ifndef FLOATIFY
#define FLOATIFY(INTVAL)  ((INTVAL).f)
#endif

#ifndef FLOAT_ARG_TYPE
#define FLOAT_ARG_TYPE union flt_or_int
#endif

union flt_or_value
{
	FLOAT_VALUE_TYPE i;
	float f;
};

union flt_or_int
{
	long int i;
	float f;
};
#endif

#if 0									/* NOTE: all these come from elsewhere now */
#ifdef L_lshrsi3
#if 0
SItype __lshrsi3(a, b)
unsigned SItype a,
	b;
{
	return a >> b;
}
#endif

asm("	.text
	.even
.globl ___lshrsi3
___lshrsi3:
	movel sp@(4),d0
	movel sp@(8),d1
	lsrl d1,d0
	rts
");
#endif

#ifdef L_lshlsi3
#if 0
SItype __lshlsi3(a, b)
unsigned SItype a,
	b;
{
	return a << b;
}
#endif

asm("   .text
	.even
.globl ___lshlsi3
___lshlsi3:
	movel sp@(4),d0
	movel sp@(8),d1
	lsll d1,d0
	rts
");
#endif

#ifdef L_ashrsi3
#if 0
SItype __ashrsi3(a, b)
SItype a,
	b;
{
	return a >> b;
}
#endif

asm("   .text
	.even
.globl ___ashrsi3
___ashrsi3:
	movel sp@(4),d0
	movel sp@(8),d1
	asrl d1,d0
	rts
");
#endif

#ifdef L_ashlsi3
#if 0
SItype __ashlsi3(a, b)
SItype a,
	b;
{
	return a << b;
}
#endif

asm("	.text
	.even
.globl ___ashlsi3
___ashlsi3:
	movel sp@(4),d0
	movel sp@(8),d1
	asll d1,d0
	rts
");
#endif

#ifdef L_eqdf2
#if 0
SItype __eqdf2(a, b)
double a,
	b;
{
	/* Value == 0 iff a == b.  */
	return !(a == b);
}
#endif

asm("	.text
	.even
.globl ___eqdf2
___eqdf2:
	moveml #0x3000,sp@-
	movel sp@(12),d1
	movel sp@(16),d2
	movel sp@(24),sp@-
	movel sp@(24),sp@-
	movel d2,sp@-
	movel d1,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	sne d0
	moveq #1,d3
	andl d3,d0
	moveml sp@+,#0xc
	rts
");
#endif

#ifdef L_nedf2
#if 0
SItype __nedf2(a, b)
double a,
	b;
{
	/* Value != 0 iff a != b.  */
	return a != b;
}
#endif

asm("	.text
	.even
.globl ___nedf2
___nedf2:
	moveml #0x3000,sp@-
	movel sp@(12),d1
	movel sp@(16),d2
	movel sp@(24),sp@-
	movel sp@(24),sp@-
	movel d2,sp@-
	movel d1,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	sne d0
	moveq #1,d3
	andl d3,d0
	moveml sp@+,#0xc
	rts
");
#endif

#ifdef L_gtdf2
#if 0
SItype __gtdf2(a, b)
double a,
	b;
{
	/* Value > 0 iff a > b.  */
	return a > b;
}
#endif

asm("	.text
	.even
.globl ___gtdf2
___gtdf2:
	moveml #0x3000,sp@-
	movel sp@(12),d1
	movel sp@(16),d2
	movel sp@(24),sp@-
	movel sp@(24),sp@-
	movel d2,sp@-
	movel d1,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	sgt d0
	moveq #1,d3
	andl d3,d0
	moveml sp@+,#0xc
	rts
");
#endif

#ifdef L_gedf2
#if 0
SItype __gedf2(a, b)
double a,
	b;
{
	/* Value >= 0 iff a >= b.  */
	return (a >= b) - 1;
}
#endif

asm("	.text
	.even
.globl ___gedf2
___gedf2:
	moveml #0x3000,sp@-
	movel sp@(12),d1
	movel sp@(16),d2
	movel sp@(24),sp@-
	movel sp@(24),sp@-
	movel d2,sp@-
	movel d1,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	sge d0
	moveq #1,d3
	andl d3,d0
	subql #1,d0
	moveml sp@+,#0xc
	rts
");
#endif

#ifdef L_ltdf2
#if 0
SItype __ltdf2(a, b)
double a,
	b;
{
	/* Value < 0 iff a < b.  */
	return -(a < b);
}
#endif

asm("	.text
	.even
.globl ___ltdf2
___ltdf2:
	moveml #0x3000,sp@-
	movel sp@(12),d1
	movel sp@(16),d2
	movel sp@(24),sp@-
	movel sp@(24),sp@-
	movel d2,sp@-
	movel d1,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	slt d0
	moveq #1,d3
	andl d3,d0
	negl d0
	moveml sp@+,#0xc
	rts
");
#endif

#ifdef L_ledf2
#if 0
SItype __ledf2(a, b)
double a,
	b;
{
	/* Value <= 0 iff a <= b.  */
	return 1 - (a <= b);
}
#endif

asm("	.text
	.even
.globl ___ledf2
___ledf2:
	movel d2,sp@-
	movel sp@(8),d0
	movel sp@(12),d1
	moveq #1,d2
	movel sp@(20),sp@-
	movel sp@(20),sp@-
	movel d1,sp@-
	movel d0,sp@-
	jbsr ___cmpdf2
	addw #16,sp
	tstl d0
	jgt L11
	moveq #0,d2
L11:
	movel d2,d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_fixsfsi
#if 0
SItype __fixsfsi(a)
FLOAT_ARG_TYPE a;
{
	union flt_or_value intify;

#define perform_fixsfsi(a) return (SItype) a
	perform_fixsfsi(FLOATIFY(a));
}
#endif

asm("	.text
	.even
.globl ___fixsfsi
___fixsfsi:
	movel sp@(4),sp@-
	jbsr ___extendsfdf2
	addqw #4,sp
	movel d1,sp@-
	movel d0,sp@-
	jbsr ___fixdfsi
	addqw #8,sp
	rts
");
#endif

#ifdef L_floatsisf
#if 0
FLOAT_VALUE_TYPE __floatsisf(a)
SItype a;
{
	union flt_or_value intify;

#define perform_floatsisf(a)  return INTIFY ((float) a)
	perform_floatsisf(a);
}
#endif

asm("	.text
	.even
.globl ___floatsisf
___floatsisf:
	movel sp@(4),sp@-
	jbsr ___floatsidf
	addqw #4,sp
	movel d1,sp@-
	movel d0,sp@-
	jbsr ___truncdfsf2
	addqw #8,sp
	rts
");
#endif

#ifdef L_eqsf2
#if 0
SItype __eqsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value == 0 iff a == b.  */
#define perform_eqsf2(a, b) return !(a == b)
	perform_eqsf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___eqsf2
___eqsf2:
	movel d2,sp@-
	movel sp@(8),d1
	movel sp@(12),sp@-
	movel d1,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	sne d0
	moveq #1,d2
	andl d2,d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_nesf2
#if 0
SItype __nesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value != 0 iff a != b.  */
#define perform_nesf2(a, b) return a != b
	perform_nesf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___nesf2
___nesf2:
	movel d2,sp@-
	movel sp@(8),d1
	movel sp@(12),sp@-
	movel d1,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	sne d0
	moveq #1,d2
	andl d2,d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_gtsf2
#if 0
SItype __gtsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value > 0 iff a > b.  */
#define perform_gtsf2(a, b) return a > b
	perform_gtsf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___gtsf2
___gtsf2:
	movel d2,sp@-
	movel sp@(8),d1
	movel sp@(12),sp@-
	movel d1,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	sgt d0
	moveq #1,d2
	andl d2,d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_gesf2
#if 0
SItype __gesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value >= 0 iff a >= b.  */
#define perform_gesf2(a, b) return (a >= b) - 1
	perform_gesf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___gesf2
___gesf2:
	movel d2,sp@-
	movel sp@(8),d1
	movel sp@(12),sp@-
	movel d1,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	sge d0
	moveq #1,d2
	andl d2,d0
	subql #1,d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_ltsf2
#if 0
SItype __ltsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value < 0 iff a < b.  */
#define perform_ltsf2(a, b) return -(a < b)
	perform_ltsf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___ltsf2
___ltsf2:
	movel d2,sp@-
	movel sp@(8),d1
	movel sp@(12),sp@-
	movel d1,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	slt d0
	moveq #1,d2
	andl d2,d0
	negl d0
	movel sp@+,d2
	rts
");
#endif

#ifdef L_lesf2
#if 0
SItype __lesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value <= 0 iff a <= b.  */
#define perform_lesf2(a, b) return 1 - (a <= b);	/* note bug fix from libgcc1.c */
	perform_lesf2(FLOATIFY(a), FLOATIFY(b));
}
#endif

asm("	.text
	.even
.globl ___lesf2
___lesf2:
	movel d2,sp@-
	movel sp@(8),d0
	moveq #1,d2
	movel sp@(12),sp@-
	movel d0,sp@-
	jbsr ___cmpsf2
	addqw #8,sp
	tstl d0
	jgt L20
	moveq #0,d2
L20:
	movel d2,d0
	movel sp@+,d2
	rts
");
#endif
#endif /* # if 0 to ensure that the above functions are not compiled */


#ifdef L_fxussfsi
#include <limits.h>
unsigned SItype __fixunssfsi(float a)
{
	if (a >= ((float) LONG_MAX) + 1)
		return (SItype) (a + LONG_MIN) - LONG_MIN;
	return (SItype) a;
}
#endif

#ifdef L_gccbcmp

/* Like bcmp except the sign is meaningful.
   Reult is negative if S1 is less than S2,
   positive if S1 is greater, 0 if S1 and S2 are equal.  */

int __gcc_bcmp(s1, s2, size)
unsigned char *s1,
*s2;
size_t size;
{
	while (size > 0)
	{
		unsigned char c1 = *s1++,
			c2 = *s2++;

		if (c1 != c2)
			return c1 - c2;
		size--;
	}
	return 0;
}

#endif
