/** \file server/commands/client_commands.h */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#ifndef COMMANDS_CLIENT_H
#define COMMANDS_CLIENT_H

int hello_func(Client *c, int argc, char **argv);
int bye_func(Client *c, int argc, char **argv);
int client_set_func(Client *c, int argc, char **argv);
int client_add_key_func(Client *c, int argc, char **argv);
int client_del_key_func(Client *c, int argc, char **argv);
int backlight_func(Client *c, int argc, char **argv);

#endif

