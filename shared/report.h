#ifndef DEBUG_H
#define DEBUG_H

/* DEBUGGING
To enable the debug() function on all of the software, just type:
./configure --enable-debug
and recompile with 'make'

To enable the debug() function only in specific files:
1) Configure without enabling debug (that is without --enable-debug)
2) Edit the source file that you want to debug and put this:
#define DEBUG
#include "shared/debug.h"
#undef DEBUG
Then recompile with 'make'
This way, the global DEBUG macro is off but is locally enabled in
certains parts of the software.

The debug levels use the following

  0 RPT_CRIT	Critical conditions: the program stops right after this.
		Only use this if the program is exited from the current
		function.
  1 RPT_ERR	Error conditions: serious problem, program continues.
		Use just before you return -1 from a function.
  2 RPT_WARNING	Warning conditions: request user to fix this problem.
		Ex: What a queer port did you select.
  3 RPT_NOTICE	Normal but significant condition:
		Ex: What options have been set.
  4 RPT_INFO	Informational
		Ex: What functions have been called.
  5 RPT_DEBUG	Debug-level messages: further debug messages
		Ex: what are we going to do in the next few lines of code.

Levels 4 and 5 should be reported using the debug function. The code
that this function generates will not be in the executable when compiled
without debugging. This way memory and CPU cycles are saved.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Reporting levels
#define RPT_CRIT 0
#define RPT_ERR 1
#define RPT_WARNING 2
#define RPT_NOTICE 3
#define RPT_INFO 4
#define RPT_DEBUG 5
#define RPT_DEST_STDERR 0
#define RPT_DEST_SYSLOG 1
#define RPT_DEST_STORE 2
// Don't just modify these numbers, they're related to syslog.


#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

extern int report_level;
extern int report_dest;

int set_reporting( int new_level, int new_dest );
// Sets reporting level and message destination.

void report( const int level, const char *format, .../*args*/ );
// Report the message to the selected destination if important enough

// Consider the debug function to be exactly the same as the report function.
// The only difference is that it is only compiled in if DEBUG is defined.

static inline void dont_report( const int level, const char *format, .../*args*/ )
{} // The idea is that this gets optimized out

#ifdef DEBUG
#  define debug report
#else
#  define debug dont_report
#endif /*DEBUG*/

#endif
