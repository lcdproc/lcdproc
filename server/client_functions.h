/*
 * client_functions.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *
 */

#ifndef CLIENT_FUNCTION_H
#define CLIENT_FUNCTION_H

/*
  The function list for clients is stored in a table, and the items each
  point to a function to call, defined below.
 */

typedef struct client_function {
	char *keyword;
	int (*function) (client * c, int argc, char **argv);
} client_function;

// FIXME?  Do these really need to be visible from other sources?
int test_func_func (client * c, int argc, char **argv);
int hello_func (client * c, int argc, char **argv);
int client_set_func (client * c, int argc, char **argv);
int client_add_key_func (client * c, int argc, char **argv);
int client_del_key_func (client * c, int argc, char **argv);
int screen_add_key_func (client * c, int argc, char **argv);
int screen_del_key_func (client * c, int argc, char **argv);
int screen_add_func (client * c, int argc, char **argv);
int screen_del_func (client * c, int argc, char **argv);
int screen_set_func (client * c, int argc, char **argv);
int widget_add_func (client * c, int argc, char **argv);
int widget_del_func (client * c, int argc, char **argv);
int widget_set_func (client * c, int argc, char **argv);
int menu_add_func (client * c, int argc, char **argv);
int menu_del_func (client * c, int argc, char **argv);
int menu_set_func (client * c, int argc, char **argv);
int menu_add_item_func (client * c, int argc, char **argv);
int menu_del_item_func (client * c, int argc, char **argv);
int menu_set_item_func (client * c, int argc, char **argv);
int backlight_func (client * c, int argc, char **argv);
int output_func (client * c, int argc, char **argv);
int noop_func (client * c, int argc, char **argv);

extern client_function commands[];

#endif
