AC_DEFUN(LCD_DRIVERS_SELECT, [
AC_MSG_CHECKING(for which drivers to compile)

AC_ARG_ENABLE(drivers,
  	[  --enable-drivers=<list> compile driver for LCDs in <list>.]
	[                  drivers may be separated with commas.]
	[                  \"all\" compiles all drivers],
  	drivers="$enableval", 
  	drivers=[mtxorb,cfontz,curses,text])

if test "$drivers" = "all"; then
	drivers=[mtxorb,cfontz,curses,text,hd44780,joy,irman,lircin]
  	AC_MSG_RESULT(all)
fi

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
		sli)	
			DRIVERS="$DRIVERS wirz-sli.o"
			AC_DEFINE(SLI_DRV)
			;;
		curses)
			AC_CHECK_LIB(ncurses, main, LIBCURSES="-lncurses",
				AC_CHECK_LIB(curses, main, LIBCURSES="-lcurses",
					AC_MSG_ERROR([The curses driver needs the curses library])))
			DRIVERS="$DRIVERS curses_drv.o"
			AC_DEFINE(CURSES_DRV)
			;;
		text)
			DRIVERS="$DRIVERS text.o"
			AC_DEFINE(TEXT_DRV)
			;;
		hd44780)
			DRIVERS="$DRIVERS hd44780.o hd44780-4bit.o hd44780-ext8bit.o lcd_sem.o hd44780-serialLpt.o hd44780-winamp.o"
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
		lircin)
			AC_CHECK_LIB(lirc_client, main, LIBLIRC_CLIENT="-llirc_client",
				AC_MSG_ERROR([The lirc driver needs the lirc client library]))
			DRIVERS="$DRIVERS lircin.o"
			AC_DEFINE(LIRCIN_DRV)
			;;
	*) 	
			AC_MSG_ERROR([Uknown driver $driver])
			;;
  		esac
  	done

AC_SUBST(LIBCURSES)
AC_SUBST(LIBIRMAN)
AC_SUBST(LIBLIRC_CLIENT)
AC_SUBST(DRIVERS)
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
        AC_DEFINE(STAT_STATVFS)
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
        AC_DEFINE(STAT_STATFS3_OSF1)
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
        AC_DEFINE(STAT_STATFS2_BSIZE)
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
        AC_DEFINE(STAT_STATFS4)
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
        AC_DEFINE(STAT_STATFS2_FSIZE)
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
        AC_DEFINE(STAT_STATFS2_FS_DATA)
      fi
    fi

    dnl Not supported
    dnl if test $space = no; then
    dnl # SVR2
    dnl AC_TRY_CPP([#include <sys/filsys.h>],
    dnl   AC_DEFINE(STAT_READ_FILSYS) space=yes)
    dnl fi
])

