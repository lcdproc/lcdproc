########################################################################
# You shouldn't need to touch anything below here.
########################################################################
include ../../Makefile.config

TARGET = libLCDdrivers.a
OBJ += lcd.o drv_base.o $(DOBJ)
SOURCES = Makefile lcd.c lcd.h \
	MtxOrb.c MtxOrb.h wirz-sli.c wirz-sli.h text.c text.h \
	curses_drv.c curses_drv.h drv_base.c drv_base.h \
	hd44780.c hd44780.h port.h joy.c joy.h irman.c irman.h \
	CFontz.c CFontz.h
INCLUDE = -I../../shared


###################################################################
# Compilation...
#
all: $(TARGET)


$(TARGET): $(OBJ) Makefile
	$(AR) rcs $(TARGET) $(OBJ)
#$(TARGET): $(OBJ) Makefile
#	$(GCC) -s $(MISC) $(INCLUDE) -o $(TARGET) $(OBJ) $(LIB)

%.o: %.c %.h Makefile
	$(GCC) -c $(MISC) $(INCLUDE) $<


##################################################################
# Other stuff...
#
clean:
	rm -f $(OBJ) *.o $(DOBJ) $(TARGET) *~ core

edit:
	nedit $(SOURCES) $(EXTRAS) &

todo:
	@echo ---------------===========================================
	@grep TODO $(SOURCES) | grep -v SOURCES
	@grep FIXME $(SOURCES) | grep -v SOURCES
	@echo ---------------===========================================

