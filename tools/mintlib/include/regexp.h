/* The <regexp.h> header is used by the (V8-compatible) regexp(3) routines. */

#ifndef _REGEXP_H
#define _REGEXP_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifndef _TYPES_H
#include <types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CHARBITS 0377
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	size_t regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;


__EXTERN regexp *regcomp __PROTO((char *_exp));
__EXTERN int 	regexec	__PROTO((regexp *_prog, char *_string, int _bolflag));
__EXTERN void regsub	__PROTO((regexp *_prog, char *_source, char *_dest));
__EXTERN void regerror	__PROTO((char const *_message));
__EXTERN void regdump __PROTO((regexp *r));

#ifdef __cplusplus
}
#endif

#endif /* _REGEXP_H */
