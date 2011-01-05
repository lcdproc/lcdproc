/** \file server/commands/screen_commands.h */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#ifndef COMMANDS_SCREEN_H
#define COMMANDS_SCREEN_H

int screen_add_func(Client *c, int argc, char **argv);
int screen_del_func(Client *c, int argc, char **argv);
int screen_set_func(Client *c, int argc, char **argv);
int screen_add_key_func(Client *c, int argc, char **argv);
int screen_del_key_func(Client *c, int argc, char **argv);

#endif

