#
# Makefile for rtl8012.mif
#

include ../../configvars

CFLAGS += -I../../include -I../../kernel

PROGRAMS = rtl8012.mif

OBJS = \
	mstartup.o \
	main.o \
	rtl8012.o \
	rtlasm.o \
	$(empty)

# OBJS += masqasm.o

all:: $(PROGRAMS)

EVAL_MSHORT = \
	if echo "" | $(CC) $(CFLAGS) -dM -E - | grep -q __MSHORT__; then echo 1; else echo 0; fi

.s.o:
	$(CC) -c $(CFLAGS) -Wa,-defsym,GNUC=1 -Wa,-defsym,MSHORT=$(shell $(EVAL_MSHORT)) -o $@ $<

rtl8012.mif: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
