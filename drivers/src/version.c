/************************************************************************/
/*																		*/
/*	Generic NEx000 driver for any BUS interface and STinG and MagicNet	*/
/*	and MINTNet															*/
/*	Copyright 2001-2002 Dr. Thomas Redelberger							*/
/*	Use it under the terms of the GNU General Public License			*/
/*	(See file COPYING.TXT)												*/
/*																		*/
/* Development switches													*/
/*																		*/
/* Tabsize 4, developed with Turbo-C ST 2.0								*/
/*																		*/
/************************************************************************/


#include "version.h"

#define	VersionStr(a,b)	"0" #a "." #b

char const version_str[] = VersionStr(MajVersion,MinVersion);
