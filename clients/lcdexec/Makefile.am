## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdexec.conf

bin_PROGRAMS = lcdexec

lcdexec_SOURCES = lcdexec.c menu.c menu.h
nodist_lcdexec_SOURCES = elektragen.c elektragen.h
lcdexec.$(OBJEXT): elektragen.c elektragen.h
BUILT_SOURCES= elektragen.c elektragen.h

CLEANFILES = elektragen.c elektragen.h

lcdexec_LDADD = ../../shared/libLCDstuff.a

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"

EXTRA_DIST = $(sysconf_DATA)

KDB ?= kdb
elektragen.c elektragen.h: specification/lcdexec-spec.ini
	$(KDB) gen -F ni=specification/lcdexec-spec.ini elektra "spec/sw/lcdproc/lcdexec/#0/current" elektragen initFn=loadConfiguration specloadFn=doSpecloadCheck helpFn=printHelpMessage headers=menu.h

## EOF
