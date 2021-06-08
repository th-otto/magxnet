/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if	!defined(lint)
char const copyright[] = "@(#) Copyright (c) 1989 The Regents of the University of California.\n All rights reserved.\n";

char const sccsid[] = "@(#)ping.c\t5.9 (Berkeley) 5/12/91";
#endif /* not lint */

/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

/*
 * BUG: Fselect called with wrong prototype here.
 */
#ifdef __PUREC__
#define Fselect no_Fselect
#endif
#include "stsocket.h"
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * BUG: Fselect called with wrong prototype here.
 */
#ifdef __PUREC__
#undef Fselect
long Fselect(unsigned int timeout, long rfds, long *wfds, long *xfds);
#else
#include <mint/mintbind.h>
#endif


#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

#ifdef __MINT__
/* MiNT library incorrectly declares signal handler as __CDECL */
# define SIG_CDECL __CDECL
#else
# define SIG_CDECL 
#endif

#ifdef __PUREC__
#pragma warn -stv
#endif

#undef MAXPACKET
#define	DEFDATALEN	(64 - 8)			/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(32768L - 60 - 8)	/* max packet size */
#define	MAXWAIT		10					/* max seconds to wait for response */
#define	NROUTES		9					/* number of record route slots */

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))

/* various options */
int options;

#define	F_FLOOD		0x001
#define	F_INTERVAL	0x002
#define	F_NUMERIC	0x004
#define	F_PINGFILLED	0x008
#define	F_QUIET		0x010
#define	F_RROUTE	0x020
#define	F_SO_DEBUG	0x040
#define	F_SO_DONTROUTE	0x080
#define	F_VERBOSE	0x100

/*
 * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
 * number of received sequence numbers we can keep track of.  Change 512
 * to 8192 for complete accuracy...
 */
#define	MAX_DUP_CHK	(2 * 512)
int mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl[MAX_DUP_CHK / 8];

struct sockaddr whereto;				/* who to ping */
int datalen = DEFDATALEN;
int s;									/* socket file descriptor */
u_char outpack[MAXPACKET];
char BSPACE = '\b';						/* characters written for flood */
char DOT = '.';
char *hostname;
int ident;								/* process id to identify our packets */

/* counters */
long npackets;							/* max packets to transmit */
long nreceived;							/* # of packets we got back */
long nrepeats;							/* number of duplicates */
long ntransmitted;						/* sequence # for outbound packets = #sent */
int interval = 1;						/* interval between packets */

/* timing */
int timing;								/* flag to do timing */
long tmin = 0x7fffffffL;				/* minimum round trip time */
long tmax;								/* maximum round trip time */
unsigned long tsum;						/* sum of all times, for doing average */

/*
 * BUG: this module was compiled with a buggy ip structure
 */
struct buggy_ip {
	unsigned int ip_v:4;	/* version */
	unsigned int ip_hl:4;	/* header length */
	unsigned char ip_tos;	/* type of service BUG: Pure-C will align this on even address */
	short	ip_len;			/* total length */
	u_short	ip_id;			/* identification */
	short	ip_off;			/* fragment offset field */
	u_char	ip_ttl;			/* time to live */
	u_char	ip_p;			/* protocol */
	u_short	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

#if 0
static void SIG_CDECL catcher(int sig);
#endif
static void SIG_CDECL finish(int sig);
static void tvsub(struct timeval *out, struct timeval *in);
static void pinger(void);
static void pr_pack(char *buf, int cc, struct sockaddr_in *from);
static int in_cksum(u_short *addr, long len);
static void pr_icmph(struct icmp *icp);
static void pr_iph(struct buggy_ip *ip);
static char *pr_addr(u_long l);
static void pr_retip(struct buggy_ip *ip);
static void fill(char *bp, char *patp);
static void usage(void);


int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	long n;
	
	(void)tz;
	n = clock();
	tv->tv_sec = (unsigned long)n / CLOCKS_PER_SEC;
	tv->tv_usec = (((unsigned long)n % CLOCKS_PER_SEC) * 1000000UL) / CLOCKS_PER_SEC;
	return 0;
}


#if defined(__GNUC__)
int
#else
/* BUG: should be int */
void
#endif
main(int argc, char **argv)
{
#if 0
	struct timeval timeout;
#endif
	struct hostent *hp;
	struct sockaddr_in *to;
	struct protoent *proto;
	int i;
	long ch;
	long fdmask;
	long hold;
	long packlen;
	long preload;
	u_char *datap;
	u_char *packet;
	char *target;
	char hnamebuf[MAXHOSTNAMELEN];

#ifdef IP_OPTIONS
	char rspace[3 + 4 * NROUTES + 1];	/* record route space */
#endif

	preload = 0;
	datap = &outpack[8 + sizeof(struct timeval)];
	while ((ch = getopt(argc, argv, "Rc:dfh:i:l:np:qrs:v")) != EOF)
	{
		switch ((int)ch)
		{
		case 'c':
			npackets = atoi(optarg);
			if (npackets <= 0)
			{
				fprintf(stderr, "ping: bad number of packets to transmit.\n");
				exit(1);
			}
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'f':
			if (getuid())
			{
				fprintf(stderr, "ping: %s\n", strerror(EPERM));
				exit(1);
			}
			options |= F_FLOOD;
			setbuf(stdout, NULL);
			break;
		case 'i':						/* wait between sending packets */
			interval = atoi(optarg);
			if (interval <= 0)
			{
				fprintf(stderr, "ping: bad timing interval.\n");
				exit(1);
			}
			options |= F_INTERVAL;
			break;
		case 'l':
			preload = atoi(optarg);
			if (preload < 0)
			{
				fprintf(stderr, "ping: bad preload value.\n");
				exit(1);
			}
			break;
		case 'n':
			options |= F_NUMERIC;
			break;
		case 'p':						/* fill buffer with user pattern */
			options |= F_PINGFILLED;
			fill((char *) datap, optarg);
			break;
		case 'q':
			options |= F_QUIET;
			break;
		case 'R':
			options |= F_RROUTE;
			break;
		case 'r':
			options |= F_SO_DONTROUTE;
			break;
		case 's':						/* size of packet to send */
			datalen = (int)atol(optarg);
			if (datalen > MAXPACKET)
			{
				fprintf(stderr, "ping: packet size too large.\n");
				exit(1);
			}
			if (datalen <= 0)
			{
				fprintf(stderr, "ping: illegal packet size.\n");
				exit(1);
			}
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();
	target = *argv;

	bzero((char *) &whereto, sizeof(struct sockaddr));
	to = (struct sockaddr_in *) &whereto;
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(target);
	if (to->sin_addr.s_addr != INADDR_NONE)
	{
		hostname = target;
	} else
	{
		hp = gethostbyname(target);
		if (!hp)
		{
			fprintf(stderr, "ping: unknown host %s\n", target);
			exit(1);
		}
		to->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, &to->sin_addr, hp->h_length);
		strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
		hostname = hnamebuf;
	}

	if ((options & F_FLOOD) && (options & F_INTERVAL))
	{
		fprintf(stderr, "ping: -f and -i incompatible options.\n");
		exit(1);
	}

	if ((int)datalen >= (int)sizeof(struct timeval))	/* can we time transfer */
		timing = 1;
	packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if ((packet = (u_char *) malloc((u_int) packlen)) == NULL)
	{
		fprintf(stderr, "ping: out of memory.\n");
		exit(1);
	}
	if (!(options & F_PINGFILLED))
		for (i = 8; i < datalen; ++i)
			*datap++ = i;

	ident = getpid() & 0xFFFF;

	if ((proto = getprotobyname("icmp")) == NULL)
	{
		fprintf(stderr, "ping: unknown protocol icmp.\n");
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0)
	{
		perror("ping: socket");
		exit(1);
	}
	hold = 1;
	if (options & F_SO_DEBUG)
		setsockopt(s, SOL_SOCKET, SO_DEBUG, &hold, sizeof(hold));
	if (options & F_SO_DONTROUTE)
		setsockopt(s, SOL_SOCKET, SO_DONTROUTE, &hold, sizeof(hold));

	/* record route option */
	if (options & F_RROUTE)
	{
#ifdef IP_OPTIONS
		rspace[IPOPT_OPTVAL] = IPOPT_RR;
		rspace[IPOPT_OLEN] = sizeof(rspace) - 1;
		rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace, sizeof(rspace)) < 0)
		{
			perror("ping: record route");
			exit(1);
		}
#else
		fprintf(stderr, "ping: record route not available in this implementation.\n");
		exit(1);
#endif /* IP_OPTIONS */
	}

	/*
	 * When pinging the broadcast address, you can get a lot of answers.
	 * Doing something so evil is useful if you are trying to stress the
	 * ethernet, or just want to fill the arp cache to get some stuff for
	 * /etc/ethers.
	 */
	hold = 48 * 1024L;
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &hold, sizeof(hold));

	if (to->sin_family == AF_INET)
		printf("PING %s (%s): %d data bytes\n", hostname,
					  inet_ntoa(*(struct in_addr *) &to->sin_addr.s_addr), datalen);
	else
		printf("PING %s: %d data bytes\n", hostname, datalen);

	signal(SIGINT, finish);
#if 0
	signal(SIGALRM, catcher);
#else
	if (npackets <= 0)
		npackets = 4;
#endif

	while (preload--)					/* fire off them quickies */
		pinger();

#if 0
	if ((options & F_FLOOD) == 0)
		catcher(0);						/* start things going */
#endif

	for (;;)
	{
		struct sockaddr_in from;
		int cc;
		__mint_socklen_t fromlen;

#if 0
		if (options & F_FLOOD)
		{
			pinger();
			timeout.tv_sec = 0;
			timeout.tv_usec = 10000;
			fdmask = 1 << s;
			if (select(s + 1, &fdmask, NULL, NULL, &timeout) < 1)
				continue;
		}
#else
		pinger();
		fdmask = 1 << s;
		if (Fselect(1000, 0, &fdmask, 0) == 0)
			continue;
#endif
		fromlen = (int)sizeof(from);
		if ((cc = recvfrom(s, packet, packlen, 0, (struct sockaddr *) &from, &fromlen)) < 0)
		{
			if (errno == EINTR)
				continue;
			perror("ping: recvfrom");
			continue;
		}
		pr_pack((char *) packet, cc, &from);
		if (npackets && nreceived >= npackets)
			break;
	}
	finish(0);
#if defined(__GNUC__)
	return 0;
#endif
}


#if 0
/*
 * catcher --
 *	This routine causes another PING to be transmitted, and then
 * schedules another SIGALRM for 1 second from now.
 *
 * bug --
 *	Our sense of time will slowly skew (i.e., packets will not be
 * launched exactly at 1-second intervals).  This does not affect the
 * quality of the delay and loss statistics.
 */
static void SIG_CDECL catcher(int sig)
{
	int waittime;

	(void)sig;
	pinger();
	signal(SIGALRM, catcher);
	if (!npackets || ntransmitted < npackets)
	{
		alarm(interval);
	} else
	{
		if (nreceived)
		{
			waittime = 2 * tmax / 1000;
			if (!waittime)
				waittime = 1;
		} else
		{
			waittime = MAXWAIT;
		}
		signal(SIGALRM, finish);
		alarm(waittime);
	}
}
#endif


/*
 * pinger --
 *	Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
static void pinger(void)
{
	struct icmp *icp;
	long cc;
	long i;

	icp = (struct icmp *) outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = ntransmitted++;
	icp->icmp_id = ident;				/* ID */

	CLR(icp->icmp_seq % mx_dup_ck);

	if (timing)
		gettimeofday((struct timeval *) &outpack[8], NULL);

	cc = datalen + 8;					/* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->icmp_cksum = in_cksum((u_short *) icp, cc);

	i = sendto(s, outpack, cc, 0, &whereto, sizeof(struct sockaddr));

	if (i < 0 || i != cc)
	{
		if (i < 0)
			perror("ping: sendto");
		printf("ping: wrote %s %ld chars, ret=%ld\n", hostname, cc, i);
	}
	if (!(options & F_QUIET) && options & F_FLOOD)
		write(1, &DOT, 1);
}


/*
 * pr_pack --
 *	Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
static void pr_pack(char *buf, int cc, struct sockaddr_in *from)
{
	struct icmp *icp;
	u_long l;
	int i, j;
	u_char *cp;
	u_char *dp;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];
	struct buggy_ip *ip;
	struct timeval tv;
	struct timeval *tp;
	long triptime
#ifdef __GNUC__
		= 0
#endif
		;
	int hlen;
	int dupflag;

	gettimeofday(&tv, NULL);

	/* Check the IP header */
	ip = (struct buggy_ip *) buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN)
	{
		if (options & F_VERBOSE)
			fprintf(stderr,
						   "ping: packet too short (%d bytes) from %s\n", cc,
						   inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr));
		return;
	}

	/* Now the ICMP part */
	cc -= hlen;
	icp = (struct icmp *) (buf + hlen);
	if (icp->icmp_type == ICMP_ECHOREPLY)
	{
		if (icp->icmp_id != ident)
			return;						/* 'Twas not our ECHO */
		++nreceived;
		if (timing)
		{
#ifndef icmp_data
			tp = (struct timeval *) &icp->icmp_ip;
#else
			tp = (struct timeval *) icp->icmp_data;
#endif
			tvsub(&tv, tp);
			triptime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
			tsum += triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
		}

		if (TST(icp->icmp_seq % mx_dup_ck))
		{
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else
		{
			SET(icp->icmp_seq % mx_dup_ck);
			dupflag = 0;
		}

		if (options & F_QUIET)
			return;

		if (options & F_FLOOD)
		{
			write(1, &BSPACE, 1);
		} else
		{
			printf("%d bytes from %s: icmp_seq=%u", cc,
						  inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr), icp->icmp_seq);
			printf(" ttl=%d", ip->ip_ttl);
			if (timing)
				printf(" time=%ld ms", triptime);
			if (dupflag)
				printf(" (DUP!)");
			/* check the data */
			cp = (u_char *) & icp->icmp_data[8];
			dp = &outpack[8 + sizeof(struct timeval)];
			for (i = 8; i < datalen; ++i, ++cp, ++dp)
			{
				if (*cp != *dp)
				{
					printf("\nwrong data byte #%d should be 0x%x but was 0x%x", i, *dp, *cp);
					cp = (u_char *) & icp->icmp_data[0];
					for (i = 8; i < datalen; ++i, ++cp)
					{
						if ((i % 32) == 8)
							printf("\n\t");
						printf("%x ", *cp);
					}
					break;
				}
			}
		}
	} else
	{
		/* We've got something other than an ECHOREPLY */
		if (!(options & F_VERBOSE))
			return;
		printf("%d bytes from %s: ", cc, pr_addr(from->sin_addr.s_addr));
		pr_icmph(icp);
	}

	/* Display any IP options */
	cp = (u_char *) buf + sizeof(*ip);

	for (; hlen > (int) sizeof(*ip); --hlen, ++cp)
	{
		switch (*cp)
		{
		case IPOPT_EOL:
			hlen = 0;
			break;
		case IPOPT_LSRR:
			printf("\nLSRR: ");
			hlen -= 2;
			j = *++cp;
			++cp;
			if (j > IPOPT_MINOFF)
				for (;;)
				{
					l = *++cp;
					l = (l << 8) + *++cp;
					l = (l << 8) + *++cp;
					l = (l << 8) + *++cp;
					if (l == 0)
						printf("\t0.0.0.0");
					else
						printf("\t%s", pr_addr(ntohl(l)));
					hlen -= 4;
					j -= 4;
					if (j <= IPOPT_MINOFF)
						break;
					putchar('\n');
				}
			break;
		case IPOPT_RR:
			j = *++cp;					/* get length */
			i = *++cp;					/* and pointer */
			hlen -= 2;
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0)
				continue;
			if (i == old_rrlen
				&& cp == (u_char *) buf + sizeof(*ip) + 2
				&& !bcmp((char *) cp, old_rr, i) && !(options & F_FLOOD))
			{
				printf("\t(same route)");
				i = ((i + 3) / 4) * 4;
				hlen -= i;
				cp += i;
				break;
			}
			old_rrlen = i;
			bcopy((char *) cp, old_rr, i);
			printf("\nRR: ");
			for (;;)
			{
				l = *++cp;
				l = (l << 8) + *++cp;
				l = (l << 8) + *++cp;
				l = (l << 8) + *++cp;
				if (l == 0)
					printf("\t0.0.0.0");
				else
					printf("\t%s", pr_addr(ntohl(l)));
				hlen -= 4;
				i -= 4;
				if (i <= 0)
					break;
				putchar('\n');
			}
			break;
		case IPOPT_NOP:
			printf("\nNOP");
			break;
		default:
			printf("\nunknown option %x", *cp);
			break;
		}
	}
	if (!(options & F_FLOOD))
	{
		putchar('\n');
		fflush(stdout);
	}
}


/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
static int in_cksum(u_short *addr, long len)
{
	long nleft = len;
	u_short *w = addr;
	long sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffffL);	/* add hi 16 to low 16 */
	sum += (sum >> 16);					/* add carry */
	return answer = ~sum;						/* truncate to 16 bits */
}


/*
 * tvsub --
 *	Subtract 2 timeval structs:  out = out - in.  Out is assumed to
 * be >= in.
 */
static void tvsub(struct timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000L;
	}
	out->tv_sec -= in->tv_sec;
}


/*
 * finish --
 *	Print out statistics, and give up.
 */
static void SIG_CDECL finish(int sig)
{
	(void)sig;
	signal(SIGINT, SIG_IGN);
	putchar('\n');
	fflush(stdout);
	printf("--- %s ping statistics ---\n", hostname);
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received, ", nreceived);
	if (nrepeats)
		printf("+%ld duplicates, ", nrepeats);
	if (ntransmitted)
	{
		if (nreceived > ntransmitted)
			printf("-- somebody's printing up packets!");
		else
			printf("%d%% packet loss", (int) (((ntransmitted - nreceived) * 100) / ntransmitted));
	}
	putchar('\n');
	if (nreceived && timing)
	{
		/* Only display average to microseconds */
		printf("round-trip min/avg/max = %ld/%lu/%ld ms\n", tmin, tsum / (nreceived + nrepeats), tmax);
	}
	exit(0);
}

#ifdef notdef
static char *ttab[] = {
	"Echo Reply",						/* ip + seq + udata */
	"Dest Unreachable",					/* net, host, proto, port, frag, sr + IP */
	"Source Quench",					/* IP */
	"Redirect",							/* redirect type, gateway, + IP  */
	"Echo",
	"Time Exceeded",					/* transit, frag reassem + IP */
	"Parameter Problem",				/* pointer + IP */
	"Timestamp",						/* id + seq + three timestamps */
	"Timestamp Reply",					/* " */
	"Info Request",						/* id + sq */
	"Info Reply"						/* " */
};
#endif


/*
 * pr_icmph --
 *	Print a descriptive string about an ICMP header.
 */
static void pr_icmph(struct icmp *icp)
{
	switch (icp->icmp_type)
	{
	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_UNREACH:
		switch (icp->icmp_code)
		{
		case ICMP_UNREACH_NET:
			printf("Destination Net Unreachable\n");
			break;
		case ICMP_UNREACH_HOST:
			printf("Destination Host Unreachable\n");
			break;
		case ICMP_UNREACH_PROTOCOL:
			printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_UNREACH_PORT:
			printf("Destination Port Unreachable\n");
			break;
		case ICMP_UNREACH_NEEDFRAG:
			printf("frag needed and DF set\n");
			break;
		case ICMP_UNREACH_SRCFAIL:
			printf("Source Route Failed\n");
			break;
		default:
			printf("Dest Unreachable, Bad Code: %d\n", icp->icmp_code);
			break;
		}
		/* Print returned IP header information */
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct buggy_ip *) icp->icmp_data);
#endif
		break;
	case ICMP_SOURCEQUENCH:
		printf("Source Quench\n");
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct buggy_ip *) icp->icmp_data);
#endif
		break;
	case ICMP_REDIRECT:
		switch (icp->icmp_code)
		{
		case ICMP_REDIRECT_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", icp->icmp_code);
			break;
		}
		printf("(New addr: 0x%08lx)\n", (unsigned long)icp->icmp_gwaddr.s_addr);
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct buggy_ip *) icp->icmp_data);
#endif
		break;
	case ICMP_ECHO:
		printf("Echo Request\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_TIMXCEED:
		switch (icp->icmp_code)
		{
		case ICMP_TIMXCEED_INTRANS:
			printf("Time to live exceeded\n");
			break;
		case ICMP_TIMXCEED_REASS:
			printf("Frag reassembly time exceeded\n");
			break;
		default:
			printf("Time exceeded, Bad Code: %d\n", icp->icmp_code);
			break;
		}
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct buggy_ip *) icp->icmp_data);
#endif
		break;
	case ICMP_PARAMPROB:
		printf("Parameter problem: pointer = 0x%02x\n", icp->icmp_hun.ih_pptr);
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct buggy_ip *) icp->icmp_data);
#endif
		break;
	case ICMP_TSTAMP:
		printf("Timestamp\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_TSTAMPREPLY:
		printf("Timestamp Reply\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_IREQ:
		printf("Information Request\n");
		/* XXX ID + Seq */
		break;
	case ICMP_IREQREPLY:
		printf("Information Reply\n");
		/* XXX ID + Seq */
		break;
#ifdef ICMP_MASKREQ
	case ICMP_MASKREQ:
		printf("Address Mask Request\n");
		break;
#endif
#ifdef ICMP_MASKREPLY
	case ICMP_MASKREPLY:
		printf("Address Mask Reply\n");
		break;
#endif
	default:
		printf("Bad ICMP type: %d\n", icp->icmp_type);
		break;
	}
}


/*
 * pr_iph --
 *	Print an IP header with options.
 */
static void pr_iph(struct buggy_ip *ip)
{
	int hlen;
	u_char *cp;

	hlen = ip->ip_hl << 2;
	cp = (u_char *) ip + 20;			/* point to options */

	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
	printf(" %1x  %1x  %02x %04x %04x", ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
	printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13, (ip->ip_off) & 0x1fff);
	printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
	printf(" %s ", inet_ntoa(*(struct in_addr *) &ip->ip_src.s_addr));
	printf(" %s ", inet_ntoa(*(struct in_addr *) &ip->ip_dst.s_addr));
	/* dump and option bytes */
	while (hlen-- > 20)
	{
		printf("%02x", *cp++);
	}
	putchar('\n');
}


/*
 * pr_addr --
 *	Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
static char *pr_addr(u_long l)
{
	struct hostent *hp;
	static char buf[80];

#ifdef __GNUC__
	struct in_addr *addr = (struct in_addr *)&l;
	if ((options & F_NUMERIC) || (hp = gethostbyaddr(&l, 4, AF_INET)) == NULL)
		sprintf(buf, "%s", inet_ntoa(*addr));
	else
		sprintf(buf, "%s (%s)", hp->h_name, inet_ntoa(*addr));
#else
	if ((options & F_NUMERIC) || (hp = gethostbyaddr((void *)&l, 4, AF_INET)) == NULL)
		sprintf(buf, "%s", inet_ntoa(*(struct in_addr *) &l));
	else
		sprintf(buf, "%s (%s)", hp->h_name, inet_ntoa(*(struct in_addr *) &l));
#endif
	return buf;
}


/*
 * pr_retip --
 *	Dump some info on a returned (via ICMP) IP packet.
 */
static void pr_retip(struct buggy_ip *ip)
{
	int hlen;
	u_char *cp;

	pr_iph(ip);
	hlen = ip->ip_hl << 2;
	cp = (u_char *) ip + hlen;

	if (ip->ip_p == 6)
		printf("TCP: from port %u, to port %u (decimal)\n",
					  (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	else if (ip->ip_p == 17)
		printf("UDP: from port %u, to port %u (decimal)\n",
					  (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
}


static void fill(char *bp, char *patp)
{
	int ii;
	int jj;
	int kk;
	int pat[16];
	char *cp;

	for (cp = patp; *cp; cp++)
		if (!isxdigit(*cp))
		{
			fprintf(stderr, "ping: patterns must be specified as hex digits.\n");
			exit(1);
		}
	ii = sscanf(patp,
				"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
				&pat[0], &pat[1], &pat[2], &pat[3], &pat[4], &pat[5], &pat[6],
				&pat[7], &pat[8], &pat[9], &pat[10], &pat[11], &pat[12], &pat[13], &pat[14], &pat[15]);

	if (ii > 0)
		for (kk = 0; kk <= MAXPACKET - (8 /* + sizeof(struct timeval) */ + ii); kk += ii)
			for (jj = 0; jj < ii; ++jj)
				bp[jj + kk] = pat[jj];
	if (!(options & F_QUIET))
	{
		printf("PATTERN: 0x");
		for (jj = 0; jj < ii; ++jj)
			printf("%02x", bp[jj] & 0xFF);
		printf("\n");
	}
}


static void usage(void)
{
	fprintf(stderr,
				   "usage: ping [-Rdfnqrv] [-c count] [-i wait] [-l preload]\n\t[-p pattern] [-s packetsize] host\n");
	exit(1);
}
