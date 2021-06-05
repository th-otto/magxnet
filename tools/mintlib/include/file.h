#ifndef _FILE_H
#define _FILE_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __MINT__
/* lockf() comands */
#define LOCK_SH	1	/* shared lock */
#define LOCK_EX	2	/* exclusive lock */
#define LOCK_NB	4	/* non-blocking */
#define LOCK_UN	8	/* unlock */

#define FREAD  1
#define FWRITE 2

__EXTERN int		flock	__PROTO((int, int));
#endif

#include <fcntl.h>

#endif /* _FILE_H */
