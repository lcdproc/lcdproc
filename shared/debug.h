#ifndef DEBUG_H
#define DEBUG_H

#include "../config.h"

#ifdef DEBUG

#define debug printf

#else

#define debug						  /* printf */

#endif

#endif
