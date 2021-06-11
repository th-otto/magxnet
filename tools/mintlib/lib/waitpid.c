/* waitpid() emulation for MiNT, by Howard Chu. From wait3.c by Eric R. Smith
 */

#include <types.h>
#include <wait.h>
#include <time.h>
#include <resource.h>
#include <mintbind.h>
#include <errno.h>
#include <signal.h>

extern long __waitval;					/* in thread.c */

pid_t waitpid(pid, _status, options)
pid_t pid;
__WP _status;
int options;
{
	long r;
	int exit_status,
	 sig_term;
	union wait *statwait;

#ifdef _EXPERIMENTAL_WAIT_MACROS
	int *status = _status.__wi;
#else
	int *status = _status;
#endif

	statwait = (union wait *) status;

	r = Pwaitpid(pid, options, 0L);

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
	if (pid)
	{
		if (statwait)
		{
			statwait->_i = 0;

			if (((short) r) == -32)
			{
				sig_term = SIGINT;
				exit_status = 0;
			} else
			{
				exit_status = (int) (r & 0x000000ffL);
				sig_term = (int) ((r & 0x00007f00L) >> 8);
			}
			if (sig_term >= NSIG)
				sig_term = 0;
			if (sig_term && exit_status != 0 && exit_status != 0177)
				sig_term = 0;
			if (exit_status == 0177 && sig_term)
			{
				statwait->w_termsig = WSTOPPED;
				statwait->w_stopsig = sig_term;
			} else
			{
				statwait->w_termsig = sig_term;
				statwait->w_retcode = exit_status;
			}
		}
	}
	return pid;
}
