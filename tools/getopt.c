#include <stdio.h>
#include <string.h>

/* WTF? */
#undef stderr
extern FILE _iob[];
#define stderr (&_iob[2])

#include "mintlib/lib/getopt.c"
