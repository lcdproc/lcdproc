/** \file shared/str.c
 * Commmand / argument parsing functions (for use in clients).
 */

/*-
 * This file is part of LCDproc.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "report.h"
#include "str.h"

/** Split elements of a string into an array of strings.
 * Elements are typically commands and arguments.
 * \param **argv    Pointer to the array which will store the arguments
 * \param *str      The string to be parsed
 * \param max_args  Number of arguments to parse (typically the size of argv)
 * \retval <0       Error.
 * \retval >=0      The number of arguments parsed.
 */
int
get_args (char **argv, char *str, int max_args)
{
	char *delimiters = " \n\0";
	char *item;
	int i = 0;

	if (!argv)
		return -1;
	if (!str)
		return 0;
	if (max_args < 1)
		return 0;

	debug(RPT_DEBUG, "get_args(%i): string=%s", max_args, str);

	/* Parse the command line... */
	for (item = strtok (str, delimiters); item; item = strtok (NULL, delimiters)) {
		debug(RPT_DEBUG, "get_args: item=%s", item);
		if (i < max_args) {
			argv[i] = item;
			i++;
		} else
			return i;
	}

	return i;
}
