#ifndef DEBUG_H
#define DEBUG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG

#define debug printf

#else

#define debug						  /* printf */

#endif

#endif
