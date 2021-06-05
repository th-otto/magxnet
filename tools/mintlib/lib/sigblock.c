/* signal blocking stuff for MiNT */

#include <signal.h>
#include <errno.h>
#include <mintbind.h>
#include <unistd.h>

extern long _sigpending,
 _sigmask;

static void checksigs __PROTO((void));

/* checksigs: for TOS: this checks to see if we just unmasked any
 * pending signals
 */

static void checksigs()
{
	register int sig;
	register long send;

	if ((send = (_sigpending & ~_sigmask)) != 0)
	{
		for (sig = 0; sig < NSIG; sig++)
		{
			if (send & (1L << sig))
				kill(0, sig);
		}
	}
}

#ifdef __MSHORT__
long
#else
int
#endif
sigsetmask(mask)
long mask;
{
	register long omask;
	long r;

	if ((r = Psigsetmask(mask)) != -EINVAL)
		return r;

	omask = _sigmask;
	_sigmask = mask;
	checksigs();
	return omask;
}

#ifdef __MSHORT__
long
#else
int
#endif
sigblock(mask)
long mask;
{
	register long omask;
	long r;

	if ((r = Psigblock(mask)) != -EINVAL)
		return r;

	omask = _sigmask;
	_sigmask |= mask;
	return omask;
}
