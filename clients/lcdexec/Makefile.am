## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdexec.conf

bin_PROGRAMS = lcdexec

lcdexec_SOURCES = lcdexec.c menu.c menu.h

lcdexec_LDADD = ../../shared/libLCDstuff.a

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"


EXTRA_DIST = $(sysconf_DATA)

## EOF
