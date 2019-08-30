## Process this file with automake to produce Makefile.in
sysconf_DATA = lcdvc.conf

bin_PROGRAMS = lcdvc

lcdvc_SOURCES = lcdvc.c lcdvc.h lcd_link.c lcd_link.h vc_link.c vc_link.h
nodist_lcdvc_SOURCES = elektragen.c elektragen.h
lcdvc.$(OBJEXT): elektragen.c elektragen.h

CLEANFILES = elektragen.c elektragen.h elektragen.spec.eqd elektragen.mount.sh

lcdvc_LDADD = ../../shared/libLCDstuff.a

if DARWIN
AM_LDFLAGS = -framework CoreFoundation -framework IOKit
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"

#AM_CFLAGS = -g -O0
#AM_LDFLAGS = -g


EXTRA_DIST = $(sysconf_DATA)

scriptsdir = $(prefix)/libexec
scripts_DATA = lcdvc.mount.sh

specdir = $(prefix)/share/lcdproc
spec_DATA = lcdvc.spec.eqd

KDB ?= kdb
elektragen.c elektragen.h elektragen.spec.eqd elektragen.mount.sh: specification/lcdvc-spec.ini
	$(KDB) gen -F ni=specification/lcdvc-spec.ini highlevel "spec/sw/lcdproc/lcdvc/#0/current" elektragen initFn=loadConfiguration helpFn=printHelpMessage embeddedSpec=defaults helpFn=printHelpMessage

lcdvc.mount.sh: elektragen.mount.sh
	cp elektragen.mount.sh lcdvc.mount.sh

lcdvc.spec.eqd: elektragen.spec.eqd
	cp elektragen.spec.eqd lcdvc.spec.eqd

## EOF
