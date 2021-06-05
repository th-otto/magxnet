#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "lib.h"

/*
 * %efg were loosing big time
 *	fixed  ++jrb
 * all floating conversion now done by atof. much is gained by this.
 *	++jrb
 *
 * hacked to use stdarg by bm
 */

#ifndef __NO_FLOAT__
#define FLOATS 1
#endif

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define	skip() \
  if (c == EOF) goto done ; \
  while (isspace(c)) { charcnt++; c = (*get)(ip); if (c == EOF) goto done; }
#define TEN_MUL(X)	((((X) << 2) + (X)) << 1)

#if FLOATS
/* fp scan actions */
#define F_NADA	0						/* just change state */
#define F_SIGN	1						/* set sign */
#define F_ESIGN	2						/* set exponent's sign */
#define F_INT	3						/* adjust integer part */
#define F_FRAC	4						/* adjust fraction part */
#define F_EXP	5						/* adjust exponent part */
#define F_QUIT	6

#define NSTATE	8
#define FS_INIT		0					/* initial state */
#define FS_SIGNED	1					/* saw sign */
#define FS_DIGS		2					/* saw digits, no . */
#define FS_DOT		3					/* saw ., no digits */
#define FS_DD		4					/* saw digits and . */
#define FS_E		5					/* saw 'e' */
#define FS_ESIGN	6					/* saw exp's sign */
#define FS_EDIGS	7					/* saw exp's digits */

#define FC_DIG		0
#define FC_DOT		1
#define FC_E		2
#define FC_SIGN		3

/* given transition,state do what action? */
static const int fp_do[][NSTATE] = {
	{F_INT, F_INT, F_INT,
	 F_FRAC, F_FRAC,
	 F_EXP, F_EXP, F_EXP},				/* see digit */
	{F_NADA, F_NADA, F_NADA,
	 F_QUIT, F_QUIT, F_QUIT, F_QUIT, F_QUIT},	/* see '.' */
	{F_QUIT, F_QUIT,
	 F_NADA, F_QUIT, F_NADA,
	 F_QUIT, F_QUIT, F_QUIT},			/* see e/E */
	{F_SIGN, F_QUIT, F_QUIT, F_QUIT, F_QUIT,
	 F_ESIGN, F_QUIT, F_QUIT},			/* see sign */
};

/* given transition,state what is new state? */
static const int fp_ns[][NSTATE] = {
	{FS_DIGS, FS_DIGS, FS_DIGS,
	 FS_DD, FS_DD,
	 FS_EDIGS, FS_EDIGS, FS_EDIGS},		/* see digit */
	{FS_DOT, FS_DOT, FS_DD,
	 },									/* see '.' */
	{0, 0,
	 FS_E, 0, FS_E,
	 },									/* see e/E */
	{FS_SIGNED, 0, 0, 0, 0,
	 FS_ESIGN, 0, 0},					/* see sign */
};

/* which states are valid terminators? */
static const int fp_sval[NSTATE] = {
	0, 0, 1, 0, 1, 1, 1, 1
};
#endif

#ifdef __STDC__
int _scanf(register FILE * ip, int (*get)(FILE *), int(*unget)(int, FILE *), const char *_fmt, va_list args)
#else
int _scanf(ip, get, unget, _fmt, args)
FILE *ip;
int (*get) __PROTO( (FILE *));
int (*unget) __PROTO( (int, FILE *));
const char *_fmt;
char **args;
#endif
{
	register long n;
	register int c,
	 width,
	 lval,
	 sval,
	 cnt = 0,
		charcnt = 1;

#if defined (PRINTF_LONGLONG) || defined (__STDC__)
	register int llval = 0;
#endif
#ifdef PRINTF_LONGLONG
	register long long lln;
#endif
	int store,
	 neg,
	 base,
	 endnull,
	 c2;
	register unsigned char *p = 0;
	const unsigned char *fmt = (const unsigned char *) _fmt;
	char delim[256];
	char const *q;

#if FLOATS
	double fx;
	char fbuf[128],
	*fbp;
	int fstate,
	 trans;
	extern double atof __PROTO((const char *));
#endif

	c = (*get) (ip);
	while (*fmt)
	{
		if (*fmt == '%')
		{
			width = -1;
			lval = FALSE;
#if defined (PRINTF_LONGLONG) || defined (__STDC__)
			llval = FALSE;
#endif
			sval = FALSE;
			store = TRUE;

			if (*++fmt == '*')
			{
				store = FALSE;
				++fmt;
			}

			if (isdigit(*fmt))			/* width digit(s) */
			{
				width = *fmt++ - '0';
				while (isdigit(*fmt))
					width = TEN_MUL(width) + *fmt++ - '0';
			}

		  fmtnxt:
			switch (*fmt++)
			{
			case 'l':					/* long data */
#ifdef PRINTF_LONGLONG
				if (lval)
					llval = TRUE;
#endif
				lval = TRUE;
				goto fmtnxt;

#ifdef __STDC__
			case 'L':					/* long double */
				llval = TRUE;
				goto fmtnxt;
#endif

			case 'h':					/* short data (for compatibility) */
				sval = TRUE;
				goto fmtnxt;

			case 'i':					/* any-base numeric */
				base = 0;
				neg = -1;
				goto numfmt;

			case 'b':					/* unsigned binary, non-standard */
			case 'B':					/* non-standard */
				base = 2;
				neg = 0;
				goto numfmt;

			case 'o':					/* unsigned octal */
			case 'O':					/* non-standard */
				base = 8;
				neg = 0;
				goto numfmt;

			case 'p':					/* pointer */
				lval = TRUE;
#ifdef PRINTF_LONGLONG
				llval = FALSE;
#endif
				/* fall through */

			case 'x':					/* unsigned hexadecimal */
			case 'X':					/* non-standard */
				base = 16;
				neg = 0;
				goto numfmt;

			case 'd':					/* SIGNED decimal */
			case 'D':					/* non-standard */
				base = 10;
				neg = -1;
				goto numfmt;

			case 'u':					/* unsigned decimal */
			case 'U':					/* non-standard */
				base = 10;
				neg = 0;
			  numfmt:
				skip();
				memset(delim, -1, sizeof(delim));
				if (isupper(fmt[-1]))
				{
					/* non-standard */
#ifdef PRINTF_LONGLONG
					if (lval)
						llval = TRUE;
#endif
					lval = TRUE;
				}
				n = 0;
#ifdef PRINTF_LONGLONG
				lln = 0;
#endif
				if (width == 0)
					goto savnum;
				if (!base)
				{
					if (c == '%')		/* non-standard */
					{
						base = 2;
						neg = 0;
						goto skip1;
					} else if (c == '0')
					{
						if (--width == 0)
							goto savnum;
						charcnt++;
						c = (*get) (ip);
						if (c == EOF)
							goto savnum;
						if ((c != 'x') && (c != 'X'))
						{
							base = 8;
							neg = 0;
							for (c2 = 0; c2 < 8; c2++)
								delim[c2 + '0'] = c2;
							goto zeroin;
						}
						base = 16;
						neg = 0;
						goto skip1;
					} else
						base = 10;
				}

				/* Check for 0x prefix */
				if (base == 16 && c == '0')
				{
					if (--width == 0)
						goto savnum;
					charcnt++;
					c = (*get) (ip);
					if (c == EOF)
						goto savnum;
					if (c == 'x' || c == 'X')
						goto skip1;
					/* else unget the character, it may already be a
					   non-digit and the number would be rejected */
					charcnt--;
					width++;
					(*unget) (c, ip);
					c = '0';
				} else if (neg == -1)
				{
					neg = c == '-';
					if (neg || c == '+')
					{
					  skip1:
						if (--width == 0)
							goto done;
						charcnt++;
						c = (*get) (ip);
						if (c == EOF)
							goto done;
					}
				}

				/* delim[c] -> value of c or -1 */
				p = (unsigned char *) "FEDCBAfedcba9876543210";
				q = "\17\16\15\14\13\12\17\16\15\14\13\12\11\10\7\6\5\4\3\2\1\0";
				if (base < 16)
				{
					/* skip invalid digits */
					p += 22 - base;
					q += 22 - base;
				}
				while (*p)
					delim[*p++] = *q++;

				if (delim[c] == (char) -1)
					goto done;

				while (width--)
				{
#ifdef PRINTF_LONGLONG
					if (llval)
						lln = (lln * base) + delim[c];
					else
#endif
						n = (n * base) + delim[c];
					charcnt++;
					c = (*get) (ip);
					if (c == EOF)
						break;
				  zeroin:
					if (delim[c] == (char) -1)
						break;
				}
			  savnum:
				if (store)
				{
#ifdef __STDC__
					p = va_arg(args, void *);
#else
					p = ((unsigned char *) *args);
#endif
#ifdef PRINTF_LONGLONG
					if (llval)
					{
						if (neg)
							lln = -lln;
						*((long long *) p) = lln;
					} else
#endif
					{
						if (neg)
							n = -n;
						if (lval)
							*((long *) p) = n;
						else if (sval)
							*((short *) p) = (short) n;
						else
							*((int *) p) = (int) n;
						++cnt;
					}
				}
				break;

#if FLOATS
			case 'E':					/* non-standard */
			case 'F':
			case 'G':
				lval = TRUE;
				/* fall through */

			case 'e':					/* float */
			case 'f':
			case 'g':
				skip();

				memset(delim, -1, sizeof(delim));
				for (c2 = '0'; c2 <= '9'; c2++)
					delim[c2] = FC_DIG;
				delim['.'] = FC_DOT;
				delim['+'] = delim['-'] = FC_SIGN;
				delim['e'] = delim['E'] = FC_E;

				fstate = FS_INIT;
				fbp = fbuf;
				while (c != EOF && width--)
				{
					trans = delim[c];
					if (trans == (char) -1)
						break;
					if (fbp - fbuf + 1 < sizeof(fbuf))
						*fbp++ = c;

					if (fp_do[trans][fstate] == F_QUIT)
						break;
					fstate = fp_ns[trans][fstate];
					charcnt++;
					c = (*get) (ip);
				}

				*fbp = '\0';
				if (!fp_sval[fstate])
					goto done;
				if (store)
				{
					fx = (*fbuf == '\0') ? 0.0 : atof(fbuf);
#ifdef __STDC__
					p = va_arg(args, void *);
#else
					p = (unsigned char *) *args;
#endif
#ifdef __STDC__
					/* partial support for long double */
#ifdef __M68881__						/* currently only with m68881 */
					if (llval)
						*(long double *) p = (long double) fx;
					else
#endif
#endif
					if (lval)
						*((double *) p) = fx;
					else
						*((float *) p) = (float) fx;
					++cnt;
				}
				break;
#endif

			case 'n':
				if (store)
				{
#ifdef __STDC__
					p = va_arg(args, void *);
#else
					p = (unsigned char *) *args;
#endif
					/* Compensate for lookahead */
					*((int *) p) = charcnt - 1;
				}
				break;

			case 'c':					/* character data */
				if (width == -1)
					width = 1;
				endnull = FALSE;
				memset(delim, 0, sizeof(delim));
				if (c == EOF)
					goto done;
				goto strproc;

			case '[':					/* string w/ delimiter set */
				endnull = TRUE;

				/* get delimiters */
				neg = FALSE;
				if (*fmt == '^')
				{
					fmt++;
					neg = TRUE;
				}

				memset(delim, !neg, sizeof(delim));

				if ((*fmt == ']') || (*fmt == '-'))
				{
					delim[*fmt++] = neg;
				}

				while (c2 = *fmt++, c2 != ']')
				{
					if (c2 == '\0')
						break;
					if (*fmt == '-' && fmt[1] && fmt[1] != ']')
					{
						while (c2 <= fmt[1])
							delim[c2++] = neg;
						fmt += 2;
					} else
						delim[c2] = neg;
				}

				goto strproc;

			case 's':					/* string data */
				skip();
				memset(delim, 0, sizeof(delim));
				delim['\t'] = 1;
				delim['\n'] = 1;
				delim['\v'] = 1;
				delim['\f'] = 1;
				delim['\r'] = 1;
				delim[' '] = 1;
				endnull = TRUE;
			  strproc:
				/* process string */
#ifdef __STDC__
				if (store)
					p = va_arg(args, void *);
#else
				p = ((unsigned char *) *args);
#endif

				/* if the 1st char fails, match fails */
				if (width)
				{
					if (c == EOF || delim[c])
					{
						if (endnull)
							if (store)
								*p = '\0';
						goto done;
					}
				}

				for (;;)				/* FOREVER */
				{
					if (store)
						*p++ = c;
					charcnt++;
					c = (*get) (ip);
					if (c == EOF || --width == 0)
						break;
					if (delim[c])
						break;
				}

				if (store)
				{
					if (endnull)
						*p = '\0';
					++cnt;
				}
				break;

			case '%':
				--fmt;
				goto cmatch;

			default:
				goto done;
			}
		} else if (isspace(*fmt))		/* skip whitespace */
		{
			fmt++;
			skip();
		} else
		{								/* normal match char */
		  cmatch:
			if (c != *fmt++)
				goto done;
			charcnt++;
			c = (*get) (ip);
		}

#ifdef __STDC__
		/* nothing to do */
#else
		if (store)
			args++;
#endif
	}

  done:								/* end of scan */
	if (c != EOF)
		(*unget) (c, ip);
	if (c == EOF && cnt == 0)
		return (EOF);
#if defined (PRINTF_LONGLONG) || defined (__STDC__)
	(void)llval;
#endif
	return (cnt);
}
