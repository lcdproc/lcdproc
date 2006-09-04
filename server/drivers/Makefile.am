## Process this file with automake to produce Makefile.in

## We use bin_ to generate the shared libraries, because automake only
## supports libtool shared libraries. But we don't want to use libtool...
##
## configure replaces all occurrences of so to the appropriate extension
## (without dot!)

## Fill in loadable module extension
EXEEXT = @SO@

## How to compile the files for the modules:
AM_CFLAGS = @CCSHARED@

## And how to build them
AM_LDFLAGS = @LDSHARED@

## Forget the libs that the server core requires
#LIBS =

pkglib_PROGRAMS = @DRIVERS@
EXTRA_PROGRAMS = bayrad CFontz CFontz633 CFontzPacket curses CwLnx EyeboxOne g15 glcdlib glk hd44780 icp_a106 imon IOWarrior irman joy lb216 lcdm001 lcterm lirc MD8800 ms6931 mtc_s16209x MtxOrb NoritakeVFD pyramid sed1330 sed1520 serialVFD stv5730 svga t6963 text tyan sli ula200 xosd
noinst_LIBRARIES = libLCD.a libbignum.a

IOWarrior_CFLAGS =   @libusb_cflags@ $(AM_CFLAGS)
hd44780_CFLAGS =     @libusb_cflags@ $(AM_CFLAGS)
g15_CFLAGS =         @libusb_cflags@ $(AM_CFLAGS)

CFontz_LDADD =       libLCD.a libbignum.a
CFontz633_LDADD =    libLCD.a libbignum.a
CFontzPacket_LDADD = libLCD.a libbignum.a
curses_LDADD =       @LIBCURSES@
CwLnx_LDADD =        libLCD.a
g15_LDADD =          libLCD.a @LIBG15@
glcdlib_LDADD =      libLCD.a @LIBGLCD@
hd44780_LDADD =      libLCD.a @HD44780_DRIVERS@ @libusb_libs@ libbignum.a
hd44780_DEPENDENCIES = @HD44780_DRIVERS@
icp_a106_LDADD =     libLCD.a
imon_LDADD =         libLCD.a
IOWarrior_LDADD =    @libusb_libs@ libLCD.a libbignum.a
irman_LDADD =        @LIBIRMAN@
lcterm_LDADD =       libLCD.a
lirc_LDADD =         @LIBLIRC_CLIENT@
MD8800_LDADD =       libLCD.a
mtc_s16209x_LDADD =  libLCD.a
NoritakeVFD_LDADD =  libLCD.a
pyramid_LDADD =      libLCD.a
serialVFD_LDADD =    libLCD.a libbignum.a
svga_LDADD =         @LIBSVGA@
t6963_LDADD =        libLCD.a
tyan_LDADD =         libLCD.a libbignum.a
ula200_LDADD =       libLCD.a @LIBFTDI@
sli_LDADD =          libLCD.a
xosd_LDADD =         @LIBXOSD@

libLCD_a_SOURCES =   lcd_lib.h lcd_lib.c
libbignum_a_SOURCES = adv_bignum.h  adv_bignum.c 

bayrad_SOURCES =     lcd.h lcd_lib.h bayrad.h bayrad.c report.h
CFontz_SOURCES =     lcd.h lcd_lib.h CFontz.c CFontz.h CFontz-charmap.h report.h adv_bignum.h
CFontz633_SOURCES =  lcd.h lcd_lib.h CFontz633.c CFontz633.h CFontz-charmap.h CFontz633io.c CFontz633io.h report.h adv_bignum.h
CFontzPacket_SOURCES = lcd.h lcd_lib.h CFontzPacket.c CFontzPacket.h CFontz-charmap.h CFontz633io.c CFontz633io.h report.h adv_bignum.h
curses_SOURCES =     lcd.h curses_drv.h curses_drv.c report.h
CwLnx_SOURCES =      lcd.h lcd_lib.h CwLnx.c CwLnx.h report.h
EyeboxOne_SOURCES =   lcd.h lcd_lib.h EyeboxOne.c EyeboxOne.h report.h
g15_SOURCES = 	      lcd.h lcd_lib.h g15.h g15-num.c g15.c report.h
glcdlib_SOURCES =    lcd.h lcd_lib.h glcdlib.h glcdlib.c report.h
glk_SOURCES =        lcd.h glk.c glk.h glkproto.c glkproto.h report.h
hd44780_SOURCES =    lcd.h lcd_lib.h hd44780.h hd44780.c hd44780-drivers.h hd44780-low.h hd44780-charmap.h report.h adv_bignum.h
EXTRA_hd44780_SOURCES = hd44780-4bit.c hd44780-4bit.h hd44780-ext8bit.c hd44780-ext8bit.h lcd_sem.c lcd_sem.h hd44780-serialLpt.c hd44780-serialLpt.h hd44780-serial.c hd44780-serial.h hd44780-winamp.c hd44780-winamp.h hd44780-bwct-usb.c hd44780-bwct-usb.h hd44780-lis2.c hd44780-lis2.h hd44780-i2c.c hd44780-i2c.h port.h lpt-port.h timing.h

icp_a106_SOURCES =   lcd.h lcd_lib.h icp_a106.c icp_a106.h report.h
imon_SOURCES =       lcd.h lcd_lib.h imon.h imon.c report.h
IOWarrior_SOURCES =  lcd.h lcd_lib.h hd44780-charmap.h IOWarrior.c IOWarrior.h report.h adv_bignum.h
irman_SOURCES =      lcd.h irmanin.c irmanin.h report.h
joy_SOURCES =        lcd.h joy.c joy.h port.h report.h
lb216_SOURCES =      lcd.h lb216.c lb216.h report.h
lcdm001_SOURCES =    lcd.h lcdm001.c lcdm001.h report.h
lcterm_SOURCES =     lcd.h lcd_lib.h lcterm.c lcterm.h report.h
lirc_SOURCES =       lcd.h lircin.c lircin.h report.h
MD8800_SOURCES =     lcd.h lcd_lib.h MD8800.c MD8800.h report.h
ms6931_SOURCES =     lcd.h lcd_lib.h ms6931.h ms6931.c report.h
mtc_s16209x_SOURCES =  lcd.h lcd_lib.h mtc_s16209x.c mtc_s16209x.h report.h
MtxOrb_SOURCES =     lcd.h lcd_lib.h MtxOrb.c MtxOrb.h report.h bigfont.h
NoritakeVFD_SOURCES = lcd.h lcd_lib.h NoritakeVFD.c NoritakeVFD.h report.h
pyramid_SOURCES =    lcd.h pylcd.c pylcd.h
sed1330_SOURCES =    lcd.h sed1330.h sed1330.c port.h lpt-port.h timing.h report.h
sed1520_SOURCES =    lcd.h sed1520.c sed1520.h sed1520fm.c sed1520fm.h port.h report.h
serialVFD_SOURCES =  lcd.h lcd_lib.h serialVFD.c serialVFD.h report.h adv_bignum.h serialVFD_displays.c serialVFD_displays.h serialVFD_io.c serialVFD_io.h
stv5730_SOURCES =    lcd.h stv5730.c stv5730.h report.h
svga_SOURCES =       lcd.h svgalib_drv.c svgalib_drv.h report.h
t6963_SOURCES =      lcd.h lcd_lib.h t6963.c t6963.h t6963_font.h report.h
text_SOURCES =       lcd.h text.h text.c report.h
tyan_SOURCES =       lcd.h lcd_lib.h tyan_lcdm.h tyan_lcdm.c report.h adv_bignum.h
ula200_SOURCES =     lcd.h lcd_lib.h ula200.h ula200.c report.h
sli_SOURCES =        lcd.h lcd_lib.h wirz-sli.h wirz-sli.c report.h
xosd_SOURCES =       lcd.h xosdlib_drv.c xosdlib_drv.h report.h

AM_CPPFLAGS = -I$(top_srcdir)

## EOF
