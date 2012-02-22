/** \file server/commands/command_list.h
 * Declares client command dispatcher function.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 */

#ifndef COMMANDS_COMMAND_LIST_H
#define COMMANDS_COMMAND_LIST_H

#include "client.h"

/**
 * The function list for clients is stored in a table, and the items each
 * point to a function to call, defined below.
 */
typedef int (*CommandFunc) (Client *c, int argc, char **argv);

/** Defines an entry in the command table */
typedef struct client_function {
	char *keyword;		/**< Command string in the protocol */
	CommandFunc function;	/**< Pointer to the associated function */
} client_function;


CommandFunc get_command_function(char *cmd);

#endif
