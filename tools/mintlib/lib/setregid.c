#include <types.h>
#include <unistd.h>
#include <osbind.h>
#include <mintbind.h>
#include <errno.h>

int setregid(rgid, egid)
int rgid,
	egid;

{
	long r;
	static short have_setregid = 1;

	if (have_setregid)
	{
		r = Psetregid(rgid, egid);
		if (r == -ENOSYS)
			have_setregid = 0;
		else if (r < 0)
		{
			errno = (int) -r;
			return -1;
		} else
			return 0;

	}
	return setgid(egid);
}
