include ../Makefile.config

########################################################################
# Modify these variables to fit your system.
########################################################################

##################### Serial device
# This should be whatever your display is connected to.
# "make install" will create a link from /dev/lcd to DEVICE
DEVICE = /dev/ttyS0

##################### Operating System
# Uncomment the line for your OS.
OS=-DLINUX
#OS=-DIRIX

##################### Modescreen Options

##### Load Screen
# Max:  Load Avg at which the backlight will start blinking
MISC += -DLOAD_MAX=1.3
# Min:  Load Avg at which the backlight will turn off (asleep) 
MISC += -DLOAD_MIN=0.05

##### Filesystems to display stats for... (comment out ones you don't want)
# NFS (Network File System)
MISC += -DSTAT_NFS
# SMBFS (Samba filesystem -- Windows drives)
MISC += -DSTAT_SMBFS


##################### Drivers
# Comment out each section you don't want.

# Matrix-Orbital displays
DRIVERS += -DMTXORB_DRV
OBJ += MtxOrb.o

# Curses text-based output
DRIVERS += -DCURSES_DRV
OBJ += curses_drv.o
LIB += -lncurses

# Regular text output (ugly)
DRIVERS += -DTEXT_DRV
OBJ += text.o

# Debugging output (very ugly)
DRIVERS += -DDEBUG_DRV
OBJ += debug.o

# HD44780 parallel port lcd displays
#DRIVERS += -DHD44780_DRV
#OBJ += hd44780.o
# Pick one of the following ports to use
#LPTPORT = 0x378
#LPTPORT = 0x278
#LPTPORT = 0x3bc



########################################################################
# You shouldn't need to touch anything below here.
########################################################################

#GCC = gcc -Wall -Wp,-lang-c-c++-comments -O6
#OBJ += main.o mode.o lcd.o sockets.o config.o drv_base.o
#MISC += $(OS) $(DRIVERS)
#VERSION = 0.4


###################################################################
# Compilation...
#
#MISC=-DCURSES_DRV -g -DDEBUG

test:
	$(GCC) -c $(MISC) menutest.c
	$(GCC) -c $(MISC) lcd.c
	$(GCC) -c $(MISC) menu.c
	$(GCC) -c $(MISC) drivers/curses_drv.c
	$(GCC) -c $(MISC) drivers/drv_base.c
	$(GCC) -o menutest curses_drv.o menutest.o lcd.o drv_base.o menu.o -lncurses

test2:
	$(GCC) -c $(MISC) LL.c
	$(GCC) -c $(MISC) LL_test.c
	$(GCC) -o LL_test LL_test.o LL.o

test3:
	$(GCC) -c $(MISC) config.c
	$(GCC) -c $(MISC) config_test.c
	$(GCC) -o config_test config.o config_test.o

driver_test:
	$(GCC) -g -c $(MISC) drv_test.c
	$(GCC) -g -o drv_test drv_test.o ../server/drivers/libLCDdrivers.a \
		../shared/libLCDstuff.a -lncurses

socktest:
#	$(GCC) -g -c $(MISC) socktest.c
#	$(GCC) -g -o st socktest.o ../server/drivers/libLCDdrivers.a \
#		../shared/libLCDstuff.a -lncurses
	$(GCC) -c $(MISC) srvtest.c
	$(GCC) -c $(MISC) ../server/sock.c
	$(GCC) $(MISC) -o srv srvtest.o ../server/*.o \
		../server/drivers/libLCDdrivers.a \
		../shared/libLCDstuff.a -lncurses





##################################################################
# Installation
#
install:
	@echo Nothing to install in tests...

##################################################################
# Other stuff...
#
clean:
	rm -f *.o core *~

edit:
	emacs . &

