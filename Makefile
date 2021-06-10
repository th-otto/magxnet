SUBDIRS = socklib gluestik ovl magxconf kernel tools

all clean::
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done
