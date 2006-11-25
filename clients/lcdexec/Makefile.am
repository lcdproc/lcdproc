## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdexec.conf

bin_PROGRAMS = lcdexec

lcdexec_SOURCES = lcdexec.c menu.c menu.h

lcdexec_LDADD = ../../shared/libLCDstuff.a @ldap_libs@  

AM_CPPFLAGS = -I$(top_srcdir) -DSYSCONFDIR=\"$(sysconfdir)\"


EXTRA_DIST = $(sysconf_DATA)

## EOF
