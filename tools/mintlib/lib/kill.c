/* kill() for MiNT */

#include <stdlib.h>
#include <errno.h>
#include <osbind.h>
#include <signal.h>
#include <unistd.h>
#include <mintbind.h>


/* vector of signal handlers (for TOS) */
extern __Sigfunc _sig_handler[];		/* in signal.c */

/* vector giving which signals are currently blocked from delivery (for TOS) */
extern long _sigmask;					/* in signal.c */

/* which signals are pending? */
extern long _sigpending;

short _have_pkill = 1;

int kill(pid, sig)
int pid,
	sig;
{
	long r;
	__Sigfunc hndle;

	if (_have_pkill)
	{
		r = Pkill(pid, sig);
		if (r == -ENOSYS)
			_have_pkill = 0;
		else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		} else
			return 0;
	}
	/* fall through to TOS emulation */

	if (sig < 0 || sig >= NSIG || (pid && pid != getpid()))
	{
		errno = ERANGE;
		return -1;
	}
	hndle = _sig_handler[sig];
	if (hndle == SIG_IGN)
		return 0;
	/* check here for masked signals */
	else if (sig != SIGKILL && (_sigmask & (1L << sig)))
		_sigpending |= (1L << sig);
	else
	{
		_sigpending &= ~(1L << sig);
		if (hndle == SIG_DFL)
		{
			switch (sig)
			{
			case SIGCONT:
			case SIGCHLD:
				return 0;
			default:
				exit(sig << 8);
			}
		} else
		{
			(*hndle) (sig);
		}
	}

	return 0;
}
