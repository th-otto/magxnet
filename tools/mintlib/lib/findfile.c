#include <compiler.h>
#include <limits.h>						/* needed for PATH_MAX */
#include <support.h>

char *findfile(fname, fpath, fext)
const char *fname,
*fpath;
char const *const *fext;
{
	/* simply calls _buffindfile */
	static char try[PATH_MAX];

	return _buffindfile(fname, fpath, fext, try);
}
