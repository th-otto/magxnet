/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
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

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


#if (defined(__MINT__) || defined(__PUREC__)) && !defined(__atarist__)
#define __atarist__ 1
#endif

#ifdef __atarist__
/* temporary hack to get binary identical results */
#undef __socklen_t
#define __socklen_t int
#undef socklen_t
#define socklen_t __socklen_t
#define __mint_socklen_t uint32_t
#endif

#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <mint/mintbind.h>
#undef ENOSYS
#define ENOSYS 32

#ifdef __atarist__

/* WTF? */
#undef stderr
extern FILE _iob[];
#define stderr (&_iob[2])

#include <resolv.h>
#include <mint/mintbind.h>
#include <netinet/in.h>
#include <net/if.h>						/* for struct ifconf */
#include <arpa/inet.h>
#define SIOCGIFCONF       (('S' << 8) | 12)       /* get iface list */
#define SIOCGIFNETMASK	(('S' << 8) | 21)	/* get iface network mask */
#define SIOCGIFADDR	(('S' << 8) | 15)	/* get iface address */
#ifndef ECONNRESET
#define	ECONNRESET		(316)		/* Connection reset by peer.  */
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT 320
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED 321
#endif
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif
#else
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <net/if.h>						/* for struct ifconf */
#include <sockios.h>					/* for SIOC* */
#endif

#define NS_MAXDNAME	256	/* maximum domain name */
#define NS_QFIXEDSZ    4       /* #/bytes of fixed data in query */
#define NS_MAXLABEL	63	/* maximum length of domain label */
#define NS_RRFIXEDSZ	10	/* #/bytes of fixed data in r record */
#define NS_PACKETSZ    512     /* maximum packet size */

#undef _PATH_HOSTS
#define _PATH_HOSTS     "u:\\etc\\hosts" /* BUG: u not uppercase */
#undef _PATH_HOSTCONF
#define _PATH_HOSTCONF  "u:\\etc\\host.conf" /* BUG: u not uppercase */
#undef _PATH_RESCONF
#define _PATH_RESCONF   "u:\\etc\\resolv.conf"


#define	MAXALIASES	35
#define	MAXADDRS	35
#define MAXTRIMDOMAINS  4
#define HOSTDB		_PATH_HOSTS

#define SERVICE_NONE	0
#define SERVICE_BIND	1
#define SERVICE_HOSTS	2
#define SERVICE_NIS		3
#define SERVICE_MAX		3

#define CMD_ORDER	"order"
#define CMD_TRIMDOMAIN	"trim"
#define CMD_HMA		"multi"
#define CMD_SPOOF	"nospoof"
#define CMD_SPOOFALERT	"alert"
#define CMD_REORDER	"reorder"
#define CMD_ON		"on"
#define CMD_OFF		"off"
#define CMD_WARN	"warn"
#define CMD_NOWARN	"warn off"

#define ORD_BIND	"bind"
#define ORD_HOSTS	"hosts"
#define ORD_NIS		"nis"

#define ENV_HOSTCONF	"RESOLV_HOST_CONF"
#define ENV_SERVORDER	"RESOLV_SERV_ORDER"
#define ENV_SPOOF	"RESOLV_SPOOF_CHECK"
#define ENV_TRIM_OVERR	"RESOLV_OVERRIDE_TRIM_DOMAINS"
#define ENV_TRIM_ADD	"RESOLV_ADD_TRIM_DOMAINS"
#define ENV_HMA		"RESOLV_MULTI"
#define ENV_REORDER	"RESOLV_REORDER"

#define TOKEN_SEPARATORS " ,;:"


#if NS_PACKETSZ > 1024
#define	MAXPACKET	NS_PACKETSZ
#else
#define	MAXPACKET	1024
#endif

#ifdef __PUREC__
#define strcasecmp(a,b)		stricmp(a,b)
#define strncasecmp(a,b,c)	strnicmp(a,b,c)
#endif

struct state {
	int	retrans;	 	/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	long options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct sockaddr_in nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr nsaddr_list[0]		/* for backward compatibility */
	uint16_t id;		/* current packet id */
	char defdname[NS_MAXDNAME];	/* default domain */
	char *dnsrch[MAXDNSRCH+1];	/* components of domain to search */
};

#undef __set_errno
#define __set_errno(e) (errno = (e))

#ifndef howmany
# define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

extern int h_errno;
extern struct state _res;


int __dn_skipname(const uint8_t *comp_dn, const uint8_t *eom);
int dn_expand(const uint8_t *msg, const uint8_t *eomorig, const uint8_t *comp_dn, char *exp_dn, int length);
int dn_comp(const char *exp_dn, uint8_t *comp_dn, int length, uint8_t **dnptrs, uint8_t **lastdnptr);

int res_search(const char *name, int class, int type, uint8_t *answer, int anslen);
int res_query(const char *name, int class, int type, uint8_t *answer, int anslen);
int res_mkquery(int op, const char *dname, int class, int type, char *data, int datalen, struct rrec *newrr, uint8_t *buf, int buflen);
char *__hostalias(const char *name);
int res_init(void);
int res_send(const uint8_t *buf, int buflen, uint8_t *answer, int anslen);
int res_querydomain(const char *name, const char *domain, int class, int type, uint8_t *answer, int anslen);
