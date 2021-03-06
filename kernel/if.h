# ifndef _if_h
# define _if_h

#include "buf.h"
#include <net/if.h>

/* net interface flags */
#ifndef IFF_UP
# define IFF_UP			0x0001	/* if is up */
# define IFF_BROADCAST		0x0002	/* if supports broadcasting */
# define IFF_DEBUG		0x0004	/* if debugging is on */
# define IFF_LOOPBACK		0x0008	/* if is software loopback */
# define IFF_POINTOPOINT	0x0010	/* if for p2p connection */
# define IFF_NOTRAILERS		0x0020	/* if should not use trailer encaps. */
# define IFF_RUNNING		0x0040	/* if ressources are allocated */
# define IFF_NOARP		0x0080	/* if should not use arp */
#endif
#ifndef IFF_PROMISC
# define IFF_PROMISC            0x0100  /* Receive all packets */
# define IFF_ALLMULTI           0x0200  /* Receive all multicast packets */
# define IFF_IGMP               0x0400  /* Supports multicast */
# define IFF_MASK		(IFF_UP|IFF_DEBUG|IFF_NOTRAILERS|IFF_NOARP)

# define IF_NAMSIZ		16	/* maximum if name len */
#endif

#define IF_MAXQ		60	/* maximum if queue len */
#define IF_SLOWTIMEOUT		1000	/* one second */
#define IF_PRIORITY_BITS	1
#define IF_PRIORITIES		(1 << IF_PRIORITY_BITS)

struct netif;

/* structure for holding address information, assumes internet style */
struct kernel_ifaddr
{
	union {
		struct sockaddr		sa;
		struct sockaddr_in	in;
	} adr;
	union {
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
		} broadadr;
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
		} dstadr;
	} ifu;
	struct netif	*ifp;		/* interface this belongs to */
	struct kernel_ifaddr	*next;		/* next ifaddr */
	short		family;		/* address family */
	ushort		flags;		/* flags */

	/* AF_INET specific */
	ulong		net;		/* network id */
	ulong		netmask;	/* network mask */
	ulong		subnet;		/* subnet id */
	ulong		subnetmask;	/* subnet mask */
	ulong		net_broadaddr;	/* logical broadcast addr */
};

/* Interface packet queue */
struct ifq
{
	short		maxqlen;
	short		qlen;
	short		curr;
	BUF		*qfirst[IF_PRIORITIES];
	BUF		*qlast[IF_PRIORITIES];
};

/* Hardware address */
struct hwaddr
{
	short		len;
	union
	{
		unsigned char	bytes[10];
		unsigned short	words[5];
		unsigned long	longs[2];
	} adr;
};

/* structure describing a net interface */
struct netif
{
	char		name[IF_NAMSIZ];/* interface name */
	short		unit;		/* interface unit */

	ushort		flags;		/* interface flags */
	ulong		metric;		/* routing metric */
	ulong		mtu;		/* maximum transmission unit */
	ulong		timer;		/* timeout delta in ms */
	short		hwtype;		/* hardware type */
/*
 * These must match the ARP hardware types in arp.h
 */
# define HWTYPE_ETH	1		/* ethernet */
# define HWTYPE_NONE	200		/* pseudo hw type are >= this */
# define HWTYPE_SLIP	201
# define HWTYPE_PPP	202
# define HWTYPE_PLIP	203
	
	struct hwaddr	hwlocal;	/* local hardware address */
	struct hwaddr	hwbrcst;	/* broadcast hardware address */
	
	struct kernel_ifaddr	*addrlist;	/* addresses for this interf. */
	struct ifq	snd;		/* send and recv queue */
	struct ifq	rcv;
	
	long		cdecl (*open)(struct netif *);
	long		cdecl (*close)(struct netif *);
	long		cdecl (*output)(struct netif *, BUF *, const char *, short, short);
	long		cdecl (*ioctl)(struct netif *, short, long);
	void		(*timeout)(struct netif *); /* BUG: not declared cdecl */

	void		*data;		/* extra data the if may want */
	
	ulong		in_packets;	/* # input packets */
	ulong		in_errors;	/* # input errors */
	ulong		out_packets;	/* # output packets */
	ulong		out_errors;	/* # output errors */
	ulong		collisions;	/* # collisions */
	
	struct netif	*next;		/* next interface */
	
	short		maxpackets;	/* max. number of packets the harware
					 * can receive in fast succession.
					 * 0 means unlimited. (this is used
					 * for ethercards with few receive
					 * buffers and slow io to don't over-
					 * flow them with packets.
					 */
	struct bpf	*bpf;		/* packet filter list */
	unsigned char *base_addr;	/* base address of a board (exact meaning
					 * depends on the device driver)
					 */
	void		(*igmp_mac_filter)(struct netif *, ulong, char action);
	long		reserved[2];
};

/* argument structure for the SIOC* ioctl()'s on sockets */
struct kernel_ifreq
{
	char	ifr_name[IF_NAMSIZ];		/* interface name */
	union {
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
			struct sockaddr_hw	hw;
		} adr;
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
		} dstadr;
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
		} broadadr;
		union {
			struct sockaddr		sa;
			struct sockaddr_in	in;
		} netmsk;
		
		short	flags;			/* if flags, IFF_* */
		long	metric;			/* routing metric */
		long	mtu;			/* max transm. unit */
		struct	ifstat stats;		/* interface statistics */
		void	*data;			/* other data */
	} ifru;
};

/* value types for ifopt.valtype */
# define IFO_INT	0	/* integer, uses v_long */
# define IFO_STRING	1	/* string, uses v_string */
# define IFO_HWADDR	2	/* hardware address, v_string[0..5] */


extern struct netif *allinterfaces;
extern struct netif *if_lo;
#ifndef NOTYET
extern struct netif *if_primary;
#endif

short cdecl if_enqueue	(struct ifq *, BUF *, short pri);
short cdecl if_putback	(struct ifq *, BUF *, short pri); /* FIXME: no need for cdecl */
BUF *cdecl if_dequeue	(struct ifq *);
void cdecl if_flushq	(struct ifq *);
long cdecl if_register	(struct netif *);
long cdecl if_deregister	(struct netif *);
long		if_init		(void);
short cdecl if_input	(struct netif *, BUF *, long, short);
#ifdef _timeout_h
void if_doinput(PROC *proc, long arg);
#endif


/*
 * These must match ethernet protcol types
 */
# define PKTYPE_IP	0x0800
# define PKTYPE_ARP	0x0806
# define PKTYPE_RARP	0x8035
# define PKTYPE_IPV6    0x86DD

long		if_ioctl	(short cmd, long arg);
long		if_config	(struct ifconf *);

struct netif *	if_name2if	(char *);
struct netif *	if_net2if	(ulong);
long		if_setifaddr	(struct netif *, struct sockaddr *);
struct kernel_ifaddr *	if_af2ifaddr	(struct netif *, short fam);
short cdecl if_getfreeunit	(char *);

long		if_open		(struct netif *);
long		if_close	(struct netif *);
long		if_send		(struct netif *, BUF *, ulong, short);
void		if_load		(void);

# endif /* _if_h */
