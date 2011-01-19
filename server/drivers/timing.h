/** \file server/drivers/timing.h
 * Utility header file for timing functions
 *
 * Made by Guillaume Filion, moved from the HD44780 driver.
 *
 * From hd44780.c:
 * Modified July 2000 by Charles Steinkuehler to use one of 3 methods for delay
 * timing.  I/O reads, gettimeofday, and nanosleep.  Of the three, nanosleep
 * seems to work best, so that's what is set by default.
 */

/*-
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Guillaume Filion <gfk@logidac.com>
 *                2001 Joris Robijn <joris@robijn.net>
 *                2000 Charles Steinkuehler <cstein@newtek.com>
 */

#ifndef _TIMING_H
#define _TIMING_H

/*
 * Uncomment one of the lines below this paragraph to select your desired
 * delay generation mechanism.
 * Mechanism DELAY_NANOSLEEP seems to provide the best performance.
 * Mechanism DELAY_IOCALLS can be quite inaccurate.
 * Mechanism DELAY_AUTOSELECT lets the system determine a mechanism, and is
 * the default.
 */

#define DELAY_AUTOSELECT
//#define DELAY_GETTIMEOFDAY
//#define DELAY_NANOSLEEP
//#define DELAY_IOCALLS


#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Autoselect...  Does this always work well ? */
#ifdef DELAY_AUTOSELECT
# if defined HAVE_SCHED_H && defined HAVE_SCHED_SETSCHEDULER
#  define DELAY_NANOSLEEP
# else
#  define DELAY_GETTIMEOFDAY
# endif
#endif

/* Include the correct time.h stuff (regardless of selected mechanism) */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* Only one alternate delay method at a time, please ;-) */
#if defined DELAY_GETTIMEOFDAY
# undef DELAY_NANOSLEEP
# undef DELAY_IOCALLS
#elif defined DELAY_IOCALLS && defined HAVE_PCSTYLE_LPT_CONTROL
# undef DELAY_GETTIMEOFDAY
# undef DELAY_NANOSLEEP
# include "port.h"
#else /* assume DELAY_NANOSLEEP */
# undef DELAY_GETTIMEOFDAY
# undef DELAY_IOCALLS
# include <sched.h>
#endif

/*
 * Convenience macros for operations on timevals. These are usually defined
 * in sys/time.h. If your system does not have them, the defines from below
 * are used.
 *
 * NOTE: `timercmp' does not work for >= or <=.
 */
#ifndef timerisset
# define timerisset(tvp)        ((tvp)->tv_sec || (tvp)->tv_usec)
#endif
#ifndef timerclear
# define timerclear(tvp)        ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif
#ifndef timercmp
# define timercmp(a, b, CMP)                                                  \
  (((a)->tv_sec == (b)->tv_sec) ?                                             \
   ((a)->tv_usec CMP (b)->tv_usec) :                                          \
   ((a)->tv_sec CMP (b)->tv_sec))
#endif
#ifndef timeradd
# define timeradd(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                          \
    if ((result)->tv_usec >= 1000000)                                         \
      {                                                                       \
        ++(result)->tv_sec;                                                   \
        (result)->tv_usec -= 1000000;                                         \
      }                                                                       \
  } while (0)
#endif
#ifndef timersub
# define timersub(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#endif


/**
 * Do necessary initialization for the selected waiting method.
 * \return  0 if successful, -1 on error.
 */
static inline int
timing_init()
{
#if defined DELAY_NANOSLEEP
	/* Change to Round-Robin scheduling for nanosleep */
	{
		/* Set priority to 1 */
		struct sched_param param;
		param.sched_priority=1;
		if (( sched_setscheduler(0, SCHED_RR, &param)) == -1) {
			return -1;
		}
	}
#elif defined DELAY_IOCALLS
	if (port_access(0x3BD) == -1) {
		return -1;
	}
#endif
	return 0;
}


/**
 * Delay operation for some time using either gettimeofday (more or less an
 * active waiting loop), nanosleep, or an I/O call.
 * \param usecs  Microsecond to pause
 */
static inline void
timing_uPause(int usecs)
{
#if defined DELAY_GETTIMEOFDAY
	struct timeval current_time,delay_time,wait_time;

	/* Get current time first thing */
	gettimeofday(&current_time,NULL);

	/* Calculate when delay is over */
	delay_time.tv_sec  = 0;
	delay_time.tv_usec = usecs;
	timeradd(&current_time,&delay_time,&wait_time);

	do {
		gettimeofday(&current_time,NULL);
	} while (timercmp(&current_time,&wait_time,<));

#elif defined DELAY_NANOSLEEP
	struct timespec delay_time,remaining;

	delay_time.tv_sec = 0;
	delay_time.tv_nsec = usecs * 1000;
	while ( nanosleep(&delay_time,&remaining) == -1 )
	{
		delay_time.tv_sec  = remaining.tv_sec;
		delay_time.tv_nsec = remaining.tv_nsec;
	}
#else	/* using I/O timing */
	int i;
	for (i = 0; i < usecs; ++i)
		port_in(0x3BD);	/* Assuming every port I/O takes 1us */
#endif
}


#endif /* _TIMING_H */
