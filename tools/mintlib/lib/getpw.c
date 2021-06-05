/*
Subject:  v08i080:  Public-domain getpw*(3) routines
Newsgroups: mod.sources
Approved: mirror!rs

Submitted by: emoryu1!emoryu2!arnold (Arnold D. Robbins)
Mod.sources: Volume 8, Issue 80
Archive-name: getpw

Here is public domain re-implementation of the getpwent(3) routines. I have
not included a manual page, since every Unix system has one. I also haven't
even bothered to include a <pwd.h> file; you should be able to use the one
on your system.

There is one additional routine:
      setpwfile (file)
      char *file;
which will cause the routines to find password records from a file besides
/etc/passwd.  This is useful should you need to use saved password files,
for instance in doing Unix accounting, where you wish to keep info around on
old accounts, but take the old accounts out of the live password file.
(Can you guess why I just whipped these up?)

To switch files, call setpwfile as the very first thing, or call endpwent(),
then setpwfile ("/some/file").

Anyway, I hope this is useful to somene out there.
Arnold Robbins
arnold@emoryu1.{CSNET, UUCP, ARPA, BITNET}
*/

/* minorly customized by ERS for the MiNT library */

/****************************************************************/
/* Module name:   getpw.c                                       */
/* Library name:  mintlibs for Atari ST                         */
/* Author:        Hildo Biersma (boender@dutiws.twi.tudelft.nl) */
/* Date:          January 29, 1993                              */
/* Revision:      1 (add password aging, add System V routines, */
/*                   include three man pages, add example code) */
/****************************************************************/

/*
NAME
    getpw - get name from UID

SYNOPSIS
    #include <pwd.h>

    int getpw(int uid, char *buf);

DESCRIPTION
    getpw searches the password file for a user id number that
    equals uid, copies the line of the password file in which uid
    was found into the array pointed to by buf, and returns 0.
    getpw returns non-zero if uid cannot be found.

    This routine is included only for compatibility with old UN*X
    systems and should not be used in any new programs; see
    getpwent for routines to use instead.

FILES
    /etc/passwd

SEE ALSO
    getpwent

AUTHOR
    Hildo Biersma, with the help of a UN*X System V man page.
*/

/*
NAME
    getpwent, getpwuid, getpwnam, setpwent, endpwent,
    fgetpwent, setpwfile - get password file entry

SYNOPSIS
    #include <stdio.h>  // Only needed for fgetpwent
    #include <pwd.h>

    struct passwd *getpwent(void);

    struct passwd *getpwuid(int uid);

    struct passwd *getpwnam(const char *name);

    void setpwent(void);

    void endpwent(void);

    struct passwd *fgetpwent(FILE *f);

    void setpwfile(const char *name);

DESCRIPTION
    getpwent, getpwuid and getpwnam each returns a pointer to an
    object containing the broken-out fields of a line in the
    /etc/passwd file.
    Each line in the file contains a "passwd" structure, declared
    in the <pwd.h> header file. For the definition of this structure
    and the meaning of the fields, see the <pwd.h> header file.

    getpwent when first called returns a pointer to the first
    passwd structure in the file; thereafter, it returns a pointer
    to the next passwd structure in the file; so successive calls
    can be used to search the entire file.

    getpwuid searches from the beginning of the file until a
    numerical user ID is found matching uid is found and returns
    a pointer to the particular structure in which it was found.
    Thus, it cannot be found to search for several users having
    duplicate numerical user IDs.

    getpwnam searches from the beginning of the file until a
    login name matching name is found and returns a pointer to
    the particular structure in which it was found.

    If an EOF or an error is encountered on reading, getpwent,
    getpwuid and getpwnam return a NULL pointer.

    A call to setpwent has the effect of rewinding the password
    file to allow repeated searches.

    endpwent may be called to close the password file when
    processing is complete.

    fgetpwent returns a pointer to the next passwd structure in
    the stream f, which matches the format of /etc/passwd.

    setpwfile will set the password file to be read from by all
    the above routines except fgetpwent.   

FILES
    /etc/passwd

SEE ALSO
    getlogin, getgrent, putpwent

NOTES
    All information is contained in a static area, so it must
    be copied if it is to be saved.

    Password aging is supported.

    In order not to suffer from possible future changes in the
    format of the passwd file or in the passwd structure,
    these routines should always be used for reading from file,
    while putpwent should always be used for writing to file.

    These routines expect the passwd file to be of type text,
    and can therefore read both DOS and UN*X format files.

AUTHORS
    Arnold D. Robbins - original version
    Eric Smith        - adapted for the mintlibs
    Hildo Biersma     - included System V routines and password aging
*/

/*
PASSWORD AGING

DESCRIPTION
    Password aging is in effect for a particular user if his
    encrypted password in the password file is followed by a
    comma and a non-null string of characters from the 
    alphabet (., /, 0-9, A-Z, a-z) also used for the encrypted
    password. (Such a string must be introduced in the first
    place by the super-user.)

    The first character of the age, M say, denotes the maximum
    number of weeks for which a password is valid. A user who
    attempts to login after his password has expired should be
    forced to supply a new one. The next character, N say,
    denotes the minimum number of weeks which must expire
    before the password may be changed. The remaining characters
    define the week (counted from the beginning of 1970) when
    the password was last changed. (A null string is equivalent
    to zero.) M and N have numerical values in the range 0-63
    that correspond to the 64-character alphabet mentioned
    above (i.e., / = 1 week; z = 63 weeks). If M = N = 0
    (derived from the string . or ..) the user should be forced
    to change his password the next time he logs in (and the
    "age" should disappear from his entry in the password file).
    If N > M (signified, e.g., by the string /.) only the
    super-user should be able to change the password.

SEE ALSO
    a64l, getpwent, putpwent

NOTE
    The compliance with password aging in MiNT is a joke, since
    users are not forced to login. Also, the current login program
    supplied in init(1) does not support password aging. This may
    change in the future.
*/

/*
  FIXME:
  - test *thoroughly*
  - write example code for modifying passwd files, include below
*/

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <types.h>

static char const *pwdfile = "/etc/passwd";	/* default passwd file */

static FILE *fp = NULL;
static struct passwd curentry;			/* static data to return */

static int nextent __PROTO((FILE * fp));

/* Get name from uid */
int getpw(uid, buf)
int uid;
char *buf;
{
	FILE *pw;
	char line[256];						/* Assume this will always be enough */
	char *ptr;

	if ((pw = fopen("/etc/passwd", "rt")) == NULL)
		return (-1);					/* Failure: file could not be opened */

	line[255] = 0x00;
	while (!feof(pw))
	{
		fgets(line, 255, pw);
		if ((ptr = strchr(line, ':')) != NULL)	/* Password field found */
			if ((ptr = strchr(ptr + 1, ':')) != NULL)	/* UID field found */
				if (atoi(ptr + 1) == uid)
				{
					strcpy(buf, line);
					return (0);			/* Success */
				}
	}
	return (-1);						/* Failure */
}										/* End of getpw() */

/* Set the name of the passwd file */
void setpwfile(cp)
char *cp;
{
	endpwent();
	if (cp != NULL)
		pwdfile = cp;
}										/* End of setpwfile() */

/* Rewind the passwd file */
void setpwent()
{
	if (fp != NULL)
		rewind(fp);
	else if ((fp = fopen(pwdfile, "rt")) == NULL)
	{
#ifdef VERBOSE
		fprintf(stderr, "setpwent: %s non-existant or unreadable.\n", pwdfile);
#endif
	}
}										/* End of setpwent() */

/* End passwd file processing */
void endpwent()
{
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
}										/* End of endpwent() */

/* Get the next passwd entry from the passwd file */
struct passwd *getpwent()
{
	if (fp == NULL)
	{
		setpwent();
		if (fp == NULL)
			return (NULL);
	}

	if (nextent(fp) == 0)
		return (NULL);
	else
		return (&curentry);
}										/* End of getpwent() */

/* Get first passwd entry from file with pw_uid matching uid */
#ifdef __STDC__
struct passwd *getpwuid(uid_t uid)
#else
struct passwd *getpwuid(uid)
uid_t uid;
#endif
{
	setpwent();
	while (nextent(fp) != 0)
		if (curentry.pw_uid == uid)
		{
			endpwent();
			return (&curentry);
		}

	endpwent();
	return (NULL);
}										/* End of getpwuid() */

/* Get first passwd entry from file with pw_name matching name */
struct passwd *getpwnam(name)
const char *name;
{
	setpwent();

	while (nextent(fp) != 0)
		if (strcmp(curentry.pw_name, name) == 0)
		{
			endpwent();
			return (&curentry);
		}

	endpwent();
	return (NULL);
}										/* End of getpwnam() */

/* Read the next password structure from a given file */
struct passwd *fgetpwent(f)
FILE *f;
{
	if (f == NULL)
		return (NULL);					/* Failure */

	if (nextent(f) != 0)
		return (&curentry);				/* Success */

	return (NULL);						/* Failure */
}										/* End of fgetpwent() */

#if defined(atarist) || defined(__SOZOBON__)
static char savbuf[256];				/* BUFSIZ seems bigger than necessary! */
#else
static char savbuf[BUFSIZ];
#endif

/* Static function getting next entry from passwd file */
static int nextent(fp)
FILE *fp;
{
	char *cp;

	if (fp == NULL)
	{
		setpwent();
		if (fp == NULL)
			return (0);
	}

	while (fgets(savbuf, (int) sizeof(savbuf), fp) != NULL)
	{
		curentry.pw_age = NULL;

		for (cp = savbuf; ((*cp != 0x00) && (*cp != ':')); cp++)
			;
		curentry.pw_name = savbuf;
		*cp++ = 0x00;
		curentry.pw_passwd = cp;
		for (; ((*cp != 0x00) && (*cp != ':')); cp++)
			if ((*cp == ',') && (curentry.pw_age == NULL))
			{
				/* Password age field found */
				*cp++ = 0x00;
				curentry.pw_age = cp;
			}
		*cp++ = 0x00;
		curentry.pw_uid = atoi(cp);
		for (; ((*cp != 0x00) && (*cp != ':')); cp++)
			;
		*cp++ = 0x00;
		curentry.pw_gid = atoi(cp);
		for (; ((*cp != 0x00) && (*cp != ':')); cp++)
			;
		*cp++ = 0x00;
		curentry.pw_gecos = cp;
		curentry.pw_comment = cp;
		for (; ((*cp != 0x00) && (*cp != ':')); cp++)
			;
		*cp++ = 0x00;
		curentry.pw_dir = cp;
		for (; ((*cp != 0x00) && (*cp != ':')); cp++)
			if (*cp == '\\')
				*cp = '/';
		*cp++ = 0x00;
		curentry.pw_shell = cp;
		for (; ((*cp != 0x00) && (*cp != ':') && (*cp != '\n')); cp++)
			if (*cp == '\\')
				*cp = '/';
		*cp = 0x00;
		if (curentry.pw_age == NULL)
			curentry.pw_age = cp;		/* Don't return NULL, return a null string */
		return (1);
	}
	return (0);
}										/* End of nextent() */

/* Old test program removed - Hildo Biersma */
#ifdef EXAMPLE

/*
EXAMPLE PROGRAM

DESCRIPTION
    Read all lines from the password file and print all users
    with password aging info. Also print a description of
    the meaning of the aging information of this user.

AUTHOR
    Hildo Biersma (boender@dutiws.twi.tudelft.nl)

STATUS
    Example code; in the public domain.
*/

#include <support.h>

void main()
{
	struct passwd *this_entry;
	int max_age,
	 min_age;
	char *ptr,
	 min[2],
	 max[2];
	time_t time;

	min[1] = max[1] = 0x00;
	while ((this_entry = getpwent()) != NULL)
		if ((this_entry->pw_age[0]) != 0x00)
		{
			if ((max[0] = this_entry->pw_age[0]) != 0x00)
			{
				max_age = (int) a64l(max);
				if ((min[0] = this_entry->pw_age[1]) != 0x00)
				{
					min_age = (int) a64l(min);
					ptr = this_entry->pw_age + 2;
					time = a64l(ptr) * (3600L * 24 * 7);	/* Seconds per week */
				} else
					time = 0L;
			} else
			{
				min_age = 0;
				time = 0L;
			}
			fprintf(stdout, "Name: %s, UID: %d.\n", this_entry->pw_name, this_entry->pw_uid);
			if ((max_age == 0) && (min_age == 0))
				fprintf(stdout, "User should change password on next login.\n\n");
			else if (min_age > max_age)
				fprintf(stdout, "Only super-user may change password.\n\n");
			else
			{
				fprintf(stdout, "Must change every %d weeks.\n", max_age);
				fprintf(stdout, "May change every %d weeks.\n", min_age);
				fprintf(stdout, "Last changed in week starting %s\n\n", ctime(&time));
			}
		}

	endpwent();
}										/* End of main() */
#endif /* EXAMPLE */
