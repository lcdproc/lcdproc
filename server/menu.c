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
 * Handles a menu and all actions that can be performed on it. Note that a
 * menu is itself also a menuitem.
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

#include "menuitem.h"
#include "menu.h"
#include "shared/report.h"
#include "drivers.h"

#include "screen.h"

Menu *
menu_create (char *id, MenuEventFunc(*event_func),
	char *text, void *association)
{
	Menu *new_menu;

	debug (RPT_DEBUG, "%s( id=\"%s\", event_func=%p, text=\"%s\", association=%p )", __FUNCTION__, id, event_func, text, association);

	new_menu = menuitem_create (MENUITEM_MENU, id, event_func, text);

	new_menu->data.menu.contents = LL_new();
	new_menu->data.menu.association = association;

	return new_menu;
}

void
menu_destroy (Menu *menu)
{
	debug (RPT_DEBUG, "%s( menu=\"%s\" )", __FUNCTION__, menu->id);

	menu_destroy_all_items (menu);
	LL_Destroy (menu->data.menu.contents);
	menu->data.menu.contents = NULL;

	/* After this the general menuitem routine destroys the rest... */
}

void
menu_add_item (Menu *menu, MenuItem *item)
{
	debug (RPT_DEBUG, "%s( menu=\"%s\", item=\"%s\" )", __FUNCTION__, menu->id, item->id);

	if (!menu) return;

	/* Add the item to the menu */
	LL_Push (menu->data.menu.contents, item);
	item->parent = menu;
}

void
menu_remove_item (Menu *menu, MenuItem *item)
{
	debug (RPT_DEBUG, "%s( menu=\"%s\", item=\"%s\" )", __FUNCTION__, menu->id, item->id);

	LL_Remove (menu->data.menu.contents, item);
}

void
menu_destroy_all_items (Menu *menu)
{
	MenuItem * item;

	debug (RPT_DEBUG, "%s( menu=\"%s\" )", __FUNCTION__, menu->id);

	for( item = menu_getfirst_item(menu); item; item = menu_getfirst_item(menu) ) {
		menuitem_destroy (item);
		LL_Remove (menu->data.menu.contents, item);
	}
}

MenuItem *menu_find_item (Menu *menu, char *id, bool recursive)
{
	MenuItem * item;

	debug (RPT_DEBUG, "%s( menu=\"%s\", id=\"%s\" )", __FUNCTION__, menu->id, id);

	for( item = menu_getfirst_item(menu); item; item = menu_getnext_item(menu) ) {
		if ( strcmp(item->id, id) == 0 ) {
			return item;
		}
		else if (recursive && item->type == MENUITEM_MENU) {
			MenuItem * res;
			res = menu_find_item (item, id, recursive);
			if (res) {
				return res;
			}
		}
	}
	return NULL;
}

void menu_reset (Menu *menu)
{
	menu->data.menu.selector_pos = 0;
	menu->data.menu.scroll = 0;
}

void menu_build_screen (MenuItem *menu, Screen *s)
{
	Widget * w;
	MenuItem * subitem;
	int itemnr;

	debug (RPT_DEBUG, "%s( menu=\"%s\", screen=\"%s\" )", __FUNCTION__, menu->id, s->id);

	/* TODO: Put menu in a frame to do easy scrolling */
	/* Problem: frames are not handled correctly by renderer */

	/* Create menu title widget */
	w = widget_create ("title", WID_TITLE, s);
	screen_add_widget (s, w);
	w->text = strdup(menu->text);
	w->x = 1;

	/* Create widgets for each subitem in the menu */
	for (subitem = LL_GetFirst (menu->data.menu.contents), itemnr = 0;
	     subitem;
	     subitem = LL_GetNext (menu->data.menu.contents), itemnr ++ )
	{
		char buf[10];

		snprintf (buf, sizeof(buf)-1, "text%d", itemnr);
		buf[sizeof(buf)-1] = 0;
		w = widget_create (buf, WID_STRING, s);
					/* (buf will be copied) */
		screen_add_widget (s, w);
		w->x = 2;

		switch (subitem->type) {
		  case MENUITEM_CHECKBOX:

			/* Limit string length */
			w->text = strdup (subitem->text);
			if (strlen(subitem->text) >= display_props->width-2) {
				(w->text)[display_props->width-2] = 0;
			}

			/* Add icon for checkbox */
			snprintf (buf, sizeof(buf)-1, "icon%d", itemnr);
			buf[sizeof(buf)-1] = 0;
			w = widget_create (buf, WID_ICON, s);
						/* (buf will be copied) */
			screen_add_widget (s, w);
			w->x = display_props->width - 1;
			w->length = ICON_CHECKBOX_OFF;
			break;
		  case MENUITEM_RING:
			/* Create string for text + ringtext */
			w->text = malloc (display_props->width);
			break;
		  case MENUITEM_MENU:
		  case MENUITEM_ACTION:
		  case MENUITEM_SLIDER:
		  case MENUITEM_NUMERIC:
		  case MENUITEM_ALPHA:
			/* Limit string length */
			w->text = strdup (subitem->text);
			if (strlen(subitem->text) >= display_props->width-1) {
				(w->text)[display_props->width-1] = 0;
			}
			break;
		}
	}

	/* Add arrow for selection on the left */
	w = widget_create ("selector", WID_ICON, s);
	screen_add_widget (s, w);
	w->length = ICON_SELECTOR_AT_LEFT;
	w->x = 1;

	/* Add scrollers on the right side on top and bottom */
	/* TODO: when menu is in a frame, these can be removed */
	w = widget_create ("upscroller", WID_ICON, s);
	screen_add_widget (s, w);
	w->length = ICON_ARROW_UP;
	w->x = display_props->width;
	w->y = 1;

	w = widget_create ("downscroller", WID_ICON, s);
	screen_add_widget (s, w);
	w->length = ICON_ARROW_DOWN;
	w->x = display_props->width;
	w->y = display_props->height;

}

void menu_update_screen (MenuItem *menu, Screen *s)
{
	Widget * w;
	MenuItem * subitem;
	int itemnr;

	debug (RPT_INFO, "%s( menu=\"%s\", screen=\"%s\" )", __FUNCTION__, menu->id, s->id);

	/* Update widgets for the title */
	w = screen_find_widget (s, "title");
	if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "title");
	w->y = 1 - menu->data.menu.scroll;

	/* TODO: remove next 5 limes when rendering is safe */
	if (w->y > 0 && w->y <= display_props->height) {
		w->type = WID_TITLE;
	} else {
		w->type = WID_NONE; /* make invisible */
	}

	/* Update widgets for each subitem in the menu */
	for (subitem = LL_GetFirst (menu->data.menu.contents), itemnr = 0;
	     subitem;
	     subitem = LL_GetNext (menu->data.menu.contents), itemnr ++ )
	{
		char buf[10];
		char * p;

		snprintf (buf, sizeof(buf)-1, "text%d", itemnr);
		buf[sizeof(buf)-1] = 0;
		w = screen_find_widget (s, buf);
		if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, buf);
		w->y = 2 + itemnr - menu->data.menu.scroll;

		/* TODO: remove next 5 limes when rendering is safe */
		if (w->y > 0 && w->y <= display_props->height) {
			w->type = WID_STRING;
		} else {
			w->type = WID_NONE; /* make invisible */
		}

		switch (subitem->type) {
		  case MENUITEM_CHECKBOX:

			/* Update icon value for checkbox */
			snprintf (buf, sizeof(buf)-1, "icon%d", itemnr);
			buf[sizeof(buf)-1] = 0;
			w = screen_find_widget (s, buf);
			if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, buf);
			w->y = 2 + itemnr - menu->data.menu.scroll;
			w->length = ((int[]){ICON_CHECKBOX_OFF,ICON_CHECKBOX_ON,ICON_CHECKBOX_GRAY})[subitem->data.checkbox.value];

			/* TODO: remove next 5 limes when rendering is safe */
			if (w->y > 0 && w->y <= display_props->height) {
				w->type = WID_ICON;
			} else {
				w->type = WID_NONE; /* make invisible */
			}
			break;
		  case MENUITEM_RING:
			if (subitem->data.ring.value >= LL_Length(subitem->data.ring.strings)) {
				/* No strings available */
				memcpy (w->text, subitem->text, display_props->width - 2);
				w->text[ display_props->width - 2 ] = 0;
			}
			else {
				/* Limit string length and add ringstring */
				p = LL_GetByIndex (subitem->data.ring.strings, subitem->data.ring.value);
				if (strlen(p) > display_props->width - 3) {
					short a = display_props->width - 3;
					/* We need to limit the ring string and DON'T
					 * display the item text */
					strcpy (w->text, " ");
					memcpy (w->text + 1, p, a);
					w->text[a + 1] = 0;
				}
				else {
					short b = display_props->width - 2 - strlen (p);
					short c = min (strlen (subitem->text), b - 1);
					/* We don't limit the ring string */
					memset (w->text, ' ', b);
					memcpy (w->text, subitem->text, c);
					strcpy (w->text + b, p);
				}
			}
			break;
		  default:
		}
	}

	/* Update selector position */
	w = screen_find_widget (s, "selector");
	if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "selector");
	w->y = 2 + menu->data.menu.selector_pos - menu->data.menu.scroll;

	/* Enable upscroller */
	w = screen_find_widget (s, "upscroller");
	if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "upscroller");
	if (menu->data.menu.scroll > 0) {
		w->type = WID_ICON;
	} else {
		w->type = WID_NONE; /* disable it this way */
	}
	/* Enable downscroller */
	w = screen_find_widget (s, "downscroller");
	if (!w)	report (RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "downscroller");
	if (LL_Length(menu->data.menu.contents) - menu->data.menu.scroll - display_props->height + 1 > 0 ) {
		w->type = WID_ICON;
	} else {
		w->type = WID_NONE; /* disable it this way */
	}
}

MenuResult menu_process_input	(Menu *menu, MenuToken token, char * key)
{
	MenuItem *subitem;

	debug (RPT_DEBUG, "%s( menu=\"%s\", token=%d, key=\"%s\" )", __FUNCTION__, menu->id, token, key);

	switch (token) {
	  case MENUTOKEN_MENU:
		return MENURESULT_CLOSE;
	  case MENUTOKEN_ENTER:
		subitem = LL_GetByIndex (menu->data.menu.contents,
					menu->data.menu.selector_pos);
		if (!subitem) break;
		switch (subitem->type) {
		  case MENUITEM_ACTION:
			if (subitem->event_func)
				subitem->event_func (subitem, MENUEVENT_SELECT);
			return subitem->data.action.menu_result;				return MENURESULT_QUIT;
		  case MENUITEM_CHECKBOX:
			if (subitem->data.checkbox.allow_gray) {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 3;
			}
			else {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 2;
			}
			if (subitem->event_func)
				subitem->event_func (subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  case MENUITEM_RING:
			subitem->data.ring.value = (subitem->data.ring.value + 1) % LL_Length (subitem->data.ring.strings);
			if (subitem->event_func)
				subitem->event_func (subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  case MENUITEM_MENU:
		  case MENUITEM_SLIDER:
		  case MENUITEM_NUMERIC:
		  case MENUITEM_ALPHA:
			//if (subitem->event_func)
			//	subitem->event_func (subitem, MENUEVENT_ENTER);
			return MENURESULT_ENTER;
		  default:
			break;
		}
		return MENURESULT_ERROR;
	  case MENUTOKEN_UP:
		if (menu->data.menu.selector_pos > 0) {
			menu->data.menu.selector_pos --;
			if (menu->data.menu.selector_pos + 1 < menu->data.menu.scroll) {
				menu->data.menu.scroll --;
			}
		}
		else if (menu->data.menu.selector_pos == 0) {
			if (menu->data.menu.selector_pos < menu->data.menu.scroll) {
				menu->data.menu.scroll --;
			}
		}
		return MENURESULT_NONE;
	  case MENUTOKEN_DOWN:
		if (menu->data.menu.selector_pos < LL_Length(menu->data.menu.contents) - 1) {
			menu->data.menu.selector_pos ++;
		}
		if (menu->data.menu.selector_pos - menu->data.menu.scroll + 2 > display_props->height) {
			menu->data.menu.scroll ++;
		}
		return MENURESULT_NONE;
	  case MENUTOKEN_OTHER:
		/* TODO: move to the selected number and enter it */
		return MENURESULT_NONE;
	}
	return MENURESULT_ERROR;
}
