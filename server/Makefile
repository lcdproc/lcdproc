include ../Makefile.config

########################################################################
# You shouldn't need to touch anything below here.
########################################################################

TARGET = LCDd
DIRS = drivers
OBJ += sock.o clients.o parse.o client_functions.o client_data.o screen.o \
	widget.o screenlist.o render.o serverscreens.o main.o input.o \
	menu.o menus.o
LIB += drivers/libLCDdrivers.a ../shared/libLCDstuff.a

ifeq ($(IRMAN),1)
  LIB += ../../libirman-0.4.1b/libirman.a
endif


include ../Makefile.config

###################################################################
# Compilation...
#
#all: $(OBJ)
all: $(TARGET)

$(TARGET): $(OBJ) $(LIB) Makefile
	$(GCC) -s $(MISC) -o $(TARGET) $(OBJ) $(LIB)

%.o: %.c %.h Makefile
	$(GCC) -c $(MISC) $<


##################################################################
# Installation
#
install: $(TARGET)
	@echo Installing LCD Server...
	install -m 0755 -o root -g root $(TARGET) /usr/local/bin/


##################################################################
# Other stuff...
#
clean:
	@for i in $(DIRS); do \
		(cd $$i; [ -f Makefile ] && $(MAKE) $@) \
	done
	rm -f $(OBJ) $(TARGET) *~ core

edit:
	emacs . &
