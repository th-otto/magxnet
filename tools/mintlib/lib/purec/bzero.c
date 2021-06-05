#include <stddef.h>
#include <string.h>
#include <assert.h>

#undef ODD
#define ODD(x) (((short)(x)) & 1)	/* word ops are faster */

/*
 * zero out a chunk efficiently
 * handles odd address
 *
 *   ++jrb  bammi@dsrgsun.ces.cwru.edu
 */

#define INC(b, size) b = (void *)( ((char *)(b)) + (size) )

void _bzero(b, n)
void * b;
register unsigned long n;
{
    register unsigned long l, w;
    
    if(ODD(b))
    {
	*(char *)b = (char)0;
	INC(b, 1);
	n--;
    }

    l = (n >> 2); /* # of longs */
    n -= (l << 2);
    w = (n >> 1); /* # of words */
    n -= (w << 1); /* n == # of residual bytes */

    while(l--) {
	*((long *)b) = 0L;
	INC(b, sizeof(long));
    }
    while(w--) {
	*((short *)b) = (short)0;
	INC(b, sizeof(short));
    }
    while(n--) {
	*(char *)b = (char)0;
	INC(b, 1);
    }
}

void bzero(b, n)
void * b;
size_t n;
{
    _bzero(b, (unsigned long) n);
}
