/** \file server/commands/widget_commands.h */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#ifndef COMMANDS_WIDGET_H
#define COMMANDS_WIDGET_H

int widget_add_func(Client *c, int argc, char **argv);
int widget_del_func(Client *c, int argc, char **argv);
int widget_set_func(Client *c, int argc, char **argv);

#endif
