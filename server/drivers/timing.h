#ifndef _TIMING_H
#define _TIMING_H

/*
 * Utility header file for timing functions
 *
 * Made by Guillaume Filion, moved from the HD44780 driver.
 *
 * From hd44780.c:
 * Modified July 2000 by Charles Steinkuehler to use one of 3 methods for delay
 * timing.  I/O reads, gettimeofday, and nanosleep.  Of the three, nanosleep
 * seems to work best, so that's what is set by default.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c)  2001 Guillaume Filion <gfk@logidac.com>
 *                2000 Charles Steinkuehler <cstein@newtek.com>
 */

// Uncomment one of the lines below to select your desired delay generation
// mechanism.  If both defines are commented, the original I/O read timing
// loop is used.  Using DELAY_NANOSLEEP  seems to provide the best performance.
//#define DELAY_GETTIMEOFDAY
#ifdef HAVE_SCHED_H
# define DELAY_NANOSLEEP
#else
# define DELAY_GETTIMEOFDAY
#endif

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

// Only one alternate delay method at a time, please ;-)
#if defined DELAY_GETTIMEOFDAY
# undef DELAY_NANOSLEEP
#elif defined DELAY_NANOSLEEP
# include <sched.h>
# include <time.h>
#endif

static int delayMult = 0;	 // Delay multiplier for slow displays

/////////////////////////////////////////////////////////////////
// Initialisation
//
static inline int timing_init() {
#if defined DELAY_NANOSLEEP
	// Change to Round-Robin scheduling for nanosleep
	{
		// Set priority to 1
		struct sched_param param;
		param.sched_priority=1;
		if (( sched_setscheduler(0, SCHED_RR, &param)) == -1) {
			fprintf (stderr, "timing_init: failed (%s)\n", strerror (errno));
			return -1;
		}
	}
#endif
	return 0;
}

/////////////////////////////////////////////////////////////////
// IO delay to avoid a task switch
//
static inline void timing_uPause (int usecs) {
	int delay = usecs * delayMult;  // To correct for slower displays

#if defined DELAY_GETTIMEOFDAY
	struct timeval current_time,delay_time,wait_time;

	// Get current time first thing
	gettimeofday(&current_time,NULL);

	// Calculate when delay is over
	delay_time.tv_sec  = 0;
	delay_time.tv_usec = delay;
	timeradd(&current_time,&delay_time,&wait_time);

	do {
		gettimeofday(&current_time,NULL);
	} while (timercmp(&current_time,&wait_time,<));

#elif defined DELAY_NANOSLEEP
	struct timespec delay_time,remaining;

	delay_time.tv_sec = 0;
	delay_time.tv_nsec = delay * 1000;
	while ( nanosleep(&delay_time,&remaining) == -1 )
	{
		delay_time.tv_sec  = remaining.tv_sec;
		delay_time.tv_nsec = remaining.tv_nsec;
	}
#else // using I/O timing
      // Assuming every port I/O takes 1ms, we'll do at least one.
	int i=0;
	do { port_in (port); } while (i++ < delay/1000);
#endif
}


#endif // _TIMING_H
