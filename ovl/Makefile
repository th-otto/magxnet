#
# Makefile for magx_sld.ovl
#

include ../configvars

PROGRAMS = magx_sld.ovl

OBJS = \
	main.o \
	$(empty)

all:: $(PROGRAMS)

magx_sld.ovl: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lgem

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
