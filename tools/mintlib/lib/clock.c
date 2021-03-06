/* _clock -- return process time used so far, in units of CLK_TCK ticks
   per second (under TOS, 200 per second) */
/* written by ERS */

#include <time.h>
#include <osbind.h>
#include <mintbind.h>
#include <ssystem.h>
#include <sysvars.h>

extern clock_t _starttime;				/* in main.c */

#ifndef ARP_HACK
static clock_t now;

/* this must execute in supervisor mode; it fetches the system variable
 * containing the number of 200HZ ticks since the system was booted
 */


static long getnow(void)
{
	now = *((unsigned long *) 0x4baL);
	return 0;
}
#endif

clock_t _clock(void)
{
#ifdef ARP_HACK
	return get_sysvar((void *)0x000004baL) - _starttime;
#else
	if (Ssystem(-1, NULL, NULL))
	{
		(void) Supexec(getnow);
		return (now - _starttime);
	} else
		return (Ssystem(S_GETLVAL, 0x000004baL, NULL) - _starttime);
#endif
}

/* This next bit of nonsense is temporary...clock() should be fixed! */

#ifdef __GNUC__
asm(".stabs \"_clock\",5,0,0,__clock");	/* dept of clean tricks */
#else /* ! __GNUC__ */
clock_t clock()
{
	return _clock();
}
#endif /* ! __GNUC__ */
