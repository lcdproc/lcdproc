/*
 * menu_commands.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 *
 * This particular file defines actions concerning client supplied menus.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "shared/report.h"
#include "shared/sockets.h"

#include "client.h"

/*************************************************************
 * Adds a menu to the client; handled by the server...
 *
 * usage: menu_add ...?
 */
int
menu_add_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/****************************************************************
 * Removes a client's menu and all contents from the server
 *
 * usage: menu_del ...?
 */
int
menu_del_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/***********************************************************************
 * Sets info about a menu, but not its items
 *
 * For example, should the menu be top-level, or buried somewhere?
 *
 * usage: menu_set ...?
 */
int
menu_set_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/*************************************************************************
 * Adds an item to a menu
 *
 * usage: menu_add_item ...?
 */
int
menu_add_item_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/*************************************************************************
 * Deletes an item from a menu
 *
 + usage: menu_del_item ...?
 */
int
menu_del_item_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

/**************************************************************************
 * Sets the info about a menu item
 *
 * For example, text displayed, widget type, value, etc...
 *
 * usage: menu_set_item ...?
 */
int
menu_set_item_func (Client * c, int argc, char **argv)
{

	if (!c->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

