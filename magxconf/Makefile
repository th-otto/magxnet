#
# Makefile for magxconf.prg
#

include ../configvars
LDFLAGS += -Wl,--mprg-flags,0x1

PROGRAMS = magxconf.prg

OBJS = \
	magxconf.o \
	ifconfig.o \
	$(empty)

all:: $(PROGRAMS)

magxconf.prg: $(OBJS) $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SOCKLIB) -lgem

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
