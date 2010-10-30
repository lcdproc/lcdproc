#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
*/

static inline void debug(const char *format, .../*args*/) {
#ifdef DEBUG
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
#endif /*DEBUG*/
}

#endif
