noinst_LIBRARIES = libLCDdrivers.a
libLCDdrivers_a_SOURCES = lcd.c lcd.h drv_base.c
EXTRA_libLCDdrivers_a_SOURCES = MtxOrb.c MtxOrb.h text.c text.h \
        curses_drv.c curses_drv.h drv_base.c drv_base.h \
        hd44780.c hd44780.h port.h joy.c joy.h irmanin.c irmanin.h \
        CFontz.c CFontz.h lircin.c lircin.h debug.h 
libLCDdrivers_a_DEPENDENCIES = @DRIVERS@
libLCDdrivers_a_LIBADD = @DRIVERS@
