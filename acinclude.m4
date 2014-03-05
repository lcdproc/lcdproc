dnl
dnl Define function/macro for driver selection using the --enable-drivers=... option
dnl
AC_DEFUN([LCD_DRIVERS_SELECT], [
AC_MSG_NOTICE([checking which drivers to compile...])

AC_ARG_ENABLE(drivers,
	[  --enable-drivers=<list> compile drivers for LCDs in <list>,]
	[                  which is a comma-separated list of drivers.]
	[                  Possible drivers are:]
	[                    bayrad,CFontz,CFontzPacket,curses,CwLnx,ea65,]
	[                    EyeboxOne,g15,glcd,glcdlib,glk,hd44780,i2500vfd,]
	[                    icp_a106,imon,imonlcd,IOWarrior,irman,irtrans,]
	[                    joy,lb216,lcdm001,lcterm,lirc,lis,MD8800,mdm166a,]
	[                    ms6931,mtc_s16209x,MtxOrb,mx5000,NoritakeVFD,]
	[                    picolcd,pyramid,rawserial,sdeclcd,sed1330,sed1520,]
	[                    serialPOS,serialVFD,shuttleVFD,sli,stv5730,SureElec,]
	[                    svga,t6963,text,tyan,ula200,vlsys_m428,xosd]
	[                    ]
	[                  'all' compiles all drivers;]
	[                  'all,!xxx,!yyy' de-selects previously selected drivers],
	drivers="$enableval",
	drivers=[bayrad,CFontz,CFontzPacket,curses,CwLnx,glk,lb216,lcdm001,MtxOrb,pyramid,text])

allDrivers=[bayrad,CFontz,CFontzPacket,curses,CwLnx,ea65,EyeboxOne,g15,glcd,glcdlib,glk,hd44780,i2500vfd,icp_a106,imon,imonlcd,IOWarrior,irman,irtrans,joy,lb216,lcdm001,lcterm,lirc,lis,MD8800,mdm166a,ms6931,mtc_s16209x,MtxOrb,mx5000,NoritakeVFD,picolcd,pyramid,sdeclcd,sed1330,sed1520,serialPOS,serialVFD,shuttleVFD,sli,stv5730,SureElec,svga,t6963,text,tyan,ula200,vlsys_m428,xosd,rawserial]
if test "$debug" = yes; then
	allDrivers=["${allDrivers},debug"]
fi

dnl replace special keyword "all" in a secure manner
drivers=[" $drivers "]
drivers=`echo " $drivers " | sed -e "s/ all,/ ${allDrivers} /"`
drivers=`echo " $drivers " | sed -e "s/ all / ${allDrivers} /"`
drivers=`echo $drivers | sed -e 's/,/ /g'`

dnl ignore unwanted drivers
selectdrivers=" "
for driver in $drivers ; do
	case $driver in
		!*)
			driver=`echo "$driver" | sed -e 's/^.//'`
			selectdrivers=[`echo " $selectdrivers " | sed -e "s/ $driver / /g"`]
			;;
		*)
			selectdrivers=["$selectdrivers $driver "]
			;;
	esac
done

dnl check for wanted drivers and their dependencies
for driver in $selectdrivers ; do
	case "$driver" in
		bayrad)
			DRIVERS="$DRIVERS bayrad${SO}"
			actdrivers=["$actdrivers bayrad"]
			;;
		CFontz)
			DRIVERS="$DRIVERS CFontz${SO}"
			actdrivers=["$actdrivers CFontz"]
			;;
		CFontzPacket)
			DRIVERS="$DRIVERS CFontzPacket${SO}"
			actdrivers=["$actdrivers CFontzPacket"]
			AC_CHECK_FUNCS(select, [
				AC_CHECK_HEADERS(sys/select.h)
			],[
				AC_MSG_WARN([The CFontzPacket driver needs the select() function])
			])
			;;
		curses)
			AC_CHECK_HEADERS(ncurses.h curses.h)
			AC_CHECK_LIB(ncurses, main, [
				AC_CHECK_HEADER(ncurses.h, [
					dnl We have ncurses.h and libncurses, add driver.
	 				LIBCURSES="-lncurses"
					DRIVERS="$DRIVERS curses${SO}"
					actdrivers=["$actdrivers curses"]
				],[
dnl				else
					AC_MSG_WARN([Could not find ncurses.h])],
				[])
			],[
dnl			else
				AC_CHECK_LIB(curses, main, [
					AC_CHECK_HEADER(curses.h, [
						dnl We have curses.h and libcurses, add driver.
						LIBCURSES="-lcurses"
						DRIVERS="$DRIVERS curses${SO}"
						actdrivers=["$actdrivers curses"]
					],[
dnl					else
						AC_MSG_WARN([Could not find curses.h])],
					[])
				],[
dnl				else
					AC_MSG_WARN([The curses driver needs the curses (or ncurses) library.])],
				[])
			])

			AC_CURSES_ACS_ARRAY

			AC_CACHE_CHECK([for redrawwin() in curses], ac_cv_curses_redrawwin,
			[oldlibs="$LIBS"
			 LIBS="$LIBS $LIBCURSES"
			 AC_TRY_LINK_FUNC(redrawwin, ac_cv_curses_redrawwin=yes, ac_cv_curses_redrawwin=no)
			 LIBS="$oldlibs"
			])
			if test "$ac_cv_curses_redrawwin" = yes; then
				AC_DEFINE(CURSES_HAS_REDRAWWIN,[1],[Define to 1 if you have the redrawwin function in the curses library])
			fi

			AC_CACHE_CHECK([for wcolor_set() in curses], ac_cv_curses_wcolor_set,
			[oldlibs="$LIBS"
			 LIBS="$LIBS $LIBCURSES"
			 AC_TRY_LINK_FUNC(wcolor_set, ac_cv_curses_wcolor_set=yes, ac_cv_curses_wcolor_set=no)
			 LIBS="$oldlibs"
			])
			if test "$ac_cv_curses_wcolor_set" = yes; then
				AC_DEFINE(CURSES_HAS_WCOLOR_SET,[1],[Define to 1 if you have the wcolor_set function in the curses library])
			fi
			;;
		CwLnx)
			DRIVERS="$DRIVERS CwLnx${SO}"
			actdrivers=["$actdrivers CwLnx"]
			;;
		debug)
			DRIVERS="$DRIVERS debug${SO}"
			actdrivers=["$actdrivers debug"]
			;;
		ea65)
			DRIVERS="$DRIVERS ea65${SO}"
			actdrivers=["$actdrivers ea65"]
			;;
		EyeboxOne)
			DRIVERS="$DRIVERS EyeboxOne${SO}"
			actdrivers=["$actdrivers EyeboxOne"]
			;;
		g15)
			AC_CHECK_HEADERS([g15daemon_client.h],[
				AC_CHECK_LIB(g15daemon_client, new_g15_screen,[
					LIBG15="-lg15daemon_client"
				],[
dnl				else
					AC_MSG_WARN([The g15 driver needs libg15daemon_client-1.2 or better])
				],
				[-lg15daemon_client]
				)
			],[
dnl			else
				AC_MSG_WARN([The g15 driver needs g15daemon_client.h])
			])
			AC_CHECK_HEADERS([libg15render.h],[
				AC_CHECK_LIB(g15render, g15r_initCanvas,[
					LIBG15="$LIBG15 -lg15render"
					DRIVERS="$DRIVERS g15${SO}"
					actdrivers=["$actdrivers g15"]
				],[
dnl				else
					AC_MSG_WARN([the g15 driver needs libg15render])
				],
				[-lg15render]
				)
			],[
dnl			else
				AC_MSG_WARN([The g15driver needs libg15render.h])
			])
			;;
		glcd)
			GLCD_DRIVERS=""
			if test "$ac_cv_port_have_lpt" = yes ; then
				GLCD_DRIVERS="$GLCD_DRIVERS glcd-glcd-t6963.o t6963_low.o"
			fi
			if test "$enable_libpng" = yes ; then
				GLCD_DRIVERS="$GLCD_DRIVERS glcd-glcd-png.o"
			fi
			if test "$enable_libusb" = yes ; then
				GLCD_DRIVERS="$GLCD_DRIVERS glcd-glcd-glcd2usb.o glcd-glcd-picolcdgfx.o"
			fi
			AC_CHECK_HEADERS([serdisplib/serdisp.h],[
				AC_CHECK_LIB(serdisp, serdisp_nextdisplaydescription,[
					AC_DEFINE(HAVE_SERDISPLIB,[1],[Define to 1 if you have working serdisplib])
					LIBSERDISP="-lserdisp"
					GLCD_DRIVERS="$GLCD_DRIVERS glcd-glcd-serdisp.o"
				],[
					AC_MSG_WARN([serdisp library not working])
				])
			])
			AC_SUBST(LIBSERDISP)
			if test "$enable_libX11" = yes ; then
				GLCD_DRIVERS="$GLCD_DRIVERS glcd-glcd-x11.o"
			fi
			DRIVERS="$DRIVERS glcd${SO}"
			actdrivers=["$actdrivers glcd"]
			;;
		glcdlib)
			AC_CHECK_HEADERS([glcdproclib/glcdprocdriver.h],[
				AC_CHECK_LIB(glcdprocdriver, main,[
					LIBGLCD="-lglcddrivers -lglcdgraphics -lglcdprocdriver"
					DRIVERS="$DRIVERS glcdlib${SO}"
					actdrivers=["$actdrivers glcdlib"]
				],[
dnl				else
					AC_MSG_WARN([The glcdlib driver needs the glcdprocdriver library])
				],
				[-lglcddrivers -lglcdgraphics -lglcdprocdriver]
				)
			],[
dnl			else
				AC_MSG_WARN([The glcdlib driver needs glcdproclib/glcdprocdriver.h])
			])
			;;
		glk)
			DRIVERS="$DRIVERS glk${SO}"
			actdrivers=["$actdrivers glk"]
			;;
		hd44780)
			HD44780_DRIVERS="hd44780-hd44780-serial.o hd44780-hd44780-lis2.o hd44780-hd44780-usblcd.o"
			if test "$ac_cv_port_have_lpt" = yes ; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-4bit.o hd44780-hd44780-ext8bit.o hd44780-lcd_sem.o hd44780-hd44780-winamp.o hd44780-hd44780-serialLpt.o"
			fi
			if test "$enable_libusb" = yes ; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-bwct-usb.o hd44780-hd44780-lcd2usb.o hd44780-hd44780-uss720.o hd44780-hd44780-usbtiny.o hd44780-hd44780-usb4all.o"
			fi
			if test "$enable_libftdi" = yes ; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-ftdi.o"
			fi
			if test "$enable_ethlcd" = yes ; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-ethlcd.o"
				AC_DEFINE(WITH_ETHLCD,[1],[Define to 1 if you want to build hd44780 driver with ethlcd support])
			fi
			if test "$x_ac_have_i2c" = yes; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-i2c.o"
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-piplate.o"
			fi
			if test "$x_ac_have_spi" = yes; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-spi.o hd44780-hd44780-pifacecad.o"
			fi
dnl			The hd4470-rpi driver only works on a Raspberry Pi,
dnl			which is an ARM platform. Require people to compile on
dnl			(or for) ARM to get it.
			case $host in
			arm*-*-linux*)
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-hd44780-rpi.o"
				AC_DEFINE([WITH_RASPBERRYPI],[1],[Define if you are using a Raspberry Pi.])
				;;
			esac
			DRIVERS="$DRIVERS hd44780${SO}"
			actdrivers=["$actdrivers hd44780"]
			;;
		i2500vfd)
			if test "$enable_libftdi" = yes ; then
				DRIVERS="$DRIVERS i2500vfd${SO}"
				actdrivers=["$actdrivers i2500vfd"]
			else
				AC_MSG_WARN([The i2500vfd driver needs the ftdi library])
			fi
			;;
		icp_a106)
			DRIVERS="$DRIVERS icp_a106${SO}"
			actdrivers=["$actdrivers icp_a106"]
			;;
		imon)
			DRIVERS="$DRIVERS imon${SO}"
			actdrivers=["$actdrivers imon"]
			;;
		imonlcd)
			DRIVERS="$DRIVERS imonlcd${SO}"
			actdrivers=["$actdrivers imonlcd"]
			;;
		IOWarrior)
			if test "$enable_libusb" = yes ; then
				DRIVERS="$DRIVERS IOWarrior${SO}"
				actdrivers=["$actdrivers IOWarrior"]
			else
				AC_MSG_WARN([The IOWarrior driver needs the libusb library.])
			fi
			;;
		irman)
			AC_CHECK_LIB(irman, main,[
				LIBIRMAN="-lirman"
				DRIVERS="$DRIVERS irman${SO}"
				actdrivers=["$actdrivers irman"]
				],[
dnl				else
				AC_MSG_WARN([The irman driver needs the irman library.])
			])
			;;
		irtrans)
			DRIVERS="$DRIVERS irtrans${SO}"
			actdrivers=["$actdrivers irtrans"]
			;;
		joy)
			AC_CHECK_HEADER(linux/joystick.h, [
				DRIVERS="$DRIVERS joy${SO}"
				actdrivers=["$actdrivers joy"]
				],[
dnl				else
				AC_MSG_WARN([The joy driver needs header file linux/joystick.h.])
			])
			;;
		lb216)
			DRIVERS="$DRIVERS lb216${SO}"
			actdrivers=["$actdrivers lb216"]
			;;
		lcdm001)
			DRIVERS="$DRIVERS lcdm001${SO}"
			actdrivers=["$actdrivers lcdm001"]
			;;
		lcterm)
			DRIVERS="$DRIVERS lcterm${SO}"
			actdrivers=["$actdrivers lcterm"]
			;;
		lirc)
			AC_CHECK_LIB(lirc_client, main,[
				LIBLIRC_CLIENT="-llirc_client"
				DRIVERS="$DRIVERS lirc${SO}"
				actdrivers=["$actdrivers lirc"]
				],[
dnl				else
				AC_MSG_WARN([The lirc driver needs the lirc client library])
			])
			;;
		lis)
			AC_CHECK_HEADERS([pthread.h],[
				AC_CHECK_LIB(pthread, pthread_create,[
					LIBPTHREAD_LIBS="-lpthread"
					ac_cv_lis_pthread=yes
				],[
dnl				else
					ac_cv_lis_pthread=no
					AC_MSG_WARN([The lis driver needs the pthread library and pthread_create() from it])
				])
			],[
dnl			else
				ac_cv_lis_pthread=no
				AC_MSG_WARN([The lis driver needs pthread.h])
			])
			if test "$enable_libftdi" = yes ; then
				if test "$enable_libusb" = yes; then
					if test "$ac_cv_lis_pthread" = yes; then
						DRIVERS="$DRIVERS lis${SO}"
						actdrivers=["$actdrivers lis"]
					else
						AC_MSG_WARN([The lis driver needs the pthread library])
					fi
				else
					AC_MSG_WARN([The lis driver needs the usb library])
				fi
			else
				AC_MSG_WARN([The lis driver needs the ftdi library])
			fi
			;;
		MD8800)
			DRIVERS="$DRIVERS MD8800${SO}"
			actdrivers=["$actdrivers MD8800"]
			;;
		mdm166a)
			if test "$enable_libhid" = yes ; then
				DRIVERS="$DRIVERS mdm166a${SO}"
				actdrivers=["$actdrivers mdm166a"]
			else
				AC_MSG_WARN([The mdm166a driver needs the hid library])
			fi
			;;
		ms6931)
			DRIVERS="$DRIVERS ms6931${SO}"
			actdrivers=["$actdrivers ms6931"]
			;;
		mtc_s16209x)
			DRIVERS="$DRIVERS mtc_s16209x${SO}"
			actdrivers=["$actdrivers mtc_s16209x"]
			;;
		MtxOrb)
			DRIVERS="$DRIVERS MtxOrb${SO}"
			actdrivers=["$actdrivers MtxOrb"]
			;;
		mx5000)
			AC_CHECK_HEADERS([libmx5000/mx5000.h],[
				AC_CHECK_LIB(mx5000, mx5000_sc_new_static,[
					LIBMX5000="-lmx5000"
					DRIVERS="$DRIVERS mx5000${SO}"
					actdrivers=["$actdrivers mx5000"]
				],[
dnl				else
					AC_MSG_WARN([The mx5000 driver needs the mx5000tools library])
				])
			],[
dnl			else
				AC_MSG_WARN([The mx5000 driver needs libmx5000/mx5000.h])
			])
			;;
		NoritakeVFD)
			DRIVERS="$DRIVERS NoritakeVFD${SO}"
			actdrivers=["$actdrivers NoritakeVFD"]
			;;
		rawserial)
			DRIVERS="$DRIVERS rawserial${SO}"
			actdrivers=["$actdrivers rawserial"]
			;;
		picolcd)
			if test "$enable_libusb" = yes ; then
				DRIVERS="$DRIVERS picolcd${SO}"
				actdrivers=["$actdrivers picolcd"]
				if test "$enable_libusb_1_0" = yes ; then
					AC_MSG_RESULT([The picolcd driver is using the libusb-1.0 library.])
				else
					AC_MSG_RESULT([The picolcd driver is using the libusb-0.1 library.])
				fi
			else
				AC_MSG_WARN([The picolcd driver needs the libusb library.])
			fi
			;;
		pyramid)
			DRIVERS="$DRIVERS pyramid${SO}"
			actdrivers=["$actdrivers pyramid"]
			;;
		sdeclcd)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS sdeclcd${SO}"
				actdrivers=["$actdrivers sdeclcd"]
			else
				AC_MSG_WARN([The sdeclcd driver needs a parallel port.])
			fi
			;;
		sed1330)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS sed1330${SO}"
				actdrivers=["$actdrivers sed1330"]
			else
				AC_MSG_WARN([The sed1330 driver needs a parallel port.])
			fi
			;;
		sed1520)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS sed1520${SO}"
				actdrivers=["$actdrivers sed1520"]
			else
				AC_MSG_WARN([The sed1520 driver needs a parallel port.])
			fi
			;;
		serialPOS)
			DRIVERS="$DRIVERS serialPOS${SO}"
			actdrivers=["$actdrivers serialPOS"]
			;;
		serialVFD)
			DRIVERS="$DRIVERS serialVFD${SO}"
			actdrivers=["$actdrivers serialVFD"]
			;;
		shuttleVFD)
			if test "$enable_libusb" = yes ; then
				DRIVERS="$DRIVERS shuttleVFD${SO}"
				actdrivers=["$actdrivers shuttleVFD"]
			else
				AC_MSG_WARN([The shuttleVFD driver needs the libusb library.])
			fi
			;;
		sli)
			DRIVERS="$DRIVERS sli${SO}"
			actdrivers=["$actdrivers sli"]
			;;
		stv5730)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS stv5730${SO}"
				actdrivers=["$actdrivers stv5730"]
			else
				AC_MSG_WARN([The stv5730 driver needs a parallel port.])
			fi
			;;
		SureElec)
			DRIVERS="$DRIVERS SureElec${SO}"
			actdrivers=["$actdrivers SureElec"]
			;;
		svga)
			AC_CHECK_HEADER([vga.h], [
				AC_CHECK_HEADER([vgagl.h],[
					AC_CHECK_LIB(vga, main,[
						LIBSVGA="-lvga -lvgagl"
						DRIVERS="$DRIVERS svga${SO}"
						actdrivers=["$actdrivers svga"]
					],[
dnl					else
						AC_MSG_WARN([The svga driver needs the vga library])
					])
				],[
dnl				else
					AC_MSG_WARN([The svga driver needs vgagl.h])
				])
			],[
dnl			else
				AC_MSG_WARN([The svga driver needs vga.h])
			])
			;;
		t6963)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS t6963${SO}"
				actdrivers=["$actdrivers t6963"]
			else
				AC_MSG_WARN([The sed1330 driver needs a parallel port.])
			fi
			;;
		text)
			DRIVERS="$DRIVERS text${SO}"
			actdrivers=["$actdrivers text"]
			;;
		tyan)
			DRIVERS="$DRIVERS tyan${SO}"
			actdrivers=["$actdrivers tyan"]
			;;
		ula200)
			if test "$enable_libftdi" = yes ; then
				DRIVERS="$DRIVERS ula200${SO}"
				actdrivers=["$actdrivers ula200"]
			else
				AC_MSG_WARN([The ula200 driver needs the ftdi library])
			fi
			;;
		vlsys_m428)
			DRIVERS="$DRIVERS vlsys_m428${SO}"
			actdrivers=["$actdrivers vlsys_m428"]
			;;
		xosd)
			AC_PATH_PROG([LIBXOSD_CONFIG], [xosd-config], [no])
			if test "$LIBXOSD_CONFIG" = "no"; then
				AC_MSG_WARN([The xosd driver needs the xosd library])
			else
				LIBXOSD_CFLAGS=`$LIBXOSD_CONFIG --cflags`
				LIBXOSD_LIBS=`$LIBXOSD_CONFIG --libs`
				AC_SUBST(LIBXOSD_CFLAGS)
				AC_SUBST(LIBXOSD_LIBS)
				DRIVERS="$DRIVERS xosd${SO}"
				actdrivers=["$actdrivers xosd"]
			fi
			;;
		*)
			AC_MSG_ERROR([Unknown driver $driver])
			;;
	esac
done

AC_MSG_RESULT([---------------------------------------])
AC_MSG_RESULT([LCDd will be compiled with the drivers:])
for driver in $actdrivers; do
	AC_MSG_RESULT([    -  $driver])
done
AC_MSG_RESULT([---------------------------------------])

AC_SUBST(LIBCURSES)
AC_SUBST(LIBIRMAN)
AC_SUBST(LIBLIRC_CLIENT)
AC_SUBST(LIBSVGA)
AC_SUBST(DRIVERS)
AC_SUBST(HD44780_DRIVERS)
AC_SUBST(GLCD_DRIVERS)
AC_SUBST(LIBG15)
AC_SUBST(LIBGLCD)
AC_SUBST(LIBFTDI)
AC_SUBST(LIBXOSD)
AC_SUBST(LIBPTHREAD_LIBS)
AC_SUBST(LIBMX5000)
])


dnl
dnl Curses test to check if we use _acs_char* or acs_map*
dnl
AC_DEFUN([AC_CURSES_ACS_ARRAY], [
	AC_CACHE_CHECK([for acs_map in curses.h], ac_cv_curses_acs_map,
	[AC_TRY_COMPILE([#include <curses.h>], [ char map = acs_map['p'] ], ac_cv_curses_acs_map=yes, ac_cv_curses_acs_map=no)])

	if test "$ac_cv_curses_acs_map" = yes
	then
		AC_DEFINE(CURSES_HAS_ACS_MAP,[1],[Define to 1 if <curses.h> defines the acs_map array])
	else

		AC_CACHE_CHECK([for _acs_char in curses.h], ac_cv_curses__acs_char,
		[AC_TRY_COMPILE([#include <curses.h>], [ char map = _acs_char['p'] ], ac_cv_curses__acs_char=yes, ac_cv_curses__acs_char=no)])

		if test "$ac_cv_curses__acs_char" = yes
		then
			AC_DEFINE(CURSES_HAS__ACS_CHAR,[1],[Define to 1 if <curses.h> defines the _acs_char array])
		fi

	fi
])


dnl
dnl Find out where is the mounted filesystem table
dnl
AC_DEFUN([AC_FIND_MTAB_FILE], [
	AC_CACHE_CHECK([for your mounted filesystem table], ac_cv_mtab_file, [
		dnl Linux
		if test -f "/etc/mtab"; then
			ac_cv_mtab_file=/etc/mtab
		else
			dnl Solaris
			if test -f "/etc/mnttab"; then
				ac_cv_mtab_file=/etc/mnttab
			else
				dnl BSD
				if test -f "/etc/fstab"; then
					ac_cv_mtab_file=/etc/fstab
				fi
			fi
		fi
	])
	if test ! -z "$ac_cv_mtab_file"; then
		AC_DEFINE_UNQUOTED([MTAB_FILE], ["$ac_cv_mtab_file"], [Location of your mounted filesystem table file])
	fi
])


dnl
dnl Filesystem information detection
dnl
dnl To get information about the disk, mount points, etc.
dnl
dnl This code is stolen from mc-4.5.41, which in turn has stolen it
dnl from GNU fileutils-3.12.
dnl
AC_DEFUN([AC_GET_FS_INFO], [
    AC_CHECK_HEADERS(fcntl.h sys/dustat.h sys/param.h sys/statfs.h sys/fstyp.h)
    AC_CHECK_HEADERS(mnttab.h mntent.h utime.h sys/statvfs.h sys/vfs.h)
    AC_CHECK_HEADERS(sys/filsys.h sys/fs_types.h)
    AC_CHECK_HEADERS(sys/mount.h, [], [],
        [#if HAVE_SYS_PARAM_H
         #include <sys/param.h>
        #endif
        ])
    AC_CHECK_FUNCS(getmntinfo)

    AC_MSG_NOTICE([checking how to get filesystem space usage...])
    space=no

    # Here we'll compromise a little (and perform only the link test)
    # since it seems there are no variants of the statvfs function.
    if test $space = no; then
      # SVR4
      AC_CHECK_FUNCS(statvfs)
      if test $ac_cv_func_statvfs = yes; then
        space=yes
        AC_DEFINE(STAT_STATVFS,[1],[Define to 1 if you have the statvfs function])
      fi
    fi

    if test $space = no; then
      # DEC Alpha running OSF/1
      AC_MSG_CHECKING([for 3-argument statfs function (DEC OSF/1)])
      AC_CACHE_VAL(fu_cv_sys_stat_statfs3_osf1,
      [AC_TRY_RUN([
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mount.h>
      main ()
      {
        struct statfs fsd;
        fsd.f_fsize = 0;
        exit (statfs (".", &fsd, sizeof (struct statfs)));
      }],
      fu_cv_sys_stat_statfs3_osf1=yes,
      fu_cv_sys_stat_statfs3_osf1=no,
      fu_cv_sys_stat_statfs3_osf1=no)])
      AC_MSG_RESULT($fu_cv_sys_stat_statfs3_osf1)
      if test $fu_cv_sys_stat_statfs3_osf1 = yes; then
        space=yes
        AC_DEFINE(STAT_STATFS3_OSF1,[1],[Define to 1 if you have the 3-argument statfs function (DEC OSF/1)])
      fi
    fi

    if test $space = no; then
    # AIX
      AC_MSG_CHECKING([for two-argument statfs with statfs.bsize member (AIX, 4.3BSD)])
      AC_CACHE_VAL(fu_cv_sys_stat_statfs2_bsize,
      [AC_TRY_RUN([
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
      main ()
      {
      struct statfs fsd;
      fsd.f_bsize = 0;
      exit (statfs (".", &fsd));
      }],
      fu_cv_sys_stat_statfs2_bsize=yes,
      fu_cv_sys_stat_statfs2_bsize=no,
      fu_cv_sys_stat_statfs2_bsize=no)])
      AC_MSG_RESULT($fu_cv_sys_stat_statfs2_bsize)
      if test $fu_cv_sys_stat_statfs2_bsize = yes; then
        space=yes
        AC_DEFINE(STAT_STATFS2_BSIZE,[1],[Define to 1 if you have the two-argument statfs function with statfs.bsize member (AIX, 4.3BSD)])
      fi
    fi

    if test $space = no; then
    # SVR3
      AC_MSG_CHECKING([for four-argument statfs (AIX-3.2.5, SVR3)])
      AC_CACHE_VAL(fu_cv_sys_stat_statfs4,
      [AC_TRY_RUN([#include <sys/types.h>
#include <sys/statfs.h>
      main ()
      {
      struct statfs fsd;
      exit (statfs (".", &fsd, sizeof fsd, 0));
      }],
        fu_cv_sys_stat_statfs4=yes,
        fu_cv_sys_stat_statfs4=no,
        fu_cv_sys_stat_statfs4=no)])
      AC_MSG_RESULT($fu_cv_sys_stat_statfs4)
      if test $fu_cv_sys_stat_statfs4 = yes; then
        space=yes
        AC_DEFINE(STAT_STATFS4,[1],[Define to 1 if you have the four-argument statfs function (AIX-3.2.5, SVR3)])
      fi
    fi
    if test $space = no; then
    # 4.4BSD and NetBSD
      AC_MSG_CHECKING([for two-argument statfs with statfs.fsize dnl
    member (4.4BSD and NetBSD)])
      AC_CACHE_VAL(fu_cv_sys_stat_statfs2_fsize,
      [AC_TRY_RUN([#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
      main ()
      {
      struct statfs fsd;
      fsd.f_fsize = 0;
      exit (statfs (".", &fsd));
      }],
      fu_cv_sys_stat_statfs2_fsize=yes,
      fu_cv_sys_stat_statfs2_fsize=no,
      fu_cv_sys_stat_statfs2_fsize=no)])
      AC_MSG_RESULT($fu_cv_sys_stat_statfs2_fsize)
      if test $fu_cv_sys_stat_statfs2_fsize = yes; then
        space=yes
        AC_DEFINE(STAT_STATFS2_FSIZE,[1],[Define to 1 if you have the two-argument statfs function with statfs.fsize member (4.4BSD and NetBSD)])
      fi
    fi

    if test $space = no; then
      # Ultrix
      AC_MSG_CHECKING([for two-argument statfs with struct fs_data (Ultrix)])
      AC_CACHE_VAL(fu_cv_sys_stat_fs_data,
      [AC_TRY_RUN([
#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_FS_TYPES_H
#include <sys/fs_types.h>
#endif
      main ()
      {
      struct fs_data fsd;
      /* Ultrix's statfs returns 1 for success,
         0 for not mounted, -1 for failure.  */
      exit (statfs (".", &fsd) != 1);
      }],
      fu_cv_sys_stat_fs_data=yes,
      fu_cv_sys_stat_fs_data=no,
      fu_cv_sys_stat_fs_data=no)])
      AC_MSG_RESULT($fu_cv_sys_stat_fs_data)
      if test $fu_cv_sys_stat_fs_data = yes; then
        space=yes
        AC_DEFINE(STAT_STATFS2_FS_DATA,[1],[Define to 1 if you have the two-argument statfs function with struct fs_data (Ultrix)])
      fi
    fi

    dnl Not supported
    dnl if test $space = no; then
    dnl # SVR2
    dnl AC_TRY_CPP([#include <sys/filsys.h>],
    dnl   AC_DEFINE(STAT_READ_FILSYS) space=yes)
    dnl fi
])


dnl 1.1 (2001/07/26) -- Miscellaneous @ ac-archive-0.5.32
dnl Warren Young <warren@etr-usa.com>
dnl This macro checks for the SysV IPC header files. It only checks
dnl that you can compile a program with them, not whether the system
dnl actually implements working SysV IPC.
dnl http://ac-archive.sourceforge.net/Miscellaneous/etr_sysv_ipc.html
AC_DEFUN([ETR_SYSV_IPC],
[
AC_CACHE_CHECK([for System V IPC headers], ac_cv_sysv_ipc, [
        AC_TRY_COMPILE(
                [
                        #include <sys/types.h>
                        #include <sys/ipc.h>
                        #include <sys/msg.h>
                        #include <sys/sem.h>
                        #include <sys/shm.h>
                ],, ac_cv_sysv_ipc=yes, ac_cv_sysv_ipc=no)
])

        if test x"$ac_cv_sysv_ipc" = "xyes"
        then
                AC_DEFINE(HAVE_SYSV_IPC, 1, [ Define if you have System V IPC ])
        fi
]) dnl ETR_SYSV_IPC


dnl 1.1 (2001/07/26) -- Miscellaneous @ ac-archive-0.5.32
dnl Warren Young <warren@etr-usa.com>
dnl This macro checks to see if sys/sem.h defines union semun. Some
dnl systems do, some systems don't. Your code must be able to deal with
dnl this possibility; if HAVE_STRUCT_SEMUM isn't defined for a given system,
dnl you have to define this structure before you can call functions
dnl like semctl().
dnl You should call ETR_SYSV_IPC before this macro, to separate the check
dnl for System V IPC headers from the check for struct semun.
dnl http://ac-archive.sourceforge.net/Miscellaneous/etr_struct_semun.html
AC_DEFUN([ETR_UNION_SEMUN],
[
AC_CACHE_CHECK([for union semun], ac_cv_union_semun, [
        AC_TRY_COMPILE(
                [
                        #include <sys/types.h>
                        #include <sys/ipc.h>
                        #include <sys/sem.h>
                ],
                [ union semun s ],
                ac_cv_union_semun=yes,
                ac_cv_union_semun=no)
])

        if test x"$ac_cv_union_semun" = "xyes"
        then
                AC_DEFINE(HAVE_UNION_SEMUN, 1,
                        [ Define if your system's sys/sem.h file defines union semun ])
        fi
]) dnl ETR_UNION_SEMUN


dnl Loadable modules determination.
dnl Joris Robijn, 2002
dnl I choose not to use libtool.
dnl If your platform does not compile or link the modules correctly, add
dnl appropriate flags here.
AC_DEFUN([AC_MODULES_INFO],
[
dnl $SO is the extension of shared libraries (including the dot!)
dnl It is available in the program code as MODULE_EXTENSION
AC_MSG_CHECKING(module extension)
case $host in
  hp*|HP*)
	SO=.sl;;
  *cygwin*)
	SO=.dll;;
  *)
	SO=.so;;
esac
AC_DEFINE_UNQUOTED(MODULE_EXTENSION,"$SO",[Set to the extension for dynamically loaded modules])
AC_MSG_RESULT($SO)

dnl CCSHARED are the flags used to compile the sources for the shared library
dnl LDSHARED are the flags used to create shared library.

dnl By default assume a GNU compatible build system
CCSHARED="-fPIC"
LDSHARED="-shared"
dnl Now override flags that should be different
case "$host" in
  SunOS/4*)
  	LDSHARED=""
  	;;
  SunOS/5*)
	if test ! "$GCC" = "yes" ; then
		LDSHARED="-G";
	fi
	;;
  hp*|HP*)
	if test ! "$GCC" = "yes"; then
		CCSHARED="+z";
  		LDSHARED="-b"
	fi
  	;;
  *cygwin*)
  	CCSHARED="-DUSE_DL_IMPORT"
	LDSHARED="-shared -Wl,--enable-auto-image-base"
	;;
  BSD/OS*/4*)
	CCSHARED="-fpic"
	;;
  *darwin*)
      	LDSHARED="-bundle -flat_namespace -undefined suppress"
	;;
esac

dnl checks for libraries
AC_CHECK_LIB(dl, dlopen)	# Dynamic linking for Linux, *BSD, SunOS/Solaris and SYSV
AC_CHECK_LIB(dld, shl_load)	# Dynamic linking for HP-UX

AC_SUBST(SO)
AC_SUBST(CCSHARED)
AC_SUBST(LDSHARED)
dnl End of loadable modules determination
]) dnl AC_MODULES_INFO


dnl stolen from cppunit project (http://cppunit.sourceforge.net/)
AC_DEFUN([BB_ENABLE_DOXYGEN],
[
AC_ARG_ENABLE(doxygen,
	[AS_HELP_STRING([--enable-doxygen], [enable documentation generation with doxygen (auto)])])
AC_ARG_ENABLE(dot,
	[AS_HELP_STRING([--enable-dot], [use 'dot' to generate graphs in doxygen (auto)])])
AC_ARG_ENABLE(html-dox,
	[AS_HELP_STRING([--enable-html-dox], [enable HTML generation with doxygen (yes)])],
	[],
	[enable_html_dox=yes])
AC_ARG_ENABLE(latex-dox,
	[AS_HELP_STRING([--enable-latex-dox], [enable LaTeX documentation generation with doxygen (no)])],
	[],
	[enable_latex_dox=no])
if test "x$enable_doxygen" = xno; then
        enable_dox=no
else
        AC_PATH_PROG(DOXYGEN, doxygen, , $PATH)
        if test "x$DOXYGEN" = x; then
                if test "x$enable_doxygen" = xyes; then
                        AC_MSG_ERROR([could not find doxygen])
                fi
                enable_dox=no
        else
                enable_dox=yes
                AC_PATH_PROG(DOT, dot, , $PATH)
        fi
fi
AM_CONDITIONAL(DOX, test x$enable_dox = xyes)

if test x$DOT = x; then
        if test "x$enable_dot" = xyes; then
                AC_MSG_ERROR([could not find dot])
        fi
        enable_dot=no
else
        enable_dot=yes
fi
AC_SUBST(enable_dot)
AC_SUBST(enable_html_dox)
AC_SUBST(enable_latex_dox)
])


dnl Check if a given flag is understood and add it to CFLAGS.
dnl From http://www.gnu.org/software/autoconf-archive/ax_cflags_gcc_option.html
dnl Copyright (c) 2008 Guido U. Draheim <guidod@gmx.de>
dnl serial 12
dnl Note: This is the last version released under GPLv2.
AC_DEFUN([AX_CFLAGS_GCC_OPTION_OLD], [dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ax_cv_cflags_gcc_option_$2])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for gcc m4_ifval($2,$2,-option)],
VAR,[AS_VAR_SET([VAR],["no, unknown"])
 AC_LANG_SAVE
 AC_LANG_C
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic -Werror % m4_ifval($2,$2,-option)"  dnl   GCC
   "-pedantic % m4_ifval($2,$2,-option) %% no, obsolete"  dnl new GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [AS_VAR_SET([VAR],[`echo $ac_arg | sed -e 's,.*% *,,'`]); break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
m4_ifdef([AS_VAR_COPY],[AS_VAR_COPY([var],[VAR])],[var=AS_VAR_GET([VAR])])
case ".$var" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $var " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $var])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $var"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $var"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])


dnl the only difference - the LANG selection... and the default FLAGS

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION_OLD], [dnl
AS_VAR_PUSHDEF([FLAGS],[CXXFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ax_cv_cxxflags_gcc_option_$2])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for gcc m4_ifval($2,$2,-option)],
VAR,[AS_VAR_SET([VAR],["no, unknown"])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic -Werror % m4_ifval($2,$2,-option)"  dnl   GCC
   "-pedantic % m4_ifval($2,$2,-option) %% no, obsolete"  dnl new GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [AS_VAR_SET([VAR],[`echo $ac_arg | sed -e 's,.*% *,,'`]); break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
m4_ifdef([AS_VAR_COPY],[AS_VAR_COPY([var],[VAR])],[var=AS_VAR_GET([VAR])])
case ".$var" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $var " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $var])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $var"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $var"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])

dnl -------------------------------------------------------------------------

AC_DEFUN([AX_CFLAGS_GCC_OPTION_NEW], [dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ax_cv_cflags_gcc_option_$1])dnl
AC_CACHE_CHECK([m4_ifval($2,$2,FLAGS) for gcc m4_ifval($1,$1,-option)],
VAR,[AS_VAR_SET([VAR],["no, unknown"])
 AC_LANG_SAVE
 AC_LANG_C
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic -Werror % m4_ifval($1,$1,-option)"  dnl   GCC
   "-pedantic % m4_ifval($1,$1,-option) %% no, obsolete"  dnl new GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [AS_VAR_SET([VAR],[`echo $ac_arg | sed -e 's,.*% *,,'`]); break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
m4_ifdef([AS_VAR_COPY],[AS_VAR_COPY([var],[VAR])],[var=AS_VAR_GET([VAR])])
case ".$var" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($2,$2,FLAGS) " | grep " $var " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($2,$2,FLAGS) does contain $var])
   else AC_RUN_LOG([: m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $var"])
                      m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $var"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])


dnl the only difference - the LANG selection... and the default FLAGS

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION_NEW], [dnl
AS_VAR_PUSHDEF([FLAGS],[CXXFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ax_cv_cxxflags_gcc_option_$1])dnl
AC_CACHE_CHECK([m4_ifval($2,$2,FLAGS) for gcc m4_ifval($1,$1,-option)],
VAR,[AS_VAR_SET([VAR],["no, unknown"])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic -Werror % m4_ifval($1,$1,-option)"  dnl   GCC
   "-pedantic % m4_ifval($1,$1,-option) %% no, obsolete"  dnl new GCC
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [AS_VAR_SET([VAR],[`echo $ac_arg | sed -e 's,.*% *,,'`]); break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
m4_ifdef([AS_VAR_COPY],[AS_VAR_COPY([var],[VAR])],[var=AS_VAR_GET([VAR])])
case ".$var" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($2,$2,FLAGS) " | grep " $var " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($2,$2,FLAGS) does contain $var])
   else AC_RUN_LOG([: m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $var"])
                      m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) $var"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])

AC_DEFUN([AX_CFLAGS_GCC_OPTION],[ifelse(m4_bregexp([$2],[-]),-1,
[AX_CFLAGS_GCC_OPTION_NEW($@)],[AX_CFLAGS_GCC_OPTION_OLD($@)])])

AC_DEFUN([AX_CXXFLAGS_GCC_OPTION],[ifelse(m4_bregexp([$2],[-]),-1,
[AX_CXXFLAGS_GCC_OPTION_NEW($@)],[AX_CXXFLAGS_GCC_OPTION_OLD($@)])])

dnl
dnl Check if system has SA_RESTART defined. Copied from GNUs make configure.
dnl
AC_DEFUN([LCD_SA_RESTART], [
AC_CACHE_CHECK([for SA_RESTART], lcd_cv_sa_restart, [
  AC_TRY_COMPILE([#include <signal.h>],
      [return SA_RESTART;],
      lcd_cv_sa_restart=yes,
      lcd_cv_sa_restart=no)])
if test "$lcd_cv_sa_restart" != no; then
  AC_DEFINE([HAVE_SA_RESTART], [1],
     [Define to 1 if <signal.h> defines the SA_RESTART constant.])
fi
])

dnl
dnl Check for PNG library
dnl
AC_DEFUN([LCD_PNG_LIB], [
AC_MSG_CHECKING([if PNG support has been enabled]);
AC_ARG_ENABLE(libpng,
	[AS_HELP_STRING([--disable-libpng],[disable PNG support using libpng])],
	[ if test "$enableval" != "no"; then
		enable_libpng=yes
	fi ],
	[ enable_libpng=yes ]
)
AC_MSG_RESULT($enable_libpng)

if test "$enable_libpng" = "yes"; then
	AC_PATH_PROG([_png_config], [libpng-config])
	_libpng_save_libs=$LIBS
	_libpng_save_cflags=$CFLAGS

	if test x$_png_config != "x" ; then
		_libpng_try_libs=`$_png_config --ldflags`
		_libpng_try_cflags=`$_png_config --cflags`
	fi

	LIBS="$LIBS $_libpng_try_libs"
	CFLAGS="$CFLAGS $_libpng_try_cflags"

	AC_MSG_CHECKING([whether libpng is present and sane])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <png.h>],[
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		])],enable_libpng=yes,enable_libpng=no)
	AC_MSG_RESULT([$enable_libpng])

	if test "$enable_libpng" = "yes" ; then
		AC_DEFINE(HAVE_LIBPNG, [1], [Define to 1 if you have libpng])
		AC_SUBST(LIBPNG_CFLAGS, $_libpng_try_cflags)
		AC_SUBST(LIBPNG_LIBS, $_libpng_try_libs)
	fi

	LIBS=$_libpng_save_libs
	CFLAGS=$_libpng_save_cflags

	unset _libpng_save_libs
	unset _libpng_save_cflags
	unset _libpng_try_libs
	unset _libpng_try_cflags
	unset _png_config
fi
])
