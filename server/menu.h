/*
 * menu.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 *
 * Defines all the menu data and actions.
 *
 */

#include "menuitem.h"
/* These headers are placed here on purpose ! (circular references) */

#ifndef MENU_H
#define MENU_H

#ifndef bool
# define bool short
# define true 1
# define false 0
#endif

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#include "shared/LL.h"

typedef MenuItem Menu;
/* A Menu is a MenuItem too.
 * This definition is only for better understanding of this code.
 */

#include "screen.h"

Menu *menu_create (char *id, MenuEventFunc(*event_func),
	char *text, void *association);
/* Creates a new menu. */

void menu_destroy (Menu *menu);
/* Deletes menu from memory.
 * Destructors will be called for all subitems.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_destroy INSTEAD !
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
	return (MenuItem*) ((menu != NULL)
			    ? LL_GetFirst(menu->data.menu.contents)
			    : NULL);
}

static inline MenuItem *menu_getnext_item (Menu *menu)
/* Enumeration function.
 * Retrieves the next item from the list of items in the menu.
 * No other menu calls should be made between menu_first_item() and
 * this function, to keep the list-cursor where it is.
 */
{
	return (MenuItem*) ((menu != NULL)
			    ? LL_GetNext(menu->data.menu.contents)
			    : NULL);
}

static inline MenuItem *menu_get_current_item (Menu *menu)
/* Retrieves the current item from the list of items in the menu. */
{
	return (MenuItem*) ((menu != NULL)
			    ? LL_GetByIndex(menu->data.menu.contents,
					    menu->data.menu.selector_pos)
			    : NULL);
}

MenuItem *menu_find_item (Menu *menu, char *id, bool recursive);
/* Finds an item in the menu by the given id. */

void menu_reset (Menu *menu);
/* Resets it to initial state.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_reset_screen INSTEAD !
 */

void menu_build_screen (Menu *menu, Screen *s);
/* Builds the selected menuitem on screen using widgets.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_rebuild_screen INSTEAD !
 */

void menu_update_screen (Menu *menu, Screen *s);
/* Updates the widgets of the selected menuitem
 * DO NOT CALL THIS FUNCTION, CALL menuitem_update_screen INSTEAD !
 */

MenuResult menu_process_input (Menu *menu, MenuToken token, char * key, bool extended);
/* Does something with the given input.
 * key is only used if token is MENUTOKEN_OTHER.
 * DO NOT CALL THIS FUNCTION, CALL menuitem_process_input INSTEAD !
 */

#endif
