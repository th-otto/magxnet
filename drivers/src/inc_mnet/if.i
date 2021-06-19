*********************************************************************************
* Data structure per ethernet device MagicNet/MintNet                           *
*                                                                               *
*       Copyright 2001-2002 Dr. Thomas Redelberger                              *
*       Use it under the terms of the GNU General Public License                *
*       (See file COPYING.TXT)                                                  *
*                                                                               *
* Credits:                                                                      *
* Although written in 68000 assembler this source code is based on the source   *
* module if.h of MintNet originally due to Kay Roemer                           *
*                                                                               *
* Tabsize 8, developed with DEVPAC assembler 2.0.                               *
*********************************************************************************

IF_NAMSIZ               =     16                      /* maximum if name len */
IF_MAXQ                 =     60                      /* maximum if queue len */
IF_SLOWTIMEOUT          =     1000                    /* one second */
IF_PRIORITY_BITS        =     1
IF_PRIORITIES           =     (1<<IF_PRIORITY_BITS)

HWTYPE_ETH              =     1                       /* ethernet */

ETH_ALEN                =     6                       /* HW addr length */
ETH_HLEN                =     14                      /* Eth frame header length */
ETH_MIN_DLEN            =     46                      /* minimum data length */
ETH_MAX_DLEN            =     1500                    /* maximum data length */


* struct ifq
                .OFFSET  0

q_maxqlen:      .ds.w    1                               /* short maxqlen; */
q_qlen:         .ds.w    1                               /* short qlen; */
q_curr:         .ds.w    1                               /* short curr; */
q_qfirst:       .ds.l    IF_PRIORITIES                   /* BUF   *qfirst[IF_PRIORITIES]; */
q_qlast:        .ds.l    IF_PRIORITIES                   /* BUF   *qlast[IF_PRIORITIES]; */
Nqueue:


* struct netif
                .offset  0

if_name:        .ds.b    IF_NAMSIZ                       /* char name[IF_NAMSIZ] */
if_unit:        .ds.w    1                               /* short unit */
if_flags:       .ds.w    1                               /* ushort flags */
if_metric:      .ds.l    1                               /* ulong metric */
if_mtu:         .ds.l    1                               /* ulong mtu */
if_timer:       .ds.l    1                               /* ulong timer */
if_hwtype:      .ds.w    1                               /* short hwtype */
if_hwlocalLen:  .ds.w    1                               /* hwlocal.len */
if_hwlocalAddr: .ds.b    10                              /* hwlocal.addr */
if_hwbrctsLen:  .ds.w    1                               /* hwbrcst.len */
if_hwbrctsAddr: .ds.b    10                              /* hwbrcst.addr */
if_addrlist:    .ds.l    1                               /* struct ifaddr* addrlist */
if_snd:         .ds.b    Nqueue                          /* struct ifq snd */
if_rcv:         .ds.b    Nqueue                          /* struct ifq rcv */
if_open:        .ds.l    1                               /* *open */
if_close:       .ds.l    1                               /* *close */
if_output:      .ds.l    1                               /* *output */
if_ioctl:       .ds.l    1                               /* *ioctl */
if_timeout:     .ds.l    1                               /* *timeout */
if_data:        .ds.l    1                               /* void* data */
if_in_packets:  .ds.l    1                               /* ulong in_packets */
if_in_errors:   .ds.l    1                               /* ulong in_errors */
if_out_packets: .ds.l    1                               /* ulong out_packets */
if_out_errors:  .ds.l    1                               /* ulong out_errors */
if_collisions:  .ds.l    1                               /* ulong collisions */
if_next:        .ds.l    1                               /* struct netif* next */
if_maxpackets:  .ds.w    1                               /* short maxpackets */
if_bpf:         .ds.l    1                               /* struct bpf *bpf */
                .ds.l    4                               /* long reserved[4] */
Nif:

