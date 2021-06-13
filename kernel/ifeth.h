/*
 *	10 Mbit Ethernet definitions.
 *
 *	12/14/94, Kay Roemer.
 */

# ifndef _ifeth_h
# define _ifeth_h

# include "buf.h"
# include "if.h"


# define ETH_ALEN	6	/* HW addr length */
# define ETH_HLEN	14	/* Eth frame header length */
# define ETH_MIN_DLEN	46	/* minimum data length */
# define ETH_MAX_DLEN	1500	/* maximum data length */

# define ETHPROTO_LOOP	0x0060	/* Eth loopback frame */
# define ETHPROTO_ECHO	0x0200	/* Eth echo frame */
# define ETHPROTO_IP	0x0800	/* IP frame */
# define ETHPROTO_ARP	0x0806	/* ARP frame */
# define ETHPROTO_RARP	0x8035	/* reverse ARP frame */
# define ETHPROTO_8023	0x0001
# define ETHPROTO_8022	0x0004

struct eth_dgram
{
	unsigned char	daddr[ETH_ALEN];
	unsigned char	saddr[ETH_ALEN];
	ushort	proto;
	unsigned char	data[0];
};

BUF *cdecl eth_build_hdr(BUF *, struct netif *, const char *, short);
short cdecl eth_remove_hdr(BUF *);


# endif /* _ifeth_h */
