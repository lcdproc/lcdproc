## We use bin_ to generate the shared libraries, because automake only
## supports libtool shared libraries. But we don't want to use libtool...
##
## configure replaces all occurrences of so to the appropriate extension
## (without dot!)
##

## Fill in loadable module extension
EXEEXT = @SO@

## How to compile the files for the modules:
CCFLAGS += @CCSHARED@

## And how to build them
LDFLAGS += @LDSHARED@

## Forget the libs that the server core requires
LIBS =

pkglib_PROGRAMS = @DRIVERS@
EXTRA_PROGRAMS = bayrad CFontz CFontz633 curses CwLnx glk hd44780 irman icp_a106 joy lb216 mtc_s16209x lcdm001 lcterm lirc MtxOrb sed1330 sed1520 stv5730 svga t6963 text wirz_sli ms6931 IOWarrior
noinst_LIBRARIES = libLCD.a

IOWarrior_CFLAGS =   @libusb_cflags@
hd44780_CFLAGS =     @libusb_cflags@

CFontz_LDADD =       libLCD.a
CFontz633_LDADD =    libLCD.a
curses_LDADD =       @LIBCURSES@
CwLnx_LDADD =        libLCD.a
CwLnx_SOURCES =      lcd.h CwLnx.c CwLnx.h report.h
hd44780_LDADD =      libLCD.a @HD44780_DRIVERS@ @libusb_libs@
hd44780_DEPENDENCIES = @HD44780_DRIVERS@
icp_a106_LDADD =     libLCD.a
IOWarrior_LDADD =    @libusb_libs@ libLCD.a
irman_LDADD =        @LIBIRMAN@
lirc_LDADD =         @LIBLIRC_CLIENT@
lcterm_LDADD =       libLCD.a
svga_LDADD =         @LIBSVGA@
t6963_LDADD =        libLCD.a

libLCD_a_SOURCES =   lcd_lib.h lcd_lib.c

bayrad_SOURCES =     lcd.h bayrad.h bayrad.c report.h
CFontz_SOURCES =     lcd.h CFontz.c CFontz.h report.h
CFontz633_SOURCES =  lcd.h CFontz633.c CFontz633.h CFontz633io.c CFontz633io.h report.h
curses_SOURCES =     lcd.h curses_drv.h curses_drv.c report.h
glk_SOURCES =        lcd.h glk.c glk.h glkproto.c glkproto.h report.h
hd44780_SOURCES =    lcd.h hd44780.h hd44780.c hd44780-drivers.h hd44780-low.h hd44780-charmap.h report.h
EXTRA_hd44780_SOURCES = hd44780-4bit.c hd44780-4bit.h hd44780-ext8bit.c hd44780-ext8bit.h lcd_sem.c lcd_sem.h hd44780-serialLpt.c hd44780-serialLpt.h hd44780-winamp.c hd44780-winamp.h hd44780-picanlcd.c hd44780-picanlcd.h hd44780-bwct-usb.c hd44780-bwct-usb.h port.h lpt-port.h timing.h

icp_a106_SOURCES =   lcd.h icp_a106.c icp_a106.h report.h
IOWarrior_SOURCES =  lcd.h lcd_lib.h hd44780-charmap.h IOWarrior.c IOWarrior.h report.h
irman_SOURCES =      lcd.h irmanin.c irmanin.h report.h
joy_SOURCES =        lcd.h joy.c joy.h port.h report.h
lb216_SOURCES =      lcd.h lb216.c lb216.h report.h
mtc_s16209x_SOURCES =  lcd.h mtc_s16209x.c mtc_s16209x.h report.h
lcterm_SOURCES =     lcd.h lcterm.c lcterm.h report.h
lcdm001_SOURCES =    lcd.h lcdm001.c lcdm001.h report.h
lirc_SOURCES =       lcd.h lircin.c lircin.h report.h
MtxOrb_SOURCES =     lcd.h MtxOrb.c MtxOrb.h report.h bigfont.h
sed1330_SOURCES =    lcd.h sed1330.h sed1330.c port.h lpt-port.h timing.h report.h
sed1520_SOURCES =    lcd.h sed1520.c sed1520.h sed1520fm.c sed1520fm.h port.h report.h
stv5730_SOURCES =    lcd.h stv5730.c stv5730.h report.h
svga_SOURCES =       lcd.h svgalib_drv.c svgalib_drv.h report.h
t6963_SOURCES =      lcd.h t6963.c t6963.h t6963_font.h report.h
text_SOURCES =       lcd.h text.h text.c report.h
wirz_sli_SOURCES =   lcd.h wirz-sli.h wirz-sli.c report.h
ms6931_SOURCES =     lcd.h ms6931.h ms6931.c report.h

INCLUDES = -I$(top_srcdir)
