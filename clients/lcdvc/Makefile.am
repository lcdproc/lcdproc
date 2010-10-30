## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdvc.conf

bin_PROGRAMS = lcdvc

lcdvc_SOURCES = lcdvc.c lcdvc.h lcd_link.c lcd_link.h vc_link.c vc_link.h

lcdvc_LDADD = ../../shared/libLCDstuff.a

if DARWIN
AM_LDFLAGS = -framework CoreFoundation -framework IOKit
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"

#AM_CFLAGS = -g -O0
#AM_LDFLAGS = -g


EXTRA_DIST = $(sysconf_DATA)

## EOF
