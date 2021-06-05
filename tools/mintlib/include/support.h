/*
 * support.h
 *  prototypes for miscellaneous functions in the library
 *		++jrb
 */
#ifndef _SUPPORT_H
#define _SUPPORT_H

#ifndef _COMPILER_H
#include <compiler.h>
#endif

#include <time.h>	/* for time_t */

#ifdef __cplusplus
extern "C" {
#endif


/* filename mapping function type */
#ifndef __FNMAP
#define __FNMAP
#ifdef __STDC__
typedef void (*fnmapfunc_t)(const char *, char *);
#else
typedef void (*fnmapfunc_t)();
#endif
#endif

__EXTERN int _unx2dos __PROTO((const char *, char *, size_t));
__EXTERN int _dos2unx __PROTO((const char *, char *, size_t));
__EXTERN int _full_dos2unx __PROTO((char *dos, char *unx));
__EXTERN void fnmapfunc __PROTO((fnmapfunc_t u2dos, fnmapfunc_t dos2u));
__EXTERN int unx2dos __PROTO((const char *u, char *d));
__EXTERN int dos2unx __PROTO((const char *d, char *u));
__EXTERN void _set_unixmode __PROTO((char *mode));
__EXTERN void _uniquefy __PROTO((char *dos));

__EXTERN int spawnve __PROTO((int, const char *, char *const *, char *const *));

__EXTERN int console_input_status __PROTO((int));
__EXTERN unsigned int console_read_byte __PROTO((int));
__EXTERN void console_write_byte __PROTO((int, int));

__EXTERN time_t dostime __PROTO((time_t));
__EXTERN time_t unixtime __PROTO((unsigned dostime, unsigned dosdate));

__EXTERN char *_buffindfile __PROTO((const char *fname, const char *fpath,
					char const *const *fext, char *buffer));
__EXTERN char *findfile __PROTO((const char *fname, const char *fpath,
					char const *const *fext));

__EXTERN char *_ultoa __PROTO((unsigned long n, char *buffer, int radix));
__EXTERN char *_ltoa __PROTO((long n, char *buffer, int radix));
__EXTERN char *_itoa __PROTO((int n, char *buffer, int radix));

__EXTERN long get_sysvar __PROTO((void *var));
__EXTERN void set_sysvar_to_long __PROTO((void *var, long val));

__EXTERN __EXITING	__exit	__PROTO((long status)) __NORETURN;
__EXTERN __EXITING _exit	__PROTO((int)) __NORETURN;

__EXTERN int _fork __PROTO((char *save_to));
__EXTERN int _wait __PROTO((int *exit_code));

__EXTERN void monstartup __PROTO((void *lowpc, void *highpc));
__EXTERN void monitor __PROTO((void *lowpc, void *highpc, void *buffer, unsigned long bufsize, unsigned int nfunc));
__EXTERN void moncontrol __PROTO((long flag));
__EXTERN void _mcleanup __PROTO((void));
__EXTERN int profil __PROTO((void *buff, unsigned long bufsiz, unsigned long offset, int shift));

__EXTERN long a64l __PROTO((const char *s));
__EXTERN char *l64a __PROTO((long l));

__EXTERN long tfork __PROTO((int (*func)(long), long arg));

__EXTERN int _isctty __PROTO((int));

__EXTERN int putenv __PROTO((const char *));

__EXTERN int _console_read_byte __PROTO((int));
__EXTERN void _console_write_byte __PROTO((int, int));
__EXTERN int _text_read __PROTO((int, char *, int));
__EXTERN int _text_write __PROTO((int, const char *, int));

__EXTERN int	getdtablesize __PROTO((void));
__EXTERN int	nice	__PROTO((int));
__EXTERN int	mknod	__PROTO((const char *, int, int));

__EXTERN int	sync	__PROTO((void));
__EXTERN int	fsync	__PROTO((int fd));

__EXTERN int	ffs	__PROTO((int));
__EXTERN int	gethostname __PROTO((char *buf, __SIZE_TYPEDEF__ len));

#ifdef __cplusplus
}
#endif

#endif /* _SUPPORT_H */
