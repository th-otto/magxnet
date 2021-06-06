#
# Makefile for ifconfig etc
#

include ../configvars

PROGRAMS = ifconfig.ttp
PROGRAMS += arp.ttp
PROGRAMS += iflink.ttp
PROGRAMS += ifstats.ttp
PROGRAMS += netstat.ttp
PROGRAMS += route.ttp

all:: $(PROGRAMS)

ifconfig.ttp: ifconfig.o ifopts.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ifconfig.o ifopts.o $(SOCKLIB)

arp.ttp: arp.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ arp.o $(SOCKLIB)

iflink.ttp: iflink.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ iflink.o $(SOCKLIB)

ifstats.ttp: ifstats.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ifstats.o $(SOCKLIB)

netstat.ttp: netstat.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ netstat.o $(SOCKLIB)

route.ttp: route.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ route.o $(SOCKLIB)

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
