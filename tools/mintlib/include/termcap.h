#ifndef _TERMCAP_H
#define _TERMCAP_H

#ifndef _COMPILER_H
#  include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* tgetent.c */
__EXTERN char *tgetent __PROTO((char *bp, char *name));

/* tgetflag.c */
__EXTERN int tgetflag __PROTO((char *id));

/* tgetnum.c */
__EXTERN int tgetnum __PROTO((char *id));

/* tgetstr.c */
__EXTERN char *tgetstr __PROTO((char *id, char **area));

/* tgoto.c */
__EXTERN char *tgoto __PROTO((char *cm, int destcol, int destline));

/* tputs.c */
__EXTERN void tputs __PROTO((char *cp, int affcnt, int (*outc)(int )));

#ifdef __cplusplus
}
#endif

#endif /* _TERMCAP_H */
