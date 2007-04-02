/*
 * menu.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
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
 * For separation this file should never need to include menuscreen.h.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "config.h"

#include "menuitem.h"
#include "menu.h"
#include "shared/report.h"
#include "drivers.h"

#include "screen.h"
#include "widget.h"


extern Menu *custom_main_menu;

/** Basicly a patched version of LL_GetByIndex() that ignores hidden
 * entries completely. (But it takes a menu as an argument.) */
static void *
menu_get_subitem(Menu *menu, int index)
{
	MenuItem *item;
	int i = 0;

	debug(RPT_DEBUG, "%s(menu=[%s], index=%d)", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"), index);
	for (item = LL_GetFirst(menu->data.menu.contents);
	     item != NULL;
	     item = LL_GetNext(menu->data.menu.contents))
	{
		if (! item->is_hidden)
		{
			if (i == index)
				return item;
			/* hidden items don't count at all... */
			++i;
		}
	}
	return NULL;
}

/**
 * Searches for a subitem with id item_id. This function ignores hidden
 * entries completely.
 *
 * @return index of subitem if found and -1 otherwise. */
static int
menu_get_index_of(Menu *menu, char *item_id)
{
	MenuItem *item;
	int i = 0;

	debug(RPT_DEBUG, "%s(menu=[%s], item_id=%s)", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"), item_id);
	for (item = LL_GetFirst(menu->data.menu.contents);
	     item != NULL;
	     item = LL_GetNext(menu->data.menu.contents))
	{
		if (! item->is_hidden)
		{
			if (strcmp(item_id, item->id) == 0)
				return i;
			/* hidden items don't count at all... */
			++i;
		}
	}
	return -1;
}

static int
menu_visible_item_count(Menu *menu)
{
	MenuItem *item;
	int i = 0;

	for (item = LL_GetFirst(menu->data.menu.contents);
	     item != NULL;
	     item = LL_GetNext(menu->data.menu.contents))
	{
		if (! item->is_hidden)
			++i;
	}
	return i;
}


Menu *
menu_create(char *id, MenuEventFunc(*event_func),
	char *text, Client *client)
{
	Menu *new_menu;

	debug(RPT_DEBUG, "%s(id=\"%s\", event_func=%p, text=\"%s\", client=%p)",
	       __FUNCTION__, id, event_func, text, client);

	new_menu = menuitem_create(MENUITEM_MENU, id, event_func, text, client);

	if (new_menu != NULL) {
		new_menu->data.menu.contents = LL_new();
		new_menu->data.menu.association = NULL;
	}	

	return new_menu;
}

void
menu_destroy(Menu *menu)
{
	debug(RPT_DEBUG, "%s(menu=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"));

	if (menu == NULL)
		return;

	if (custom_main_menu == menu)
		custom_main_menu = NULL;

	menu_destroy_all_items(menu);
	LL_Destroy(menu->data.menu.contents);
	menu->data.menu.contents = NULL;

	/* After this the general menuitem routine destroys the rest... */
}

void
menu_add_item(Menu *menu, MenuItem *item)
{
	debug(RPT_DEBUG, "%s(menu=[%s], item=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"),
			((item != NULL) ? item->id : "(null)"));

	if ((menu == NULL) || (item == NULL))
		return;

	/* Add the item to the menu */
	LL_Push(menu->data.menu.contents, item);
	item->parent = menu;
}

void
menu_remove_item(Menu *menu, MenuItem *item)
{
	int i;
	MenuItem *item2;

	debug(RPT_DEBUG, "%s(menu=[%s], item=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"),
			((item != NULL) ? item->id : "(null)"));

	if ((menu == NULL) || (item == NULL))
		return;

	/* Find the item */
	for (item2 = LL_GetFirst(menu->data.menu.contents), i = 0;
	     item2 != NULL;
	     item2 = LL_GetNext(menu->data.menu.contents), i++) {
		if (item == item2) {
			LL_DeleteNode(menu->data.menu.contents);
			if (menu->data.menu.selector_pos >= i) {
				menu->data.menu.selector_pos--;
				if (menu->data.menu.scroll > 0)
					menu->data.menu.scroll--;
			}
			return;
		}
	}
}

void
menu_destroy_all_items(Menu *menu)
{
	MenuItem *item;

	debug(RPT_DEBUG, "%s(menu=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"));

	if (menu == NULL)
		return;

	for (item = menu_getfirst_item(menu); item != NULL; item = menu_getfirst_item(menu)) {
		menuitem_destroy(item);
		LL_Remove(menu->data.menu.contents, item);
	}
}

MenuItem *menu_get_current_item(Menu *menu)
{
	return (MenuItem*) ((menu != NULL)
			    ? menu_get_subitem(menu, menu->data.menu.selector_pos)
			    : NULL);
}

MenuItem *menu_find_item(Menu *menu, char *id, bool recursive)
{
	MenuItem *item;

	debug(RPT_DEBUG, "%s(menu=[%s], id=\"%s\", recursive=%d)", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"), id, recursive);

	if ((menu == NULL) || (id == NULL))
		return NULL;
	if (strcmp(menu->id, id) == 0)
		return menu;

	for (item = menu_getfirst_item(menu); item != NULL; item = menu_getnext_item(menu)) {
		if (strcmp(item->id, id) == 0) {
			return item;
		}
		else if (recursive && item->type == MENUITEM_MENU) {
			MenuItem *res;
			res = menu_find_item(item, id, recursive);
			if (res) {
				return res;
			}
		}
	}
	return NULL;
}

void menu_set_association(Menu *menu, void *assoc)
{
	menu->data.menu.association = assoc;
}

void menu_reset(Menu *menu)
{
	debug(RPT_DEBUG, "%s(menu=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"));

	if (menu == NULL)
		return;

	menu->data.menu.selector_pos = 0;
	menu->data.menu.scroll = 0;
}

void menu_build_screen(MenuItem *menu, Screen *s)
{
	Widget *w;
	MenuItem *subitem;
	int itemnr;

	debug(RPT_DEBUG, "%s(menu=[%s], screen=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((menu == NULL) || (s == NULL))
		return;

	/* TODO: Put menu in a frame to do easy scrolling */
	/* Problem: frames are not handled correctly by renderer */

	/* Create menu title widget */
	w = widget_create("title", WID_TITLE, s);
	if (w != NULL) {
		screen_add_widget(s, w);
		w->text = strdup(menu->text);
		w->x = 1;
	}	

	/* Create widgets for each subitem in the menu */
	for (subitem = LL_GetFirst(menu->data.menu.contents), itemnr = 0;
	     subitem != NULL;
	     subitem = LL_GetNext(menu->data.menu.contents), itemnr ++)
	{
		char buf[10];

		if (subitem->is_hidden)
			continue;
		snprintf(buf, sizeof(buf)-1, "text%d", itemnr);
		buf[sizeof(buf)-1] = 0;
		w = widget_create(buf, WID_STRING, s);
					/* (buf will be copied) */
		if (w != NULL) {
			screen_add_widget(s, w);
			w->x = 2;

			switch (subitem->type) {
			  case MENUITEM_CHECKBOX:

				/* Limit string length */
				w->text = strdup(subitem->text);
				if (strlen(subitem->text) >= display_props->width-2) {
					(w->text)[display_props->width-2] = 0;
				}

				/* Add icon for checkbox */
				snprintf(buf, sizeof(buf)-1, "icon%d", itemnr);
				buf[sizeof(buf)-1] = 0;
				w = widget_create(buf, WID_ICON, s);
						/* (buf will be copied) */
				screen_add_widget(s, w);
				w->x = display_props->width - 1;
				w->length = ICON_CHECKBOX_OFF;
				break;
			  case MENUITEM_RING:
				/* Create string for text + ringtext */
				w->text = malloc(display_props->width);
				break;
			  case MENUITEM_MENU:
				/* Limit string length */
				w->text = malloc(strlen(subitem->text) + 4);
				strcpy(w->text, subitem->text);
				strcat(w->text, " >");
				if (strlen(subitem->text) >= display_props->width-1) {
					(w->text)[display_props->width-1] = '\0';
				}
				break;
			  case MENUITEM_ACTION:
			  case MENUITEM_SLIDER:
			  case MENUITEM_NUMERIC:
			  case MENUITEM_ALPHA:
			  case MENUITEM_IP:
				/* Limit string length */
				w->text = strdup(subitem->text);
				if (strlen(subitem->text) >= display_props->width-1) {
					(w->text)[display_props->width-1] = '\0';
				}
				break;
			 default:
				assert(!"unexpected menuitem type");
			}
		}
	}

	/* Add arrow for selection on the left */
	w = widget_create("selector", WID_ICON, s);
	if (w != NULL) {
		screen_add_widget(s, w);
		w->length = ICON_SELECTOR_AT_LEFT;
		w->x = 1;
	}

	/* Add scrollers on the right side on top and bottom */
	/* TODO: when menu is in a frame, these can be removed */
	w = widget_create("upscroller", WID_ICON, s);
	if (w != NULL) {
		screen_add_widget(s, w);
		w->length = ICON_ARROW_UP;
		w->x = display_props->width;
		w->y = 1;
	}

	w = widget_create("downscroller", WID_ICON, s);
	if (w != NULL) {
		screen_add_widget(s, w);
		w->length = ICON_ARROW_DOWN;
		w->x = display_props->width;
		w->y = display_props->height;
	}

}

void menu_update_screen(MenuItem *menu, Screen *s)
{
	Widget *w;
	MenuItem *subitem;
	int itemnr;
	int hidden_count = 0;

	debug(RPT_DEBUG, "%s(menu=[%s], screen=[%s])", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"),
			((s != NULL) ? s->id : "(null)"));

	if ((menu == NULL) || (s == NULL))
		return;

	/* Update widgets for the title */
	w = screen_find_widget(s, "title");
	if (!w)	report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "title");
	w->y = 1 - menu->data.menu.scroll;

	/* TODO: remove next 5 limes when rendering is safe */
	if (w->y > 0 && w->y <= display_props->height) {
		w->type = WID_TITLE;
	} else {
		w->type = WID_NONE; /* make invisible */
	}

	/* Update widgets for each subitem in the menu */
	for (subitem = LL_GetFirst(menu->data.menu.contents), itemnr = 0;
	     subitem;
	     subitem = LL_GetNext(menu->data.menu.contents), itemnr ++)
	{
		char buf[10];
		char *p;

		if (subitem->is_hidden)
		{
			debug(RPT_DEBUG, "%s: menu %s has hidden menu: %s",
			      __FUNCTION__, menu->id, subitem->id);
			++hidden_count;
			continue;
		}
		snprintf(buf, sizeof(buf)-1, "text%d", itemnr);
		buf[sizeof(buf)-1] = 0;
		w = screen_find_widget(s, buf);
		if (!w)	report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, buf);
		w->y = 2 + itemnr - hidden_count - menu->data.menu.scroll;

		/* TODO: remove next 5 lines when rendering is safe */
		if (w->y > 0 && w->y <= display_props->height) {
			w->type = WID_STRING;
		} else {
			w->type = WID_NONE; /* make invisible */
		}

		switch (subitem->type) {
		  case MENUITEM_CHECKBOX:

			/* Update icon value for checkbox */
			snprintf(buf, sizeof(buf)-1, "icon%d", itemnr);
			buf[sizeof(buf)-1] = 0;
			w = screen_find_widget(s, buf);
			if (!w)	report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, buf);
			w->y = 2 + itemnr - menu->data.menu.scroll;
			w->length = ((int[]){ICON_CHECKBOX_OFF,ICON_CHECKBOX_ON,ICON_CHECKBOX_GRAY})[subitem->data.checkbox.value];

			/* TODO: remove next 5 lines when rendering is safe */
			if (w->y > 0 && w->y <= display_props->height) {
				w->type = WID_ICON;
			} else {
				w->type = WID_NONE; /* make invisible */
			}
			break;
		  case MENUITEM_RING:
			if (subitem->data.ring.value >= LL_Length(subitem->data.ring.strings)) {
				/* No strings available */
				memcpy(w->text, subitem->text, display_props->width - 2);
				w->text[ display_props->width - 2 ] = 0;
			}
			else {
				/* Limit string length and add ringstring */
				p = LL_GetByIndex(subitem->data.ring.strings, subitem->data.ring.value);
				assert(p != NULL);
				if (strlen(p) > display_props->width - 3) {
					short a = display_props->width - 3;
					/* We need to limit the ring string and DON'T
					 * display the item text */
					strcpy(w->text, " ");
					memcpy(w->text + 1, p, a);
					w->text[a + 1] = 0;
				}
				else {
					short b = display_props->width - 2 - strlen(p);
					short c = min(strlen(subitem->text), b - 1);
					/* We don't limit the ring string */
					memset(w->text, ' ', b);
					memcpy(w->text, subitem->text, c);
					strcpy(w->text + b, p);
				}
			}
			break;
		  default:
                      break;
		}
	}

	/* Update selector position */
	w = screen_find_widget(s, "selector");
	if (w != NULL)
		w->y = 2 + menu->data.menu.selector_pos - menu->data.menu.scroll;
	else
		report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "selector");

	/* Enable upscroller (if necessary) */
	w = screen_find_widget(s, "upscroller");
	if (w != NULL)
		w->type = (menu->data.menu.scroll > 0) ? WID_ICON : WID_NONE;
	else
		report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "upscroller");

	/* Enable downscroller (if necessary) */
	w = screen_find_widget(s, "downscroller");
	if (w != NULL)
		w->type = (menu_visible_item_count(menu) >= menu->data.menu.scroll + display_props->height)
			? WID_ICON : WID_NONE;
	else
		report(RPT_ERR, "%s: could not find widget: %s", __FUNCTION__, "downscroller");
}

MenuItem *menu_get_item_for_predecessor_check(Menu *menu)
{
	MenuItem *subitem = menu_get_subitem(menu, menu->data.menu.selector_pos);
	if (! subitem)
		return NULL;
	switch (subitem->type) {
	case MENUITEM_ACTION:
	case MENUITEM_CHECKBOX:
	case MENUITEM_RING:
		// for types without own screen: look for menu's
		// predecessor if its subitem doesn't have one. (Since
		// menus can't have successors this problem arises
		// only for predecessors.)
		if (subitem->predecessor_id == NULL)
			return menu;
		return subitem;
	case MENUITEM_MENU:
	case MENUITEM_SLIDER:
	case MENUITEM_NUMERIC:
	case MENUITEM_ALPHA:
	case MENUITEM_IP:
		return menu;
	default:
		return NULL;
	}
}

MenuItem *menu_get_item_for_successor_check(Menu *menu)
{
	MenuItem *subitem = menu_get_subitem(menu, menu->data.menu.selector_pos);
	if (! subitem)
		return NULL;
	switch (subitem->type) {
	case MENUITEM_ACTION:
	case MENUITEM_CHECKBOX:
	case MENUITEM_RING:
		return subitem;
	case MENUITEM_MENU:
	case MENUITEM_SLIDER:
	case MENUITEM_NUMERIC:
	case MENUITEM_ALPHA:
	case MENUITEM_IP:
		return menu;
	default:
		return NULL;
	}
}

MenuResult menu_process_input(Menu *menu, MenuToken token, const char *key, bool extended)
{
	MenuItem *subitem;
	debug(RPT_DEBUG, "%s(menu=[%s], token=%d, key=\"%s\")", __FUNCTION__,
			((menu != NULL) ? menu->id : "(null)"), token, key);

	if (menu == NULL)
		return MENURESULT_ERROR;

	switch (token) {
	  case MENUTOKEN_MENU:
		subitem = menu_get_item_for_predecessor_check(menu);
		if (! subitem)
			return MENURESULT_ERROR;
		return menuitem_predecessor2menuresult(
			subitem->predecessor_id, MENURESULT_CLOSE);
	  case MENUTOKEN_ENTER:
		subitem = menu_get_subitem(menu, menu->data.menu.selector_pos);
		if (!subitem)
			break;
		switch (subitem->type) {
		  case MENUITEM_ACTION:
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_SELECT);
			return menuitem_successor2menuresult(
				subitem->successor_id, MENURESULT_NONE);
		  case MENUITEM_CHECKBOX:
			if (subitem->data.checkbox.allow_gray) {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 3;
			}
			else {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 2;
			}
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return menuitem_successor2menuresult(
				subitem->successor_id, MENURESULT_NONE);
		  case MENUITEM_RING:
			subitem->data.ring.value = (subitem->data.ring.value + 1) % LL_Length(subitem->data.ring.strings);
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return menuitem_successor2menuresult(
				subitem->successor_id, MENURESULT_NONE);
		  case MENUITEM_MENU:
		  case MENUITEM_SLIDER:
		  case MENUITEM_NUMERIC:
		  case MENUITEM_ALPHA:
		  case MENUITEM_IP:
			return MENURESULT_ENTER;
		  default:
			break;
		}
		return MENURESULT_ERROR;
	  case MENUTOKEN_UP:
		if (menu->data.menu.selector_pos > 0) {
			menu->data.menu.selector_pos --;
			if (menu->data.menu.selector_pos + 1 < menu->data.menu.scroll)
				menu->data.menu.scroll --;
		}
		else if (menu->data.menu.selector_pos == 0) {
			// wrap around to last menu entry
			menu->data.menu.selector_pos = menu_visible_item_count(menu) - 1;
			menu->data.menu.scroll = menu->data.menu.selector_pos + 2 - display_props->height;
		}
		return MENURESULT_NONE;
	  case MENUTOKEN_DOWN:
		if (menu->data.menu.selector_pos < menu_visible_item_count(menu) - 1) {
			menu->data.menu.selector_pos ++;
			if (menu->data.menu.selector_pos - menu->data.menu.scroll + 2 > display_props->height)
				menu->data.menu.scroll ++;
		}
		else {
			// wrap araound to 1st menu entry
			menu->data.menu.selector_pos = 0;
			menu->data.menu.scroll = 0;
		}	
		return MENURESULT_NONE;
	  case MENUTOKEN_LEFT:
		if (!extended)
			return MENURESULT_NONE;

		subitem = menu_get_subitem(menu, menu->data.menu.selector_pos);
		if (subitem == NULL)
			break;
		switch (subitem->type) {
		  case MENUITEM_CHECKBOX:
			/* note: this dangerous looking code works since
			 * CheckboxValue is an enum >= 0. */
			if (subitem->data.checkbox.allow_gray) {
				subitem->data.checkbox.value = (subitem->data.checkbox.value - 1) % 3;
			}
			else {
				subitem->data.checkbox.value = (subitem->data.checkbox.value - 1) % 2;
			}
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  case MENUITEM_RING:
			/* ring: jump to the end if beginning is reached */
			subitem->data.ring.value = (subitem->data.ring.value < 1)
				? LL_Length(subitem->data.ring.strings) - 1
				: (subitem->data.ring.value - 1) % LL_Length(subitem->data.ring.strings);
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  default:
			break;
		}
		return MENURESULT_NONE;
	  case MENUTOKEN_RIGHT:
		if (!extended)
			return MENURESULT_NONE;

		subitem = menu_get_subitem(menu, menu->data.menu.selector_pos);
		if (subitem == NULL)
			break;
		switch (subitem->type) {
		  case MENUITEM_CHECKBOX:
			if (subitem->data.checkbox.allow_gray) {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 3;
			}
			else {
				subitem->data.checkbox.value = (subitem->data.checkbox.value + 1) % 2;
			}
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  case MENUITEM_RING:
			subitem->data.ring.value = (subitem->data.ring.value + 1) % LL_Length(subitem->data.ring.strings);
			if (subitem->event_func)
				subitem->event_func(subitem, MENUEVENT_UPDATE);
			return MENURESULT_NONE;
		  case MENUITEM_MENU:
			return MENURESULT_ENTER;
		  default:
			break;
		}
		return MENURESULT_NONE;
	  case MENUTOKEN_OTHER:
		/* TODO: move to the selected number and enter it */
		return MENURESULT_NONE;
	}
	return MENURESULT_ERROR;
}

/** positions current item pointer on subitem subitem_id. If subitem_id is
 * hidden or not valid subitem of menu this function does nothing. */
void menu_select_subitem(Menu *menu, char *subitem_id)
{
	assert(menu != NULL);
	debug(RPT_DEBUG, "%s(menu=[%s], subitem_id=\"%s\")", __FUNCTION__,
	       menu->id, subitem_id);
	int position = menu_get_index_of(menu, subitem_id);
	if (position < 0)
	{
		debug(RPT_DEBUG, "%s: subitem \"%s\" not found"
		      " or hidden in \"%s\", ignored",
		      __FUNCTION__, subitem_id, menu->id);
		return;
	}
	// debug(RPT_DEBUG, "%s: %s->%s is at position %d,"
	//       " current item is at menu position: %d, scroll: %d",
	//       __FUNCTION__, menu->id, subitem_id, position,
	//       menu->data.menu.selector_pos, menu->data.menu.scroll);
	menu->data.menu.selector_pos = position;
	menu->data.menu.scroll = position;
}
