#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "stsocket.h"

#ifdef __PUREC__
# include <tos.h>
# ifdef __TOS /* using original header file */
#  define __XATTR
#  define st_size size
# endif
#else
# include <osbind.h>
# include <mintbind.h>
# include <sys/stat.h>
#endif

