/*
 * Filename:     gs_stik.h
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

# ifndef _gs_stik_h
# define _gs_stik_h

# include "gs.h"


extern DRV_LIST stik_driver;

#ifdef TPL_STRUCT_ARGS
const char *__CDECL do_get_err_text	(struct get_err_text_param p);
#else
const char *__CDECL do_get_err_text	(int16 code);
#endif
int	init_stik_if	(void);
void	cleanup_stik_if	(void);


# endif /* _gs_stik_h */
