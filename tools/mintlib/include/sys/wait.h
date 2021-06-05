#ifndef _WAIT_H
#define _WAIT_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifndef _TYPES_H
#include <types.h>
#endif

#ifndef _POSIX_SOURCE
#ifndef _RESOURCE_H
#include <resource.h>
#endif
#endif /* _POSIX_SOURCE */

#ifdef __cplusplus
extern "C" {
#endif

struct __wait {
#ifndef __MSHORT__
	unsigned	junk:16;	/* padding to make it 32 bits */
#endif
	unsigned	retcode:8;
	unsigned	coredump:1;
	unsigned	termsig:7;
};

union wait {
	struct __wait 	_w;
	int		_i;
};

union __waitp {
	int *__wi;
	union wait *__wu;
};
typedef union wait __union_wait_t;

/* Allow W* to get parameter in POSIX-Style (int) or BSD-Style (union wait)*/
#ifdef _EXPERIMENTAL_WAIT_MACROS
#define __W(x)  ({union{typeof(x) __in; __union_wait_t __out;}__wu; \
		__wu.__in=(x); __wu.__out; })
#define __WP	union __waitp		
#else		
#define __W(x)		(*(__union_wait_t *) &(x))
#define __WP	int *
#endif

#define w_termsig	_w.termsig
#define w_stopsig	_w.retcode
#define w_coredump	_w.coredump
#define w_retcode	_w.retcode

/* I don't know if this next one is right or not */
#define w_status	_i

#define __WSTOPPED	0177	/* fake "signal" for stopped processes */

#ifndef _POSIX_SOURCE
#define WSTOPPED __WSTOPPED
#endif

#define WIFSIGNALED(x)	(__W(x)._w.termsig != __WSTOPPED && __W(x)._w.termsig != 0)
#define WIFSTOPPED(x)	(__W(x)._w.termsig == __WSTOPPED)
#define WIFEXITED(x)	(__W(x)._w.termsig == 0)
#define WIFCOREDUMPED(x) (__W(x)._w.coredump != 0)

#define WSTOPSIG(x)	(__W(x)._w.retcode)
#define WTERMSIG(x)	(__W(x)._w.termsig)
#define WEXITSTATUS(x)	(__W(x)._w.retcode)

#define WNOHANG		1
#define WUNTRACED	2

__EXTERN pid_t wait	__PROTO((__WP status));
#ifndef _POSIX_SOURCE
__EXTERN pid_t wait3	__PROTO((union wait *status, int mode, struct rusage *rusage));
__EXTERN pid_t wait4 __PROTO((pid_t pid, union wait *status, int options, struct rusage *rusage));
#endif /* _POSIX_SOURCE */
__EXTERN pid_t waitpid	__PROTO((pid_t pid, __WP status, int options));

#ifdef __cplusplus
}
#endif

#endif
