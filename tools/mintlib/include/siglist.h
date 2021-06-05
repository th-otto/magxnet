#ifndef _SIGLIST_H
#define _SIGLIST_H

#ifdef __cplusplus
extern "C" {
#endif

extern char *sys_siglist[];
#ifndef __MINT__
extern char *signal_names[];
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SIGLIST_H */
