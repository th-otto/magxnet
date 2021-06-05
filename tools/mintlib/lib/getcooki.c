/*
 * Getcookie for MiNTlib
 * 
 * Author: jerry g geiger
 * 
 * - Ssystem extension Draco
 * - corrected by Frank Naumann, 1998-09-09
 * 
 * returns:
 * 
 * 	- E_OK if cookie was found
 * 	- negative error number if cookie is missing
 * 	
 * 	if p_value is set, cookie value is copied to *p_value
 * 
 */

#include <osbind.h>
#include <mintbind.h>
#include <ssystem.h>
#include <errno.h>

static long cookie_id;
static long cookie_value;

static long getcookieptr(void)
{
	long *cookieptr = *((long **) 0x5a0);
	if (cookieptr)
	{
		while (*cookieptr)
		{
			if (*cookieptr == cookie_id)
			{
				return cookieptr[1];
			}

			cookieptr += 2;
		}
	}
	cookie_id = 0;
	return 0;
}

int Getcookie(long cookie, long *p_value)
{

	if (Ssystem(-1, 0, 0))				/* better a global _has_ssystem? */
	{
		/* old method */

		cookie_id = cookie;
		cookie_value = Supexec(getcookieptr);

	} else
	{
		/* Ssystem supported, use it */

		cookie_value = Ssystem(S_GETCOOKIE, cookie, 0);

		if (cookie_value == -1)					/* not found */
		{
			cookie_value = cookie_id = 0;
		} else
		{
			cookie_id = 1;
		}
	}
	if (p_value)
		*p_value = cookie_value;

	if (cookie_id)
		return E_OK;
	return EERROR;
}
