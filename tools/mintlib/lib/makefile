#
# Top level Makefile for MiNT-Gcc library
#

# GCC configuration info...

# Leave the next line undefined for native compilation setup.
# CROSSDIR := /net/acae127/home/bammi/atari/cross-gcc

# Define MINTFLAG if you want to use the "normal"
# gcc libraries, too
# MINTFLAG := -mint

ifdef MINTFLAG
MINTOPT := -mint
else
MINTOPT :=
endif

PROCOPT := -m68000 -fno-defer-pop -fforce-addr
# Processor-specific options and optimizations. 
# Default processor, Motorola 68000 without floating point coproc.
# Some versions of GCC break m68000 code unless -fno-defer-pop
# is set.
ifdef CROSSDIR

# Set up to do cross-compiling...

CROSSLIB := $(CROSSDIR)/lib
CROSSBIN := $(CROSSDIR)/bin

AR	:= $(CROSSBIN)/car
AS 	:= cgcc $(MINTOPT)
CC	:= cgcc $(MINTOPT)
LIB     := $(CROSSLIB)
VPATH   := ../co
CP	:= cp

else

# Set up to run native...

AR	:= gcc-ar
AS	:= gcc $(MINTOPT)
CC	:= gcc $(MINTOPT) 
LIB	:= /usr/lib/m68000 
INCLUDE_DIR := ../include
CP	:= mv

endif

ifdef ENTROPY
# This reflects my setup, your mileage may vary.
ifndef CROSSDIR
AR	:= ar
LIB	:= /src/newlib
INCLUDE_DIR := /usr/include
else
MINTFLAG := -mint
MINTOPT := $(MINTFLAG)
INCLUDE_DIR := ../minclude
endif
XPFLAGS :=  -nostdinc -I$(INCLUDE_DIR)
XFLAGS := $(XPFLAGS) -Wall -Wmissing-prototypes -Wstrict-prototypes
XLFLAGS := -Wno-missing-prototypes -Wno-strict-prototypes
endif

# Flags for extra 'long long' support under newer gcc's
#LONGLONG=-DPRINTF_LONGLONG

# Flags for extra 'long double' support under newer gcc's
#XFLOAT=-DXFLOAT_ENABLE

# Additional options for compiling C code

XFLAGS := -nostdinc -I../include

# Dynamic selection of options for each target library...

ifneq (,$(findstring debug,$(TARGET)))
OPTIM= -O -g -Wstrict-prototypes
FRAME=
DEBUG=
g=g
else
OPTIM= -O2 
FRAME= -fomit-frame-pointer
DEBUG= -DNDEBUG
g=
endif

ifneq (,$(findstring long-int,$(TARGET)))
SHORT=
DSHORT=
ST=
else
SHORT= -mshort
DSHORT= -DSHORTLIB
ST=16
endif

ifneq (,$(findstring 68020,$(TARGET)))
PROCOPT := -m68020-40 -m68881 -fno-force-mem
LIB := /usr/lib/m68020
CC := $(CC) $(MINTOPT)
endif

ifneq (,$(findstring baserel,$(TARGET)))
CC := $(CC) -mbaserel
b=b
else
b=
endif

ifneq (,$(findstring sfp004,$(TARGET)))
SFPFLAG = -Dsfp004
SFP=sfp
else
SFPFLAG=
SFP=
endif

# common subset of options; no int size or omit-frame-pointer:
COPTS= $(DEBUG) $(OPTIM) $(XFLAGS) $(SFPFLAG) $(PROCOPT)

# Base options CC; includes int size but not omit-frame-pointer
CFFLAGS = $(SHORT) $(COPTS)

# normal CFLAGS including int size and omit-frame-pointer
CFLAGS= $(CFFLAGS) $(FRAME) $(LONGLONG)

# cflags for stuff that needs to be compiled with 32 bit ints
CLFLAGS= $(COPTS) $(FRAME) $(DSHORT) $(XFLOAT) $(XLFLAGS)

# flags to $(CC) when it runs the assembler only
ASFLAGS= -c

# flags to $(CC) when running preprocessor and assembler
PPFLAGS= $(XPFLAGS) $(SHORT) $(SFPFLAG) -P -c

# library targets
ifdef MINTFLAG

CLIB= $bmint$g$(SFP)$(ST).a
CLIB020=$bmint$g$(SFP)$(ST)20.a
IIOLIB= $bmiio$g$(SFP)$(ST).a
CRT0=$bmcrt0.o
GCRT0=$bmgcrt0.o

else

CLIB= $bc$g$(SFP)$(ST).a
CLIB020=$bc$g$(SFP)$(ST)20.a
IIOLIB= $biio$g$(SFP)$(ST).a
CRT0=$bcrt0.o
GCRT0=$bgcrt0.o

endif

STARTO= $(CRT0) $(GCRT0)

# the stuff to make
ALL= $(CLIB) $(IIOLIB) $(STARTO)
ALL020 = $(CLIB020) $(STARTO)

#all: lshort llong lgshort lglong
top: lshort llong lbshort lblong
top020: l020short l020long l020bshort l020blong

%.target:
	$(MAKE) clean
	-rm -f *.target
	-touch $@

lshort: lshort.target
	$(MAKE) install

llong: llong.target
	$(MAKE) install TARGET="long-int"

lgshort: lgshort.target
	$(MAKE) install TARGET=debug

lglong: lglong.target
	$(MAKE) install TARGET="long-int debug"

l020short: l020short.target
	$(MAKE) install020 TARGET=68020
#	$(MAKE) install TARGET=68020

l020long: l020long.target
	$(MAKE) install020 TARGET="68020 long-int"
#	$(MAKE) install TARGET="68020 long-int"

lbshort: lbshort.target
	$(MAKE) install TARGET="baserel"

lblong: lblong.target
	$(MAKE) install TARGET="long-int baserel"

l020bshort: l020bshort.target
	$(MAKE) install020 TARGET="68020 baserel"

l020blong: l020blong.target
	$(MAKE) install020 TARGET="68020 long-int baserel"

lsfpshort: lsfpshort.target
	$(MAKE) install TARGET="sfp004"

lsfplong: lsfplong.target
	$(MAKE) install TARGET="long-int sfp004"

lbsfpshort: lbsfpshort.target
	$(MAKE) install TARGET="baserel sfp004"

lbsfplong: lbsfplong.target
	$(MAKE) install TARGET="long-int baserel sfp004"

bakclean:
	rm -f *~ *#
	rm -f */*~ */*#
	rm -f *.orig *.rej
	rm -f */*.orig */*.rej
	rm -f $(INCLUDE_DIR)/*~ $(INCLUDE_DIR)/*#
	rm -f $(INCLUDE_DIR)/*/*~ $(INCLUDE_DIR)/*/*#
	rm -f $(INCLUDE_DIR)/*.orig $(INCLUDE_DIR)/*.rej
	rm -f $(INCLUDE_DIR)/*/*.orig $(INCLUDE_DIR)/*/*.rej

realclean: bakclean
	rm -f *.o *.olb *.tmp
	rm -f core report nohup.out
	rm -f foo bar baz
	rm -f *.target

distclean: realclean
	rm -f diffc diffh

# Get the rules to build everything...

include mincl
