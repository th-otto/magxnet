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

#ifdef ARP_HACK

extern int no_ssystem;
static long getcookieptr (void)
{
	return *((long *) 0x5a0);
}

int Getcookie (long cookie, long *p_value)
{
	if (no_ssystem)
	{
		/* old method */
		
		long *cookieptr = (long *) Supexec (getcookieptr);
		
		if (cookieptr)
		{
			while (*cookieptr)
			{
				if (*cookieptr == cookie)
				{
					if (p_value)
						*p_value = cookieptr [1];
					
					return E_OK;
				}
				
				cookieptr += 2;
			}
		}
		/* Make sure that P_VALUE is zeroed if the cookie can't
		   be found.  Reported by Tommy Andersen
	   	(tommya@post3.tele.dk).  */
		if (p_value)
			*p_value = 0;
		
		return EERROR;
	}
	else
	{
		/* Ssystem supported, use it */
		int	r;
		long	v = -42;

		/* Make sure that P_VALUE is zeroed if the cookie can't
		   be found.  Reported by Tommy Andersen
		   (tommya@post3.tele.dk).  */
		if (p_value)
			*p_value = 0;
			
		r = (int) Ssystem(S_GETCOOKIE, cookie, &v);
		/*
		 * Backward compatibility for MiNT 1.14.7:
		 * Ssystems() returns cookie value and ignores arg2!!
		 */
		if (r != -1 && v == -42)				
			v = r;
 
		if (r == -1)							/* not found */
		{
			v = 0;
			r = EERROR;
		}
		else
			r = 0;

		if (p_value)
			*p_value = v;

		return r;
		
	}	
}

#else

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

#endif
