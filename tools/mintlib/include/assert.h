/*
 * assert.h
 *    sec 4.2 ansi draft
 */

/* Allow this file to be included multiple times
   with different settings of NDEBUG.  */
#undef assert
#undef __assert

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

__EXTERN void __eprintf __PROTO((const char *expression, const long line,
				 const char *filename));
__EXTERN __EXITING abort __PROTO((void)) __NORETURN;

#ifdef __cplusplus
}
#endif

#ifdef NDEBUG
#define	assert(cond) ((void *) 0)
#define __assert(cond) ((void *) 0)
#else /* not NDEBUG */

#if __STDC__
#define assert(cond) \
((cond) ? 0 : \
    ( __eprintf(#cond,(long)(__LINE__), __FILE__), abort(), 0 ))
#define __assert(cond) \
if(!(cond)) \
    { __eprintf(#cond,(long)(__LINE__), __FILE__); abort(); }
#else /* not __STDC__ */

#ifndef __SOZOBON__
/* There's a bug in Sozobon 2.0 whereby __LINE__ & __FILE__ are defined but
 * testing #ifndef __?I?E__ comes out as if they aren't. */
#ifndef __LINE__
#define __LINE__ 0
#endif
#ifndef __FILE__
#define __FILE__ "unknown"
#endif
#endif /* not __SOZOBON__ */

#define assert(cond) \
((cond) ? 0 : \
    ( __eprintf("cond", (long)(__LINE__), __FILE__), abort(), 0))
#define __assert(cond) \
if(!(cond)) \
    { __eprintf("cond", (long)(__LINE__), __FILE__); abort(); }
#endif /* not __STDC__ */

#endif /* not NDEBUG */
