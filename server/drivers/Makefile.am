## We use bin_ to generate the shared libraries, because automake only
## supports libtool shared libraries. But we don't want to use libtool...
##
## configure replaces all occurrences of so to the appropriate extension
## (without dot!)
##

pkglib_PROGRAMS = @DRIVERS@
EXTRA_PROGRAMS = bayrad.so CFontz.so curses_drv.so glk.so hd44780.so irman.so joy.so lb216.so lcdm001.so lirc.so MtxOrb.so sed1330.so sed1520.so stv5730.so svga.so t6963.so text.so wirz_sli.so
noinst_LIBRARIES = libLCD.a

EXTRA_DIST = Makefile.am.pre
nodist_DIST = Makefile.am

## How to compile the files for the modules:
CCFLAGS += @CCSHARED@

## And how to build them
LDFLAGS += @LDSHARED@

## Forget the libs that the server core requires
LIBS=

CFontz_so_LDADD =       libLCD.a
curses_drv_so_LDADD =   @LIBCURSES@
hd44780_so_LDADD =      libLCD.a
irman_so_LDADD =        @LIBIRMAN@
lirc_so_LDADD =         @LIBLIRC_CLIENT@
svga_so_LDADD =         @LIBSVGA@
t6963_so_LDADD =        libLCD.a

libLCD_a_SOURCES =        lcd_lib.h lcd_lib.c

bayrad_so_SOURCES =     lcd.h bayrad.h bayrad.c report.h
CFontz_so_SOURCES =     lcd.h CFontz.c CFontz.h report.h
curses_drv_so_SOURCES = lcd.h curses_drv.h curses_drv.c report.h
glk_so_SOURCES =        lcd.h glk.c glk.h glkproto.c glkproto.h report.h
hd44780_so_SOURCES =    lcd.h hd44780.h hd44780.c hd44780-drivers.h hd44780-low.h hd44780-charmap.h hd44780-4bit.c hd44780-4bit.h hd44780-ext8bit.c hd44780-ext8bit.h lcd_sem.c lcd_sem.h hd44780-serialLpt.c hd44780-serialLpt.h hd44780-winamp.c hd44780-winamp.h hd44780-picanlcd.c hd44780-picanlcd.h port.h lpt-port.h timing.h report.h
irman_so_SOURCES =      lcd.h irmanin.c irmanin.h report.h
joy_so_SOURCES =        lcd.h joy.c joy.h port.h report.h
lb216_so_SOURCES =      lcd.h lb216.c lb216.h report.h
lcdm001_so_SOURCES =    lcd.h lcdm001.c lcdm001.h report.h
lirc_so_SOURCES =       lcd.h lircin.c lircin.h report.h
MtxOrb_so_SOURCES =     lcd.h MtxOrb.c MtxOrb.h report.h bigfont.h
sed1330_so_SOURCES =    lcd.h sed1330.h sed1330.c port.h lpt-port.h timing.h report.h
sed1520_so_SOURCES =    lcd.h sed1520.c sed1520.h sed1520fm.c sed1520fm.h port.h report.h
stv5730_so_SOURCES =    lcd.h stv5730.c stv5730.h report.h
svga_so_SOURCES =       lcd.h svgalib_drv.c svgalib_drv.h report.h
t6963_so_SOURCES =      lcd.h t6963.c t6963.h t6963_font.h report.h
text_so_SOURCES =       lcd.h text.h text.c report.h
wirz_sli_so_SOURCES =   lcd.h wirz-sli.h wirz-sli.c report.h

Makefile.am: Makefile.am.pre
	@echo Makefile.am.pre has been changed, please rerun configure.
	@false

INCLUDES = -I$(top_srcdir)
