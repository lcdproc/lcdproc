include Makefile.config

########################################################################
# Please edit Makefile.config to configure LCDproc for your system.
########################################################################


########################################################################
# You shouldn't need to touch anything below here.
########################################################################

#GCC = gcc -Wall -Wp,-lang-c-c++-comments -O6
DIRS = shared clients/lcdproc clients server/drivers server
TARGETS = server clients shared
SOURCES = Makefile \
	shared/Makefile \
	shared/LL.c shared/LL.h \
	shared/config.c shared/config.h \
	shared/sockets.c shared/sockets.h \
	clients/Makefile \
	clients/mode.c clients/mode.h \
	tests/Makefile \
	server/Makefile \
	server/main.c server/main.h \
	server/lcd.c server/lcd.h \
	server/drv_base.c server/drv_base.h \
	server/drivers/MtxOrb.c server/drivers/MtxOrb.h \
	server/drivers/wirz-sli.c server/drivers/wirz-sli.h \
	server/drivers/text.c server/drivers/text.h \
	server/drivers/debug.c server/drivers/debug.h \
	server/drivers/curses_drv.c server/drivers/curses_drv.h \
	server/drivers/hd44780.c server/drivers/hd44780.h server/drivers/port.h
EXTRAS = WHATSNEW INSTALL README COPYING TODO README.IRman \
	 docs/menustuff.txt docs/netstuff.txt docs/README.dg \
	docs/README.dg2 docs/hd44780_howto.txt
DOCS = docs/lcdproc.1
#MISC += $(OS) $(DRIVERS)



###################################################################
# Compilation...
#

# TODO: Make sure lcdproc is configured first...
#@NOTCONFIGURED=`find . -newer . -name Makefile.config`
#@if [ $(NOTCONFIGURED) ]; do \
#	echo Please configure lcdproc before compiling! ;\
#	echo Edit "Makefile.config" to configure. ;\
#	exit 0 ;\
#@done;
all:
	@echo Compiling LCDproc...
	@for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done; true


server: 
	cd server; $(MAKE)

clients:
	cd clients; $(MAKE)

shared:
	cd shared; $(MAKE)

tests:
	cd tests; $(MAKE)


##################################################################
# Installation
#
install: $(TARGETS) $(DOCS)
	@echo Installing LCDproc...
#	@echo Sorry!  Cannot do that yet...
#	@echo Please read INSTALL for instructions
	@for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done; true
	if [ ! -e /dev/lcd ]; then ln -s $(DEVICE) /dev/lcd ; chmod a+rw $(DEVICE) ; fi
	@echo "All done -- enjoy!"
	@echo "Be sure to verify that /dev/lcd points to the correct device."
	@ls -l /dev/lcd

# TODO: Have the installer insert lcdproc in system startup scripts?


##################################################################
# Other stuff...
#
clean:
	@for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done
	rm -f *~ core

# Distribution file
dist:
	@echo Making distributable tarball...
	make clean
	rm -rf lcdproc-$(VERSION)
	rm -rf lcdproc-$(VERSION).tar.gz
	mkdir lcdproc-$(VERSION)
	tar cBf - . --exclude CVS --exclude lcdproc-$(VERSION) | (cd lcdproc-$(VERSION) && tar xBfv -)
	tar cfv lcdproc-$(VERSION).tar lcdproc-$(VERSION)
	gzip lcdproc-$(VERSION).tar
	rm -rf lcdproc-$(VERSION)

# This includes source...
#tarball:
#	tar cfv lcd.tar .
#	gzip lcd.tar

edit:
	emacs . &

.PHONY : todo
todo :
	@echo ---------------===========================================
	@grep -n TODO\\\|FIXME `find . -type f` | grep -v grep
	@echo ---------------===========================================

#todo:
#	@echo ---------------===========================================
#	@grep TODO $(SOURCES) | grep -v SOURCES
#	@grep FIXME $(SOURCES) | grep -v SOURCES
#	@echo ---------------===========================================

#TODO: Keep updating the "Thank You's" section of the README.
#TODO: Don't forget about homework!

