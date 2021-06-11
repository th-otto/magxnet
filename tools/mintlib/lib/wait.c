/*
 * wait.c: a version of wait() for MiNT
 */

#include <types.h>
#include <wait.h>
#include <errno.h>
#include <signal.h>
#include <osbind.h>
#include <mintbind.h>

/* under TOS, vfork() puts its result in __waitval */
extern long __waitval;					/* in thread.c */

pid_t wait(_status)
__WP _status;
{
	long r;
	int exit_status,
	 sig_term,
	 pid;

#ifdef _EXPERIMENTAL_WAIT_MACROS
	int *status = _status.__wi;
#else
	int *status = _status;
#endif

	r = Pwaitpid(-1, 0, 0L);

	if (r == -ENOSYS)
	{
		r = __waitval;
		__waitval = -ENOENT;
	}

	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}

	pid = (int) ((r & 0xffff0000L) >> 16);
	exit_status = (int) (r & 0x000000ffL);
	sig_term = (int) ((r & 0x00007f00L) >> 8);
	if (sig_term >= NSIG)
		sig_term = 0;
	if (status)
	{
		*status = (exit_status << 8) | sig_term;
	}
	return pid;
}

#ifdef TEST
/* small test for parameter of wait */
void a()
{
	union wait u;
	int i;

	wait(&u);
	wait(&i);
}
#endif
