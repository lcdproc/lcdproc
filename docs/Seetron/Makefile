
# adjust your serial port
PORT=/dev/ttyR1
# switch your display to 9600 bps i.a.w. the manual first!
SPEED=9600

all:	sgxbmp load


sgxbmp:
	cc -o sgxbmp sgxbmp.c -lm

# make sure your display isn't set to EEPROM write protection
# (see ESC-W command)

load:	sgxbmp
	# write CHARS0.BMP to EEPROM page #0
	./sgxbmp $(PORT) $(SPEED) CHARS0.BMP 0
	sleep 1
	echo -ne "\033X0" > $(PORT)
	sleep 1
	# write CHARS1.BMP to EEPROM page #1
	./sgxbmp $(PORT) $(SPEED) CHARS1.BMP 0
	sleep 1
	echo -ne "\033X1" > $(PORT)
	sleep 1
	# write logo to EEPROM page #2
	./sgxbmp $(PORT) $(SPEED) vdr-logo-small.bmp 0
	sleep 1
	echo -ne "\033X2" > $(PORT)

clean:
	/bin/rm -f sgxbmp *.o core *~

