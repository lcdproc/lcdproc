SUBDIRS=drivers
sbin_PROGRAMS=LCDd
LCDd_SOURCES= client_data.c client_data.h client_functions.c client_functions.h client_menu.h clients.c clients.h input.c input.h main.c main.h menu.c menu.h menus.c menus.h parse.c parse.h render.c render.h screen.c screen.h screenlist.c screenlist.h serverscreens.c serverscreens.h sock.c sock.h widget.c widget.h
LCDd_LDADD = drivers/libLCDdrivers.a ../shared/libLCDstuff.a @LIBCURSES@ @LIBIRMAN@ @LIBLIRC_CLIENT@
INCLUDES = -I$(top_srcdir)
