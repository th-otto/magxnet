#include <ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#define LINE_MAX 80

extern int __mint;

char *getpass(prompt)
const char *prompt;
{
	static char buf[LINE_MAX + 1];
	char *ret;
	struct sgttyb oldsb,
	 newsb;
	FILE *tty;
	int l,
	 c,
	 ttyfd;

	fflush(stdin);
	tty = stdin;
	if (__mint)
	{
		if ((tty = fopen("U:\\DEV\\TTY", "r")) == NULL)
			return NULL;
	}
	ttyfd = fileno(tty);
	fflush(tty);
	gtty(ttyfd, &oldsb);
	newsb = oldsb;
	newsb.sg_flags &= ~ECHO;
	stty(ttyfd, &newsb);
	fputs(prompt, stderr);
	fflush(stderr);
	if ((ret = fgets(buf, LINE_MAX, tty)) != 0)
	{
		/* zap the newline; if we get an EOF instead, 
		   we zap that, too. */
		l = (int)strlen(buf);
		if (buf[l - 1] != '\n')
		{
			while (((c = fgetc(tty)) != '\n') && (c != EOF))
				/* wait for a newline or an EOF */ ;
		}
		if (l > PASS_MAX)
			buf[PASS_MAX] = '\0';
		else if (buf[l - 1] == '\n')
			buf[l - 1] = '\0';
	}
	stty(ttyfd, &oldsb);
	if (__mint)
		(void) fclose(tty);
	return ret;
}
