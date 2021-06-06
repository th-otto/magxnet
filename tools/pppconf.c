/*
 * print PPP interface settings. (w) 1995, Kay Roemer.
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

#include <stdio.h>
#include <stdlib.h>
#include "stsocket.h"
#include <sys/ioctl.h>
#include <net/if_ppp.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

extern int __libc_unix_names;		/* Secret MiNTLib feature.  */

static int pppfd;

#define ISSET(m,b) (((m)[(unsigned)(b) >> 5]) & (1L << ((b) & 0x1f)))

#if __GNUC_PREREQ(8, 1)
/* ignore warnings from strncpy(), we *do* want to truncate these */
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

/*
 * BUG: calling Fcntl directly does not set errno
 */
#define ioctl(fd, cmd, arg) Fcntl(fd, (long)(arg), cmd)



static const char *map2char(long *map, int n)
{
	static char buf[200];
	char *o = buf;
	int i;
	int first;
	int last;

	*o = '\0';

	for (i = 0; i < n * 32;)
	{
		while (!ISSET(map, i) && ++i < n * 32)
			;

		if (i < n * 32)
		{
			first = i;
			while (++i < n * 32 && ISSET(map, i))
				;

			last = i - 1;
			if (first < last)
				sprintf(o, "0x%02x-0x%02x,", first, last);
			else
				sprintf(o, "0x%02x,", first);

			o += strlen(o);
		}
	}

	if (o > buf)
		o[-1] = '\0';

	return buf;
}


static const char *flags2char(long flags)
{
	static char buf[200];
	char *o = buf;

#define CHECKFLAG(f,s) \
	if (flags & (f)) { \
		strcpy (o, (s)); \
		o += strlen (s); \
	}

	*o = '\0';

	CHECKFLAG(PPPO_PROT_COMP, "PCOMP,");
	CHECKFLAG(PPPO_ADDR_COMP, "ACOMP,");
	CHECKFLAG(PPPO_IP_DOWN, "IPDOWN,");
	CHECKFLAG(PPPO_COMPRESS, "VJCOMP,");
	CHECKFLAG(PPPO_AUTOCOMP, "VJAUTO,");
	CHECKFLAG(PPPO_COMPCID, "VJCID,");

	if (o > buf)
		o[-1] = '\0';

	return buf;
}


static const char *getdev(const char *ifname)
{
	struct iflink ifl;
	static char device[sizeof(ifl.device)];
	int sock;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0)
		return NULL;

	strncpy(ifl.ifname, ifname, sizeof(ifl.ifname));
	if (ioctl(sock, SIOCGIFNAME, &ifl) < 0)
	{
		close(sock);
		return NULL;
	}

#if 0
	if (!__libc_unix_names)
		_dos2unx(ifl.device, device, PATH_MAX);
	else
#endif
		strcpy(device, ifl.device);

	close(sock);
	return device;
}



static void printconf(const char *ifname)
{
	long mtu;
	long mru;
	long flags;
	long xmap[8];
	long rmap[1];
	const char *device;

	ioctl(pppfd, PPPIOCGMTU, &mtu);
	ioctl(pppfd, PPPIOCGMRU, &mru);
	ioctl(pppfd, PPPIOCGXMAP, xmap);
	ioctl(pppfd, PPPIOCGRMAP, rmap);
	ioctl(pppfd, PPPIOCGFLAGS, &flags);
	device = getdev(ifname);

	printf("%10s : %s\n", "device", device ? device : "(none)");
	printf("%10s : %ld bytes\n", "mtu", mtu);
	printf("%10s : %ld bytes\n", "mru", mru);
	printf("%10s : %s\n", "xmap", map2char(xmap, 8));
	printf("%10s : %s\n", "rmap", map2char(rmap, 1));
	printf("%10s : %s\n", "flags", flags2char(flags));
}


int main(int argc, char *argv[])
{
	char fname[100];

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <ppp interface>\n", argv[0]);
		return 0;
	}

	sprintf(fname, "u:/dev/%s", argv[1]);

	pppfd = open(fname, O_RDONLY);
	if (pppfd < 0)
	{
		perror("open");
		return 0;
	}

	printconf(argv[1]);

	close(pppfd);
	return 0;
}
