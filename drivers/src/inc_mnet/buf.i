*********************************************************************************
* Data structure for packet buffers in MagicNet/MintNet                         *
*                                                                               *
*       Copyright 2001-2002 Dr. Thomas Redelberger                              *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* Credits:                                                                      *
* Although written in 68000 assembler this source code is based on the source   *
* module buff.h of MintNet originally due to Kay Roemer                         *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*********************************************************************************


* we just have a subset here relevant for a driver

BUF_NORMAL      =     0
BUF_ATOMIC      =     1


* struct buf
                .offset 0

bf_buflen:      .ds.l   1                       /* ulong buflen, including header */
bf_dstart:      .ds.l   1                       /* char  *dstart, start of data */
bf_dend:        .ds.l   1                       /* char  *dend, end of data */
bf_next:        .ds.l   1                       /* BUF   *next, next message */
bf_prev:        .ds.l   1                       /* BUF   *prev, previous message */
bf_link3:       .ds.l   1                       /* BUF   *link3, another next pointer */
bf_links:       .ds.w   1                       /* short links, usage counter */
bf_info:        .ds.l   1                       /* long  info, aux info */
* ...more follow we do not need

