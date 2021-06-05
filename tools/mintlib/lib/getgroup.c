/*
 * FILE
 *    getgroup.c
 *
 * PURPOSE
 *    get the groups the current user is in
 *
 * AUTHORS
 *    written by Ole Arndt and placed in the public domain :-)
 *
 * BUGS
 *    under BSD user has access to all groups he is in at the same time
 *    under MiNT there is only one valid group at a time.
 *  so for suid programs this function gives you the groups you can
 *    switch to for your real user.
 */

#include <stdio.h>
#include <string.h>
#include <types.h>
#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <mintbind.h>
#ifdef TEST
#include <stdlib.h>						/* for calloc() */
#endif

int getgroups(gsetlen, grpset)
int gsetlen;
gid_t *grpset;
{
	struct group *gentry;
	int numgroups;
	struct passwd *userpw;
	int i;
	gid_t currgid;
	long r;

	r = Pgetgroups(gsetlen, grpset);

	if (r != EINVAL)
	{
		if (r < 0)
		{
			errno = -(int)r;
			return -1;
		}
		return (int)r;
	}

	currgid = (gid_t) Pgetgid();
	if (currgid == (gid_t) (-EINVAL))
		return 0;

	if (gsetlen)
	{
		if (gsetlen < 0 || !grpset)
		{
			errno = EINVAL;
			return -1;
		}
		*grpset++ = currgid;
	}

	if (geteuid() && getegid())
		return 1;

	userpw = getpwuid(getuid());
	if (!userpw)
	{
		return -1;
	}

	numgroups = 1;
	setgrent();
	while ((gentry = getgrent()) != NULL)
	{
		for (i = 0; gentry->gr_mem[i]; i++)
		{
			if (!strcmp(userpw->pw_name, gentry->gr_mem[i]) && (gentry->gr_gid != currgid))
			{
				++numgroups;
				if (gsetlen)
				{
					if (numgroups > gsetlen)
					{
						errno = EINVAL;
						return -1;
					}
					*grpset++ = gentry->gr_gid;
					break;
				}
			}
		}
	}
	endgrent();
	return numgroups;
}

#ifdef TEST

int main(void)
{
	int ngroups;
	int num,
	 i;
	gid_t *grps;
	struct passwd *userpw;

	ngroups = getgroups(0, (gid_t *) NULL);
	if (ngroups < 0)
	{
		perror("getgroups() first call failed");
		exit(2);
	}
	printf("first call to getgroups(): %d\n", ngroups);
	grps = (gid_t *) calloc(ngroups, sizeof(gid_t));
	if (!grps)
	{
		perror("calloc() failed");
		exit(2);
	}
	num = getgroups(ngroups, grps);
	if (num < 0)
	{
		perror("getgroups() second call failed");
		exit(2);
	}
	printf("second call to getgroups(): %d\n", num);
	if (ngroups != num)
	{
		printf("getgroups() results do not match\n");
		exit(2);
	}

	userpw = getpwuid(getuid());
	printf("User %s is in the following groups: ", userpw->pw_name);

	if (num == 0)
		printf("none");
	else
		for (i = 0; i < num; i++)
			printf("%d ", grps[i]);
	printf(".\n");

	return 0;
}

#endif
