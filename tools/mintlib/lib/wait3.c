/* wait3() emulation for MiNT. written by Eric R. Smith and placed in
   the public domain
 */

#include <types.h>
#include <wait.h>
#include <time.h>
#include <resource.h>
#include <mintbind.h>
#include <errno.h>
#include <signal.h>

extern long __waitval;					/* in thread.c */
extern long __waittime;					/* ditto */

__EXTERN void _bzero __PROTO((void *, unsigned long));

__EXTERN void _ms2tval __PROTO((unsigned long, struct timeval *));

		/* in getrusage.c */

pid_t wait3(status, mode, rusage)
union wait *status;
int mode;
struct rusage *rusage;
{
	long r,
	 rsc[8];
	int exit_status,
	 sig_term,
	 pid;

	r = Pwaitpid(-1, mode, rsc);

	if (r == -ENOSYS)
	{
		r = __waitval;
		__waitval = -ENOENT;
		rsc[0] = __waittime;
		rsc[1] = rsc[4] = 0;
	}

	if (r < 0)
	{
		errno = (int) -r;
		return -1;
	}

	pid = (int) ((r & 0xffff0000L) >> 16);
	if (pid)
	{
		if (status)
		{
			status->_i = 0;

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
				status->w_termsig = WSTOPPED;
				status->w_stopsig = sig_term;
			} else
			{
				status->w_termsig = sig_term;
				status->w_retcode = exit_status;
			}
		}

		if (rusage)
		{
			_bzero(rusage, (unsigned long) (sizeof(struct rusage)));
			_ms2tval(rsc[0], &(rusage->ru_utime));
			_ms2tval(rsc[1], &(rusage->ru_stime));
			/* Kludge so GNU time will configure properly: */
			rusage->ru_nvcsw = 1;
		}
	}
	return pid;
}
