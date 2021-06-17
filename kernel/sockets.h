#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef __PUREC__
# include <tos.h>
# define C_MagX 0x4D616758L     /* MagX */
# define C_MiNT 0x4D694E54L     /* Mint/MultiTOS */
# define C_STiK 0x5354694BL     /* ST Internet Kit */
# define C_SCKM 0x53434B4DL     /* MagXNet (SOCKET.DEV) */
#else
# include <mint/osbind.h>
# include <mint/mintbind.h>
# include <mint/basepage.h>
# include <mint/ssystem.h>
# include <mint/cookie.h>
#endif
#define __KERNEL__
#include <mint/errno.h>

/* BUG: throughout this code, EINVAL was used as synonym for ENOSYS */
#undef EINVAL
#define EINVAL ENOSYS

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef GNU_ASM_NAME
#ifdef __GNUC__
#define GNU_ASM_NAME(x) __asm__(x)
#else
#define GNU_ASM_NAME(x)
#endif
#endif

#ifndef WORD
# define WORD short
#endif
#ifndef UWORD
# define UWORD unsigned short
#endif
#ifndef LONG
# define LONG long
#endif
#ifndef ULONG
# define ULONG unsigned long
#endif
typedef struct { int dummy; } APPL;
typedef struct { int dummy; } DMD;
#define PD BASEPAGE
#ifdef __GNUC__
#define _BasPag _base
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef NO_CONST
#  ifdef __GNUC__
#    define NO_CONST(p) __extension__({ union { const void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#    define NO_CONST(p) ((void *)(p))
#  endif
#endif

#undef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))

#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

#include "config.h"

#include "mgx_xfs.h"
#include "mgx_dfs.h"

#ifndef UNLIMITED
#define UNLIMITED (0x7fffffffL)
#endif


/* no matter what the library says, we need the MiNT definition here */
#undef O_NDELAY
#define O_NDELAY 0x0100
#define O_LOCK   0x8000


/* socket types */
#define SOCK_NONE 0

/* possible socket states */
enum so_state
{
	SS_VIRGIN = 0,
	SS_ISUNCONNECTED,
	SS_ISCONNECTING,
	SS_ISCONNECTED,
	SS_ISDISCONNECTING,
	SS_ISDISCONNECTED
};

/* possible socket flags */
# define SO_ACCEPTCON	0x0001		/* socket is accepting connections */
# define SO_RCVATMARK	0x0002		/* in-band and oob data are in sync */
# define SO_CANTRCVMORE	0x0004		/* shut down for receives */
# define SO_CANTSNDMORE	0x0008		/* shut down for sends */
# define SO_CLOSING	0x0010		/* socket is close()ing */
# define SO_DROP	0x0020		/* drop connecting socket when accept()
					   fails due to lacking file handles */


/*
 * next structures happen to be identical to MiNT
 */
struct socket {
	short	type;		/* socket type: SOCK_* */
	enum so_state	state;		/* socket state: SS_* */
	short		flags;			/* socket flags: SO_* */
	struct socket	*conn;		/* peer socket */
	struct socket	*iconn_q;	/* queue of imcomplete connections */
	struct socket	*next;		/* next connecting socket in list */
	struct dom_ops	*ops;		/* domain specific operations */
	void		*data;			/* domain specific data */
	short		error;			/* async. error */
	short		pgrp;			/* process group to send sinals to */
	APPL 		*rsel;			/* process selecting for reading */
	APPL		*wsel;			/* process selecting for writing */
	APPL		*xsel;			/* process selecting for exec. cond. */
	short		date;			/* date stamp */
	short		time;			/* time stamp */
	short		lockpid;		/* pid of locking process */
};

/* domain, as the socket level sees it */
struct dom_ops
{
	short	domain;
	struct dom_ops *next;
	
	long	(*attach)	(struct socket *s, short proto);
	long	(*dup)		(struct socket *news, struct socket *olds);
	long	(*abort)	(struct socket *s, enum so_state ostate);
	long	(*detach)	(struct socket *s);
	long	(*bind)		(struct socket *s, struct sockaddr *addr,
				 short addrlen);
	
	long	(*connect)	(struct socket *s, const struct sockaddr *addr,
				 short addrlen, short flags);
	
	long	(*socketpair)	(struct socket *s1, struct socket *s2);
	long	(*accept)	(struct socket *s, struct socket *new,
			 	 short flags);
	
	long	(*getname)	(struct socket *s, struct sockaddr *addr,
			 	 short *addrlen, short peer);
# define PEER_ADDR	0
# define SOCK_ADDR	1
	long	(*select)	(struct socket *s, short sel_type, long proc);
	long	(*ioctl)	(struct socket *s, short cmd, void *arg);
	long	(*listen)	(struct socket *s, short backlog);
	
	long	(*send)		(struct socket *s, const struct iovec *iov,
				 short niov, short block, short flags,
				 const struct sockaddr *addr, short addrlen);
	
	long	(*recv)		(struct socket *s, const struct iovec *iov,
				 short niov, short block, short flags,
				 struct sockaddr *addr, short *addrlen);
	
	long	(*shutdown)	(struct socket *s, short flags);
	long	(*setsockopt)	(struct socket *s, short level, short optname,
			 	 char *optval, long optlen);
	
	long	(*getsockopt)	(struct socket *s, short level, short optname,
			 	 char *optval, long *optlen);
};

#define DEBUG(x)
#define ALERT(x)
#define TRACE(x)
#define FATAL(x)
#define FORCE(x)


extern const char *socket_devname;

void printstr(const char *str);
extern MX_DDEV cdecl_socket_dev GNU_ASM_NAME("cdecl_socket_dev");
extern MX_DDEV socket_dev GNU_ASM_NAME("socket_dev");

void inet4_init(void);
void install_bios_handler(void *, void *) GNU_ASM_NAME("install_bios_handler");

extern struct dom_ops *alldomains;

struct socket *so_alloc(void);
long so_release(struct socket *so);
void so_sockpair(struct socket *so1, struct socket *so2);
long so_connect(struct socket *server, struct socket *client, short backlog, short nonblock, short wakeup);
long so_free(struct socket *so);
void so_register(short domain, struct dom_ops *ops);
#define so_free(sock) so_release(sock)

#define IO_Q 3

void wake(int queue, long cond);
int sleep(int queue, long cond);
long so_rselect(struct socket *so, long proc);
long so_wselect(struct socket *so, long proc);
long so_xselect(struct socket *so, long proc);
void so_wakersel(struct socket *so);
void so_wakewsel(struct socket *so);
void so_wakexsel(struct socket *so);
void wakeselect(long proc);
long cdecl unixtime(unsigned short time, unsigned short date);

void uninstall_xbra(void) GNU_ASM_NAME("uninstall_xbra");
