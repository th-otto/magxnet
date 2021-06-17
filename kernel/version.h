/*
 *	File for keeping version information.
 *
 *	11/12/93, kay roemer.
 */

# ifndef _version_h
# define _version_h


# define VER_MAJOR	1
# define VER_MINOR	4
# define VER_PL		2

# if 0
# define ALPHA
# endif

# if 1
# define BETA
# endif

#if defined(ALPHA)
# define VER_STATUS	"a"
#elif defined(BETA)
# define VER_STATUS	"b"
#else
# define VER_STATUS	""
#endif

# define MSG_VERSION __STRINGIFY(VER_MAJOR) "." __STRINGIFY(VER_MINOR) __STRINGIFY(VER_PL) VER_STATUS

# endif /* _version_h */
