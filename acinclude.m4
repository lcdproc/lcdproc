AC_DEFUN(LCD_DRIVERS_SELECT, [
AC_CHECKING(for which drivers to compile)

AC_ARG_ENABLE(drivers,
  	[  --enable-drivers=<list> compile driver for LCDs in <list>.]
	[                  drivers may be separated with commas.]
  	[                  Possible choices are:]
 	[                    mtxorb,cfontz,cfontz633,curses,cwlnx,text,lb216,]
 	[                    hd44780,joy,irman,lirc,bayrad,glk,mtc_s16209x]
 	[                    stv5730,sed1330,sed1520,svga,lcdm001,t6963]
	[                    lcterm,icp_a106,ms6931,iowarrior,glcdlib,imon,xosd,]
	[                    noritakevfd,tyan]
	[                  \"all\" compiles all drivers],
  	drivers="$enableval",
  	drivers=[lcdm001,mtxorb,cfontz,cfontz633,curses,cwlnx,text,lb216,bayrad,glk])

if test "$drivers" = "all"; then
	
drivers=[mtxorb,cfontz,cfontz633,curses,cwlnx,text,lb216,mtc_s16209x,hd44780,joy,irman,lirc,bayrad,glk,stv5730,sed1330,sed1520,svga,lcdm001,t6963,lcterm,icp_a106,ms6931,iowarrior,glcdlib,imon,xosd,noritakevfd,tyan]
fi

  	drivers=`echo $drivers | sed 's/,/ /g'`
  	for driver in $drivers
  	do

	case "$driver" in
		lcdm001)
			DRIVERS="$DRIVERS lcdm001${SO}"
			actdrivers=["$actdrivers lcdm001"]
			;;
		mtxorb)
			DRIVERS="$DRIVERS MtxOrb${SO}"
			actdrivers=["$actdrivers mtxorb"]
			;;
		glk)
			DRIVERS="$DRIVERS glk${SO}"
			actdrivers=["$actdrivers glk"]
			;;
		bayrad)
			DRIVERS="$DRIVERS bayrad${SO}"
			actdrivers=["$actdrivers bayrad"]
			;;
		cfontz)
			DRIVERS="$DRIVERS CFontz${SO}"
			actdrivers=["$actdrivers cfontz"]
			;;
		cfontz633)
			DRIVERS="$DRIVERS CFontz633${SO}"
			actdrivers=["$actdrivers cfontz633"]
			AC_CHECK_FUNCS(select, [
				AC_CHECK_HEADERS(sys/select.h)
			],[
				AC_MSG_WARN([The CFontz633 driver needs the select() function])
			])	
			;;
		noritakevfd)
			DRIVERS="$DRIVERS NoritakeVFD${SO}"
			actdrivers=["$actdrivers noritakevfd"]
			;;
		lcterm)
			DRIVERS="$DRIVERS lcterm${SO}"
			actdrivers=["$actdrivers lcterm"]
			;;
		icp_a106)
			DRIVERS="$DRIVERS icp_a106${SO}"
			actdrivers=["$actdrivers icp_a106"]
			;;
		ms6931)
			DRIVERS="$DRIVERS ms6931${SO}"
			actdrivers=["$actdrivers ms6931"]
			;;
		sli)
			DRIVERS="$DRIVERS wirz-sli${SO}"
			actdrivers=["$actdrivers sli"]
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
		cwlnx)
			DRIVERS="$DRIVERS CwLnx${SO}"
			actdrivers=["$actdrivers cwlnx"]
			;;
		text)
			DRIVERS="$DRIVERS text${SO}"
			actdrivers=["$actdrivers text"]
			;;
		lb216)
			DRIVERS="$DRIVERS lb216${SO}"
			actdrivers=["$actdrivers lb216"]
			;;
		mtc_s16209x)
			DRIVERS="$DRIVERS mtc_s16209x${SO}"
			actdrivers=["$actdrivers mtc_s16209x"]
			;;
		hd44780)
			if test "$ac_cv_port_have_lpt" = yes ; then
				HD44780_DRIVERS="hd44780-4bit.o hd44780-ext8bit.o lcd_sem.o hd44780-winamp.o hd44780-serialLpt.o hd44780-picanlcd.o hd44780-lcdserializer.o hd44780-lis2.o"
			else
				HD44780_DRIVERS="hd44780-picanlcd.o hd44780-lcdserializer.o hd44780-lis2.o"
			fi
			if test "$enable_libusb" = yes ; then
				HD44780_DRIVERS="$HD44780_DRIVERS hd44780-bwct-usb.o"
			fi
			DRIVERS="$DRIVERS hd44780${SO}"
			actdrivers=["$actdrivers hd44780"]
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
		iowarrior)
 			if test "$enable_libusb" = yes ; then
				DRIVERS="$DRIVERS IOWarrior${SO}"
				actdrivers=["$actdrivers iowarrior"]
			else
 				AC_MSG_WARN([The iowarrior driver needs the libusb library.])
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
		stv5730)
			if test "$ac_cv_port_have_lpt" = yes
			then
				DRIVERS="$DRIVERS stv5730${SO}"
				actdrivers=["$actdrivers stv5730"]
			else
				AC_MSG_WARN([The stv5730 driver needs a parallel port.])
			fi
			;;
		svga)
			AC_CHECK_HEADERS([vga.h vgagl.h],[
				AC_CHECK_LIB(vga, main,[
					LIBSVGA="-lvga -lvgagl"
					DRIVERS="$DRIVERS svga${SO}"
					actdrivers=["$actdrivers svga"]
 				],[
dnl				else
					AC_MSG_WARN([The svga driver needs the vga library])
				])
			],[
dnl			else
				AC_MSG_WARN([The svga driver needs vga.h and vgagl.h])
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
		imon)
			DRIVERS="$DRIVERS imon${SO}"
			actdrivers=["$actdrivers imon"]
			;;			
		xosd)
			AC_CHECK_HEADERS([xosd.h],[
				AC_CHECK_LIB(xosd, main,[
					LIBXOSD=`xosd-config --libs`
					DRIVERS="$DRIVERS xosd${SO}"
					actdrivers=["$actdrivers xosd"]
 				],[
dnl				else
					AC_MSG_WARN([The xosd driver needs the xosd library])
				])
			],[
dnl			else
				AC_MSG_WARN([The xosd driver needs xosd.h])
			])
			;;
		tyan)
			DRIVERS="$DRIVERS tyan${SO}"
			actdrivers=["$actdrivers tyan"]
			;;
		*)
			AC_MSG_ERROR([Unknown driver $driver])
			;;
  		esac
  	done

actdrivers=`echo $actdrivers | sed 's/ /,/g'`
AC_MSG_RESULT([Will compile drivers: $actdrivers])

AC_SUBST(LIBCURSES)
AC_SUBST(LIBIRMAN)
AC_SUBST(LIBLIRC_CLIENT)
AC_SUBST(LIBSVGA)
AC_SUBST(DRIVERS)
AC_SUBST(HD44780_DRIVERS)
AC_SUBST(LIBGLCD)
AC_SUBST(LIBXOSD)
])


dnl
dnl Curses test to check if we use _acs_char* or acs_map*
dnl
AC_DEFUN(AC_CURSES_ACS_ARRAY, [
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

AC_DEFUN(AC_GET_FS_INFO, [
    AC_CHECK_HEADERS(fcntl.h sys/dustat.h sys/param.h sys/statfs.h sys/fstyp.h)
    AC_CHECK_HEADERS(mnttab.h mntent.h utime.h sys/statvfs.h sys/vfs.h)
    AC_CHECK_HEADERS(sys/mount.h sys/filsys.h sys/fs_types.h)
    AC_CHECK_FUNCS(getmntinfo)

    AC_CHECKING(how to get filesystem space usage)
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
	fi
  	LDSHARED="-b"
  	;;
  *cygwin*)
  	CCSHARED="-DUSE_DL_IMPORT"
	LDSHARED="-shared -Wl,--enable-auto-image-base"
	;;
  BSD/OS*/4*)
	CCSHARED="-fpic"
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
AC_DEFUN(BB_ENABLE_DOXYGEN,
[
AC_ARG_ENABLE(doxygen, [  --enable-doxygen        enable documentation generation with doxygen (auto)])
AC_ARG_ENABLE(dot, [  --enable-dot            use 'dot' to generate graphs in doxygen (auto)])              
AC_ARG_ENABLE(html-dox, [  --enable-html-dox      enable HTML generation with doxygen (yes)], [], [ enable_html_dox=yes])              
AC_ARG_ENABLE(latex-dox, [  --enable-latex-dox     enable LaTeX documentation generation with doxygen (no)], [], [ enable_latex_dox=no])              
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
