/*
 * sys/utsname.h - header for uname emulation
 * Written by Dave Gymer and placed in the public domain.
 *
 * nodename and machine fields size increased by Mikko Larjava
 */

#ifndef _UTSNAME_H
#define _UTSNAME_H

#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

struct utsname {
	char sysname[9];
	char nodename[16];
	char release[9];
	char version[9];
	char machine[16];
};

__EXTERN int uname __PROTO((struct utsname *));

#ifdef __cplusplus
}
#endif

#endif /* _UTSNAME_H */
