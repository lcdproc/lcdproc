/*
 * menus.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 */

#ifndef MENUS_H
#define MENUS_H

#include "menu.h"

/* These probably don't need to be defined here... */
extern menu_item main_menu[];
extern menu_item options_menu[];
extern menu_item screens_menu[];
extern menu_item shutdown_menu[];
extern menu_item Backlight_menu[];

/* Brings up the main menu... */
void server_menu ();

#endif
