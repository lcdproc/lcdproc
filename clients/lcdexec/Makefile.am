## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdexec.conf

bin_PROGRAMS = lcdexec
bin_SCRIPTS = lcdexec-setup

lcdexec_SOURCES = lcdexec.c menu.c menu.h
nodist_lcdexec_SOURCES = elektragen.c elektragen.h
lcdexec.$(OBJEXT): elektragen.c elektragen.h
BUILT_SOURCES= elektragen.c elektragen.h

CLEANFILES = elektragen.c elektragen.h lcdexec.spec.eqd lcdexec.mount.sh

lcdexec_LDADD = ../../shared/libLCDstuff.a

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"

EXTRA_DIST = $(sysconf_DATA)

scriptsdir = $(prefix)/libexec
scripts_DATA = lcdexec.mount.sh

specdir = $(prefix)/share/lcdproc
spec_DATA = lcdexec.spec.eqd

KDB ?= kdb
elektragen.c elektragen.h lcdexec.spec.eqd lcdexec.mount.sh: specification/lcdexec-spec.ini
	$(KDB) gen -F ni=specification/lcdexec-spec.ini highlevel "spec/sw/lcdproc/lcdexec/#0/current" elektragen initFn=loadConfiguration helpFn=printHelpMessage embeddedSpec=defaults headers=menu.h tagPrefix=CONF_; mv elektragen.mount.sh lcdexec.mount.sh; mv elektragen.spec.eqd lcdexec.spec.eqd

## EOF
