/*
 *	Definitions for the dummy or localhost IP implementation.
 *
 *	01/21/94, kay roemer.
 *	07/01/99, masquerading code by Mario Becroft.
 */

# ifndef _ip_h
# define _ip_h

# include "if.h"
# include "route.h"

# include "buf.h"


/* Minimal accepable length of an IP packet */
# define IP_MINLEN	(sizeof (struct ip_dgram))

# define IP_DEFAULT_TTL	255
# define IP_DEFAULT_TOS	0

/* Some macros to access data in the ip header for higher level protocols */
# define IP_HDRLEN(buf)	(IPH_HDRLEN((struct ip_dgram *)(buf)->dstart) * 4)
# define IP_DADDR(buf)	(((struct ip_dgram *)(buf)->dstart)->daddr)
# define IP_SADDR(buf)	(((struct ip_dgram *)(buf)->dstart)->saddr)
# define IP_PROTO(buf)	(((struct ip_dgram *)(buf)->dstart)->proto)
# define IP_DATA(buf)	((buf)->dstart + IPH_HDRLEN((struct ip_dgram *)(buf)->dstart) * sizeof(long))

/* IP datagramm */
struct ip_dgram
{
# define IP_VERSION	4		/* current IP version */
#if defined(__GNUC__) || 1
	unsigned int version:4;	/* version number */
	unsigned int hdrlen:4;	/* header len */
	unsigned int tos:8;		/* type of service and precedence */
#define IPH_VERSION(iph) ((iph)->version)
#define IPH_HDRLEN(iph) ((iph)->hdrlen)
#define IPH_TOS(iph) ((iph)->tos)
#else
	/* Pure-C generates better without bitfields */
	unsigned short version_len;
#define IPH_VERSION(iph) (((iph)->version_len >> 12) & 0x0f)
#define IPH_HDRLEN(iph) (((iph)->version_len >> 8) & 0x0f)
#define IPH_TOS(iph) ((iph)->version_len & 0xff)
#endif
	ushort		length;		/* datagram length */
	ushort		id;		/* datagram id */
	ushort		fragoff;	/* fragment offset */
# define IP_MF		0x2000		/* more fragments bit */
# define IP_DF		0x4000		/* don't fragment bit */
# define IP_FRAGOFF	0x1fff		/* fragment offset */

	unsigned char ttl;		/* time to live */
	unsigned char proto;		/* next protocol id */
	short		chksum;		/* checksum */
	ulong		saddr;		/* IP source address */
	ulong		daddr;		/* IP destination address */
	char		data[0];	/* options and data */
};

struct ip_options
{
	short		pri;
	unsigned char ttl;
	unsigned char tos;
	unsigned int hdrincl:1;
#ifdef IGMP_SUPPORT
	ulong		multicast_ip;
	unsigned char multicast_loop;
#endif
};

/* IP Type Of Service */
# define IPTOS_LOWDELAY	0x10
# define IPTOS_THROUPUT	0x08
# define IPTOS_RELIABLE	0x04

# define IPTOS_PRIORITY(x)	(((x) & 0xe0) >> 5)

/* IP options */
# define IPOPT_COPY	0x80		/* copy on fragmentation flag */
# define IPOPT_CLASS	0x60		/* option class */
# define IPOPT_NUMBER	0x1f		/* option number */
# define IPOPT_TYPE	(IPOPT_CLASS|IPOPT_NUMBER)

# define IPOPT_EOL	0x00		/* end of option list */
# define IPOPT_NOP	0x01		/* no operation */
# define IPOPT_SECURITY	0x02		/* security option */
# define IPOPT_LSRR	0x03		/* loose source and record route */
# define IPOPT_SSRR	0x09		/* strict source and record route */
# define IPOPT_RR	0x07		/* record route option */
# define IPOPT_STREAM	0x08		/* SATNET stream id option */
# define IPOPT_STAMP	0x44		/* internet time stamp option */

/*
 * Return values from ip_chk_addr()
 */
# define IPADDR_NONE		0
# define IPADDR_LOCAL		1
# define IPADDR_BRDCST		2
# define IPADDR_BADCLASS	3
# define IPADDR_MULTICST        4

/*
 * Flags for ip_send()
 */
# define IP_DONTROUTE	0x01
# define IP_BROADCAST	0x02

extern struct in_ip_ops *allipprotos;
extern short ip_dgramid;

short	ip_is_brdcst_addr (in_addr_t);
short	ip_is_local_addr (in_addr_t);

in_addr_t ip_local_addr (in_addr_t);
short	ip_same_addr(in_addr_t, in_addr_t);
in_addr_t ip_dst_addr(in_addr_t);
short	ip_chk_addr (in_addr_t, struct route *);
short	ip_priority (short, unsigned char);

struct in_ip_ops;

void	ip_register (struct in_ip_ops *);
in_addr_t	ip_netmask (in_addr_t);
void	ip_input (struct netif *, BUF *);
long	ip_send (in_addr_t, in_addr_t, BUF *, short, short, struct ip_options *);
long	ip_output (BUF *);

BUF *	ip_defrag (BUF *);

long	ip_setsockopt (struct ip_options *, short, short, char *, long);
long	ip_getsockopt (struct ip_options *, short, short, char *, long *);


# endif /* _ip_h */
