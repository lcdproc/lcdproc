/*
 * menu.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 * Defines all the menu data and actions.
 */

#ifndef MENU_H
#define MENU_H


struct MenuData;
#include "menuitem.h"


#include "shared/LL.h"

#include "screen.h"

#ifndef bool
# define bool short
# define true 1
# define false 0
#endif

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

typedef MenuItem Menu;
/* A Menu is a MenuItem too.
 * This definition is only for better understanding of this code.
 */

Menu *menu_create (char *id, MenuEventFunc(*event_func),
	char *text, void *association);
/* Creates a new menu.
 */
/* Creates a string value box.
 * Association is free to set to anything you want. It's not used by the menu.
 * Generated events: MENUEVENT_ENTER upon entering this menu,
 */

void menu_destroy (Menu *menu);
/* Deletes menu from memory.
 * Destructors will be called for all subitems.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_destoy INSTEAD !
 */

void menu_add_item (Menu *menu, MenuItem *item);
/* Adds an item to the menu */

void menu_remove_item (Menu *menu, MenuItem *item);
/* Removes an item from the menu (does not destroy it) */

void menu_destroy_all_items (Menu *menu);
/* Destroys and removes all items from the menu */

static inline MenuItem *menu_getfirst_item (Menu *menu)
/* Enumeration function.
 * Retrieves the first item from the list of items in the menu.
 */
{
	return (MenuItem*) LL_GetFirst( menu->data.menu.contents );
}

static inline MenuItem *menu_getnext_item (Menu *menu)
/* Enumeration function.
 * Retrieves the next item from the list of items in the menu.
 * No other menu calls should be made between menu_first_item() and
 * this function, to keep the list-cursor where it is.
 */
{
	return (MenuItem*) LL_GetNext( menu->data.menu.contents );
}

static inline MenuItem *menu_get_current_item (Menu *menu)
/* Retrieves the current item from the list of items in the menu. */
{
	return LL_GetByIndex(
		menu->data.menu.contents,
		menu->data.menu.selector_pos);
}

MenuItem *menu_find_item (Menu *menu, char *id);
/* Retrieves the first item from the list of items in the menu. */

void menu_reset (Menu *menu);
/* Resets it to initial state.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_build_screen INSTEAD !
 */

void menu_build_screen (Menu *menu, Screen *s);
/* Builds the selected menuitem on screen using widgets.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_build_screen INSTEAD !
 */

void menu_update_screen (Menu *menu, Screen *s);
/* Updates the widgets of the selected menuitem
 * DO NOT CALL THIS FUNCTION, CALL menuitem_build_screen INSTEAD !
 */

MenuResult menu_handle_input (Menu *menu, MenuToken token, char * key);
/* Does something with the given input.
 * key is only used if token is MENUTOKEN_OTHER.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_build_screen INSTEAD !
 */

#endif
