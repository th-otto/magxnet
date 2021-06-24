#
# Makefile for slip.mif
#

include ../../configvars

CFLAGS += -I../../include -I../../kernel

PROGRAMS = slip.mif

OBJS = \
	mstartup.o \
	main.o \
	slip.o \
	serial.o \
	pppdev.o \
	ppp.o \
	slcompr.o \
	$(empty)

all:: $(PROGRAMS)

EVAL_MSHORT = \
	if echo "" | $(CC) $(CFLAGS) -dM -E - | grep -q __MSHORT__; then echo 1; else echo 0; fi

.s.o:
	$(CC) -c $(CFLAGS) -Wa,-defsym,GNUC=1 -Wa,-defsym,MSHORT=$(shell $(EVAL_MSHORT)) -o $@ $<

slip.mif: $(OBJS)
	$(CC) $(CFLAGS) -nostartfiles -Wl,-entry,entry $(LDFLAGS) -o $@ $^

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
