AC_DEFUN(LCD_DRIVERS_SELECT, [
AC_MSG_CHECKING(for which drivers to compile)

AC_ARG_ENABLE(drivers,
  	[  --enable-drivers=<list> compile driver for LCDs in <list>.]
	[                  drivers may be separated with commas.]
	[                  \"all\" compiles all drivers],
  	drivers="$enableval", 
  	drivers=[mtxorb,cfontz,curses,text])

if test "$drivers" = "all"; then
	drivers=[mtxorb,cfontz,curses,text,hd44780,joy,irman]
  	AC_MSG_RESULT(all)
else
  	drivers=`echo $drivers | sed 's/,/ /g'`
  	AC_MSG_RESULT($drivers)
  	for driver in $drivers
  	do
    		case "$driver" in
        	mtxorb)
			DRIVERS="$DRIVERS MtxOrb.o"
			AC_DEFINE(MTXORB_DRV)
			;;
		cfontz)
			DRIVERS="$DRIVERS CFontz.o"
			AC_DEFINE(CFONTZ_DRV)
			;;
		curses)
			AC_CHECK_LIB(curses, main, LIBCURSES="-lcurses",
				AC_MSG_ERROR([The curses driver needs the curses library]))
			DRIVERS="$DRIVERS curses_drv.o"
			AC_DEFINE(CURSES_DRV)
			;;
		text)
			DRIVERS="$DRIVERS text.o"
			AC_DEFINE(TEXT_DRV)
			;;
		hd44780)
			DRIVERS="$DRIVERS hd44780.o"
			AC_DEFINE(HD44780_DRV)
			;;
		joy)	
			DRIVERS="$DRIVERS joy.o"
			AC_DEFINE(JOY_DRV)
			;;
		irman)
			AC_CHECK_LIB(irman, main, LIBIRMAN="-lirman",
				AC_MSG_ERROR([The irman driver needs the irman library]))
			DRIVERS="$DRIVERS irmanin.o"
			AC_DEFINE(IRMANIN_DRV)
			;;
	*) 	
			AC_MSG_ERROR([Uknown driver $driver])
			;;
  		esac
  	done
fi
AC_SUBST(LIBCURSES)
AC_SUBST(LIBIRMAN)
AC_SUBST(DRIVERS)
])
