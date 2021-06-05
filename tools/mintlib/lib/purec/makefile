### Makefile for MiNT Library for Pure C and Turbo C
### hohmuth 19 Feb 1993
###
### This makefile needs GNU make.
###
### This makefile is pretty tough.. Make sure your make has a large stack!
### (The MiNT library's standard stack size (8K) is _not_ enough!)

###
### begin of configuration section
###

# TURBOC= yes if you still use Turbo C.
# If you do so, I strongly suggest upgrading to Pure C.
# Comment this out if you use Pure C.

#TURBOC=		yes

# COMPILERDIR= the main directory of your Turbo/Pure C installation
# As file names in rules may not contain colons (`:'), this path
# name should _not_ contain a TOS-style drive specification (like "c:").
# If you define TOSIFY_COMMANDS below, this path will be automatically 
# tosified.

#COMPILERDIR=	/dev/g/turboc
COMPILERDIR=	/tc2

# BINDIR= where your compiler's tools (pcc.ttp, plink.ttp etc.) reside

BINDIR=		$(COMPILERDIR)
#BINDIR=		$(COMPILERDIR)/bin

# LIBDIR= where to search for the compiler's libraries. Some of them are
# linked into the MiNT library.

LIBDIR=		$(COMPILERDIR)/lib

# COMPILERINCDIR= where to find your compiler's includes, especially
# float.h and math.h.
# This is only needed during "make install-includes".

COMPILERINCDIR=	$(COMPILERDIR)/include

# INCLUDEDIR= where your MiNT library's include files (mntinc) reside.
# If you define TOSIFY_NAMES below, this is path will be automatically 
# tosified for the compiler's -I switch.

INCLUDEDIR=	/h/mint/include

# OSBINDLIB= the location of the osbind library by Ole Arndt und 
# Ulf Moeller, if you have it.  This library replaces our TOS bindings.
# If you use Turbo C, and don't have `awk', you _should_ get this library.
# If you don't have it, you need to have `awk'.  (we need to hack our
# TOS bindings) 
# For Pure C users: You don't need to have this library.
# Comment this out, if you don't have the osbind library.

#OSBINDLIB=	$(LIBDIR)/osbind.lib

# INSTALLDIR= the directory you want to have the MiNT library installed to.
# This is only needed during "make install".

INSTALLDIR=	/e/mint/lib/purec

# COMMONDIR= where the main bulk of the library source resides

COMMONDIR=	..

# ASSERTIONS= yes if you want to activate the library's calls to assert().
# Comment this out if you don't.

#ASSERTIONS=	yes

# DEBUGINFO= yes, if you want your libraries created with debug info.
# Comment this out if you don't.

DEBUGINFO=	yes

# PCREL= yes, if you want all sub-routine calls to be PC relative.
# This can cause problems with bigger projects, however.
# Comment this out if you prefer absolute calls to sub-routines.

#PCREL= yes

# TOSIFY_NAMES= yes if the compiler's tools (pcc, plink etc.) don't 
# understand Unix-like path names.
# Comment this out if your compiler _does_ understand names like "/dev/c/foo".

TOSIFY_NAMES=	yes

# TOSIFY_COMMANDS= yes if your shell (see SHELL below) wants path names
# in TOS form (i.e. does not understand unix compatible file names like
# "/dev/c/foo"). Mupfel wants this.
# Comment this out if your shell groks names like "/dev/c/foo".
# NOTE: your versions of rm, cp, etc. _should_ understand
#       unix-like names

#TOSIFY_COMMANDS=	yes

# sh_backslash= the string that should be passed to the shell as a single
# backslash character. This depends on your shell (whether your
# shell needs backslashes escaped with another backslash, as do most
# unix-like shells). On contrast, Mupfel wants only one backslash, as well
# as the shell that comes with `elvis'.

sh_backslash=	\\
#sh_backslash=	\$(nothing)

# SHELL, RM, CP, INSTALL_DATA, SED, ECHO: edit to suite your taste
# These programs _should_ understand unix-like path names (like "/dev/c/foo").
# INSTALL_DATA is only needed during "make install" and "make install-includes".
# AWK is only needed when using Turbo C without the osbind library (see
# explanation on OSBINDLIB above)
# ECHO is only needed with Turbo C.
#
# all of these programs should understand the ARGV scheme.

SHELL=		/bin/sh			# a bourne shell
#SHELL=		/usr/local/bin/bash	# GNU bash
#SHELL=		c:/usr/gemini/1.89/mupfel.ttp # Mupfel 1.89 (don't try 1.21)
#SHELL=		c:/bin/shell.ttp	# simple shell (from `elvis' distribution)
#SHELL=					# no shell at all (works only with Pure C)

RM=		RM -v
CP=		CP -v
AWK=		gawk
ECHO=		echo
INSTALL_DATA=	$(CP)
#INSTALL_DATA=	install -m 644

###
### end of configuration section
###

### the compiler programs, and what flags they need

ifdef TOSIFY_NAMES
  DIRSEP=	$(sh_backslash)
else
  DIRSEP=	/
endif

ifdef TURBOC
  CC :=		$(BINDIR)/tcc.ttp -W
  AS :=		$(BINDIR)/mas.ttp
  LD :=		$(BINDIR)/tlink.ttp
else
  CC :=		$(BINDIR)/pcc.ttp -W
  AS :=		$(BINDIR)/pasm.ttp
  LD :=		$(BINDIR)/plink.ttp
endif

ifdef TOSIFY_COMMANDS
  tosify_name := $(CC)
  include tosify.mak
  CC := $(tosify_name)

  tosify_name := $(AS)
  include tosify.mak
  AS := $(tosify_name)

  tosify_name := $(LD)
  include tosify.mak
  LD := $(tosify_name)
endif

ifdef DEBUGINFO
  CCSYMFLAGS=	-Y
  ifdef TURBOC
    ASSYMFLAGS=
  else
    ASSYMFLAGS=	-Y
  endif
  LDSYMFLAGS=	-G+ -L+ -Y
else
  CCSYMFLAGS=
  ASSYMFLAGS=
  LDSYMFLAGS=	-G+
endif

ifdef PCREL
  CCRELFLAGS=
else
  CCRELFLAGS=	-P
endif

MAKEOBJ=	-J

ifdef ASSERTIONS
  CCDEBUGDEFS=
else
  CCDEBUGDEFS=	-DNDEBUG
endif

INCSPEC :=	$(INCLUDEDIR)

# tosify the include specification
ifdef TOSIFY_NAMES
  tosify_name := $(INCSPEC)
  include tosify.mak
  INCSPEC := $(tosify_name)
endif

CCINCFLAGS=	-I$(INCSPEC)

# with Turbo C, we need to work around its missing ARGV capability
ifdef TURBOC
  ARGV_WORKAROUND=	yes
else
  ARGV_WORKAROUND=
endif

### default flags to compiler, linker, assembler

CCFLAGS=	$(CCINCFLAGS) $(CCSYMFLAGS) $(CCRELFLAGS) $(CCDEBUGDEFS)
ASFLAGS=	$(ASSYMFLAGS)
LDFLAGS=	$(LDSYMFLAGS)

### what to build

MINTLIB=	mintlib.lib
MATHLIB=	mintflt.lib
FPUMATHLIB=	mint881.lib
STARTUPOBJ=	crt0.o
ALLOCAOBJ=	alloca.o
FPUALLOCAOBJ=	falloca.o

MYTOSLIB=	myosbind.lib

DIST_MATHLIB=	d_flt.lib
DIST_FPUMATHLIB=	d_881.lib

ifdef TURBOC
  ALL=		$(MINTLIB) $(MATHLIB) $(STARTUPOBJ) $(ALLOCAOBJ)
  BINDIST=	$(MINTLIB) $(DIST_MATHLIB) $(STARTUPOBJ) $(ALLOCAOBJ)
else
  ALL=		$(MINTLIB) $(MATHLIB) $(STARTUPOBJ) $(ALLOCAOBJ) \
    $(FPUMATHLIB) $(FPUALLOCAOBJ)
  BINDIST=	$(MINTLIB) $(DIST_MATHLIB) $(STARTUPOBJ) $(ALLOCAOBJ) \
    $(DIST_FPUMATHLIB) $(FPUALLOCAOBJ)
endif

.PHONY:		all
all:		$(ALL)

.PHONY:		bindist
bindist:	$(BINDIST)

### things we link from the compiler's distribution

ifdef TURBOC
  CCFLOATLIB :=	$(LIBDIR)/tcfltlib.lib
else
  CCFLOATLIB :=	$(LIBDIR)/pcfltlib.lib
  CC881LIB :=	$(LIBDIR)/pc881lib.lib
  CCOSLIB := $(LIBDIR)/pcextlib.lib $(LIBDIR)/pcgemlib.lib $(LIBDIR)/pcstdlib.lib
endif

CCFLOATLIBSPEC :=	$(CCFLOATLIB)
CC881LIBSPEC :=	$(CC881LIB)

# tosify the library specifications, if nessecary
ifdef TOSIFY_NAMES
  tosify_name := $(CCFLOATLIBSPEC)
  include tosify.mak
  CCFLOATLIBSPEC := $(tosify_name)

  tosify_name := $(CC881LIBSPEC)
  include tosify.mak
  CC881LIBSPEC := $(tosify_name)
endif

### which OS bindings library to link

BUILD_MYTOSLIB_DEP :=

# the osbind library replaces our tos library.
ifdef OSBINDLIB
  TOSLIB :=	$(OSBINDLIB)
else
  ifdef TURBOC
    TOSLIB :=	$(MYTOSLIB)

    # used as a file dependency for $(MINTLIB)
    BUILD_MYTOSLIB_DEP :=	$(MYTOSLIB)
  else
    TOSLIB :=
  endif
endif

TOSLIBSPEC :=	$(TOSLIB)

# tosify the library specification, if nessecary
ifdef TOSIFY_NAMES
  tosify_name := $(TOSLIBSPEC)
  include tosify.mak
  TOSLIBSPEC := $(tosify_name)
endif

### how to build MINTLIB

COMMONSRC :=	a64l.c abort.c abs.c access.c alarm.c alphasor.c atexit.c \
  atoi.c atol.c bblink.c bcmp.c binmode.c bsearch.c buffindf.o calloc.c \
  cfgetisp.c cfgetosp.c cfsetisp.c cfsetosp.c chdir.c \
  chmod.c chown.c clock.c close.c closedir.c console.c crtinit.c ctermid.c \
  ctime.c ctype.c cuserid.o defmode.c difftime.o div.o \
  doprnt.c do_lock.c do_stat.c dup.c dup2.c eprintf.c \
  enoent.c execl.c execle.c execp.c execv.c execve.c \
  fclose.c fcntl.c fdopen.c fflush.c ffs.o \
  fgetc.c fgetpos.c fgets.c filbuf.c findfile.c flock.c fopen.c \
  fopen_i.c fopenp.c fork.c fprintf.c \
  fputc.c fputs.c fread.c freopen.c frwbin.c \
  fscanf.c fseek.c fsetpos.c fstat.c ftell.c ftw.c \
  fungetc.c fwrite.c getbuf.c getcwd.c \
  getdtabl.c getenv.c getegid.c geteuid.c getgid.c getgroup.c \
  gethostn.c getitimer.o getloadavg.o getlogin.c getopt.c getpages.c \
  getpass.c getpid.c getppid.c getpw.c gnuaux.o \
  getrusag.c gets.c getuid.c getw.c getwd.c \
  grp.c heapbase.c ic.c ident.c ig.c \
  il.c initgroups.o inistack.c initsig.c inode.c ioctl.c \
  ip.c isatty.c isctty.c iw.c kill.c killpg.c \
  l64a.c labs.c ldiv.c link.c localtim.c lockf.c lseek.c lstat.c \
  ltoa.c main.c malloc.c memccpy.c \
  memchr.c memcmp.c mkdir.c mkfifo.c mknod.c mktemp.c nice.c nlist.c \
  obstack.c open.c opendir.c \
  pause.c perror.c pipe.c pgrp.c popen.c \
  printf.c psignal.c putenv.c puts.c putpwent.o \
  qsort.c raise.c \
  rand.c random.c read.c readdir.c realloc.c \
  regexp.c regsup.c rename.c rewind.c rewinddi.c rmdir.c \
  sbrk.c scandir.c scanf.c seekdir.c select.c \
  setbuf.c setegid.c seteuid.c setitimer.o setlocal.c \
  setregid.c setreuid.c setrlimi.c setgid.c setuid.c \
  setvbuf.c sgtty.c sigactio.c sigblock.c siglist.c signal.c sleep.c \
  spawn.c spawnve.c spawnvp.c sprintf.c sscanf.c stat.c \
  statfs.c stime.o stksiz.c \
  strcat.c strchr.c strcmp.c strcoll.c strcpy.c strcspn.c strdup.c \
  strerror.c strftime.c stricmp.c strlen.c strlwr.c strncat.c strncmp.c \
  strncpy.c strnicmp.c strpbrk.c strrchr.c strrev.c strspn.c strstr.c \
  strtok.c strtol.c strtoul.c strupr.c sync.o symlink.c sysconf.c system.c \
  sysvar.c tcattr.c tcbreak.c tcdrain.c tcflow.c tcflush.c tcpgrp.c \
  telldir.c textio.c thread.c time.c timeoday.c times.c tmpfile.c tmpnam.c \
  toxxx.c truncate.c ttyname.c uidgid.c uname.c unlink.c unx2dos.c utime.c \
  utmp.c vfprintf.c vfscanf.c vprintf.c vscanf.c wait.c \
  wait3.c wait4.c waitpid.c wcmb.c \
  wcscat.c wcscmp.c wcscpy.c wcslen.c wnull.c write.c wtmp.c 
COMMONSRC := $(addprefix $(COMMONDIR)/,$(COMMONSRC))

SRC :=		bzero.c memset.c alloca2.c \
  bcopy.s linea.s setjmp.s vfork.s

ifndef TOSLIB
  # no tos bindings library -- link our bindings.
  SRC :=	$(SRC) osbind.s bios.s xbios.s gemdos.s mintbind.s
  bios.o xbios.o gemdos.o mintbind.o:	osmacros.s
endif

ifdef TURBOC
  SUPPORTSRC=	fpuinit.c stkover.s
else
  SUPPORTSRC=	fpuinit.c stkover.s pclong.s 
endif

MINTSRC=	$(COMMONSRC) $(SUPPORTSRC) $(SRC)

MINTOBJ :=	$(patsubst %.c,%.o,$(MINTSRC))
MINTOBJ :=	$(patsubst %.s,%.o,$(MINTOBJ))

ifdef ARGV_WORKAROUND
  MINTLIB_CMD=	$(basename $(MINTLIB)).cmd
  MINTLIB_PARAM=	-C=$(MINTLIB_CMD)
  MINTLIB_DEP=	$(MINTOBJ) $(MINTLIB_CMD)

  $(MINTLIB_CMD):
	-$(RM) -f $@
	$(ECHO) -n $(subst /,$(DIRSEP),$(MINTOBJ)) > $@
else
  MINTLIB_PARAM=	$(subst /,$(DIRSEP),$(MINTOBJ))
  MINTLIB_DEP=	$(MINTOBJ)
endif

ifneq (,$(wildcard $(firstword $(MINTSRC))))
  $(MINTLIB):	$(MINTLIB_DEP) $(BUILD_MYTOSLIB_DEP)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ $(MINTLIB_PARAM) $(TOSLIBSPEC) $(CCOSLIB)
#	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ $(MINTLIB_PARAM)
else
# if the source is not present (binary distribution), we just check for
# the presence of the library.
  $(MINTLIB): ;
endif

%.o:		%.s
	$(AS) $(ASFLAGS) -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

%.o:		%.c
	$(CC) $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

doprnt.o:		../doprnt.c
	$(CC) -r $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

scanf.o:		../scanf.c
	$(CC) -r $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

$(COMMONDIR)/crtinit.o:	$(COMMONDIR)/crtinit.c
	$(CC) -P -S- -T- $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

$(COMMONDIR)/main.o:	$(COMMONDIR)/main.c
	$(CC) -P $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

$(COMMONDIR)/qsort.o:	$(COMMONDIR)/qsort.c
	$(CC) -S -T $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

$(COMMONDIR)/thread.o:	$(COMMONDIR)/thread.c
	$(CC) -P -S- $(CCFLAGS) -D__NO_FLOAT__ \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

setjmp.o:	setjmp.s
	$(AS) $(ASFLAGS) -D=FPU=0 \
	  -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

### how to build MYTOSLIB, our own tos bindings library

ifdef TURBOC
  # for Turbo C, this is quite tricky.  Turbo's assembler MAS doesn't
  # understand the directives ".MODULE" and ".ENDMOD", but we still want
  # one module per binding.  That's why we use awk to cut modules
  # from the source files and build one object file per binding.

  MYBINDSRC :=	bios.s xbios.s gemdos.s mintbind.s
  
  # we make a list of object file we want to build and re-instantiate `make'.
  # in the new instance, we include that list into our makefile
  MAKEINCL :=	myosbind.mak

  ifndef MAKEINCL_OK
    $(MAKEINCL):	$(MYBINDSRC)
	$(AWK) 'BEGIN { FS = " |:|\t" ; printf "MYBINDOBJ := " } /^\.MODULE/ { printf "%s.o ", $$2 }' \
	  $^ > $@

    $(MYTOSLIB):	$(MYBINDSRC) osmacros.s osbind.s $(MAKEINCL)
	$(MAKE) $(MYTOSLIB) MAKEINCL_OK=yes
  else

    # this should define MYBINDOBJ
    include $(MAKEINCL)

    $(MYBINDOBJ):	%.o:	$(MYBINDSRC) osmacros.s
	-$(RM) -f temp.s
	$(AWK) -v name=$(patsubst %.o,%,$@) -f filter.awk $(MYBINDSRC) > temp.s
	$(AS) $(ASFLAGS) -O=$@ temp.s

    ifdef ARGV_WORKAROUND
      MYTOSLIB_CMD=	$(basename $(MYTOSLIB)).cmd
      MYTOSLIB_PARAM=	-C=$(MYTOSLIB_CMD)
      MYTOSLIB_DEP=	$(MYBINDOBJ) $(MYTOSLIB_CMD)
    
      $(MYTOSLIB_CMD):	$(MAKEINCL)
	-$(RM) -f $@
	$(ECHO) -n $(MYBINDOBJ) > $@
    else
      MYTOSLIB_PARAM=	$(MYBINDOBJ)
      MYTOSLIB_DEP=	$(MYBINDOBJ)
    endif

    $(MYTOSLIB):	osbind.o $(MYTOSLIB_DEP)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ osbind.o $(MYTOSLIB_PARAM)

  endif
else
  # with Pure C, everything's so much easier :-)
  # however, it's unlikely that you'll need this.

  MYTOSSRC :=	osbind.s bios.s xbios.s gemdos.s mintbind.s
  MYTOSOBJ :=	$(patsubst %.s,%.o,$(MYTOSSRC))

  bios.o xbios.o gemdos.o mintbind.o:	osmacros.s

  $(MYTOSLIB):	$(MYTOSOBJ)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ $^

endif

### how to build MATHLIB

MATHOBJ=	m_main.o m_doprnt.o m_scanf.o m_difftime.o

ifneq ($(DIST_MATHLIB),$(wildcard $(DIST_MATHLIB)))
  $(MATHLIB):	$(MATHOBJ) # $(CCFLOATLIB)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(subst /,$(DIRSEP),$(MATHOBJ)) $(CCFLOATLIBSPEC)
else
# link with the bindist's rudimentary math library, if it exists
  $(MATHLIB):	$(DIST_MATHLIB) # $(CCFLOATLIB)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(DIST_MATHLIB) $(CCFLOATLIBSPEC)
endif

ifneq (,$(wildcard $(patsubst m_%.o,$(COMMONDIR)/%.c,$(firstword $(MATHOBJ)))))
  $(DIST_MATHLIB):	$(MATHOBJ)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(subst /,$(DIRSEP),$(MATHOBJ))
else
# if the source is not present (binary distribution), we just check for
# the presence of the distributable library.
  $(DIST_MATHLIB): ;
endif

m_%.o:		$(COMMONDIR)/%.c
	$(CC) -T- $(CCFLAGS) \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

m_main.o:	$(COMMONDIR)/main.c
	$(CC) -T- -P $(CCFLAGS) \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

### how to build FPUMATHLIB

FPUMATHOBJ =	f_main.o f_doprnt.o f_scanf.o f_difftime.o f_setjmp.o

ifneq ($(DIST_FPUMATHLIB),$(wildcard $(DIST_FPUMATHLIB)))
  $(FPUMATHLIB):	$(FPUMATHOBJ) # $(CC881LIB)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(subst /,$(DIRSEP),$(FPUMATHOBJ)) $(CC881LIBSPEC)
else
# link with the bindist's rudimentary math library, if it exists
  $(FPUMATHLIB):	$(DIST_FPUMATHLIB) # $(CC881LIB)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(DIST_FPUMATHLIB) $(CC881LIBSPEC)
endif

ifneq (,$(wildcard $(patsubst f_%.o,$(COMMONDIR)/%.c,$(firstword $(FPUMATHOBJ)))))
  $(DIST_FPUMATHLIB):	$(FPUMATHOBJ)
	$(LD) $(MAKEOBJ) $(LDFLAGS) -O=$@ \
	  $(subst /,$(DIRSEP),$(FPUMATHOBJ))
else
# if the source is not present (binary distribution), we just check for
# the presence of the distributable library.
  $(DIST_FPUMATHLIB): ;
endif

f_%.o:		$(COMMONDIR)/%.c
	$(CC) -T- -8 $(CCFLAGS) \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

f_main.o:	$(COMMONDIR)/main.c
	$(CC) -T- -8 -P $(CCFLAGS) \
	  -O$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

f_setjmp.o:	setjmp.s
	$(AS) $(ASFLAGS) -D=FPU=1 \
	  -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

### how to build the startup code

STARTUPSRC=	crt0.s

ifneq (,$(wildcard $(STARTUPSRC)))
  $(STARTUPOBJ):	$(STARTUPSRC)
	$(AS) $(ASFLAGS) -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)
else
# if the source is not present (binary distribution), we just check for
# the presence of the object files.
  $(STARTUPOBJ): ;
endif

### how to build the fast versions of alloca()

ALLOCASRC=	alloca.s

ifneq (,$(wildcard $(ALLOCASRC)))
  $(ALLOCAOBJ):	$(ALLOCASRC)
	$(AS) $(ASFLAGS) -D=STACKCH=1 -D=FPU=0 \
	  -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)

  $(FPUALLOCAOBJ):	$(ALLOCASRC)
	$(AS) $(ASFLAGS) -D=STACKCH=1 -D=FPU=1 \
	  -O=$(subst /,$(DIRSEP),$@) $(subst /,$(DIRSEP),$<)
else
# if the source is not present (binary distribution), we just check for
# the presence of the object files.
  $(ALLOCAOBJ): ;
  $(FPUALLOCAOBJ): ;
endif

### how to build the test program

.PHONY:		test
test:		test.tos

ifdef TURBOC
  CCSTDLIB :=	$(LIBDIR)/tcstdlib.lib
  CCSTDLIBSPEC :=	$(CCSTDLIB)
  ifdef TOSIFY_NAMES
    tosify_name :=	$(CCSTDLIBSPEC)
    include tosify.mak
    CCSTDLIBSPEC :=	$(tosify_name)
  endif

  TEST_LINKLIBS=	$(CCSTDLIBSPEC)
else
  TEST_LINKLIBS=
endif

test.tos:	$(MINTLIB) $(STARTUPOBJ) test.o
	$(LD) $(LDFLAGS) -S=0 -O=$@ $(STARTUPOBJ) test.o \
	  $(MINTLIB) $(TEST_LINKLIBS)

test.o:		test.c
	$(CC) -S $(CCFLAGS) -O$@ $<
	
### how to make clean

.PHONY:		clean
clean:
	-$(RM) -f $(COMMONDIR)/*.o
	-$(RM) -f *.o
	-$(RM) -f *.cmd
	-$(RM) -f temp.s test.tos

.PHONY:		realclean
realclean:	clean
	-$(RM) -f *.lib
ifdef MAKEINCL
	-$(RM) -f $(MAKEINCL)
endif

### how to install

.PHOHY:		install
install:	$(ALL)
	$(INSTALL_DATA) $(ALL) $(INSTALLDIR)

.PHONY:		install-includes
install-includes:
	$(INSTALL_DATA) $(COMPILERINCDIR)/float.h $(INCLUDEDIR)/tcfloat.h
	$(INSTALL_DATA) $(COMPILERINCDIR)/math.h $(INCLUDEDIR)/tcmath.h
