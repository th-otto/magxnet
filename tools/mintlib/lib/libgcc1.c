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

#ifndef perform_lshrsi3
#define perform_lshrsi3(a, b) return a >> b
#endif

#ifndef perform_lshlsi3
#define perform_lshlsi3(a, b) return a << b
#endif

#ifndef perform_ashrsi3
#define perform_ashrsi3(a, b) return a >> b
#endif

#ifndef perform_ashlsi3
#define perform_ashlsi3(a, b) return a << b
#endif

#ifndef perform_fixsfsi
#define perform_fixsfsi(a) return (SItype) a
#endif

#ifndef perform_floatsisf
#define perform_floatsisf(a)  return INTIFY ((float) a)
#endif

/* Note that eqdf2 returns a value for "true" that is == 0,
   nedf2 returns a value for "true" that is != 0,
   gtdf2 returns a value for "true" that is > 0,
   and so on.  */

#ifndef perform_eqdf2
#define perform_eqdf2(a, b) return !(a == b)
#endif

#ifndef perform_nedf2
#define perform_nedf2(a, b) return a != b
#endif

#ifndef perform_gtdf2
#define perform_gtdf2(a, b) return a > b
#endif

#ifndef perform_gedf2
#define perform_gedf2(a, b) return (a >= b) - 1
#endif

#ifndef perform_ltdf2
#define perform_ltdf2(a, b) return -(a < b)
#endif

#ifndef perform_ledf2
#define perform_ledf2(a, b) return 1 - (a <= b)
#endif

#ifndef perform_eqsf2
#define perform_eqsf2(a, b) return !(a == b)
#endif

#ifndef perform_nesf2
#define perform_nesf2(a, b) return a != b
#endif

#ifndef perform_gtsf2
#define perform_gtsf2(a, b) return a > b
#endif

#ifndef perform_gesf2
#define perform_gesf2(a, b) return (a >= b) - 1
#endif

#ifndef perform_ltsf2
#define perform_ltsf2(a, b) return -(a < b)
#endif

#ifndef perform_lesf2
#define perform_lesf2(a, b) return 1 - (a <= b);
#endif

SItype __lshrsi3(a, b)
unsigned SItype a,
	b;
{
	perform_lshrsi3(a, b);
}

SItype __lshlsi3(a, b)
unsigned SItype a,
	b;
{
	perform_lshlsi3(a, b);
}

SItype __ashrsi3(a, b)
SItype a,
	b;
{
	perform_ashrsi3(a, b);
}

SItype __ashlsi3(a, b)
SItype a,
	b;
{
	perform_ashlsi3(a, b);
}

/* Note that eqdf2 returns a value for "true" that is == 0,
   nedf2 returns a value for "true" that is != 0,
   gtdf2 returns a value for "true" that is > 0,
   and so on.  */

SItype __eqdf2(a, b)
double a,
	b;
{
	/* Value == 0 iff a == b.  */
	perform_eqdf2(a, b);
}

SItype __nedf2(a, b)
double a,
	b;
{
	/* Value != 0 iff a != b.  */
	perform_nedf2(a, b);
}

SItype __gtdf2(a, b)
double a,
	b;
{
	/* Value > 0 iff a > b.  */
	perform_gtdf2(a, b);
}

SItype __gedf2(a, b)
double a,
	b;
{
	/* Value >= 0 iff a >= b.  */
	perform_gedf2(a, b);
}

SItype __ltdf2(a, b)
double a,
	b;
{
	/* Value < 0 iff a < b.  */
	perform_ltdf2(a, b);
}

SItype __ledf2(a, b)
double a,
	b;
{
	/* Value <= 0 iff a <= b.  */
	perform_ledf2(a, b);
}

SItype __fixsfsi(a)
FLOAT_ARG_TYPE a;
{
	union flt_or_value intify;

	perform_fixsfsi(FLOATIFY(a));
}

FLOAT_VALUE_TYPE __floatsisf(a)
SItype a;
{
	union flt_or_value intify;

	perform_floatsisf(a);
}

SItype __eqsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value == 0 iff a == b.  */
	perform_eqsf2(FLOATIFY(a), FLOATIFY(b));
}

SItype __nesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value != 0 iff a != b.  */
	perform_nesf2(FLOATIFY(a), FLOATIFY(b));
}

SItype __gtsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value > 0 iff a > b.  */
	perform_gtsf2(FLOATIFY(a), FLOATIFY(b));
}

SItype __gesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value >= 0 iff a >= b.  */
	perform_gesf2(FLOATIFY(a), FLOATIFY(b));
}

SItype __ltsf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value < 0 iff a < b.  */
	perform_ltsf2(FLOATIFY(a), FLOATIFY(b));
}

SItype __lesf2(a, b)
FLOAT_ARG_TYPE a,
	b;
{
	union flt_or_int intify;

	/* Value <= 0 iff a <= b.  */
	perform_lesf2(FLOATIFY(a), FLOATIFY(b));
}
