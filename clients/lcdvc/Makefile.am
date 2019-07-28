## Process this file with automake to produce Makefile.in
sysconf_DATA = lcdvc.conf

bin_PROGRAMS = lcdvc

lcdvc_SOURCES = lcdvc.c lcdvc.h lcd_link.c lcd_link.h vc_link.c vc_link.h
nodist_lcdvc_SOURCES = elektragen.c elektragen.h
lcdvc.$(OBJEXT): elektragen.c elektragen.h

CLEANFILES = elektragen.c elektragen.h

lcdvc_LDADD = ../../shared/libLCDstuff.a

if DARWIN
AM_LDFLAGS = -framework CoreFoundation -framework IOKit
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"

#AM_CFLAGS = -g -O0
#AM_LDFLAGS = -g


EXTRA_DIST = $(sysconf_DATA)

KDB ?= kdb
elektragen.c elektragen.h elektragen.spec.eqd: specification/lcdvc-spec.ini
	$(KDB) gen -F ni=specification/lcdvc-spec.ini elektra "spec/sw/lcdproc/lcdvc/#0/current" elektragen initFn=loadConfiguration helpFn=printHelpMessage specLocation=external defaultsHandling=speconly helpFn=printHelpMessage

## EOF
