/** \file server/menuscreens.h
 * Creates all menuscreens, menus and handles the keypresses for the
 * menuscreens.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2003, Joris Robijn
 */

#ifndef MENUSCREENS_H
#define MENUSCREENS_H

#include "menu.h"
#include "screen.h"

extern Screen *menuscreen;
extern Menu *main_menu;

int menuscreens_init(void);

int menuscreens_shutdown(void);

/** This function indicates to the input part whether this key was the
 * reserved menu key.
 */
bool is_menu_key(const char *key);

/** Meant for other parts of the program to inform the menuscreen that the
 * item is about to be removed.
 */
void menuscreen_inform_item_destruction(MenuItem *item);

/** Meant for other parts of the program to inform the menuscreen that some
 * properties of the item have been modified.
 */
void menuscreen_inform_item_modified(MenuItem *item);

/** This handler handles the keypresses for the menu.
 */
void menuscreen_key_handler(const char *key);

/** Adds a menu for the given screen */
void menuscreen_add_screen(Screen *s);

/** Removes the menu of the given screen */
void menuscreen_remove_screen(Screen *s);

/** switches to menu. */
int menuscreen_goto(Menu *menu);

/** sets custom_main_menu. */
int menuscreen_set_main(Menu *menu);

#endif
