## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libLCDstuff.a

libLCDstuff_a_SOURCES = LL.c LL.h sockets.c sockets.h str.c str.h report.c report.h snprintf.c snprintf.h sring.c sring.h

libLCDstuff_a_LIBADD = @LIBOBJS@

AM_CPPFLAGS = -I$(top_srcdir)

EXTRA_DIST =  defines.h

## EOF
