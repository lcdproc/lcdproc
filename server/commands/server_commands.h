/** \file server/commands/server_commands.h */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#ifndef COMMANDS_SERVER_H
#define COMMANDS_SERVER_H

int output_func(Client *c, int argc, char **argv);
int test_func_func(Client *c, int argc, char **argv);
int noop_func(Client *c, int argc, char **argv);
int info_func(Client *c, int argc, char **argv);
int sleep_func(Client *c, int argc, char **argv);

#endif
