/** \file shared/defines.h
 * Define macros commonly used in other parts of LCDproc.
 */

/*-
 * This file is part of LCDproc.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#ifndef SHARED_DEFINES_H
#define SHARED_DEFINES_H

/* Define min() and max() */
#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
# define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif
