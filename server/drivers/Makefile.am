
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

## Keep the lists sorted!

lcdexecbindir = $(pkglibdir)
lcdexecbin_PROGRAMS = @DRIVERS@
EXTRA_PROGRAMS = bayrad CFontz CFontzPacket curses CwLnx debug ea65 EyeboxOne futaba g15 glcd glcdlib glk hd44780 i2500vfd icp_a106 imon imonlcd IOWarrior irman irtrans joy jw002 lb216 lcdm001 lcterm linux_input lirc lis MD8800 mdm166a ms6931 mtc_s16209x MtxOrb mx5000 NoritakeVFD Olimex_MOD_LCD1x9 picolcd pyramid rawserial sdeclcd sed1330 sed1520 serialPOS serialVFD shuttleVFD sli stv5730 SureElec svga t6963 text tyan ula200 vlsys_m428 xosd yard2LCD
noinst_LIBRARIES = libLCD.a libbignum.a

futaba_CFLAGS =      @LIBUSB_CFLAGS@ @LIBUSB_1_0_CFLAGS@ $(AM_CFLAGS)
g15_CFLAGS =         @LIBUSB_CFLAGS@ @FT2_CFLAGS@ $(AM_CFLAGS)
glcd_CFLAGS =        @FT2_CFLAGS@ @LIBPNG_CFLAGS@ @LIBUSB_CFLAGS@ @LIBX11_CFLAGS@ $(AM_CFLAGS)
hd44780_CFLAGS =     @LIBUSB_CFLAGS@ @LIBFTDI_CFLAGS@ $(AM_CFLAGS)
i2500vfd_CFLAGS =    @LIBFTDI_CFLAGS@ $(AM_CFLAGS)
IOWarrior_CFLAGS =   @LIBUSB_CFLAGS@ $(AM_CFLAGS)
lis_CFLAGS =         @LIBFTDI_CFLAGS@ $(AM_CFLAGS)
mdm166a_CFLAGS =     @LIBHID_CFLAGS@ $(AM_CFLAGS)
picolcd_CFLAGS =     @LIBUSB_CFLAGS@ @LIBUSB_1_0_CFLAGS@ $(AM_CFLAGS)
shuttleVFD_CFLAGS =  @LIBUSB_CFLAGS@ $(AM_CFLAGS)
ula200_CFLAGS =      @LIBFTDI_CFLAGS@ $(AM_CFLAGS)
xosd_CFLAGS =        @LIBXOSD_CFLAGS@ $(AM_CFLAGS)

bayrad_LDADD =       libLCD.a
CFontz_LDADD =       libLCD.a libbignum.a
CFontzPacket_LDADD = libLCD.a libbignum.a
curses_LDADD =       @LIBCURSES@
CwLnx_LDADD =        libLCD.a libbignum.a
futaba_LDADD =       @LIBUSB_LIBS@ @LIBUSB_1_0_LIBS@ libLCD.a
g15_LDADD =          @LIBG15@
glcd_LDADD =         libLCD.a @GLCD_DRIVERS@ @FT2_LIBS@ @LIBPNG_LIBS@ @LIBSERDISP@ @LIBUSB_LIBS@ @LIBX11_LIBS@
glcd_DEPENDENCIES =  @GLCD_DRIVERS@ glcd-glcd-render.o libLCD.a
glcdlib_LDADD =      @LIBGLCD@
glk_LDADD =          libbignum.a
hd44780_LDADD =      libLCD.a @HD44780_DRIVERS@ @HD44780_I2C@ @LIBUSB_LIBS@ @LIBFTDI_LIBS@ @LIBUGPIO@ libbignum.a
hd44780_DEPENDENCIES = @HD44780_DRIVERS@ @HD44780_I2C@ libLCD.a libbignum.a
i2500vfd_LDADD =     @LIBFTDI_LIBS@
imon_LDADD =         libLCD.a libbignum.a
imonlcd_LDADD =      libLCD.a
IOWarrior_LDADD =    @LIBUSB_LIBS@ libLCD.a libbignum.a
irman_LDADD =        @LIBIRMAN@
jw002_LDADD =        libLCD.a libbignum.a
lb216_LDADD =        libLCD.a
lcterm_LDADD =       libLCD.a libbignum.a
lirc_LDADD =         @LIBLIRC_CLIENT@
lis_LDADD =          libLCD.a @LIBFTDI_LIBS@ @LIBPTHREAD_LIBS@ libbignum.a
mdm166a_LDADD =      @LIBHID_LIBS@ libLCD.a
mtc_s16209x_LDADD =  libLCD.a
MtxOrb_LDADD =       libLCD.a libbignum.a
mx5000_LDADD =       @LIBMX5000@
NoritakeVFD_LDADD =  libbignum.a
picolcd_LDADD =      @LIBUSB_LIBS@ @LIBUSB_1_0_LIBS@ libLCD.a libbignum.a
pyramid_LDADD =      libLCD.a libbignum.a
sdeclcd_LDADD =      libLCD.a libbignum.a
serialPOS_LDADD =    libbignum.a
serialVFD_LDADD =    libLCD.a libbignum.a
shuttleVFD_LDADD =   @LIBUSB_LIBS@
sli_LDADD =          libLCD.a
SureElec_LDADD =     libLCD.a libbignum.a
svga_LDADD =         @LIBSVGA@
t6963_LDADD =        libLCD.a
tyan_LDADD =         libLCD.a libbignum.a
ula200_LDADD =       @LIBFTDI_LIBS@
xosd_LDADD =         @LIBXOSD_LIBS@ libbignum.a

libLCD_a_SOURCES =   lcd_lib.h lcd_lib.c
libbignum_a_SOURCES = adv_bignum.h  adv_bignum.c

bayrad_SOURCES =     lcd.h lcd_lib.h bayrad.h bayrad.c
CFontz_SOURCES =     lcd.h lcd_lib.h CFontz.c CFontz.h CFontz-charmap.h adv_bignum.h
CFontzPacket_SOURCES = lcd.h lcd_lib.h CFontzPacket.c CFontzPacket.h CFontz-charmap.h CFontz633io.c CFontz633io.h adv_bignum.h
curses_SOURCES =     lcd.h curses_drv.h curses_drv.c
CwLnx_SOURCES =      lcd.h lcd_lib.h CwLnx.c CwLnx.h
debug_SOURCES =      lcd.h debug.c debug.h
ea65_SOURCES =       lcd.h ea65.h ea65.c
EyeboxOne_SOURCES =  lcd.h lcd_lib.h EyeboxOne.c EyeboxOne.h
futaba_SOURCES =     lcd.h futaba.c futaba.h
g15_SOURCES =        lcd.h lcd_lib.h g15.h g15-num.c g15.c hidraw_lib.c
glcd_SOURCES =       lcd.h glcd_drv.c glcd_drv.h glcd-low.h glcd-drivers.h glcd-render.c glcd-render.h
EXTRA_glcd_SOURCES = glcd-t6963.c t6963_low.c t6963_low.h glcd-png.c glcd-serdisp.c glcd-glcd2usb.c glcd-glcd2usb.h glcd-x11.c glcd-picolcdgfx.c
glcdlib_SOURCES =    lcd.h lcd_lib.h glcdlib.h glcdlib.c
glk_SOURCES =        lcd.h glk.c glk.h glkproto.c glkproto.h
hd44780_SOURCES =    lcd.h lcd_lib.h hd44780.h hd44780.c hd44780-drivers.h hd44780-low.h hd44780-charmap.h adv_bignum.h i2c.h
EXTRA_hd44780_SOURCES = port.h lpt-port.h timing.h i2c.c hd44780-4bit.c hd44780-4bit.h hd44780-bwct-usb.c hd44780-bwct-usb.h hd44780-ethlcd.c hd44780-ethlcd.h hd44780-ext8bit.c hd44780-ext8bit.h hd44780-ftdi.c hd44780-ftdi.h hd44780-gpio.c hd44780-gpio.h hd44780-i2c.c hd44780-i2c.h hd44780-lcd2usb.c hd44780-lcd2usb.h hd44780-lis2.c hd44780-lis2.h hd44780-pifacecad.c hd44780-pifacecad.h hd44780-piplate.c hd44780-piplate.h hd44780-rpi.c hd44780-rpi.h hd44780-serial.c hd44780-serial.h hd44780-serialLpt.c hd44780-serialLpt.h hd44780-spi.c hd44780-spi.h hd44780-usb4all.c hd44780-usb4all.h hd44780-usblcd.c hd44780-usblcd.h hd44780-usbtiny.c hd44780-usbtiny.h hd44780-uss720.c hd44780-uss720.h hd44780-winamp.c hd44780-winamp.h  hd44780-lcm162.c hd44780-lcm162.h
i2500vfd_SOURCES =   lcd.h i2500vfd.c i2500vfd.h glcd_font5x8.h
icp_a106_SOURCES =   lcd.h lcd_lib.h icp_a106.c icp_a106.h
imon_SOURCES =       lcd.h lcd_lib.h hd44780-charmap.h imon.h imon.c adv_bignum.h
imonlcd_SOURCES =    lcd.h lcd_lib.h imonlcd.h imonlcd_font.h imonlcd.c
IOWarrior_SOURCES =  lcd.h lcd_lib.h hd44780-charmap.h IOWarrior.c IOWarrior.h adv_bignum.h
irman_SOURCES =      lcd.h irmanin.c irmanin.h
irtrans_SOURCES =    lcd.h irtrans.c irtrans.h irtrans_network.h irtrans_remote.h irtrans_errcode.h
joy_SOURCES =        lcd.h joy.c joy.h port.h
jw002_SOURCES =      lcd.h lcd_lib.h jw002.c jw002.h adv_bignum.h
lb216_SOURCES =      lcd.h lcd_lib.h lb216.c lb216.h
lcdm001_SOURCES =    lcd.h lcdm001.c lcdm001.h
lcterm_SOURCES =     lcd.h lcd_lib.h lcterm.c lcterm.h
linux_input_SOURCES = lcd.h linux_input.h linux_input.c
lirc_SOURCES =       lcd.h lircin.c lircin.h
lis_SOURCES =        lcd.h lcd_lib.h lis.h lis.c
MD8800_SOURCES =     lcd.h lcd_lib.h MD8800.c MD8800.h
mdm166a_SOURCES =    lcd.h mdm166a.c mdm166a.h glcd_font5x8.h
ms6931_SOURCES =     lcd.h lcd_lib.h ms6931.h ms6931.c
mtc_s16209x_SOURCES =  lcd.h lcd_lib.h mtc_s16209x.c mtc_s16209x.h
MtxOrb_SOURCES =     lcd.h lcd_lib.h MtxOrb.c MtxOrb.h adv_bignum.h
mx5000_SOURCES =     lcd.h mx5000.c mx5000.h
NoritakeVFD_SOURCES = lcd.h lcd_lib.h NoritakeVFD.c NoritakeVFD.h adv_bignum.h
Olimex_MOD_LCD1x9_SOURCES =  lcd.h i2c.h i2c.c Olimex_MOD_LCD1x9.h Olimex_MOD_LCD1x9.c Olimex_MOD_LCD1x9_font.h
rawserial_SOURCES =  lcd.h rawserial.c rawserial.h
picolcd_SOURCES =    lcd.h picolcd.h picolcd.c
pyramid_SOURCES =    lcd.h pylcd.c pylcd.h
sdeclcd_SOURCES =    lcd.h sdeclcd.h sdeclcd.c lcd_lib.h adv_bignum.h port.h lpt-port.h timing.h
sed1330_SOURCES =    lcd.h sed1330.h sed1330.c port.h lpt-port.h timing.h
sed1520_SOURCES =    lcd.h sed1520.c sed1520.h port.h glcd_font5x8.h sed1520fm.h
serialPOS_SOURCES =  lcd.h lcd_lib.h serialPOS.c serialPOS.h serialPOS_aedex.c serialPOS_cd5220.c serialPOS_common.c serialPOS_common.h serialPOS_epson.c serialPOS_logic_controls.c adv_bignum.h
serialVFD_SOURCES =  lcd.h lcd_lib.h serialVFD.c serialVFD.h adv_bignum.h serialVFD_displays.c serialVFD_displays.h serialVFD_io.c serialVFD_io.h
shuttleVFD_SOURCES = lcd.h shuttleVFD.c shuttleVFD.h
sli_SOURCES =        lcd.h lcd_lib.h wirz-sli.h wirz-sli.c
stv5730_SOURCES =    lcd.h stv5730.c stv5730.h
SureElec_SOURCES =   lcd.h lcd_lib.h SureElec.c SureElec.h adv_bignum.h
svga_SOURCES =       lcd.h svgalib_drv.c svgalib_drv.h
t6963_SOURCES =      lcd.h lcd_lib.h t6963.c t6963.h glcd_font5x8.h t6963_low.h t6963_low.c
text_SOURCES =       lcd.h text.h text.c
tyan_SOURCES =       lcd.h lcd_lib.h tyan_lcdm.h tyan_lcdm.c adv_bignum.h
ula200_SOURCES =     lcd.h adv_bignum.h ula200.h ula200.c
vlsys_m428_SOURCES = lcd.h vlsys_m428.c vlsys_m428.h
xosd_SOURCES =       lcd.h xosdlib_drv.c xosdlib_drv.h adv_bignum.h
yard2LCD_SOURCES =   lcd.h yard2LCD.c yard2LCD.h


AM_CPPFLAGS = -I$(top_srcdir)

## EOF
