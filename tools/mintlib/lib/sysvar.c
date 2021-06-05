#include <support.h>
#include <osbind.h>
#include <mintbind.h>
#include <ssystem.h>

long get_sysvar(var)
void *var;
{
	long ret;
	long save_ssp;

	if (Ssystem(-1, NULL, NULL))
	{
		save_ssp = (long) Super((void *) 0L);
		/* note: dont remove volatile, otherwise gcc will reorder these
		   statements and we get bombs */
		ret = *((volatile long *) var);
		(void) Super((void *) save_ssp);
		return ret;
	} else
		return Ssystem(S_GETLVAL, var, NULL);
}

void set_sysvar_to_long(var, val)
void *var;
long val;
{
	long save_ssp;

	if (Ssystem(-1, NULL, NULL))
	{
		save_ssp = (long) Super((void *) 0L);
		*((volatile long *) var) = val;
		(void) Super((void *) save_ssp);
	} else
		(void) Ssystem(S_SETLVAL, var, val);	/* note: root only! */
}
