noinst_LIBRARIES = libLCDdrivers.a
libLCDdrivers_a_SOURCES = lcd.c lcd.h lcd_lib.c
EXTRA_libLCDdrivers_a_SOURCES = MtxOrb.c MtxOrb.h text.c text.h \
        lcdm001.c lcdm001.h \
        curses_drv.c curses_drv.h drv_base.h \
        hd44780.c hd44780.h lb216.c lb216.h \
	hd44780-4bit.c hd44780-4bit.h \
	hd44780-ext8bit.c hd44780-ext8bit.h lcd_sem.c lcd_sem.h \
	hd44780-serialLpt.c hd44780-serialLpt.h \
	hd44780-winamp.c hd44780-winamp.h \
	hd44780-low.h hd44780-drivers.h \
	sed1520.c sed1520.h sed1520fm.c \
	stv5730.c stv5730.h \
	port.h joy.c joy.h irmanin.c irmanin.h \
	bayrad.h bayrad.c \
	CFontz.c CFontz.h lircin.c lircin.h debug.h \
	glk.c glk.h glkproto.c glkproto.h \
	svgalib_drv.c svgalib_drv.h \
	t6963.c t6963.h t6963_font.h
libLCDdrivers_a_DEPENDENCIES = @DRIVERS@
libLCDdrivers_a_LIBADD = @DRIVERS@
# NOTE: $(srcdir)/..  is to get .h files from the server directory
#       $(top_srcdir)  is to get .h files from shared/...
# These are necessary because a VPATH build has split source and build
#    directories and these files are in the VPATH'd source area.
INCLUDES = -I$(srcdir)/.. -I$(top_srcdir)
