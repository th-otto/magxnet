#ifndef _GRP_H
#define _GRP_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifndef _POSIX_SOURCE
#ifndef _STDIO_H
#include <stdio.h>
#endif
#endif /* _POSIX_SOURCE */

#ifdef __cplusplus
extern "C" {
#endif

struct group
{
  char *gr_name;    /* The name of the group        */
  _GID_T gr_gid;    /* The numerical group ID       */
  char **gr_mem;    /* array of member names        */
  char *__gr_passwd;/* The encrypted group password */
};

#ifndef _POSIX_SOURCE
#define gr_passwd __gr_passwd
__EXTERN void   setgrent __PROTO((void));
__EXTERN void   endgrent __PROTO((void));
__EXTERN struct group * getgrent __PROTO((void));
__EXTERN struct group * fgetgrent __PROTO((FILE *f));
#endif /* _POSIX_SOURCE */

__EXTERN struct group * getgrgid __PROTO((int gid));
__EXTERN struct group * getgrnam __PROTO((const char *name));

#ifdef __cplusplus
}
#endif

#endif /* _GRP_H */
