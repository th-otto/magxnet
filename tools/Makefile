#
# Makefile for ifconfig etc
#

include ../configvars

PROGRAMS = ifconfig.ttp

all:: $(PROGRAMS)

ifconfig.ttp: ifconfig.o ifopts.o $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ifconfig.o ifopts.o $(SOCKLIB)

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
