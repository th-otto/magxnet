*********************************************************************************
* Declarations for call interface of functions above MIF/XIF in                 *
* MagicNet/MintNet                                                              *
*                                                                               *
*       Copyright 2001-2002 Dr. Thomas Redelberger                              *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* Credits:                                                                      *
* Although written in 68000 assembler this source code is based on the source   *
* module netinfo.h of MintNet originally due to Kay Roemer                      *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*********************************************************************************


* struct netinfo
                .OFFSET  0

buf_alloc:      .ds.l    1
buf_free:       .ds.l    1
buf_reserve:    .ds.l    1
buf_deref:      .ds.l    1

if_enqueue:     .ds.l    1
if_dequeue:     .ds.l    1
if_register:    .ds.l    1
if_input:       .ds.l    1
if_flushq:      .ds.l    1

in_chksum:      .ds.l    1
if_getfreeunit: .ds.l    1

eth_build_hdr:  .ds.l    1
eth_remove_hdr: .ds.l    1

ni_fname:       .ds.l    1

bpf_input:      .ds.l    1

*
* macro definitions
* Assume the global var netinfo to point to the MNet function structure
*


                MACRO If_getfreeunit name
                pea     name
                move.l  netinfo,a0
                move.l  if_getfreeunit(a0),a0
                jsr     (a0)
                addq    #4,sp                   /* pop arg */
                ENDM


                MACRO If_register nif
                pea     nif                     /* nif */
                move.l  netinfo,a0
                move.l  if_register(a0),a0
                jsr     (a0)
                addq    #4,sp                   /* pop arg */
                ENDM


                MACRO If_dequeue buf
                pea     buf                     /* buf */
                move.l  netinfo,a0
                move.l  if_dequeue(a0),a0
                jsr     (a0)
                addq    #4,sp                   /* pop arg */
                ENDM


                MACRO Buf_alloc length,pad,mode
                move    mode,-(sp)      /* mode */
                move.l  pad,-(sp)               /* padding front */
                move.l  length,-(sp)            /* length */
                move.l  netinfo,a0
                movea.l buf_alloc(a0),a0
                jsr     (a0)
                lea     10(sp),sp               /* pop args */
                ENDM


                MACRO Buf_deref buf,mode
                move    mode,-(sp)              /* mode */
                pea     buf                     /* buf */
                move.l  netinfo,a0
                move.l  buf_deref(a0),a0
                jsr     (a0)
                addq    #6,sp                   /* pop args */
                ENDM


                MACRO Eth_build_hdr buf,nif,hwaddr,pktype
                move    pktype,-(sp)            /* pktype */
                pea     hwaddr                  /* hwaddr */
                pea     nif                     /* nif */
                pea     buf                     /* buf */
                move.l  netinfo,a0
                move.l  eth_build_hdr(a0),a0
                jsr     (a0)
                lea     14(sp),sp               /* pop args */
                ENDM


                MACRO Eth_remove_hdr
                move.l  RrpBuf,-(sp)            /* buf */
                move.l  netinfo,a0
                movea.l eth_remove_hdr(a0),a0   /* returns packet type */
                jsr     (a0)
                addq.l  #4,sp                   /* pop arg */
                ENDM


                MACRO Bpf_input nif,buf
                pea     buf                     /* buf */
                pea     nif                     /* nif */
                move.l  netinfo,a0
                move.l  bpf_input(a0),a0
                jsr     (a0)
                addq    #8,sp                   /* pop args */
                ENDM


                MACRO If_enqueue if_snd,buf,info
                move    info,-(sp)              /* info */
                pea     buf                     /* buf */
                pea     if_snd                  /* if_snd */
                move.l  netinfo,a0
                move.l  if_enqueue(a0),a0
                jsr     (a0)
                lea     10(sp),sp               /* pop args */
                ENDM


                MACRO If_input nif,buf,delay,pktype
                move    pktype,-(sp)            /* packet type */
                move.l  delay,-(sp)             /* delay */
                pea     buf                     /* buf */
                pea     nif                     /* nif */
                move.l  netinfo,a0
                movea.l if_input(a0),a0
                jsr     (a0)
                lea     14(sp),sp               /* pop args */
                ENDM

