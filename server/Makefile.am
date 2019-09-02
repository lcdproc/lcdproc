## Process this file with automake to produce Makefile.in

SUBDIRS=commands . drivers

sbin_PROGRAMS=LCDd

LCDd_SOURCES= client.c client.h clients.c clients.h input.c input.h main.c main.h menuitem.c menuitem.h menu.c menu.h menuscreens.c menuscreens.h parse.c parse.h render.c render.h screen.c screen.h screenlist.c screenlist.h serverscreens.c serverscreens.h sock.c sock.h widget.c widget.h drivers.c drivers.h driver.c driver.h
nodist_LCDd_SOURCES = elektragen.c elektragen.h
LCDd.$(OBJEXT): elektragen.c elektragen.h
BUILT_SOURCES = elektragen.c elektragen.h

CLEANFILES = elektragen.c elektragen.h LCDd.spec.eqd LCDd.mount.sh

LDADD = ../shared/libLCDstuff.a commands/libLCDcommands.a @LIBPTHREAD_LIBS@

if !DARWIN
AM_LDFLAGS = -rdynamic
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\"

scriptsdir = $(prefix)/libexec
scripts_DATA = LCDd.mount.sh

specdir = $(prefix)/share/lcdproc
spec_DATA = LCDd.spec.eqd

KDB ?= kdb
elektragen.c elektragen.h LCDd.spec.eqd LCDd.mount.sh: specification/LCDd-spec.ini
	$(KDB) gen -F ni=specification/LCDd-spec.ini highlevel "spec/sw/lcdproc/lcdd/#0/current" elektragen initFn=loadConfiguration helpFn=printHelpMessage embeddedSpec=defaults; mv elektragen.mount.sh LCDd.mount.sh; mv elektragen.spec.eqd LCDd.spec.eqd

## EOF
