########################################################################
# You shouldn't need to touch anything below here.
########################################################################
include ../Makefile.config


TARGET = libLCDstuff.a
OBJ = LL.o sockets.o str.o
#OBJ = LL.o config.o sockets.o
#TODO: config and sockets stuff...

###################################################################
# Compilation...
#
all: $(TARGET)

$(TARGET): $(OBJ) Makefile
	ar rcs $(TARGET) $(OBJ)
#$(TARGET): $(OBJ) Makefile
#	$(GCC) -s $(MISC) -o $(TARGET) $(OBJ) $(LIB)

%.o: %.c %.h Makefile
	$(GCC) -c $(MISC) $<


##################################################################
# Installation
#
install: $(TARGET)
	@echo No shared stuff will be installed...


##################################################################
# Other stuff...
#
clean:
	rm -f $(OBJ) $(TARGET) *~ core

edit:
	emacs . &

