#ifndef _RESOURCE_H
#define _RESOURCE_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RLIM_INFINITY	0x7fffffffL

#define RLIMIT_CPU	1		/* max cpu time allowed */
#define RLIMIT_RSS	2		/* max memory allowed (total) */
#define RLIMIT_DATA	3		/* max malloc'd memory allowed */
#define RLIMIT_STACK	4		/* max stack size (not used) */
#define RLIMIT_FSIZE	5		/* max file size (not used) */
#define RLIMIT_CORE	6		/* max core file size (not used) */

#define RLIM_NLIMITS	6

#define RUSAGE_SELF	0
#define RUSAGE_CHILDREN	1

#ifndef _COMPILER_H
#include <compiler.h>
#endif

struct rusage {
	struct timeval	ru_utime;	/* user time used */
	struct timeval	ru_stime;	/* system time used */
/* The following rusage elements are fake.  They will all contain 0 or
   some other fake value until such time as they are supported under MiNT.
*/
	long		ru_maxrss;	/* maximum resident set size */
	long		ru_ixrss;	/* integral shared memory size */
	long		ru_idrss;	/* integral unshared data size */
	long		ru_isrss;	/* integral unshared stack size */
	long		ru_minflt;	/* page reclaims */
	long		ru_majflt;	/* page faults (requiring I/O) */
	long		ru_nswap;	/* memory swaps */
	long		ru_inblock;	/* block input operations */
	long		ru_oublock;	/* block output operations */
	long		ru_msgsnd;	/* messages sent */
	long		ru_msgrcv;	/* messages received */
	long		ru_nsignals;	/* signals received */
	long		ru_nvcsw;	/* voluntary context switches */
	long		ru_nivcsw;	/* involuntary context switches */
};

struct rlimit {
	long rlim_cur;
	long rlim_max;
};

__EXTERN int	setrlimit	__PROTO((int mode, struct rlimit *rl));
__EXTERN int	getrlimit	__PROTO((int mode, struct rlimit *rl));
__EXTERN int	getrusage	__PROTO((int which, struct rusage *r));

#ifdef __cplusplus
}
#endif

#endif /* _RESOURCE_H */
