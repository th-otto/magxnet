#include <stat.h>
#include "lib.h"

__EXTERN int _do_stat __PROTO((const char *_path, struct stat * st, int lflag));

int stat(path, st)
const char *path;
struct stat *st;
{
	return _do_stat(path, st, 0);
}
