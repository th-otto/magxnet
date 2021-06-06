#include <stdio.h>
#include <time.h>

typedef void (*ExitFn) __PROTO( (void));

int errno;
ExitFn *_at_exit;
void *_FilSysVec;
int __mint;								/* 0 for TOS, MiNT version number otherwise */
int _pdomain;							/* errorcode of Pdomain call */

char _rootdir;							/* user's preferred root directory */

clock_t _starttime;						/* 200 HZ tick when we started the program */
clock_t _childtime;						/* time consumed so far by our children */
FILE _iob[FOPEN_MAX];
char __unused[10];
