/*
 * client_menu.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef CLIENT_MENU_H
#define CLIENT_MENU_H

/*
  hee-hee..  this isn't implemented yet.

  Eventually, it'll handle client-supplied menus, and send back info about
  what the user does.
 */

typedef struct client_menu {
	char id[];
	LL *items;
} client_menu;

typedef struct client_menu_item {
	char id[];
	int type;				/* Title, function, submenu, slider, checkbox, etc...*/
	int value;				/* Holds stuff like "true", 43, etc...*/
	char text[];				/* Text to display here...*/
	char child[];				/* For the "submenu" type*/
} client_menu;

#endif
