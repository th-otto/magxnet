/*
 * Filename:     main.c
 * Project:      GlueSTiK
 * 
 * Note:         Please send suggestions, patches or bug reports to
 *               the MiNT mailing list <freemint-discuss@lists.sourceforge.net>
 * 
 * Copying:      Copyright 1999 Frank Naumann <fnaumann@freemint.de>
 * 
 * Portions copyright 1997, 1998, 1999 Scott Bigham <dsb@cs.duke.edu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

# include <stdio.h>
# include <netdb.h>
# include <string.h>

# ifdef __PUREC__
# include <tos.h>
#define C_MagX 0x4D616758L     /* MagX */
#define C_MiNT 0x4D694E54L     /* Mint/MultiTOS */
#define C_STiK 0x5354694BL     /* ST Internet Kit */
# else
# include <mint/osbind.h>
# include <mint/mintbind.h>
# include <mint/basepage.h>
# include <mint/ssystem.h>
# include <mint/cookie.h>
#endif

# include "gs.h"

# include "gs_conf.h"
# include "gs_func.h"
# include "gs_mem.h"
# include "gs_stik.h"
# include "version.h"

/*
 * We need the MiNT definitions here, not any library definitions
 */
#undef SIGHUP
#define SIGHUP 1
#undef SIGINT
#define SIGINT 2
#undef SIGQUIT
#define SIGQUIT 3
#undef SIGABRT
#define SIGABRT 6
#undef SIGALRM
#define SIGALRM 14
#undef SIGTERM
#define SIGTERM 15
#undef SIGTSTP
#define SIGTSTP 18
#undef SIGUSR1
#define SIGUSR1 29
#undef SIGUSR2
#define SIGUSR2 30


# define MSG_VERSION	str (VER_MAJOR) "." str (VER_MINOR)
# define MSG_BUILDDATE	__DATE__

#if 0
# define MSG_BOOT	\
	"\033p GlueSTiK\277 STiK emulator for MiNTnet version " \
	MSG_VERSION " \033q\r\n"


#else

# define MSG_BOOT	\
	"\033p GlueSTiK\277 STiK emulator for MagiCNet version 0.15 \033q\r\n"

/*
 * If you change anything here,
 * also take a look at the TPL trampoline in gs_stik.c
 */
# define MSG_GREET	\
	"Redirect Daemon\r\n" \
	"\275 1996-98 Scott Bigham.\r\n" \
	"\275 2000 Frank Naumann.\r\n" \
	"\275 Mar 22 2002 Vassilis Papathanassiou.\r\n"

#endif

# define MSG_ALPHA	\
	"\033p WARNING: This is an unstable version - alpha! \033q\7\r\n"

# define MSG_MINT	\
	"\7This program requires FreeMiNT!\r\n"
# define MSG_MAGIC	\
	"\7This program requires MagiC!\r\n"

# define MSG_ALREADY	\
	"\7There is an active STiK Cookie!\r\n"

# define MSG_FAILURE	\
	"\7Sorry, driver NOT installed - initialization failed!\r\n\r\n"

int (*init_funcs [])(void) =
{
	init_stik_if,
	load_config_file,
	init_mem,
	NULL
};

void (*cleanup_funcs [])(void) =
{
	cleanup_config,
	cleanup_mem,
	cleanup_stik_if,
	NULL
};

static void
cleanup (void)
{
	int i;
	
	for (i = 0; cleanup_funcs [i]; i++)
		(*cleanup_funcs [i])();
}


/* ------------------
   | Install cookie |
   ------------------ */
#if 0
static int
install_cookie (void)
{
	long dummy;
	
	if (Ssystem (-1, 0, 0) == -32)
	{
		(void) Cconws (MSG_MINT);
		return 1;
	}

	if (Ssystem (S_GETCOOKIE, C_STiK, (long)&dummy) == 0)
	{
		(void) Cconws (MSG_ALREADY);
		return 1;
	}
	
	if (Ssystem (S_SETCOOKIE, C_STiK, (long) &stik_driver) != 0)
	{
		return 1;
	}
	
	return 0;
}

/* ------------------
   | Remove cookie |
   ------------------ */
static void
uninstall_cookie (void)
{
# ifndef S_DELCOOKIE
# define S_DELCOOKIE	26
# endif

	Ssystem(S_DELCOOKIE, C_STiK, 0L);
}

#else

static long get_jar(void)
{
	return *((long *)0x5a0);
}


static long *get_cookie (long id, long *value)
{
	long *jar;
	
	jar = (long *)Supexec(get_jar);
	if (jar != NULL)
	{
		while (jar[0] != 0)
		{
			if (jar[0] == id)
			{
				if (value)
					*value = *++jar;
				return jar;
			}
			jar += 2;
		}
	}
	return NULL;
}


static int install_cookie(long *values)
{
	unsigned long *jar;
	int size;
	unsigned long value;

	jar = (unsigned long *)Supexec(get_jar);
	size = 0;
	if (jar != NULL)
	{
		while ((value = jar[0]) != 0)
		{
			jar += 2;
			size++;
		}
		if ((unsigned long)size < jar[1])
		{
			jar[2] = value;
			jar[3] = jar[1];
			*jar++ = *values++;
			*jar++ = *values++;
			return TRUE;
		}
	}
	return FALSE;
}


static int install_stik_cookie(void)
{
	long value;
	long cookie[2];
	
	if (get_cookie(C_MagX, &value) == NULL && /* 'MagX' */
		get_cookie(C_MiNT, &value) == NULL)   /* 'MiNT */
	{
		(void) Cconws (MSG_MAGIC);
		return 1;
	}
	if (get_cookie(C_STiK, &value) != NULL)
	{
		(void) Cconws(MSG_ALREADY);
		return 1;
	}
	cookie[0] = C_STiK;
	cookie[1] = (long) &stik_driver;
	if (install_cookie(cookie) == FALSE)
		return 1;
	
	return 0;
}


/* ------------------
   | Remove cookie |
   ------------------ */
static void uninstall_cookie (void)
{
	long sp;
	long *jar;
	
	sp = Super(NULL);
	jar = *((long **)0x5a0);
	if (jar != NULL)
	{
		while (jar[0] != 0)
		{
			if (jar[0] == C_STiK)
			{
				*jar++ = 0x46524545L; /* 'FREE' */
				*jar++ = 0;
			}
			jar += 2;
		}
	}
	Super((void *)sp);
}


#endif


int
main (void)
{
	int i;

	(void) Cconws (MSG_BOOT);
	(void) Cconws (MSG_GREET);
# ifdef ALPHA
	(void) Cconws (MSG_ALPHA);
# endif
	(void) Cconws ("\r\n");
	
	for (i = 0; init_funcs [i]; ++i)
	{
		if (!(*init_funcs [i])())
		{
			(void) Cconws (MSG_FAILURE);
			
			cleanup ();
			exit (1);
		}
	}
		
	if (install_stik_cookie ())
	{
		(void) Cconws (MSG_FAILURE);
		
		cleanup ();
		exit (1);
	}
		
	Ptermres(-1, 0);

	cleanup();
	uninstall_cookie ();
	
	return 0;
}
