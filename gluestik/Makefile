#
# Makefile for gluestik, a stik emulator for magicnet/mintnet
#

include ../configvars

PROGRAMS = gluestik.prg

OBJS = \
	gs_conf.o \
	gs_func.o \
	gs_mem.o \
	gs_stik.o \
	main.o \
	bzero.o \
	$(empty)

ifneq ($(CROSS),)
OBJS += mt_aes.o
endif

all:: $(PROGRAMS)

gluestik.prg: $(OBJS) $(SOCKLIB_DEP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(SOCKLIB)

$(OBJS): gs.h gs_conf.h gs_func.h gs_mem.h gs_stik.h ../include/transprt.h
gs_stik.o main.o: version.h

mt_aes.o: ../socklib/mt_aes.s
	$(AS) -c -o $@ $<

bzero.o: ../socklib/bzero.c
	$(AS) -c -o $@ $<

clean::
	rm -f *.o *.pdb *.prg *.ttp *.map $(PROGRAMS)
