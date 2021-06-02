#include "ifconfig.h"

#ifndef SIOCSLNKFLAGS
#define SIOCSLNKFLAGS	(('S' << 8) | 25)	/* set link level flags */
#define SIOCGLNKFLAGS	(('S' << 8) | 26)	/* get link level flags */
#endif
#ifndef SIOCSIFLINK
#define SIOCSIFLINK	(('S' << 8) | 11)	/* connect iface to device */
#endif

#define C_SCKM 0x53434B4DL     /* MagXNet (SOCKET.DEV) */

#ifdef __PUREC__
#pragma warn -stv /* for inet_lnaof */
#endif

/* FIXME: different message than in magx_sld.ovl */
static char const not_installed[] = " MAGX-NeT device driver NOT installed!\r\n";
static char const format_s[] = "%s";


struct magxnet_cookie *sockets_dev;
long cookie;
int sock_fd;
char ifname_link[IFNAMSIZ];


static long get_jar(void)
{
	return *((long *)0x5a0);
}


static long *get_cookie(long id, long *value)
{
	long *jar;
	
	jar = (long *)Supexec(get_jar);
	if (jar != NULL)
	{
		while (jar[0] != 0)
		{
			if (jar[0] == id)
			{
				if (value)
					*value = *++jar;
				return jar;
			}
			jar += 2;
		}
	}
	return NULL;
}


int open_socket(int flag)
{
	if (flag == 1)
	{
		if (get_cookie(C_SCKM, (long *)&cookie) == NULL)
		{
			(void) Cconws(not_installed);
			return -1;
		}
		if (cookie)
		{
			sockets_dev = (struct magxnet_cookie *)cookie;
		}
		if (sockets_dev->o44 != 0)
			return -1;
		Supexec(sockets_dev->o12);
		sockets_dev->o44 = 1;
	}
	sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock_fd < 0)
		return -1;
	return sock_fd;
}



/*
 * functions mostly borrowed from net-tools/ifconfig.c
 */
int get_if_flags(const char *ifname)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	if (Fcntl(sock_fd, (long)&ifr, SIOCGIFFLAGS) < 0)
		return -1;
	return ifr.ifr_flags;
}


void set_if_flags(const char *ifname, int flags)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_flags = flags;
	Fcntl(sock_fd, (long)&ifr, SIOCSIFFLAGS);
	/* FIXME: should return error code */
}


/*
 * Get interface link level flags
 */
int get_link_flags(const char *ifname, int *flags)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	if (Fcntl(sock_fd, (long)&ifr, SIOCGLNKFLAGS) < 0)
		return -1;
	*flags = ifr.ifr_flags;
	return 0;
}


/*
 * Set interface link level flags
 */
int set_link_flags(const char *ifname, int flags)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_flags = flags;
	if (Fcntl(sock_fd, (long)&ifr, SIOCSLNKFLAGS) < 0)
		return -1;
	return 0;
}


int set_addr(const char *ifname, short which, in_addr_t addr)
{
	struct ifreq ifr;
	struct sockaddr_in in;

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = addr;
	in.sin_port = 0;
	strcpy(ifr.ifr_name, ifname);
	memcpy(&ifr.ifr_addr, &in, sizeof(in));

	if (Fcntl(sock_fd, (long)&ifr, which) < 0)
		return -1;
	return 0;
}


int set_mtu_metric(const char *ifname, short which, long val)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_metric = val;
	if (Fcntl(sock_fd, (long)&ifr, which) < 0)
		return -1;
	return 0;
}


int if_link(const char *device, const char *ifname)
{
	struct iflink ifl;
	long r;

	/* FIXME: use unx2dos, line in mintlib */
	if (device[1] != ':')
		return -1;
	strncpy(ifl.device, device, sizeof(ifl.device));
	strncpy(ifl.ifname, ifname, sizeof(ifl.ifname));
	r = Fcntl(sock_fd, (long)&ifl, SIOCSIFLINK);
	if (r < 0)
		return -1;
	
	sprintf(ifname_link, format_s, ifl.ifname); /* WTF */
	return 0;
}


/*
 * function mostly borrowed from net-tools/route.c
 */
struct in_addr resolve(const char *name)
{
	struct hostent *hent;
	struct in_addr ina = { INADDR_ANY };

	if (strcasecmp(name, "default") != 0)
	{
		hent = gethostbyname(name);
		if (hent != NULL)
			ina = *(struct in_addr *) hent->h_addr;
	}

	return ina;
}


int is_host(struct in_addr ina)
{
	return ina.s_addr != INADDR_ANY && inet_lnaof(ina) != 0;
}



#ifdef __PUREC__
/* FIXME: unused */
static void usage(void)
{
	printf("route\t [del <target>]\n");
	printf("\t [add <target> <interface> [gw <gateway>] [metric NN]]\n");
	exit(0);
}
#endif


int add_route(int argc, char *argv[])
{
#define NEXTARG(i) { if (++i >= argc) return -1; }
#define SIN(x)	((struct sockaddr_in *)(x))
	struct rtentry rt;
	int i = 1;

	if (argc <= 1)
		return -1;

	memset(&rt, 0, sizeof (rt));
	SIN(&rt.rt_dst)->sin_family = AF_INET;
	SIN(&rt.rt_gateway)->sin_family = AF_INET;
	rt.rt_flags = RTF_UP;

	SIN(&rt.rt_dst)->sin_addr = resolve(argv[i]);
	if (is_host(SIN(&rt.rt_dst)->sin_addr))
		rt.rt_flags |= RTF_HOST;
	
	++i; /* skip interface name, we do not need it */
	while (++i < argc)
	{
		if (strcmp(argv[i], "gw") == 0)
		{
			NEXTARG(i);
			rt.rt_flags |= RTF_GATEWAY;
			SIN(&rt.rt_gateway)->sin_addr = resolve(argv[i]);
			if (!is_host(SIN(&rt.rt_gateway)->sin_addr))
			{
				printf("GATEWAY cannot be a NETWORK\n");
				return -1;
			}
		} else if (strcmp(argv[i], "metric") == 0)
		{
			NEXTARG(i);
			rt.rt_metric = atol(argv[i]);
		} else
		{
			return -1;
		}
	}

	if (Fcntl(sock_fd, (long)&rt, SIOCADDRT) < 0)
	{
		/* BUG: calling Fcntl directly does not set errno */
		perror("cannot add route");
		return -1;
	}
	return 0;
}

