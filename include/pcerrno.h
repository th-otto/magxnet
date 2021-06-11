#include <errno.h>
#ifdef __PUREC__
/*
 * We need the MiNT error codes here, not any
 * UNIX-alike error code like defined in Pure-C <errno.h>
 */
#undef ENOSYS
#define	ENOSYS			(32)		/* Function not implemented.  */
#undef EMFILE
#define	EMFILE			(35)		/* Too many open files.  */
#undef EACCES
#define EACCES			(36)		/* Permission denied.  */
#undef EBADF
#define	EBADF			(37)		/* Bad file descriptor.  */
#undef ENOMEM
#define	ENOMEM			(39)		/* Cannot allocate memory.  */
#undef EBADARG
#define	EBADARG			(64)		/* Bad argument.  */
#undef EINTERNAL
#define	EINTERNAL		(65)		/* Internal error.  */
#undef ESBLOCK
#define	ESBLOCK			(67)		/* Memory block growth failure.  */
#undef EINTR
#define EINTR	        (128)		/* Interrupted function call.  */
#undef ENOTSOCK
#define	ENOTSOCK		(300)		/* Socket operation on non-socket.  */
#undef ESOCKTNOSUPPORT
#define	ESOCKTNOSUPPORT	(306)		/* Socket type not supported.  */
#undef EOPNOTSUPP
#define	EOPNOTSUPP		(307)		/* Operation not supported.  */
#undef EAFNOSUPPORT
#define	EAFNOSUPPORT	(309)		/* Address family not supported by protocol. */
#undef EADDRINUSE
#define	EADDRINUSE		(310)		/* Address already in use */
#undef EADDRNOTAVAIL
#define	EADDRNOTAVAIL	(311)		/* Cannot assign requested address.  */
#undef ECONNRESET
#define	ECONNRESET		(316)		/* Connection reset by peer.  */
#undef EISCONN
#define	EISCONN			(317)		/* Socket is already connected.  */
#undef ETIMEDOUT
#define	ETIMEDOUT		(320)		/* Connection timed out.  */
#undef ECONNREFUSED
#define	ECONNREFUSED	(321)		/* Connection refused.  */
#undef ENETDOWN
#define	ENETDOWN		(312)		/* Network is down.  */
#undef ENETUNREACH
#define	ENETUNREACH		(313)		/* Network is unreachable.  */
#undef EHOSTDOWN
#define	EHOSTDOWN		(322)		/* Host is down.  */
#undef EHOSTUNREACH
#define	EHOSTUNREACH	(323)		/* No route to host.  */
#undef EINPROGRESS
#define	EINPROGRESS		(325)		/* Operation now in progress.  */
#undef EWOULDBLOCK
#define EWOULDBLOCK		(326)		/* Operation would block.  */
#endif
