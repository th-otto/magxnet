/*
 * lockf(3) and flock(2) emulation for MiNT by Dave Gymer
 * Placed in the public domain; do with me as you will!
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <file.h>
#include "lib.h"

int flock(fd, op)
int fd,
	op;
{
	int cmd;

	if (op & (LOCK_SH | LOCK_EX))
		cmd = (op & LOCK_NB) ? F_TLOCK : F_LOCK;
	else if (op & LOCK_UN)
		cmd = F_ULOCK;
	else
	{
		errno = -EINVAL;
		return -1;
	}
	return _do_lock(fd, cmd, 0L, 0);
}
