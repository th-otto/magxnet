SUBDIRS = socklib gluestik

all clean::
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ || exit 1; done