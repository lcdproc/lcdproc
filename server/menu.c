/*
 * menu.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * Handles server-supplied menus defined by a table.  Read menu.h for
 * more information.
 *
 * Menus are similar to "pull-down" menus, but have some extra features.
 * They can contain "normal" menu items, checkboxes, sliders, "movers",
 * etc..
 *
 * I should probably find a more elegant way of doing this in order
 * to handle dynamically-changing menus such as the client list.  Tcl/Tk
 * has neat ways to do it.  Hmm...
 *
 * NEW DESCRIPTION
 *
 * Handles a menu and all actions that can be performed on it.
 *
 * Menus are similar to "pull-down" menus, but have some extra features.
 * They can contain "normal" menu items, checkboxes, sliders, "movers",
 * etc..
 *
 * The servermenu is created from servermenu.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#include "menu.h"
#include "shared/report.h"
#include "drivers.h"

#include "screen.h"


/* Internal menu item drawing functions */
void menu_build_menu_screen	(MenuItem *item, Screen *s);
void menu_build_slider_screen	(MenuItem *item, Screen *s);
void menu_build_numeric_screen	(MenuItem *item, Screen *s);
void menu_build_string_screen	(MenuItem *item, Screen *s);
void menu_update_menu_screen	(MenuItem *item, Screen *s);
void menu_update_slider_screen	(MenuItem *item, Screen *s);
void menu_update_numeric_screen	(MenuItem *item, Screen *s);
void menu_update_string_screen	(MenuItem *item, Screen *s);


MenuItem *menu_create_add_item (Menu *into_menu, int type, char *id,
		EventFunc(*event_func))
{
	Menu *new_item;

	/* Allocate space and fill struct */
	new_item = malloc (sizeof(MenuItem));
	if (!new_item) {
		report (RPT_ERR, "menu_add_new_item: Could not allocate memory");
		return NULL;
	}
	new_item->type = type;
	new_item->id = strdup (id);
	if (!new_item->id) {
		report (RPT_ERR, "menu_add_new_item: Could not allocate memory");
		return NULL;
	}
	new_item->event_func = event_func;

	/* Clear the type specific data part */
	memset ( &(new_item->data), 0, sizeof(new_item->data));

	if (type == MENUITEM_MENU ) {
		/* We have created a menu, create our needed data */
		new_item->data.menu.parent = into_menu;
		LL_new (new_item->data.menu.contents);
	}

	if (into_menu) {
		/* Add the item to the menu */
		LL_Push (into_menu->data.menu.contents, new_item);
	}

	return new_item;
}

int menu_destroy_item (Menu *menu, MenuItem *item)
{
	/* Is the item a menu ? */
	if (item->type == MENUITEM_MENU ) {
		/* Check if the menu is empty */
		if ( LL_Length(item->data.menu.contents) != 0 ) {
			report (RPT_ERR, "menu not empty, cannot be destroyed");
			return -1;
		}
		/* Destroy our menu data */
		if (item->data.menu.text)
			free (item->data.menu.text);
		LL_Destroy (item->data.menu.contents);
	}
	free (item->id);
	free (item);

	/* Remove the item from the menu */
	LL_Remove (menu->data.menu.contents, item);

	return 0;
}

MenuItem *menu_find_item (Menu *menu, char *id)
{
	MenuItem * item;
	for( item = menu_getfirst_item(menu); item; item = menu_getnext_item(menu) ) {
		if ( strcmp(item->id, id) == 0 ) {
			return item;
		}
	}
	return NULL;
}


/******** MENU SCREEN BUILD FUNCTIONS ********/

void menu_build_screen (MenuItem *item, Screen *s)
{
	Widget * w;

	if (display_props) {
		/* Nothing to build if no display size is known */
		report (RPT_ERR, "menu_build_screen: display size unknown");
		return;
	}

	/* First remove all widgets from the screen */
	for ( w=screen_getfirst_widget(s); w; w=screen_getnext_widget(s) ) {
		/* We know these widgets don't have subwidgets */
		screen_remove_widget (s, w);
		widget_destroy (w);
	}

	switch (item->type) {
	  case MENUITEM_MENU:
		menu_build_menu_screen (item, s);
		break;
	  case MENUITEM_SLIDER:
		menu_build_slider_screen (item, s);
		break;
	  case MENUITEM_NUMERIC_INPUT:
		menu_build_slider_screen (item, s);
		break;
	  case MENUITEM_STRING_INPUT:
		menu_build_slider_screen (item, s);
		break;
	  default:
		report (RPT_ERR, "menu_build_screen: given menuitem cannot be active");
		return;
	}
	menu_update_screen (item, s);
}

void menu_build_menu_screen	(MenuItem *item, Screen *s)
{
	Widget * w;
	int linenr;

	for (linenr=1; linenr<display_props->width; linenr++) {
		char buf[8];
		snprintf (buf, sizeof(buf)-1, "line%d", linenr);
		buf[sizeof(buf)-1] = 0;
		w = widget_create (buf, WID_STRING, s);
					/* (buf will be copied) */
		w->text = strdup("");
		screen_add_widget (s, w);
	}
}

void menu_build_slider_screen	(MenuItem *item, Screen *s)
{
}

void menu_build_numeric_screen	(MenuItem *item, Screen *s)
{
}

void menu_build_string_screen	(MenuItem *item, Screen *s)
{
}

void menu_build_password_screen	(MenuItem *item, Screen *s)
{
}

/******** MENU SCREEN UPDATE FUNCTIONS ********/

void menu_update_screen (MenuItem *item, Screen *s) {
	switch (item->type) {
		case MENUITEM_MENU:
			menu_update_menu_screen (item, s);
			break;
		case MENUITEM_SLIDER:
			menu_update_slider_screen (item, s);
			break;
		case MENUITEM_NUMERIC_INPUT:
			menu_update_slider_screen (item, s);
			break;
		case MENUITEM_STRING_INPUT:
			menu_update_slider_screen (item, s);
			break;
		default:
			report (RPT_ERR, "menu_update_screen: given menuitem cannot be active");
	}
}

void menu_update_menu_screen	(MenuItem *item, Screen *s)
{
}

void menu_update_slider_screen	(MenuItem *item, Screen *s)
{

}

void menu_update_numeric_screen	(MenuItem *item, Screen *s)
{

}

void menu_update_string_screen	(MenuItem *item, Screen *s)
{

}

