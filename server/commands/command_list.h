/*
 * commands/command_list.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef COMMANDS_COMMAND_LIST_H
#define COMMANDS_COMMAND_LIST_H

#include "../client.h"

/*
  The function list for clients is stored in a table, and the items each
  point to a function to call, defined below.
 */

typedef struct client_function {
	char *keyword;
	int (*function) (Client * c, int argc, char **argv);
} client_function;

/* FIXME?  Do these really need to be visible from other sources?*/

extern client_function commands[];

#endif
