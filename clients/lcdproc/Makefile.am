## Process this file with automake to produce Makefile.in

sysconf_DATA = lcdproc.conf

bin_PROGRAMS = lcdproc

lcdproc_SOURCES = main.c main.h mode.c mode.h batt.c batt.h chrono.c chrono.h cpu.c cpu.h cpu_smp.c cpu_smp.h disk.c disk.h load.c load.h mem.c mem.h eyebox.c eyebox.h machine.h machine_Linux.c machine_OpenBSD.c machine_FreeBSD.c machine_NetBSD.c machine_Darwin.c machine_SunOS.c util.c util.h iface.c iface.h

lcdproc_LDADD = ../../shared/libLCDstuff.a

if DARWIN
AM_LDFLAGS = -framework CoreFoundation -framework IOKit
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/shared -DSYSCONFDIR=\"$(sysconfdir)\" -DPIDFILEDIR=\"$(pidfiledir)\"


EXTRA_DIST = $(sysconf_DATA)

## EOF
