/*
 * menu.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 * Defines all the menu data and action.
 */

#ifndef MENU_H
#define MENU_H

#include "shared/LL.h"

#include "screen.h"

/*********************************************************************
 * Data definitions of the menustuff
 * Do not modify attributes from your code, but call functions.
 */

typedef enum MenuItemType {
	MENUITEM_NONE = 0,
	MENUITEM_TITLE,
	MENUITEM_MENU,
	MENUITEM_ACTION,
	MENUITEM_CHECKBOX,
	MENUITEM_SLIDER,
	MENUITEM_NUMERIC_INPUT,
	MENUITEM_STRING_INPUT,
} MenuItemType;

typedef enum CheckboxValue {
	CHECKBOX_OFF = 0, CHECKBOX_ON, CHECKBOX_GRAY
} CheckboxValue;



/* Return codes from an event value */
typedef enum MenuEventFuncResult {
	MENUEVENTRES_ERROR = -1,	/* Something has gone wrong */
	MENUEVENTRES_OK = 0,	/* Event handled OK */
	MENUEVENTRES_CLOSE,	/* Event handled OK, close the menu now */
	MENUEVENTRES_QUIT	/* Event handled OK, close ALL menus now */
} MenuEventFuncResult;

/* Events that can happen */
typedef enum MenuEventType {
	MENUEVENT_SELECT,	/* Item has been selected
				(menu opened, action selected) */
	MENUEVENT_UPDATE,	/* Item has been modified
				(checkbox, numeric, string, password) */
	MENUEVENT_PLUS,	/* Item has been modified in positive direction
				 (slider moved) */
	MENUEVENT_MINUS	/* Item has been modified in negative direction
				(slider moved) */
} MenuEventType;

#define EventFunc(f) MenuEventFuncResult (f) (void *object, MenuEventType event)

typedef struct MenuItem {
	int type;	/* Type as defined above */
	char *id;	/* Internal name for client supplied menus */
	EventFunc (*event_func);
			/* Defines event_func to be an event function */
	union data {
		struct menu {
			char *text;	/* Visible name of the submenu */
			int current_item_index; /* At what menuitem is the selector */
			struct MenuItem *parent; /* Parent of this menu */
			LinkedList *contents;	/* What's in this menu */
		} menu;
		struct checkbox {
			char *text;	/* Visible name of item */
			CheckboxValue value;	/* Current value */
			int allowgray;	/* is CHECKBOX_GRAY allowed ? */
		} checkbox;
		struct slider {
			char *text;	/* Visible name of item */
			char *lefttext;	/* Text at minimal value */
			char *righttext;/* Text at minimal value */
			int value;	/* Current value */
			int minvalue;
			int maxvalue;
			int stepsize;
		} slider;
		struct numeric {
			char *text;	/* Visible name of item */
			int value;	/* Current value */
			int maxvalue;
			int minvalue;
		} numeric;
		struct string {
			char *text;	/* Visible name of item */
			char *value;	/* Current value (in a buffer) */
			char passwordchar; /* For passwords */
			int maxlength;
		} string;
		struct password {
			char *text;	/* Visible name of item */
			char *value;	/* Current value (in a buffer) */
			int maxlength;	/* Max allowed length */
		} password;
	} data;
} MenuItem;

#define Menu MenuItem
/* A Menu is a MenuItem too.
 * This definition is only for better understanding of this code.
 */

/*********************************************************************
 * Functions to use the menustuff
 */

MenuItem *menu_create_add_item (Menu *into_menu, int type, char *name,
	EventFunc(*event_func));
/* Creates a new item and adds it to the given menu.
 * All attributes if the new item are cleared.
 * into_menu	What menu should it be in.
 * type:	type as defined above (determines also how the attributes will
 *		be filled).
 * name:	internal name of the item. Never visible. String will be
 *		copied.
 * event_func:	the event function that should be called upon actions on this
 *		item. see event.h.
 * The data part of the new item is filled with 0's.
 *
 * Return value: the new item, of NULL on error.
 *
 * If you create a new menu, the menu data will automatically be allocated,
 * and the contents pointer set to this menu.
 *
 * To initialize data for the data part, do things like:
 *
 *	yourname = menu_create_add_item (m, MENUITEM_STRING_INPUT, "YourName",
 *			yourname_change_handler);
 *	yourname->data.text = strdup ("Your name");
 *	yourname->data.maxlength = 10;
 *	yourname->data.value = malloc (10+1);   <--- maxlength + 1
 *	yourname->data.value[0] = 0;            <--- clear the string
 *
 * The data NEEDS to be initialized immediately (at least before it's first
 * used).
 *
 */

int menu_destroy_item (Menu *menu, MenuItem *item);
/* Deletes item from menu and removes item from memory
 *
 * If the item is a menu, the menu data will also be deallocated.
 *
 * Return value: 0 for OK, -1 for error (eg. submenus not empty)
 *
 * To destroy the data in the data struct, do things like:
 *
 *	free (yourname->data.value);
 *	free (yourname->data.text);
 *	menu_destroy_item (yourname);
 */

static inline MenuItem *menu_getfirst_item (Menu *menu)
/* Retrieves the first item from the list of items in the menu.  */
{
	return (MenuItem*) LL_GetFirst( menu->data.menu.contents );
}

static inline MenuItem *menu_getnext_item (Menu *menu)
/* Retrieves the next item from the list of items in the menu.
 * No other menu calls should be made between menu_first_item() and
 * this function, to keep the list-cursor where it is. */
{
	return (MenuItem*) LL_GetNext( menu->data.menu.contents );
}

MenuItem *menu_find_item (Menu *menu, char *id);
/* Retrieves the first item from the list of items in the menu.  */

void menu_build_screen (MenuItem *item, Screen *s);
/* Builds the selected menuitem on screen using widgets
 * Returns the initial state.
 */

void menu_update_screen (MenuItem *item, Screen *s);
/* Updates the widgets of the selected menuitem
 * Returns the new state.
 */

#endif
