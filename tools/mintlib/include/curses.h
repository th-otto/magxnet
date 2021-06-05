/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)curses.h	5.4 (Berkeley) 6/30/88
 */

# ifndef WINDOW

# include	<stdio.h>
# include	<sgtty.h>
# include	<string.h>

#ifdef atarist
#  include <memory.h>
#endif

#ifndef _COMPILER_H
#  include <compiler.h>
#endif

#ifdef __STDC__
#include <stddef.h>
#endif

#include <termcap.h>

#ifdef __cplusplus
extern "C" {
#endif

# define	bool	char
# define	reg	register

#ifndef TRUE
# define	TRUE	(1)
# define	FALSE	(0)
#endif
# define	ERR	(0)
# define	OK	(1)

# define	_ENDLINE	001
# define	_FULLWIN	002
# define	_SCROLLWIN	004
# define	_FLUSH		010
# define	_FULLLINE	020
# define	_IDLINE		040
# define	_STANDOUT	0200
# define	_NOCHANGE	-1

#ifndef DEBUG
#define _putchar	fputc
#else
#define _fputchar	_putchar
#endif

# define	_puts(s)	tputs(s, 0, _fputchar)

typedef	struct sgttyb	SGTTY;

/*
 * Capabilities from termcap
 */

#ifdef HZ
#undef HZ	/* in case they included sys/param.h */
#endif

extern bool     AM, BS, CA, DA, DB, EO, HC, HZ, IN, MI, MS, NC, NS, OS, UL,
		XB, XN, XT, XS, XX;
extern char	*AL, *BC, *BT, *CD, *CE, *CL, *CM, *CR, *CS, *DC, *DL,
		*DM, *DO, *ED, *EI, *K0, *K1, *K2, *K3, *K4, *K5, *K6,
		*K7, *K8, *K9, *HO, *IC, *IM, *IP, *KD, *KE, *KH, *KL,
		*KR, *KS, *KU, *LL, *MA, *ND, *NL, *RC, *SC, *SE, *SF,
		*SO, *SR, *TA, *TE, *TI, *UC, *UE, *UP, *US, *VB, *VS,
		*VE, *AL_PARM, *DL_PARM, *UP_PARM, *DOWN_PARM,
		*LEFT_PARM, *RIGHT_PARM;
extern char	PC;

/*
 * From the tty modes...
 */

extern bool	GT, NONL, UPPERCASE, normtty, _pfast;

struct _win_st {
	short		_cury, _curx;
	short		_maxy, _maxx;
	short		_begy, _begx;
	short		_flags;
	short		_ch_off;
	bool		_clear;
	bool		_leave;
	bool		_scroll;
	char		**_y;
	short		*_firstch;
	short		*_lastch;
	struct _win_st	*_nextp, *_orig;
};

# define	WINDOW	struct _win_st

extern bool	My_term, _echoit, _rawmode, _endwin;

extern char	*Def_term, ttytype[];

extern int	LINES, COLS, _tty_ch, _res_flg;

extern SGTTY	_tty;

extern WINDOW	*stdscr, *curscr;

/*
 *	Define VOID to stop lint from generating "null effect"
 * comments.
 */
# ifdef lint
int	__void__;
# define	VOID(x)	(__void__ = (int) (x))
# else
# define	VOID(x)	(x)
# endif

/*
 * psuedo functions for standard screen
 */
# define	addch(ch)	VOID(waddch(stdscr, ch))
# define	getch()		VOID(wgetch(stdscr))
# define	addbytes(da,co)	VOID(waddbytes(stdscr, da,co))
# define	addstr(str)	VOID(waddbytes(stdscr, str, (int)strlen(str)))
# define	getstr(str)	VOID(wgetstr(stdscr, str))
# define	move(y, x)	VOID(wmove(stdscr, y, x))
# define	clear()		VOID(wclear(stdscr))
# define	erase()		VOID(werase(stdscr))
# define	clrtobot()	VOID(wclrtobot(stdscr))
# define	clrtoeol()	VOID(wclrtoeol(stdscr))
# define	insertln()	VOID(winsertln(stdscr))
# define	deleteln()	VOID(wdeleteln(stdscr))
# define	refresh()	VOID(wrefresh(stdscr))
# define	inch()		VOID(winch(stdscr))
# define	insch(c)	VOID(winsch(stdscr,c))
# define	delch()		VOID(wdelch(stdscr))
# define	standout()	VOID(wstandout(stdscr))
# define	standend()	VOID(wstandend(stdscr))

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddbytes(win,y,x,da,co) \
		VOID(wmove(win,y,x)==ERR?ERR:waddbytes(win,da,co))
#define	mvwaddstr(win,y,x,str) \
		VOID(wmove(win,y,x)==ERR?ERR:waddbytes(win,str,(int)strlen(str)))
#define mvwgetstr(win,y,x,str)  VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : winch(win))
#define	mvwdelch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : wdelch(win))
#define	mvwinsch(win,y,x,c)	VOID(wmove(win,y,x) == ERR ? ERR:winsch(win,c))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define	mvaddbytes(y,x,da,co)	mvwaddbytes(stdscr,y,x,da,co)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define mvgetstr(y,x,str)       mvwgetstr(stdscr,y,x,str)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)
#define	mvdelch(y,x)		mvwdelch(stdscr,y,x)
#define	mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

/*
 * psuedo functions
 */

#define	clearok(win,bf)	 (win->_clear = bf)
#define	leaveok(win,bf)	 (win->_leave = bf)
#define	scrollok(win,bf) (win->_scroll = bf)
#define flushok(win,bf)	 (bf ? (win->_flags |= _FLUSH):(win->_flags &= ~_FLUSH))
#define	getyx(win,y,x)	 y = win->_cury, x = win->_curx
#define	winch(win)	 (win->_y[win->_cury][win->_curx] & 0177)

#define raw()	 (_tty.sg_flags|=RAW, _pfast=_rawmode=TRUE, stty(_tty_ch,&_tty))
#define noraw()	 (_tty.sg_flags&=~RAW,_rawmode=FALSE,_pfast=!(_tty.sg_flags&CRMOD),stty(_tty_ch,&_tty))
#define cbreak() (_tty.sg_flags |= CBREAK, _rawmode = TRUE, stty(_tty_ch,&_tty))
#define nocbreak() (_tty.sg_flags &= ~CBREAK,_rawmode=FALSE,stty(_tty_ch,&_tty))
#define crmode() cbreak()	/* backwards compatability */
#define nocrmode() nocbreak()	/* backwards compatability */
#define echo()	 (_tty.sg_flags |= ECHO, _echoit = TRUE, stty(_tty_ch, &_tty))
#define noecho() (_tty.sg_flags &= ~ECHO, _echoit = FALSE, stty(_tty_ch, &_tty))
#define nl()	 (_tty.sg_flags |= CRMOD,_pfast = _rawmode,stty(_tty_ch, &_tty))
#define nonl()	 (_tty.sg_flags &= ~CRMOD, _pfast = TRUE, stty(_tty_ch, &_tty))
#define	savetty() ((void) gtty(_tty_ch, &_tty), _res_flg = _tty.sg_flags)
#define	resetty() (_tty.sg_flags = _res_flg, (void) stty(_tty_ch, &_tty))

#define	erasechar()	(_tty.sg_erase)
#define	killchar()	(_tty.sg_kill)
#define baudrate()	(_tty.sg_ospeed)

__EXTERN int		waddbytes __PROTO((WINDOW *, char *, int));
__EXTERN void		_id_subwins __PROTO((WINDOW *));
__EXTERN void		_set_subwin_ __PROTO((WINDOW *, WINDOW *));
__EXTERN int 		_sprintw __PROTO((WINDOW *, char *, char *));
__EXTERN int 		_sscans __PROTO((WINDOW *, char *, ...));
__EXTERN void		_swflags_ __PROTO((WINDOW *));

__EXTERN void		box __PROTO((WINDOW *, int, int));

__EXTERN void		delwin __PROTO((WINDOW *));

__EXTERN void		endwin __PROTO((void));

__EXTERN void		fgoto __PROTO((void));
__EXTERN char *		fullname __PROTO((char *, char *));

__EXTERN char *		getcap __PROTO((char *));
__EXTERN void		gettmode __PROTO((void));

__EXTERN void		idlok __PROTO((WINDOW *, int));
__EXTERN WINDOW *	initscr __PROTO((void));

__EXTERN char *		longname __PROTO((char *, char *));

__EXTERN void		mvcur __PROTO((int, int, int, int));

__EXTERN int		mvprintw __PROTO((int, int, char *, ...));
__EXTERN int 		mvwprintw __PROTO((WINDOW *, int, int, char *, ...));
__EXTERN int		mvscanw __PROTO((int, int, char *, ...));
__EXTERN int		mvwscanw __PROTO((WINDOW *, int, int, char *, ...));

__EXTERN int		mvwin __PROTO((WINDOW *, int, int));

__EXTERN WINDOW *	newwin __PROTO((int, int, int, int));

__EXTERN void		overlay __PROTO((WINDOW *, WINDOW *));
__EXTERN void		overwrite __PROTO((WINDOW *, WINDOW *));

__EXTERN int		printw __PROTO((char *, ...));

__EXTERN int		scanw __PROTO((char *, ...));
__EXTERN int		scroll __PROTO((WINDOW *));
__EXTERN int		setterm __PROTO((char *));
__EXTERN WINDOW *	subwin __PROTO((WINDOW *, int, int, int, int));

__EXTERN int		tabcol __PROTO((int, int));
__EXTERN void		touchline __PROTO((WINDOW *, int, int, int));
__EXTERN void		touchoverlap __PROTO((WINDOW *, WINDOW *));
__EXTERN void		touchwin __PROTO((WINDOW *));
__EXTERN void		tstp __PROTO((void));

__EXTERN int		waddch __PROTO((WINDOW *, int));
__EXTERN int		waddstr __PROTO((WINDOW *, char *));
__EXTERN int		wclear __PROTO((WINDOW *));
__EXTERN void		wclrtobot __PROTO((WINDOW *));
__EXTERN void		wclrtoeol __PROTO((WINDOW *));
__EXTERN int		wdelch __PROTO((WINDOW *));
__EXTERN int		wdeleteln __PROTO((WINDOW *));
__EXTERN void		werase __PROTO((WINDOW *));
__EXTERN int		wgetch __PROTO((WINDOW *));
__EXTERN int		wgetstr __PROTO((WINDOW *, char *));
__EXTERN int		winsch __PROTO((WINDOW *, int));
__EXTERN void		winsertln __PROTO((WINDOW *));
__EXTERN int		wmove __PROTO((WINDOW *, int, int));

__EXTERN int		wprintw __PROTO((WINDOW *, char *, ...));
__EXTERN int		wrefresh __PROTO((WINDOW *));
__EXTERN int		wscanw __PROTO((WINDOW *, char *, ...));
__EXTERN char *		wstandend __PROTO((WINDOW *));
__EXTERN char *		wstandout __PROTO((WINDOW *));

__EXTERN void		zap __PROTO((void));

/*
 * Used to be in unctrl.h.
 */
#if 0
#define	unctrl(c)	_unctrl[(c) & 0177]
extern char *_unctrl[];
#else
__EXTERN char *unctrl __PROTO((int c));
#endif

#ifdef __cplusplus
}
#endif

#endif
