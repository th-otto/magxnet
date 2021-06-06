/*
 * ifconfig(8) utility for MintNet, (w) 1994, Kay Roemer.
 *
 * Modified to support Pure-C, Thorsten Otto.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "stsocket.h"

#if defined(__PUREC__) && !defined(__MINT__)
# include <tos.h>
#else
# include <osbind.h>
# include <mintbind.h>
# include <sys/stat.h>
#endif

#include <sys/ioctl.h>
#include <net/route.h>
#include "ifopts.h"

#ifndef SIOCSIFHWADDR
#define SIOCSIFHWADDR	(('S' << 8) | 49)	/* set hardware address, currently missing from MiNTlib */
#endif

#ifdef __PUREC__
#pragma warn -stv
#endif


int sockfd;

/*
 * BUG: calling Fcntl directly does not set errno
 */
#define ioctl(fd, cmd, arg) Fcntl(fd, (long)(arg), cmd)


#ifdef NOTYET
static const char *which2str(short which)
{
	const char *ret = "(unknown)";

	switch (which)
	{
	case SIOCSIFADDR:
	case SIOCGIFADDR:
		ret = "ADDRESS";
		break;
	case SIOCSIFBRDADDR:
	case SIOCGIFBRDADDR:
		ret = "BROADCAST ADDRESS";
		break;
	case SIOCSIFDSTADDR:
	case SIOCGIFDSTADDR:
		ret = "DESTINATION ADDRESS";
		break;
	case SIOCSIFNETMASK:
	case SIOCGIFNETMASK:
		ret = "NETMASK";
		break;
	case SIOCSIFMETRIC:
	case SIOCGIFMETRIC:
		ret = "METRIC";
		break;
	case SIOCSIFMTU:
	case SIOCGIFMTU:
		ret = "MTU";
		break;
	}

	return ret;
}
#endif


static void get_stats(const char *ifname, struct ifstat *stats)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sockfd, SIOCGIFSTATS, &ifr) < 0)
		memset(stats, 0, sizeof(*stats));
	else
		*stats = ifr.ifr_stats;
}


static long get_mtu_metric(const char *ifname, short which)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sockfd, which, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot get %s: %s\n", ifname,
#ifdef NOTYET
			which2str(which),
#else
			which == SIOCGIFMETRIC ? "METRIC" : "MTU",
#endif
			strerror(errno));
		exit(1);
	}

	return ifr.ifr_metric;
}


static void set_mtu_metric(const char *ifname, short which, long val)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_metric = val;
	if (ioctl(sockfd, which, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot set %s: %s\n", ifname,
#ifdef NOTYET
			which2str(which),
#else
			which == SIOCSIFMETRIC ? "METRIC" : "MTU",
#endif
			strerror(errno));
		exit(1);
	}
}


static short get_flags(const char *ifname)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot get FLAGS: %s\n", ifname, strerror(errno));
		exit(1);
	}

	return (ifr.ifr_flags);
}


static void set_flags(const char *ifname, short flags)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_flags = flags;
	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot set FLAGS: %s\n", ifname, strerror(errno));
		if (errno == ENODEV && (flags & (IFF_UP | IFF_RUNNING)) == IFF_UP)
			fprintf(stderr, "hint: probably the interface is not linked to a device. Use iflink!\n");
		exit(1);
	}
}


/*
 * Get interface link level flags
 */
static int get_lnkflags(const char *ifname, int *flags)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sockfd, SIOCGLNKFLAGS, &ifr) < 0)
		return -1;

	*flags = ifr.ifr_flags;
	return 0;
}


/*
 * Set interface link level flags
 */
static int set_lnkflags(const char *ifname, int flags)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_flags = flags;

	if (ioctl(sockfd, SIOCSLNKFLAGS, &ifr) < 0)
		return -1;

	return 0;
}


static void set_addr(const char *ifname, short which, in_addr_t addr)
{
	struct ifreq ifr;
	struct sockaddr_in in;

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = addr;
	in.sin_port = 0;
	strcpy(ifr.ifr_name, ifname);
	memcpy(&ifr.ifr_addr, &in, sizeof(in));

	if (ioctl(sockfd, which, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot set %s: %s\n", ifname,
#ifdef NOTYET
			which2str(which),
#else
			which == SIOCSIFMETRIC ? "ADDRESS" :
			which == SIOCSIFBRDADDR ? "BROADCAST ADDRESS" :
			which == SIOCSIFDSTADDR ? "DESTINATION ADDRESS" :
			"NETMASK",
#endif
			strerror(errno));
		exit(1);
	}
}


static unsigned long get_addr(const char *ifname, short which)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(sockfd, which, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot get %s: %s\n", ifname,
#ifdef NOTYET
			which2str(which),
#else
			which == SIOCGIFMETRIC ? "ADDRESS" :
			which == SIOCGIFBRDADDR ? "BROADCAST ADDRESS" :
			which == SIOCGIFDSTADDR ? "DESTINATION ADDRESS" :
			"NETMASK",
#endif
			strerror(errno));
		exit(1);
	}

	return ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;
}


#ifdef NOTYET
#define HWTYPE_ETH	1					/* this is defined in inet4/if.h, but not in MiNTlib */

static int get_hwaddr(const char *ifname, unsigned char *hwaddr)
{
	struct ifreq ifr;
	struct sockaddr_hw *shw = (struct sockaddr_hw *) &ifr.ifr_ifru.ifru_hwaddr;

	strcpy(ifr.ifr_name, ifname);

	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
	{
		fprintf(stderr, "%s: cannot get HW address\n", ifname);
		exit(1);
	}

	if (shw->shw_type == HWTYPE_ETH)
	{
		memcpy(hwaddr, shw->shw_addr, shw->shw_len);
		return 0;
	}

	return -1;							/* we can only display the hardware address for ethernet interfaces */
}
#endif


static char *decode_flags(short flags)
{
	static char decode_flags_str[200];

	decode_flags_str[0] = '\0';

	if (flags & IFF_UP)
		strcat(decode_flags_str, "UP,");
	if (flags & IFF_BROADCAST)
		strcat(decode_flags_str, "BROADCAST,");
	if (flags & IFF_DEBUG)
		strcat(decode_flags_str, "DEBUG,");
	if (flags & IFF_LOOPBACK)
		strcat(decode_flags_str, "LOOPBACK,");
	if (flags & IFF_POINTOPOINT)
		strcat(decode_flags_str, "POINTOPOINT,");
	if (flags & IFF_NOTRAILERS)
		strcat(decode_flags_str, "NOTRAILERS,");
	if (flags & IFF_RUNNING)
		strcat(decode_flags_str, "RUNNING,");
	if (flags & IFF_NOARP)
		strcat(decode_flags_str, "NOARP,");

	if (strlen(decode_flags_str))
		decode_flags_str[strlen(decode_flags_str) - 1] = '\0';

	return decode_flags_str;
}


static void print_if(char *name)
{
	int lflags;
	short flags;
	char *sflags;
	struct in_addr addr;
	long mtu_metric;
#ifdef NOTYET
	unsigned char hw_addr[6];
#endif
	struct ifstat stats;

	flags = get_flags(name);
	sflags = decode_flags(flags);
	printf("%s:\tflags=0x%x<%s>", name, flags, sflags);

	if (get_lnkflags(name, &lflags) == 0)
		printf("\n\tlink-level-flags=0x%x", lflags);

	if (flags & IFF_UP)
	{
		printf("\n\t");
		addr.s_addr = get_addr(name, SIOCGIFADDR);
		printf("inet %s ", inet_ntoa(addr));

		addr.s_addr = get_addr(name, SIOCGIFNETMASK);
		printf("netmask %s ", inet_ntoa(addr));

		if (flags & IFF_POINTOPOINT)
		{
			addr.s_addr = get_addr(name, SIOCGIFDSTADDR);
			printf("dstaddr %s ", inet_ntoa(addr));
		}

		if (flags & IFF_BROADCAST)
		{
			addr.s_addr = get_addr(name, SIOCGIFBRDADDR);
			printf("broadcast %s ", inet_ntoa(addr));
		}
	}

	printf("\n\t");
	mtu_metric = get_mtu_metric(name, SIOCGIFMETRIC);
	printf("metric %ld ", mtu_metric);

	mtu_metric = get_mtu_metric(name, SIOCGIFMTU);
#ifdef NOTYET
	printf("mtu %ld ", mtu_metric);

	/* display hardware address for ethernet type interfaces */
	if (!get_hwaddr(name, hw_addr))
		printf("hwaddr %02x:%02x:%02x:%02x:%02x:%02x\n\t",
			   hw_addr[0], hw_addr[1], hw_addr[2], hw_addr[3], hw_addr[4], hw_addr[5]);
	else
		printf("\n\t");
#else
	printf("mtu %ld\n\t", mtu_metric);
#endif

	get_stats(name, &stats);
	printf("in-packets  %lu in-errors  %lu collisions %lu\n\t", stats.in_packets, stats.in_errors, stats.collisions);
	printf("out-packets %lu out-errors %lu\n", stats.out_packets, stats.out_errors);
}


static void list_all_if(short all)
{
	struct ifconf ifc;
	struct ifreq ifr[50];
	int i, n;

	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_req = ifr;
	if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
	{
		perror("cannot get interface list");
		exit(1);
	}

	n = (int)(ifc.ifc_len / sizeof(struct ifreq));
	for (i = 0; i < n; ++i)
	{
		if (ifr[i].ifr_addr.sa_family != AF_INET)
			continue;

		if (all || (get_flags(ifr[i].ifr_name) & IFF_UP))
			print_if(ifr[i].ifr_name);
	}
}


static void usage(void)
{
	printf("ifconfig [-a|-v] [<interface name>]\n");
	printf("\t [addr <local address>] [netmask aa.bb.cc.dd]\n");
	printf("\t [dstaddr <point to point destination address>]\n");
	printf("\t [broadaddr aa.bb.cc.dd]\n");
	printf("\t [up|down|[-]arp|[-]trailers|[-]debug]\n");
#ifdef NOTYET
	printf("\t [hwaddr aa:bb:cc:dd:ee:ff]\n");
#endif
	printf("\t [mtu NN] [metric NN]\n");
	printf("\t [linkNN] [-linkNN]\n");
	printf("\t [-f <option file>]\n");
	exit(0);
}


int main(int argc, char **argv)
{
#define NEXTARG(i)	{ if (++i >= argc) usage (); }
	int i;
	int show_all = 0;
	char *ifname = NULL;
	unsigned long addr;
	int flags;
	int lflags;

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("cannot open socket");
		exit(1);
	}

	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-a"))
		{
			show_all = 1;
		} else if (!strcmp(argv[i], "-v"))
		{
			usage();
		} else
		{
			ifname = argv[i++];
			break;
		}
	}

	if (!ifname)
	{
		list_all_if(show_all);
	} else if (i == argc)
	{
		print_if(ifname);
	} else
	{
		flags = get_flags(ifname);
		lflags = 0;
		get_lnkflags(ifname, &lflags);

		for (; i < argc; ++i)
		{
			if (!strcmp(argv[i], "addr"))
			{
				struct hostent *hent;

				NEXTARG(i);
				hent = gethostbyname(argv[i]);
				if (!hent)
				{
					fprintf(stderr, "cannot lookup host %s", argv[i]);
					herror("");
					exit(1);
				}
				memcpy(&addr, hent->h_addr, hent->h_length);
				set_addr(ifname, SIOCSIFADDR, addr);
				flags |= IFF_UP;
			} else if (!strcmp(argv[i], "broadaddr"))
			{
				NEXTARG(i);
				addr = inet_addr(argv[i]);
				set_addr(ifname, SIOCSIFBRDADDR, addr);
			} else if (!strcmp(argv[i], "dstaddr"))
			{
				struct hostent *hent;

				NEXTARG(i);
				hent = gethostbyname(argv[i]);
				if (!hent)
				{
					fprintf(stderr, "cannot lookup host %s", argv[i]);
					herror("");
					exit(1);
				}
				memcpy(&addr, hent->h_addr, hent->h_length);
				set_addr(ifname, SIOCSIFDSTADDR, addr);
			} else if (!strcmp(argv[i], "netmask"))
			{
				NEXTARG(i);
				addr = inet_addr(argv[i]);
				set_addr(ifname, SIOCSIFNETMASK, addr);
			} else if (!strcmp(argv[i], "up"))
			{
				flags |= IFF_UP;
			} else if (!strcmp(argv[i], "down"))
			{
				flags &= ~IFF_UP;
			} else if (!strcmp(argv[i], "arp"))
			{
				flags &= ~IFF_NOARP;
			} else if (!strcmp(argv[i], "-arp"))
			{
				flags |= IFF_NOARP;
			} else if (!strcmp(argv[i], "trailers"))
			{
				flags &= ~IFF_NOTRAILERS;
			} else if (!strcmp(argv[i], "-trailers"))
			{
				flags |= IFF_NOTRAILERS;
			} else if (!strcmp(argv[i], "debug"))
			{
				flags |= IFF_DEBUG;
			} else if (!strcmp(argv[i], "-debug"))
			{
				flags &= ~IFF_DEBUG;
			} else if (!strcmp(argv[i], "mtu"))
			{
				NEXTARG(i);
				set_mtu_metric(ifname, SIOCSIFMTU, atol(argv[i]));
			} else if (!strcmp(argv[i], "metric"))
			{
				NEXTARG(i);
				set_mtu_metric(ifname, SIOCSIFMETRIC, atol(argv[i]));
			} else if (!strncmp(argv[i], "link", 4))
			{
				int bit = atoi(&argv[i][4]);

				if (bit < 0 || bit > 15)
					usage();
				lflags |= (1 << bit);
			} else if (!strncmp(argv[i], "-link", 5))
			{
				int bit = atoi(&argv[i][5]);

				if (bit < 0 || bit > 15)
					usage();
				lflags &= ~(1 << bit);
			} else if (!strcmp(argv[i], "-f"))
			{
				NEXTARG(i);
				opt_file(argv[i], ifname);
#ifdef NOTYET
			} else if (!strcmp(argv[i], "hwaddr"))
			{
				struct ifreq ifr;
				struct sockaddr_hw *shw = (struct sockaddr_hw *) &ifr.ifr_ifru.ifru_hwaddr;
				char hwaddr[6];

				NEXTARG(i);

				if (parse_hwaddr(hwaddr, argv[i]) != 0)
				{
					fprintf(stderr, "illegal hwaddr argument %s\n", argv[i]);
				} else
				{
					fprintf(stderr, "set hwaddr to %02x:%02x:%02x:%02x:%02x:%02x\n",
							(unsigned char) hwaddr[0], (unsigned char) hwaddr[1], (unsigned char) hwaddr[2],
							(unsigned char) hwaddr[3], (unsigned char) hwaddr[4], (unsigned char) hwaddr[5]);
					strcpy(ifr.ifr_name, ifname);
					ifr.ifr_addr.sa_family = AF_INET;
					shw->shw_len = (unsigned short)sizeof(hwaddr);
					memcpy(shw->shw_addr, hwaddr, sizeof(hwaddr));
					if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0)
						fprintf(stderr, "interface does not support SIOCSIFHWADDR ioctl\n");
				}
#endif
			} else
			{
				fprintf(stderr, "unknown option %s\n", argv[i]);
				usage();
			}
		}

		set_flags(ifname, flags);
		set_lnkflags(ifname, lflags);
	}

	return 0;
}
