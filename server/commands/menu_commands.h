/*
 * menu_commands.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 */

#ifndef COMMANDS_MENU_H
#define COMMANDS_MENU_H

int menu_add_func (Client * c, int argc, char **argv);
int menu_del_func (Client * c, int argc, char **argv);
int menu_set_func (Client * c, int argc, char **argv);
int menu_add_item_func (Client * c, int argc, char **argv);
int menu_del_item_func (Client * c, int argc, char **argv);
int menu_set_item_func (Client * c, int argc, char **argv);

#endif

