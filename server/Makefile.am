SUBDIRS=drivers commands
sbin_PROGRAMS=LCDd
LCDd_SOURCES= client.c client.h clients.c clients.h client_menu.h input.c input.h main.c main.h event.h menu.c menu.h menuscreens.c menuscreens.h parse.c parse.h render.c render.h screen.c screen.h screenlist.c screenlist.h serverscreens.c serverscreens.h sock.c sock.h widget.c widget.h configfile.c configfile.h drivers.c drivers.h driver.c driver.h
LCDd_LDADD = ../shared/libLCDstuff.a commands/libLCDcommands.a
INCLUDES = -I$(top_srcdir)
