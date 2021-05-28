This is a modified version of the REAME file originaly
released by Frank Naumann.

GlueSTiK - STiK to MagiCNet gateway
===================================

This is GlueSTiK ported to Pure C and MagiCNet from MiNT-Net.
The GlueSTiK gateway for MagiCNet works like on MiNTNet except that
it is a TSR program and thus can't be killed.

Version 0.14
------------
- fixed bug that prevented listening sockets from working.

Version 0.13
------------
- fixed bug in resolve; spurious crashes
  Thanks to Erik Hall who pointed it out

Version 0.12
------------
- complete rewrite of old GlueSTiK


Features:
---------
- allow the usage of STiK-API programs under MiNT-Net/MagiCNet


Installation:
-------------
 Copy GLUESTIK.PRG to any folder. If you intend to use it regularly
 a good place should be your \GEMSYS\MAGIC\START folder or even
 the AUTO folder. It can also be started from the desktop but
 since it installs a cookie, this is not 100% legal.
 
 You need a folder called \STIK_CFG\ in your boot drive. Inside 
 this folder a DEFAULT.CFG file should exist. Many settings of this
 file are ignored, the default from any STiK distribution should
 be OK.
 
 Copying:
---------
Copyright 2001 Vassilis Papathanassiou <papval@otenet.gr>
Copyright 2000 Frank Naumann <fnaumann@cs.uni-magdeburg.de>
Portions copyright 1998, 1999 Scott Bigham <dsb@cs.duke.edu>.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


Frank Naumann
<fnaumann@cs.uni-magdeburg.de>

Magdeburg, 23.02.2000

Vassilis Papathanassiou
Athens, 12 Jun 2001
