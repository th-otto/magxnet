/* original from norbert schelenkar's stdio */
/* eff hacks	++jrb */
/* conversion to ansi spec -- mj */
/* 
 * Base can be anything between 2 and 36 or 0.
 * If not NULL then resulting *endptr points to the first
 * non-accepted character.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

/* macro to avoid most frequent long muls on a lowly 68k */
#define _BASEMUL(B, SH, X) \
    ((0 != SH) ? \
       ((10 == (B)) ? ((((X) << (SH)) + (X)) << 1) : ((X) << (SH))) : \
       ((X) * (B)))

unsigned long int strtoul(nptr, endptr, base)
register const char *nptr;
char **endptr;
int base;
{
	register short c;
	unsigned long result = 0L;
	unsigned long limit;
	short negative = 0;
	short overflow = -2;				/* if this stays negative then
										   no conversion was performed */
	short digit;
	int shift;
	unsigned long top = ULONG_MAX;

	if (endptr != NULL)
		*endptr = (char *) nptr;

	do
	{									/* skip leading white space */
		c = *nptr++;
	} while (isspace(c));

	if (c == '+')						/* handle signs */
		c = *nptr++;
	else if (c == '-')
	{
		negative = 1;
		c = *nptr++;
	}

	if ((base == 0) || (base == 16))
	{									/* discard 0x/0X prefix if hex */
		if (c == '0')
		{
			if (*nptr == 'x' || *nptr == 'X')
			{
				if (endptr != NULL)
					*endptr = (char *) nptr;
				nptr += 1;
				c = *nptr++;
				base = 16;
			}
		}
	}
	if (base == 0)						/* determine base if unknown */
		base = (c == '0') ? 8 : 10;		/* don't skip leading 0 if present */
	else if (base < 2 || base > 36)
		return result;

	limit = top / (unsigned long) base;	/* ensure no overflow */
	shift = 0;
	switch (base)
	{
	case 32:
		shift++;
	case 16:
		shift++;
	case 8:
		shift++;
	case 4:
	case 10:
		shift++;
	case 2:
		shift++;
	default:;
	}

	do
	{									/* convert the number */
		if (isdigit(c))
			digit = c - '0';
		else if (isalpha(c))
			digit = c - (isupper(c) ? 'A' : 'a') + 10;
		else
			break;
		if (digit >= base)
			break;
		if (0 == (overflow &= 1))
		{								/* valid digit
										   - some conversion performed */
			if ((result > limit) || ((unsigned long) digit > (result = _BASEMUL(base, shift, result), top - result)))
			{
				result = top;
				errno = ERANGE;
				overflow = 1;
				negative = 0;
			} else
				result += digit;
		}
	} while ((c = *nptr++) != 0);

	if (negative)						/* ANSI says we should do this! */
		result = 0L - result;

	if ((endptr != NULL) && (overflow >= 0))	/* move *endptr if some */
		*endptr = (char *) nptr - 1;	/* digits were accepted */
	return result;
}
