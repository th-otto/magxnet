/* wait4() emulation for MiNT, by Andreas Schwab.  From wait3.c by
   Eric R. Smith */

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

/* in getrusage.c */
__EXTERN void _ms2tval __PROTO((unsigned long, struct timeval *));

pid_t wait4(pid, status, options, rusage)
pid_t pid;
union wait *status;
int options;
struct rusage *rusage;
{
	long r,
	 rsc[8];
	int exit_status,
	 sig_term;

	r = Pwaitpid(pid, options, rsc);

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

			if ((short) r == -32)
			{
				sig_term = SIGINT;
				exit_status = 0;
			} else
			{
				exit_status = (int) r & 0xff;
				sig_term = ((int) r & 0x7f00) >> 8;
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
			_bzero(rusage, (unsigned long) sizeof(struct rusage));
			_ms2tval(rsc[0], &rusage->ru_utime);
			_ms2tval(rsc[1], &rusage->ru_stime);
		}
	}
	return pid;
}
