# -*- mode: m4; -*-
dnl Process this file with autoconf 2.61 (or higher) to produce a configure script.
AC_PREREQ(2.61)
AC_INIT([lcdproc], [0.5dev], [lcdproc@lists.omnipotent.net])
AC_CONFIG_SRCDIR([clients/lcdproc/batt.c])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS(config.h)

AC_CANONICAL_HOST
case "$host" in
*-*-*linux*)      dnl i586-pc-linux-gnu
	AC_DEFINE([LINUX],[1],[Define if you're using Linux.])
	ac_system_host=Linux
	;;
*-*-*solaris*)
	AC_DEFINE([SOLARIS],[1],[Define if you're using Solaris.])
	ac_system_host=Solaris
	;;
*-*-*openbsd*)    dnl i386-unknown-openbsd3.0
	AC_DEFINE([OPENBSD],[1],[Define if you're using OpenBSD.])
	ac_system_host=OpenBSD
	;;
*-*-*netbsd*)
	AC_DEFINE([NETBSD],[1],[Define if you're using NetBSD.])
	ac_system_host=NetBSD
	;;
*-*-*freebsd*)
	AC_DEFINE([FREEBSD],[1],[Define if you're using FreeBSD.])
	ac_system_host=FreeBSD
	;;
*-*-*darwin*)
	AC_DEFINE([DARWIN],[1],[Define if you're using Darwin/Mac OS X.])
	ac_system_host=Darwin
	;;
esac
AC_DEFINE_UNQUOTED([SYSTEM_HOST], [$ac_system_host], [Set this to your system host (Linux, Solaris, OpenBSD, NetBSD, FreeBSD or Darwin)])

dnl treat Darwin special in Makefiles
AM_CONDITIONAL(DARWIN, test x$ac_system_host = xDarwin)

AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug], [show debug information])],
  [ if test "$enableval" != "no"; then
        AC_DEFINE(DEBUG, [1], [Define to 1 to show debug information])
        debug="yes"
    else
        debug="no"
    fi ],
  [ debug=no ]
)
AC_MSG_RESULT($debug)

if test $debug = "yes"; then
dnl Enable debugging information with minimal optimisation if not set differently
dnl (the spaces before $CFLAGS and -O are significant)
	CFLAGS="$CFLAGS -g"
	if ! echo " $CFLAGS" | grep -q -- " -O" ; then
		CFLAGS="$CFLAGS -O"
	fi
else
dnl Maximum optimisation if not already set
dnl (the spaces before $CFLAGS and -O are significant)
	if ! echo " $CFLAGS" | grep -q -- " -O" ; then
		CFLAGS="$CFLAGS -O3"
	fi
fi

dnl Checks for programs used in building
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_PATH_PROG([XMLTO], [xmlto], [no])

dnl Avoid unused static function warnings
CFLAGS="-Wall $CFLAGS"
AX_CFLAGS_GCC_OPTION(-Wno-unused-function)
AX_CFLAGS_GCC_OPTION(-ftrampolines)
export CFLAGS

dnl Solaris
AC_CHECK_FUNC(gethostbyname,,[AC_CHECK_LIB(nsl,gethostbyname)])
AC_CHECK_FUNC(connect,,[AC_CHECK_LIB(socket,connect)])
AC_CHECK_FUNC(inet_aton,,[AC_CHECK_LIB(resolv,inet_aton)])
AC_CHECK_LIB(kstat, kstat_open)
AC_CHECK_LIB(posix4, nanosleep)
AC_CHECK_FUNCS(getloadavg swapctl)
AC_CHECK_HEADERS(procfs.h sys/procfs.h sys/loadavg.h utmpx.h)

dnl Some versions of Solaris require -lelf for -lkvm
AC_CHECK_LIB(kvm, kvm_open,[
  LIBS="-lkvm $LIBS"
],[AC_MSG_CHECKING([for kvm_open in -lkvm with -lelf])
   AC_CACHE_VAL(ac_cv_lib_kvm_with_elf,
   [ac_save_LIBS="$LIBS"
    LIBS="-lkvm -lelf $LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[char kvm_open();]], [[kvm_open()]])],[ac_cv_lib_kvm_with_elf=yes],[ac_cv_lib_kvm_with_elf=no])
    LIBS="$ac_save_LIBS"
   ])
   if test "$ac_cv_lib_kvm_with_elf" = "yes"; then
     AC_MSG_RESULT(yes)
     LIBS="-lkvm -lelf $LIBS"
   else
     AC_MSG_RESULT(no)
   fi
])

dnl NetBSD, OpenBSD and FreeBSD
AC_CHECK_HEADERS(sched.h sys/types.h machine/pio.h machine/sysarch.h sys/cpuvar.h machine/apm_bios.h)
ETR_SYSV_IPC
ETR_UNION_SEMUN

dnl machine/cpufunc.h needs additional header on FreeBSD
AC_CHECK_HEADERS([machine/cpufunc.h], [], [],
[[#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
]])

dnl sched_setscheduler on OpenBSD
AC_CHECK_FUNCS(sched_setscheduler)
AC_CHECK_LIB(posix4, sched_setscheduler, [
	AC_DEFINE([HAVE_SCHED_SETSCHEDULER], [1],
		[Define if you have the sched_setscheduler function.])
])
AC_CHECK_LIB(rt, sched_setscheduler, [
	AC_DEFINE([HAVE_SCHED_SETSCHEDULER], [1],
		[Define if you have the sched_setscheduler function.])
])

dnl i386_get_ioperm on NetBSD&OpenBSD
AC_CHECK_LIB(i386, i386_get_ioperm,
	LIBS="-li386 $LIBS"
	AC_DEFINE([HAVE_I386_IOPERM_NETBSD], [1],
		[Define if you have the NetBSD&OpenBSD version of i386_ioperm functions.]),[
        dnl i386_get_ioperm on FreeBSD
        AC_CHECK_LIB(c, i386_get_ioperm,
		AC_DEFINE([HAVE_I386_IOPERM_FREEBSD],[1],
			[Define if you have the FreeBSD version of the i386_ioperm functions.]),
		[])]
)

AC_CHECK_FUNCS(iopl)
AC_CHECK_FUNCS(ioperm)
AC_CHECK_HEADERS(sys/io.h)

dnl Check if we support this parallel (LPT) port
dnl IMPORTANT: we must do all the checks used in port.h before doing this test!
AC_CACHE_CHECK([for a parallel port], ac_cv_port_have_lpt,
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include "${srcdir}/server/drivers/port.h"]],
					    [[char val = port_in(0x350)]])],
		[ac_cv_port_have_lpt=yes],[ac_cv_port_have_lpt=no])])

if test "$ac_cv_port_have_lpt" = yes; then
	AC_DEFINE([HAVE_PCSTYLE_LPT_CONTROL], [1],
		[Define if you have a parallel port and LCDproc knows how to talk to it.])
else
	AC_MSG_WARN([Cannot talk to the parallel port, disabling drivers that use it.])
fi

x_ac_have_i2c=no
AC_CHECK_HEADERS([linux/i2c-dev.h dev/iicbus/iic.h], [x_ac_have_i2c=yes])
if test "$x_ac_have_i2c" = yes; then
	AC_DEFINE(HAVE_I2C,[1],[Define to 1 if you have the i2c headers])
fi

x_ac_have_spi=no
AC_CHECK_HEADERS([linux/spi/spidev.h], [x_ac_have_spi=yes])
if test "$x_ac_have_spi" = yes; then
	AC_DEFINE(HAVE_SPI,[1],[Define to 1 if you have the spi headers])
fi

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/time.h unistd.h sys/io.h errno.h)
AC_CHECK_HEADERS(limits.h kvm.h sys/param.h sys/dkstat.h stdbool.h)

dnl check sys/sysctl.h seperately, as it requires other headers on at least OpenBSD
AC_CHECK_HEADERS([sys/sysctl.h], [], [],
[[#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
]])

dnl On FreeBSD >= 8 checking sys/pcpu.h fails without sys/param.h and kvm.h
AC_CHECK_HEADERS([sys/pcpu.h], [], [],
[[#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#if HAVE_KVM_H
# include <kvm.h>
#endif
]])


dnl Check for particular preprocessor macros
LCD_SA_RESTART

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(select socket strdup strerror strtol uname cfmakeraw snprintf)

dnl Many people on non-GNU/Linux systems don't have getopt
AC_CONFIG_LIBOBJ_DIR(shared)
AC_CHECK_FUNC(getopt,
  [
    AC_ARG_WITH(included-getopt,
      [AS_HELP_STRING([--with-included-getopt], [Use the included getopt rather than glibc's])],
      with_getopt=$withval,
      with_getopt=$no)
    if test "x$with_getopt" = xyes; then
      AC_LIBOBJ(getopt)
      AC_LIBOBJ(getopt1)
    fi
  ],
  [
    AC_LIBOBJ(getopt)
    AC_LIBOBJ(getopt1)
  ])


dnl Check how to find the mtab file and how to get filesystem staticstics
AC_FIND_MTAB_FILE
AC_GET_FS_INFO


dnl Check for various defines and features
AC_ARG_WITH(lcdport,
	[AS_HELP_STRING([--with-lcdport=<port>], [the server port [13666]])],
	LCDPORT=$withval,
	LCDPORT="13666"
)
AC_DEFINE_UNQUOTED(LCDPORT, $LCDPORT, [Set default port where LCDd should listen])

AC_DEFINE_UNQUOTED(PROTOCOL_VERSION, "0.3", [Define version of lcdproc client-server protocol])

AC_DEFINE_UNQUOTED(API_VERSION, "0.5", [Define version of lcdproc API])


dnl Check compiler flags to dynamically load modules
AC_MODULES_INFO

dnl ######################################################################
dnl libusb support
dnl ######################################################################
AC_MSG_CHECKING([if libusb support has been enabled]);
AC_ARG_ENABLE(libusb,
	[AS_HELP_STRING([--disable-libusb],[disable USB support using libusb])],
	[ if test "$enableval" != "no"; then
		enable_libusb=yes
	fi ],
	[ enable_libusb=yes ]
)
AC_MSG_RESULT($enable_libusb)

if test "$enable_libusb" = "yes"; then
	ifdef([PKG_CHECK_MODULES],
		[PKG_CHECK_MODULES(LIBUSB, libusb,
			[AC_DEFINE(HAVE_LIBUSB, [1], [Define to 1 if you have libusb])],
			[ enable_libusb=no ])],
		[AC_MSG_WARN([pkg-config not (fully) installed; drivers requiring libusb may not be built])])
fi
AC_SUBST(LIBUSB_LIBS)
AC_SUBST(LIBUSB_CFLAGS)

dnl ######################################################################
dnl libusb-1.0 support
dnl ######################################################################
AC_MSG_CHECKING([if libusb-1-0 support has been enabled]);
AC_ARG_ENABLE(libusb_1_0,
	[AS_HELP_STRING([--disable-libusb-1-0],[disable USB support using libusb-1.0])],
	[ if test "$enableval" != "no"; then
		enable_libusb_1_0="yes"
	fi ],
	[ enable_libusb_1_0=yes ]
)
AC_MSG_RESULT($enable_libusb_1_0)

if test "$enable_libusb_1_0" = "yes"; then
	ifdef([PKG_CHECK_MODULES],
		[PKG_CHECK_MODULES(LIBUSB_1_0, libusb-1.0 >= 1.0,
			[AC_DEFINE(HAVE_LIBUSB_1_0, [1], [Define to 1 if you have libusb-1.0])],
			[ enable_libusb_1_0=no ])],
		[AC_MSG_WARN([pkg-config not (fully) installed; drivers requiring libusb-1.0 may not be built])])
fi
AC_SUBST(LIBUSB_1_0_LIBS)
AC_SUBST(LIBUSB_1_0_CFLAGS)

dnl ######################################################################
dnl libftdi support
dnl ######################################################################
AC_MSG_CHECKING([if libftdi support has been enabled]);
AC_ARG_ENABLE(libftdi,
	[AS_HELP_STRING([--disable-libftdi],[disable FTDI support using libftdi])],
	[ if test "$enableval" != "no"; then
		enable_libftdi=yes
	fi ],
	[ enable_libftdi=yes ]
)
AC_MSG_RESULT($enable_libftdi)

if test "$enable_libftdi" = "yes"; then
	ifdef([PKG_CHECK_MODULES],
	 	[PKG_CHECK_MODULES(LIBFTDI, libftdi >= 0.8,
			[AC_DEFINE(HAVE_LIBFTDI, [1], [Define to 1 if you have libftdi])],
			[ enable_libftdi=no ])],
		[AC_MSG_WARN([pkg-config not (fully) installed; drivers requiring libftdi may not be built])])
fi
AC_SUBST(LIBFTDI_LIBS)
AC_SUBST(LIBFTDI_CFLAGS)

dnl ######################################################################
dnl X11 library support
dnl ######################################################################
AC_MSG_CHECKING([if X11 support has been enabled]);
AC_ARG_ENABLE(libX11,
	[AS_HELP_STRING([--disable-libX11],[disable X11 support])],
	[ if test "$enableval" != "no"; then
		enable_libX11=yes
	fi ],
	[ enable_libX11=yes ]
)
AC_MSG_RESULT($enable_libX11)

if test "$enable_libX11" = "yes"; then
	ifdef([PKG_CHECK_MODULES],
	 	[PKG_CHECK_MODULES([LIBX11], [x11],
			[AC_DEFINE(HAVE_LIBX11, [1], [Define to 1 if you have X11 library])],
			[ enable_libX11=no ])],
		[AC_MSG_WARN([pkg-config not (fully) installed; drivers requiring X11 may not be built])])
fi
AC_SUBST(LIBX11_LIBS)
AC_SUBST(LIBX11_CFLAGS)

dnl ######################################################################
dnl libhid support
dnl ######################################################################
AC_MSG_CHECKING([if libhid support has been enabled]);
AC_ARG_ENABLE(libhid,
	[AS_HELP_STRING([--disable-libhid],[disable HID support using libhid])],
	[ if test "$enableval" != "no"; then
		enable_libhid=yes
	fi ],
	[ enable_libhid=yes ]
)
AC_MSG_RESULT($enable_libhid)

if test "$enable_libhid" = "yes"; then
	ifdef([PKG_CHECK_MODULES],
	 	[PKG_CHECK_MODULES(LIBHID, libhid >= 0.2.16,
			[AC_DEFINE(HAVE_LIBHID, [1], [Define to 1 if you have libhid])],
			[ enable_libhid=no ])],
		[AC_MSG_WARN([pkg-config not (fully) installed; drivers requiring libhid may not be built])])
fi
AC_SUBST(LIBHID_LIBS)
AC_SUBST(LIBHID_CFLAGS)

# PNG library
LCD_PNG_LIB

dnl ######################################################################
dnl freetype support
dnl ######################################################################
AC_MSG_CHECKING([if freetype support has been enabled]);
AC_ARG_ENABLE(freetype,
	[AS_HELP_STRING([--disable-freetype],[disable freetype support])],
	[ if test "$enableval" != "no"; then
		enable_freetype=yes
	fi ],
	[ enable_freetype=yes ]
)
AC_MSG_RESULT($enable_freetype)

if test "$enable_freetype" = "yes"; then
	ifdef([AC_CHECK_FT2],
		[AC_CHECK_FT2([],
			[AC_DEFINE(HAVE_FT2, [1], [Define to 1 if you have freetype])],
			[enable_freetype=no])],
		[AC_MSG_WARN([freetype does not seem to be installed])])
fi
AC_SUBST([FT2_CFLAGS])
AC_SUBST([FT2_LIBS])


dnl ######################################################################
dnl ethlcd support
dnl ######################################################################
AC_MSG_CHECKING([if ethlcd support has been enabled]);
AC_ARG_ENABLE(ethlcd,
	[AS_HELP_STRING([--disable-ethlcd],[disable ethlcd connection type for HD44780])],
	[ if test "$enableval" != "no"; then
		enable_ethlcd=yes
	fi ],
	[ enable_ethlcd=yes ]
)
AC_MSG_RESULT($enable_ethlcd)


# check for doxygen
BB_ENABLE_DOXYGEN


# Select drivers to build
LCD_DRIVERS_SELECT


# directory for PID files
pidfiledir=/var/run
# make sure the directory exists
if test ! -d $pidfiledir ; then
        pidfiledir=`eval echo ${sysconfdir}`
	case $pidfiledir in
		NONE/*) pidfiledir=`echo $pidfiledir | sed "s~NONE~$ac_default_prefix~"` ;;
	esac
fi

AC_ARG_WITH(pidfile-dir,
	[AS_HELP_STRING([--with-pidfile-dir=PATH], [specify location of pid files [/var/run]])],
	[ if test -n "$withval"  &&  test "x$withval" != "xno"  &&  \
	     test "x${withval}" != "xyes"; then
		pidfiledir=$withval
		if test ! -d $pidfiledir ; then
			AC_MSG_WARN([** no $pidfiledir directory on this system **])
		fi
	fi ]
)

AC_SUBST(pidfiledir)


# Features applicable to the server
AC_ARG_ENABLE(seamless-hbars,
	[AS_HELP_STRING([--enable-seamless-hbars], [no gaps in horizontal bar graphs (if HW supports it)])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(SEAMLESS_HBARS, [1],
			[Define to 1 to avoid gaps in horizontal bar graphs (if HW supports it)])
	fi ]
)

AC_ARG_ENABLE(testmenus,
	[AS_HELP_STRING([--enable-testmenus], [enable server test menus])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(LCDPROC_TESTMENUS, [1],
			[Define to 1 to enable server test menus])
	fi ]
)

AC_ARG_ENABLE(permissive_menu_goto,
	[AS_HELP_STRING([--enable-permissive-menu-goto], [allow transitions between different client's menus])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(LCDPROC_PERMISSIVE_MENU_GOTO, [1],
			[Define to 1 to allow transitions between different client's menus])
	fi ]
)

dnl Features applicable to the lcdproc client
AC_ARG_ENABLE(lcdproc-menus,
	[AS_HELP_STRING([--enable-lcdproc-menus], [enable menu support in lcdproc client])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(LCDPROC_MENUS, [1],
			[Define to 1 to enable menu support in lcdproc client])
	fi ]
)

AC_ARG_ENABLE(stat-nfs,
	[AS_HELP_STRING([--enable-stat-nfs], [display NFS filesystem stats in lcdproc client])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(STAT_NFS, [1],
			[Define to 1 to display NFS filesystem stats in lcdproc client])
	fi ]
)

AC_ARG_ENABLE(stat-smbfs,
	[AS_HELP_STRING([--enable-stat-smbfs], [display SMBFS filesystem stats in lcdproc client])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(STAT_SMBFS, [1],
			[Define to 1 to display SMBFS filesystem stats in lcdproc client])
	fi ]
)

AC_ARG_ENABLE(extra-charmaps,
	[AS_HELP_STRING([--enable-extra-charmaps],
		[enable additional character mapping tables in drivers])],
	[ if test "$enableval" != "no" ; then
		AC_DEFINE(EXTRA_CHARMAPS, [1],
			[Define to 1 to enable additional charmaps in drivers])
	fi ]
)


AC_CONFIG_FILES([Makefile
	shared/Makefile
	server/Makefile
	server/commands/Makefile
	server/drivers/Makefile
	clients/Makefile
	clients/lcdproc/Makefile
	clients/lcdexec/Makefile
	clients/lcdvc/Makefile
	clients/examples/Makefile
	clients/metar/Makefile
	docs/Makefile
	docs/Doxyfile
	docs/lcdproc-dev/Makefile
	docs/lcdproc-user/Makefile
	docs/lcdproc-user/drivers/Makefile
	scripts/Makefile
	scripts/init-LCDd.LSB
	scripts/init-lcdproc.LSB
	scripts/init-lcdexec.LSB
	scripts/init-lcdvc.LSB
	scripts/init-LCDd.debian
	scripts/init-lcdproc.debian
	scripts/init-lcdexec.debian
	scripts/init-lcdvc.debian
	scripts/init-LCDd.rpm
	scripts/init-lcdproc.rpm])
AC_OUTPUT
