/*
 * uname() emulation by Dave Gymer. In the public domain.
 * Bugs:
 *	The MiNT version stuff is in the release field. According to the GNU
 *	shell utils this is the way SunOS does it, so we put the TOS
 *	version number in the 'version' field (even under MiNT).
 *
 * (Modified slightly by ERS.)
 */

#include <stdlib.h>
#include <unistd.h>
#include <osbind.h>
#include <stdio.h>
#include <string.h>
#include <ssystem.h>
#include <mintbind.h>
#ifdef __TURBOC__
#include <sys\utsname.h>
#else
#include <sys/utsname.h>
#endif

extern int __mint;
__EXTERN int gethostname __PROTO((char *buf, size_t len));

static long _mch;						/* value of the _MCH cookie, if any */
static int tosvers;						/* TOS version number */

/*
 * get operating system information; must execute in supervisor mode
 */

static long getinfo __PROTO((void));
static int no_ssystem;

static long getinfo()
{
	long *cookie,
	*sysbase;

/* get _MCH cookie value */
	if (no_ssystem)
	{
		cookie = *((long **) 0x5a0L);
		if (cookie)
		{
			while (*cookie)
			{
				if (*cookie == 0x5f4d4348L)
				{						/* _MCH */
					_mch = cookie[1];
					break;
				}
				cookie += 2;
			}

		}

	} else
		_mch = Ssystem(S_GETCOOKIE, 0x5f4d4348L, NULL);

/* get TOS version number */
	if (no_ssystem)
	{
		sysbase = *((long **) (0x4f2L));
		tosvers = (int) (sysbase[0] & 0x0000ffff);
	} else
		tosvers = (int) (Ssystem(S_OSHEADER, 0L, NULL) & 0x0000ffff);

	return 0;
}

#define HILO(x) (int) ((x >> 8) & 255), (int) (x & 255)

int uname(buf)
struct utsname *buf;
{
	no_ssystem = (int)Ssystem(-1, NULL, NULL);
	if (no_ssystem)
	{
		if (!tosvers)
			(void) Supexec(getinfo);
	} else
		getinfo();

	strcpy(buf->sysname, __mint ? "MiNT" : "TOS");

	if (gethostname(buf->nodename, (size_t) sizeof(buf->nodename)))
		strcpy(buf->nodename, "??node??");
/* these should be null terminated if I'm correct... Mikko Larjava */
	buf->nodename[sizeof(buf->nodename) - 1] = '\0';

	if (__mint)
		sprintf(buf->release, "%d.%d", HILO(__mint));
	else
		buf->release[0] = 0;

	sprintf(buf->version, "%d.%d", HILO(tosvers));

	switch ((int) ((_mch >> 16) & 0x0ffffL))
	{
	case 0:
		strcpy(buf->machine, "atarist");
		break;
	case 1:
		strcpy(buf->machine, "atariste");
		break;
	case 2:
		strcpy(buf->machine, "ataritt");
		break;
	case 3:
		/* Falcon has been 5 years on the market now!!! */
		strcpy(buf->machine, "atarifalcon");
		break;
	default:
		strcpy(buf->machine, "atari");
	}

	return 0;
}
