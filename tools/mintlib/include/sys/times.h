/* structure for the times() system call */

#ifndef _TIMES_H
#define _TIMES_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TIME_T
#define _TIME_T long
typedef _TIME_T	time_t;
#endif /* _TIME_T */

struct tms {
	time_t	tms_utime;
	time_t	tms_stime;
	time_t	tms_cutime;
	time_t	tms_cstime;
};


__EXTERN long	times	__PROTO((struct tms *));

#ifdef __cplusplus
}
#endif

#endif /* _TIMES_H */
