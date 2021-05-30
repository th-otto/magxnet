#include "magxconf.h"
#include <sys/ioctl.h>
#include "sockdev.h"
#include <net/route.h>

#ifndef SIOCSLNKFLAGS
#define SIOCSLNKFLAGS	(('S' << 8) | 25)	/* set link level flags */
#define SIOCGLNKFLAGS	(('S' << 8) | 26)	/* get link level flags */
#endif

#define C_SCKM 0x53434B4DL     /* MagXNet (SOCKET.DEV) */

# define SYSVAR_bootdev	(*((unsigned short *) 0x446UL))

#ifdef __PUREC__
#pragma warn -stv /* for inet_lnaof */
#endif


static int x15b02;
static struct magxnet_cookie *sockets_dev;
static long cookie;
static int sock_fd;
static char ifname_link[IFNAMSIZ];

const char *default_rc_net = "magx_rc.net";
const char *u_etc = "u:\\etc";
const char *dev_ppp0 = "u:\\dev\\ppp0";
const char *magx_sld_ovl = "x:\\gemsys\\magic\\xtension\\magx_sld.ovl";
const char *magx_sld_env = "MAGX_SLD";


static int open_socket(int flag);
static int get_if_flags(const char *ifname);
static void set_if_flags(const char *ifname, int flags);
static int get_link_flags(const char *ifname, int *flags);
static void set_link_flags(const char *ifname, int flags);
static int set_addr(const char *ifname, short which, in_addr_t addr);
static int set_mtu_metric(const char *ifname, short which, long val);
static int if_link(const char *device, const char *ifname);
static struct in_addr resolve(const char *name);
static int is_host(struct in_addr ina);
static int add_route(int argc, char *argv[]);



static long get_bootdrive(void)
{
	return SYSVAR_bootdev;
}


static void setobaud(const char *device, long baud)
{
	int fd;
	
	fd = (int)Fopen(device, O_RDWR);
	if (fd >= 0)
	{
		Fcntl(fd, (long)&baud, TIOCOBAUD);
		Fclose(fd);
	}
}


static int parse_config(char *line)
{
	char *cmd;
	char *argv[10] = { 0 };
	char *device;
	const char *ifname;
	int argc;
	int argn;
	int flags;
	
	device = NULL;
	ifname = NULL;
	if ((line = strtok(line, " \t")) == NULL)
		return 0;
	argc = 0;
	cmd = line;
	while (line != NULL)
	{
		line = strtok(NULL, " \t");
		argv[argc] = line;
		argc++;
	}

	if (argc != 0 && strcmp(cmd, "etc_path") == 0)
	{
		Fsymlink(argv[0], u_etc);
	} else if (argc != 0 && strcmp(cmd, "ifconfig") == 0)
	{
		struct in_addr addr;
		int link_flags;
		struct hostent *hp;

		argn = argc - 1;
		ifname = argv[0];
		/* BUG: prevents 0x8000 to be used as IFF_ */
		if ((flags = get_if_flags(ifname)) < 0)
			return -1;
		link_flags = 0;
		get_link_flags(ifname, &link_flags);
		for (argc = 1; argc < argn; argc++)
		{
			if (strcmp(argv[argc], "addr") == 0)
			{
				argc++;
				hp = gethostbyname(argv[argc]);
				if (hp == NULL)
					return -1;
				/* FIXME: should check h_length to be size of in_addr */
				memcpy(&addr, hp->h_addr, hp->h_length);
				set_addr(ifname, SIOCSIFADDR, addr.s_addr);
				flags |= IFF_UP;
			} else if (strcmp(argv[argc], "dstaddr") == 0)
			{
				argc++;
				hp = gethostbyname(argv[argc]);
				if (hp == NULL)
					return -1;
				/* FIXME: should check h_length to be size of in_addr */
				memcpy(&addr, hp->h_addr, hp->h_length);
				set_addr(ifname, SIOCSIFDSTADDR, addr.s_addr);
			} else if (strcmp(argv[argc], "netmask") == 0)
			{
				argc++;
				/* FIXME: should check validity */
				addr.s_addr = inet_addr(argv[argc]);
				set_addr(ifname, SIOCSIFNETMASK, addr.s_addr);
			} else if (strcmp(argv[argc], "up") == 0)
			{
				flags |= IFF_UP;
			} else if (strcmp(argv[argc], "mtu") == 0)
			{
				argc++;
				set_mtu_metric(ifname, SIOCSIFMTU, atol(argv[argc]));
			} else if (strncmp(argv[argc], "link", 4) == 0)
			{
				int bit = atoi(&argv[argc][4]);
				if (bit < 0 || bit > 15)
					return -1;
				link_flags |= 1 << bit;
			}
		}
		set_if_flags(ifname, flags);
		set_link_flags(ifname, link_flags);
	} else if (argc != 0 && strcmp(cmd, "iflink") == 0)
	{
		long baud = 0;
		
		if (x15b02 == 0)
		{
			argn = argc - 1;
			for (argc = 0; argc < argn; argc++)
			{
				if (strcmp(argv[argc], "-i") == 0)
				{
					argc++;
					ifname = argv[argc];
				} else if (strcmp(argv[argc], "-d") == 0)
				{
					argc++;
					device = argv[argc];
				} else if (strcmp(argv[argc], "-s") == 0)
				{
					argc++;
					baud = atol(argv[argc]);
				} else
				{
					break;
				}
			}
			if (device != NULL && baud != 0)
				setobaud(device, baud);
			if (device != NULL && ifname != NULL)
				if_link(device, ifname);
		}
	} else if (argc != 0 && strcmp(cmd, "route") == 0)
	{
		argn = argc - 1;
		argc = 0;
		if (strcmp(argv[argc], "add") == 0)
			add_route(argn, argv);
	}
	
	return 0;
}


int main(void)
{
	get_bootdrive();
	parse_config(0);
	return 0;
}



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


static int open_socket(int flag)
{
	if (flag == 1)
	{
		if (get_cookie(C_SCKM, (long *)&cookie) == NULL)
		{
			(void) Cconws(" MagiCNet device driver NOT installed!\r\n");
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
static int get_if_flags(const char *ifname)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	if (Fcntl(sock_fd, (long)&ifr, SIOCGIFFLAGS) < 0)
		return -1;
	return ifr.ifr_flags;
}


static void set_if_flags(const char *ifname, int flags)
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
static int get_link_flags(const char *ifname, int *flags)
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
static void set_link_flags(const char *ifname, int flags)
{
	struct ifreq ifr;
	
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_flags = flags;
	Fcntl(sock_fd, (long)&ifr, SIOCSLNKFLAGS);
	/* FIXME: should return error code */
}


static int set_addr(const char *ifname, short which, in_addr_t addr)
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


static int set_mtu_metric(const char *ifname, short which, long val)
{
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	ifr.ifr_metric = val;
	if (Fcntl(sock_fd, (long)&ifr, which) < 0)
		return -1;
	return 0;
}


static int if_link(const char *device, const char *ifname)
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
	
	sprintf(ifname_link, "%s", ifl.ifname); /* WTF */
	return 0;
}


/*
 * function mostly borrowed from net-tools/route.c
 */
static struct in_addr resolve(const char *name)
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


static int is_host(struct in_addr ina)
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


static int add_route(int argc, char *argv[])
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

