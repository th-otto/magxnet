#ifndef _TIMEB_H
#define _TIMEB_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct timeb {
	long		time;		/* seconds since Jan 1., 1970 */
	short		millitm;	/* milliseconds since "time" */
	short		timezone;	/* minutes west of GMT */
	short		dstflag;	/* if time zone can have DST */
};

__EXTERN int	ftime	__PROTO((struct timeb *));

#ifdef __cplusplus
}
#endif

#endif
