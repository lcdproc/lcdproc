########################################################################
# You shouldn't need to touch anything below here.
########################################################################
include ../Makefile.config

DIRS = lcdproc
TARGETS = lcdproc/lcdproc

###################################################################
# Compilation...
#
all: $(TARGETS)
	for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done


##################################################################
# Installation
#
install: $(TARGETS)
	@echo Installing LCD clients...
	for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done; true


##################################################################
# Other stuff...
#
clean:
	for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done; true
	rm -f *.o *~ core

edit:
	emacs . &

