#ifdef __GNUC__
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <mint/mintbind.h>
#include <arpa/inet.h>
#undef ENOSYS
#define ENOSYS 32

#undef __set_errno
#define __set_errno(e) (errno = (e))

#if 1
extern const unsigned char *_ctype;
#define	_IScntrl	0x01		/* control character */
#define	_ISdigit	0x02		/* numeric digit */
#define	_ISupper	0x04		/* upper case */
#define	_ISlower	0x08		/* lower case */
#define	_ISspace	0x10		/* whitespace */
#define	_ISpunct	0x20		/* punctuation */
#define	_ISxdigit	0x40		/* hexadecimal */
#define _ISblank	0x80		/* blank */
#define _ISgraph	0x100		/* graph */
#define _ISprint	0x200		/* print */
#undef isdigit
#define	isdigit(c)	(_ctype[(unsigned char)((c))]&_ISdigit)
#undef isxdigit
#define	isxdigit(c)	(_ctype[(unsigned char)((c))]&_ISxdigit)
#undef islower
#define	islower(c)	(_ctype[(unsigned char)((c))]&_ISlower)
#undef isspace
#define	isspace(c)	(_ctype[(unsigned char)((c))]&_ISspace)
#undef isascii
#define	isascii(c)	!((c)&~0x7F)
#endif

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif


/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
in_addr_t inet_addr(const char *cp)
{
	struct in_addr val;

	if (inet_aton(cp, &val))
		return val.s_addr;
	return INADDR_NONE;
}


/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
in_addr_t inet_aton(const char *cp, struct in_addr *addr)
{
	in_addr_t val;
	unsigned long base;
	char c;
	unsigned long parts[4];
	unsigned long *pp = parts;
	size_t n;

	for (;;)
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		val = 0;
		base = 10;
		if (*cp == '0')
		{
			++cp;
			if (*cp == 'x' || *cp == 'X')
				base = 16, ++cp;
			else
				base = 8;
		}
		while ((c = *cp) != '\0')
		{
			if (isascii(c) && isdigit(c))
			{
				val = (val * base) + (c - '0');
				++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c))
			{
				val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
				++cp;
			} else
				break;
		}
		if (*cp == '.')
		{
			/*
			 * Internet format:
			 *  a.b.c.d
			 *  a.b.c   (with c treated as 16 bits)
			 *  a.b (with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return 0;
			if (val > 0xff)
				return 0;
			*pp++ = val;
			++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp != '\0' && (!isascii(*cp) || !isspace(*cp)))
		return 0;
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch ((int)n)
	{
	case 1:							/* a -- 32 bits */
		break;

	case 2:							/* a.b -- 8.24 bits */
		if (val > 0xffffffUL)
			return 0;
		val |= parts[0] << 24;
		break;

	case 3:							/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:							/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return 0;
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);

	return 1;
}
