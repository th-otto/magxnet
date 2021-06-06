/*
 *	iflink(8) utilitiy for MintNet (w) 1994, Kay Roemer.
 *
 * Modified to support Pure-C, Thorsten Otto.
 *
 *	Options:
 *
 *	-i <interface name>	Specify the interface name (unit number
 *				is isgnored if specified at all) to which
 *				the device should be linked.
 *
 *	-d <device path>	Specify the path of the device which should
 *				be linked to the network interface.
 */

#include <stdio.h>
#include <string.h>
#include "stsocket.h"
#include <sys/ioctl.h>
#include "ifopts.h"
#include <unistd.h>


#if __GNUC_PREREQ(8, 1)
/* ignore warnings from strncpy(ifl.ifname,...), we *do* want to truncate these */
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

/*
 * BUG: calling Fcntl directly does not set errno
 */
#define ioctl(fd, cmd, arg) Fcntl(fd, (long)(arg), cmd)

/* BUG: wrong prototype */
int _unx2dos(const char *, char *);
int _dos2unx(const char *, char *, size_t);


int sockfd;

static void do_link(char *device, char *ifname)
{
	struct iflink ifl;
	long r;

	_unx2dos(device, ifl.device);
	strncpy(ifl.ifname, ifname, sizeof(ifl.ifname));
	r = ioctl(sockfd, SIOCSIFLINK, &ifl);
	if (r < 0)
	{
		fprintf(stderr, "cannot link %s to an interface: %s\n", device, strerror(errno));
		exit(1);
	}

	printf("%s\n", ifl.ifname);
}


static void get_device(char *ifname)
{
	struct iflink ifl;
#if 0
	char device[sizeof(ifl.device)];
#endif
	long r;

	strncpy(ifl.ifname, ifname, sizeof(ifl.ifname));
	r = ioctl(sockfd, SIOCGIFNAME, &ifl);
	if (r < 0)
	{
		if (errno == EINVAL)
			fprintf(stderr, "%s: not linked to any device\n", ifname);
		else
			fprintf(stderr, "%s: cannot get the device linked to " "this interface: %s\n", ifname, strerror(errno));
		exit(1);
	}

#if 0
	_dos2unx(ifl.device, device, sizeof(device));
	printf("%s\n", device);
#else
	printf("%s\n", ifl.device);
#endif
}


static void usage(void)
{
	printf("usage: iflink -i <interface> [-d <device>]\n");
	exit(1);
}


int main(int argc, char *argv[])
{
	char *device = NULL;
	char *ifname = NULL;
	int c;

	sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sockfd < 0)
	{
		perror("cannot open socket");
		exit(1);
	}

	while ((c = getopt(argc, argv, "i:d:")) != EOF)
	{
		switch (c)
		{
		case 'i':
			ifname = optarg;
			break;

		case 'd':
			device = optarg;
			break;

		case '?':
			usage();
			break;
		}
	}

	if (device && ifname)
		do_link(device, ifname);
	else if (ifname)
		get_device(ifname);
	else
		usage();

	return 0;
}
