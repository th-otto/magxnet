#include "ifconf.h"

#ifdef __PUREC__
# include <tos.h>
# ifdef __TOS /* using original header file */
#  define __XATTR
#  define st_size size
# endif
# include <aes.h>
  int evnt_timer_purec(short lo, short hi);
#else
# include <gem.h>
#endif

#include "fxattr.h"

# define SYSVAR_bootdev	(*((unsigned short *) 0x446UL))


static char homedir[128];
static char currdir[128];
static int apid;
static int magx_sld_ovl_error;

const char *default_rc_net = "magx_rc.net";
const char *u_etc = "u:\\etc";
const char *dev_ppp0 = "u:\\dev\\ppp0";
char *magx_sld_ovl = "x:\\gemsys\\magic\\xtension\\magx_sld.ovl";
const char *magx_sld_env = "MAGX_SLD";





static long get_bootdrive(void)
{
	return SYSVAR_bootdev;
}


static void setobaud(const char *device, long baud)
{
	int fd;
	
	fd = (int)Fopen(device, O_RDWR);
	if (fd >= 0)
	{
		Fcntl(fd, (long)&baud, TIOCOBAUD);
		Fclose(fd);
	}
}

/*
 * XXX uses different address registers than original,
 * but has been manually compared to do the same thing
 */
static int parse_config(char *lineptr)
{
			struct in_addr addr;
			int link_flags;
	
	char *line = lineptr;
	char *cmd;
	char *argv[10] = { 0 };
	char *device;
	const char *ifname;
	int argc;
	int argn;
	int flags;
	
	device = NULL;
	ifname = NULL;
	line = strtok(line, " \t");
	if (line == NULL)
		return 0;

	argc = 0;
	cmd = line;
	for (;line != NULL;)
	{
		line = strtok(NULL, " \t");
		argv[argc] = line;
		argc++;
	}

	if (argc != 0 && strcmp(cmd, "etc_path") == 0)
	{
		Fsymlink(argv[0], u_etc);
	} else if (argc != 0 && strcmp(cmd, "ifconfig") == 0)
	{
		argn = argc - 1;
		ifname = argv[0];
		/* BUG: prevents 0x8000 to be used as IFF_ */
		if ((flags = get_if_flags(ifname)) < 0)
			return -1;
		link_flags = 0;
		get_link_flags(ifname, &link_flags);
		for (argc = 1; argc < argn; argc++)
		{
			if (strcmp(argv[argc], "addr") == 0)
			{
				argc++;
				line = (char *)gethostbyname(argv[argc]);
				if (line == NULL)
					return -1;
				/* FIXME: should check h_length to be size of in_addr */
				memcpy(&addr, ((struct hostent *)line)->h_addr, ((struct hostent *)line)->h_length);
				set_addr(ifname, SIOCSIFADDR, addr.s_addr);
				flags |= IFF_UP;
			} else if (strcmp(argv[argc], "dstaddr") == 0)
			{
				argc++;
				line = (char *)gethostbyname(argv[argc]);
				if (line == NULL)
					return -1;
				/* FIXME: should check h_length to be size of in_addr */
				memcpy(&addr, ((struct hostent *)line)->h_addr, ((struct hostent *)line)->h_length);
				set_addr(ifname, SIOCSIFDSTADDR, addr.s_addr);
			} else if (strcmp(argv[argc], "netmask") == 0)
			{
				argc++;
				/* FIXME: should check validity */
				addr.s_addr = inet_addr(argv[argc]);
				set_addr(ifname, SIOCSIFNETMASK, addr.s_addr);
			} else if (strcmp(argv[argc], "up") == 0)
			{
				flags |= IFF_UP;
			} else if (strcmp(argv[argc], "mtu") == 0)
			{
				argc++;
				set_mtu_metric(ifname, SIOCSIFMTU, atol(argv[argc]));
			} else if (strncmp(argv[argc], "link", 4) == 0)
			{
				int bit = atoi(&argv[argc][4]);
				if (bit < 0 || bit > 15)
					return -1;
				link_flags |= 1 << bit;
			}
		}
		set_if_flags(ifname, flags);
		set_link_flags(ifname, link_flags);
	} else if (argc != 0 && strcmp(cmd, "iflink") == 0)
	{
		long baud = 0;
		
		if (magx_sld_ovl_error == 0)
		{
			argn = argc - 1;
			for (argc = 0; argc < argn; argc++)
			{
				if (strcmp(argv[argc], "-i") == 0)
				{
					argc++;
					ifname = argv[argc];
				} else if (strcmp(argv[argc], "-d") == 0)
				{
					argc++;
					device = argv[argc];
				} else if (strcmp(argv[argc], "-s") == 0)
				{
					argc++;
					baud = atol(argv[argc]);
				} else
				{
					break;
				}
			}
			if (device != NULL && baud != 0)
				setobaud(device, baud);
			if (device != NULL && ifname != NULL)
				if_link(device, ifname);
		}
	} else if (argc != 0 && strcmp(cmd, "route") == 0)
	{
		argn = argc - 1;
		argc = 0;
		if (strcmp(argv[argc], "add") == 0)
			add_route(argn, argv);
	}
	
	return 0;
}


int main(void)
{
	struct xattr xattr;
	char linebuf[120];
	int retcode;
	char *home;
	FILE *fp;
	char *line;
	int i;
	
	retcode = 0;
	apid = appl_init();
	if (apid != -1)
	{
		if (appl_find(magx_sld_env) < 0 &&
			open_socket(1) >= 0)
		{
			*magx_sld_ovl = Supexec(get_bootdrive) + 'A';
			if (Fxattr(0, dev_ppp0, &xattr) == 0)
			{
				magx_sld_ovl_error = (int)Fxattr(0, magx_sld_ovl, &xattr);
				if (magx_sld_ovl_error == 0)
				{
					shel_write(1, 1, 100, magx_sld_ovl, NULL);
				}
			}
			/* 556 */
			home = getenv("HOME");
			if (home != NULL)
			{
				strcpy(homedir, home);
				if (homedir[strlen(homedir) - 1] != '\\')
					strcat(homedir, "\\");
				strcat(homedir, default_rc_net);
			}
			/* 594 */
			i = Dgetdrv();
			Dgetpath(&currdir[2], i + 1);
			currdir[0] = i + 'A';
			currdir[1] = ':';
			if (currdir[strlen(currdir) - 1] != '\\')
				strcat(currdir, "\\");
			strcat(currdir, default_rc_net);
			/* BUG: fp uninitialized if both files ot found */
#ifdef __GNUC__
			fp = NULL;
#endif
			if (Fxattr(0, homedir, &xattr) == 0)
			{
				if ((fp = fopen(homedir, "r")) == NULL)
					goto done;
				goto readfile; /* FIXME; unneeded goto */
			} else
			if (Fxattr(0, currdir, &xattr) != 0 || (fp = fopen(currdir, "r")) != NULL)
			{
			readfile:
				i = 0;
				/* wait for magx_sld.ovl to start up */
				/* FIXME: should be skipped if it was not started */
				while (sockets_dev->Fopen == 0)
				{
#ifdef __PUREC__
					evnt_timer_purec(1000, 0);
#else
					evnt_timer(1000);
#endif
					if (i++ > 5)
						break;
				}
				do
				{
					if ((line = fgets(linebuf, (int)sizeof(linebuf), fp)) == NULL)
						break;
					if (line[0] != '#' && line[0] != '\n' && line[1] != '\n') /* FIXME check for \r here */
					{
						line[strlen(line) - 1] = '\0';
						parse_config(line);
					}
				} while (line != NULL); /* FIXME: already checked above */
				fclose(fp);
			}
		done:
			Fclose(sock_fd);
		}
		/* 680 */
		appl_exit();
	} else
	{
		(void) Cconws("Error initializing MAGXCONF.PRG!!\n\r");
	}
	return retcode;
}

