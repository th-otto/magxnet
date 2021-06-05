#ifndef _PARAM_H
#define _PARAM_H

#define	MAXPATHLEN	128		/* same as FILENAME_MAX in stdio.h */
#define	NOFILE		20		/* same as OPEN_MAX in limits.h */

#ifdef __MINT__
# define HZ		200		/* ticks/second reported by times() */
# define NCARGS		1024		/* actually, we don't limit this */
#else
# define HZ		60		/* ticks/second reported by times() */
# define NCARGS		126		/* max. no. of characters in argv */
#endif

#define FSCALE 2048

#endif /* _PARAM_H */
