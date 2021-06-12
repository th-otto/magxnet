/*
 *	Definitions to handle io vectors.
 *
 *	11/13/93, kay roemer.
 */

# ifndef _sockets_iov_h
# define _sockets_iov_h

#include <iovec.h>

long iov2buf_cpy (char *, long, const struct iovec *, short, long);
long buf2iov_cpy (char *, long, const struct iovec *, short, long);
long iov_size (const struct iovec *iov, long n);


# endif /* _sockets_iov_h */
