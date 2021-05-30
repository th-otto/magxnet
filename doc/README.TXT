MagiCNet 1.4.2 beta
-------------------

From this version the cookie structure has been changed (for the
last time I hope) so all modules should be updated. It is now
possible to display the version and author by examining the cookie
(first two pointers). FTP Server is the first example.

There is also support for NAT and port mapping. It is currently 
used by pppd_GEM but support will continue by an external GEM
program and additional cookie entries as needed.

From v 1.4.2 beta an attempt has been made to support more MiNT
calls in the internal 'fake' MiNT kernel like s_system (S_GET/SET
COOKIE..) m_xalloc etc. Please report problems with newer ethernet
drivers that I don't have the hardware to test.

This MagiCNet release is compatible with MagiC versions older than
v 6.2x.

Please report bugs etc to:
papval@otenet.gr

Vassilis
24 Jan 2003
