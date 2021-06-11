/* sigaction() and sigset functions for MiNT; placed in the public domain */

#include <errno.h>
#include <osbind.h>
#include <mintbind.h>
#include <signal.h>
#include <stddef.h>

/* vector of signal handlers (for TOS, or for MiNT with -mshort) */
extern __Sigfunc _sig_handler[__NSIG];

#ifdef __MSHORT__
typedef void __CDECL(*__KerSigfunc) __PROTO((long, long));
__EXTERN void __CDECL _trampoline __PROTO((long sig, long code));
#else
typedef void __CDECL(*__KerSigfunc) __PROTO((int));
#endif

int sigaction(sig, act, oact)
int sig;
const struct sigaction *act;
struct sigaction *oact;
{
	long r;
	__Sigfunc oldfunc;
	static short have_psigaction = 1;

	if (have_psigaction)
	{
		if (Psigaction(-1, NULL, NULL) == -ENOSYS)
			have_psigaction = 0;
	}

	if (have_psigaction)
	{
		struct ksigact
		{
			__KerSigfunc sa_handler;	/* pointer to signal handler */
			long sa_mask;				/* additional signals masked during delivery */
			short sa_flags;				/* signal specific flags, kernel */
		} kact, koact;

		if (sig < 0 || sig >= __NSIG)
		{
			errno = ERANGE;
			return -1;
		}
#ifdef __MSHORT__
/* NOTE: MiNT passes 32 bit numbers for signals, so we want our
 * own signal dispatcher to switch these to 16 bit ints
 */
		oldfunc = _sig_handler[sig];
#endif
		if (act)
		{
			kact.sa_handler = (__KerSigfunc) act->sa_handler;
			kact.sa_mask = act->sa_mask.__sigset_data[0];
			kact.sa_flags = (short) act->sa_flags;
#ifdef __MSHORT__
			_sig_handler[sig] = (__Sigfunc) kact.sa_handler;
			if (_sig_handler[sig] != SIG_DFL && _sig_handler[sig] != SIG_IGN)
			{
				kact.sa_handler = _trampoline;
			}
#endif
		}
		r = Psigaction(sig, (act ? &kact : 0L), (oact ? &koact : 0L));
		if (r < 0)
		{
			errno = (int) -r;
			return -1;
		}
		if (oact)
		{
			oact->sa_mask.__sigset_data[0] = koact.sa_mask;
			oact->sa_flags = (int) koact.sa_flags;
#ifdef __MSHORT__
			oact->sa_handler =
				((koact.sa_handler == (__KerSigfunc) _trampoline) ? oldfunc : (__Sigfunc) koact.sa_handler);
#else
			oact->sa_handler = (__Sigfunc) koact.sa_handler;
#endif
		}
	} else
	{
		if (act)
			oldfunc = signal(sig, act->sa_handler);
		else
		{
			long omask;

			omask = sigblock(sig);
			oldfunc = signal(sig, SIG_DFL);
			signal(sig, oldfunc);		/* need to put it back */
			sigsetmask(omask);			/* may remask sig (this is what we want) */
		}
		if (oldfunc == SIG_ERR)
			return -1;
		if (oact)
		{
			oact->sa_handler = oldfunc;
			/* we could do something useful with sa_mask when __mint */
			oact->sa_flags = 0;
			oact->sa_mask.__sigset_data[0] = 0;
		}
	}
	return 0;
}

int sigaddset(set, signo)
sigset_t *set;
int signo;
{
	int idx;
	int pos;

	if ((!set) || (signo >= __NSIG))
	{
		errno = ENOSYS;
		return -1;
	}
	idx = _SIGSET_INDEX(signo);
	pos = _SIGSET_BITPOS(signo);
	set->__sigset_data[idx] |= sigmask(pos);
	return 0;
}

int sigdelset(set, signo)
sigset_t *set;
int signo;
{
	int idx;
	int pos;

	if ((!set) || (signo >= __NSIG))
	{
		errno = ENOSYS;
		return -1;
	}
	idx = _SIGSET_INDEX(signo);
	pos = _SIGSET_BITPOS(signo);
	set->__sigset_data[idx] &= ~(sigmask(pos));
	return 0;
}

int sigemptyset(set)
sigset_t *set;
{
	int idx;

	if (!set)
	{
		errno = ENOSYS;
		return -1;
	}
	for (idx = _SIGSET_MAX_INDEX; idx >= 0; idx--)
	{
		set->__sigset_data[idx] = 0UL;
	}
	return 0;
}

int sigfillset(set)
sigset_t *set;
{
	int idx;

	if (!set)
	{
		errno = ENOSYS;
		return -1;
	}
	for (idx = _SIGSET_MAX_INDEX; idx >= 0; idx--)
	{
		set->__sigset_data[idx] = ~0UL;
	}
	return 0;
}

int sigismember(set, signo)
sigset_t *set;
int signo;
{
	int idx;
	int pos;

	if ((!set) || (signo >= __NSIG))
	{
		errno = ENOSYS;
		return -1;
	}
	idx = _SIGSET_INDEX(signo);
	pos = _SIGSET_BITPOS(signo);
	return (set->__sigset_data[idx] & sigmask(pos)) ? 1 : 0;
}

int sigpending(set)
sigset_t *set;
{
	if (!set)
	{
		errno = ENOSYS;
		return -1;
	}
	(void) sigemptyset(set);
	set->__sigset_data[0] = Psigpending();
	return 0;
}

int sigprocmask(how, set, oset)
int how;
const sigset_t *set;
sigset_t *oset;
{
	int rv = 0;
	long omask = 0L;

	if (!set)
	{
		errno = ENOSYS;
		return -1;
	}
	switch (how)
	{
	case SIG_BLOCK:
		omask = Psigblock(set->__sigset_data[0]);
		break;
	case SIG_UNBLOCK:
		omask = Psigblock(0L);
		(void) Psigsetmask(omask & ~(set->__sigset_data[0]));
		break;
	case SIG_SETMASK:
		omask = Psigsetmask(set->__sigset_data[0]);
		break;
	default:
		errno = ENOSYS;
		rv = -1;
	}
	if (oset)
		oset->__sigset_data[0] = omask;
	return rv;
}

int sigsuspend(signalmask)
const sigset_t *signalmask;
{
	Psigpause(signalmask->__sigset_data[0]);
	errno = EINTR;
	return -1;
}
