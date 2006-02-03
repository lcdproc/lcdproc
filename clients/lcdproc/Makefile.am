## Process this file with automake to produce Makefile.in

bin_PROGRAMS = lcdproc

lcdproc_SOURCES = main.c main.h mode.c mode.h batt.c batt.h chrono.c chrono.h cpu.c cpu.h cpu_smp.c cpu_smp.h disk.c disk.h load.c load.h mem.c mem.h machine.h machine_Linux.c machine_OpenBSD.c machine_FreeBSD.c machine_NetBSD.c machine_Darwin.c machine_SunOS.c util.c util.h

lcdproc_LDADD = @ldap_libs@ ../../shared/libLCDstuff.a

AM_CPPFLAGS = -I$(top_srcdir)

## EOF
