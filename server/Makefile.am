## Process this file with automake to produce Makefile.in

SUBDIRS=drivers commands

sbin_PROGRAMS=LCDd

LCDd_SOURCES= client.c client.h clients.c clients.h input.c input.h main.c main.h menuitem.c menuitem.h menu.c menu.h menuscreens.c menuscreens.h parse.c parse.h render.c render.h screen.c screen.h screenlist.c screenlist.h serverscreens.c serverscreens.h sock.c sock.h widget.c widget.h drivers.c drivers.h driver.c driver.h

LDADD = ../shared/libLCDstuff.a commands/libLCDcommands.a @LIBPTHREAD_LIBS@

if !DARWIN
AM_LDFLAGS = -rdynamic -uget_args
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\"

## EOF
