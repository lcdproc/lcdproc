/*
 * menuscreens.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * Creates the server menu screen(s) and creates the menus that should be
 * displayed on this screen.
 * It also handles its keypresses and converts them to menu tokens for
 * easier processing.
 *
 * NOTE: menuscreens.c does not know whether a menuitem is displayed INSIDE
 * a menu or on a separate SCREEN, for flexibility.
 *
 */

#include <string.h>
#include <unistd.h>

#include "screen.h"
#include "screenlist.h"
#include "menuscreens.h"
#include "shared/configfile.h"
#include "shared/report.h"
#include "input.h"
#include "driver.h"
#include "drivers.h"

/* Next include files are needed for settings that we can modify */
#include "render.h"

char * menu_key;
char * enter_key;
char * up_key;
char * down_key;

Screen * menuscreen;
MenuItem * active_menuitem;
Menu * main_menu;
Menu * screens_menu;

/* Local prototypes */
void menuscreen_switch_item (MenuItem * new_menuitem);
void menuscreen_create_menu ();
MenuEventFunc (heartbeat_handler);
MenuEventFunc (backlight_handler);
MenuEventFunc (contrast_handler);
MenuEventFunc (brightness_handler);

int init_menu()
{
	active_menuitem = NULL;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	/* Get keys from config file */
	menu_key = strdup (config_get_string ("menu", "MenuKey", 0, "Menu"));
	enter_key = strdup (config_get_string ("menu", "EnterKey", 0, "Enter"));
	up_key = strdup (config_get_string ("menu", "UpKey", 0, "Up"));
	down_key = strdup (config_get_string ("menu", "DownKey", 0, "Down"));

	/* Now reserve keys */
	input_reserve_key (menu_key, true, NULL);
	input_reserve_key (enter_key, false, NULL);
	input_reserve_key (up_key, false, NULL);
	input_reserve_key (down_key, false, NULL);

	/* Create screen */
	menuscreen = screen_create ("_menu_screen", NULL);

	/* Build menu */
	menuscreen_create_menu ();

	return 0;
}


void menuscreen_inform_item_destruction (MenuItem * item)
{
	MenuItem * i;

	/* Are we currently in (a subitem of) the given item ? */
	for( i = active_menuitem; i; i = i->parent ) {
		if( i == item ) {
			menuscreen_switch_item (item->parent);
		}
	}
}

void menuscreen_inform_item_modified (MenuItem * item)
{
	/* Are we currently in the item or the parent of the item ? */
	if( active_menuitem == item || active_menuitem == item->parent ) {
		menuitem_rebuild_screen( active_menuitem, menuscreen );
	}
}

bool is_menu_key (char * key)
{
	if (strcmp (key, menu_key) == 0)
		return true;
	else
		return false;
}

void menuscreen_switch_item (MenuItem * new_menuitem)
/* This function changes the menuitem to the given one, and does necesary
 * actions.
 * The item will not be reset when the new item is a child of the last one.
 */
{
	MenuItem * old_menuitem = active_menuitem;

	/* First we do the switch */
	active_menuitem = new_menuitem;

	/* What was the state change ? */
	if (old_menuitem && !new_menuitem) {
		/* Menu is being quit */

		/* TODO: send menu to backgr */
		if (screenlist_remove (menuscreen) < 0) {
			report (RPT_ERR, "%s: Error unqueueing menu screen", __FUNCTION__);
		}
	} else if (!old_menuitem && new_menuitem) {
		/* Menu is becoming active */
		menuitem_reset (active_menuitem);
		menuitem_rebuild_screen (active_menuitem, menuscreen);

		if (screenlist_add (menuscreen) < 0) {
			report (RPT_ERR, "%s: Error queueing menu screen", __FUNCTION__);
		}
		/* TODO: raise it ! */
		return;
	} else {
		/* We're left with the usual case: a menu level switch */
		if( old_menuitem->parent != new_menuitem) {
			menuitem_reset (new_menuitem);
		}
		menuitem_rebuild_screen (active_menuitem, menuscreen);
	}
}

void menuscreen_key_handler (char *key)
{
	char token = 0;
	MenuResult res;

	debug (RPT_DEBUG, "%s( \"%s\" )", __FUNCTION__, key);

	if (strcmp (key, menu_key) == 0) {
		token = MENUTOKEN_MENU;
	}
	else if (strcmp (key, enter_key) == 0) {
		token = MENUTOKEN_ENTER;
	}
	else if (strcmp (key, up_key) == 0) {
		token = MENUTOKEN_UP;
	}
	else if (strcmp (key, down_key) == 0) {
		token = MENUTOKEN_DOWN;
	}
	else {
		token = MENUTOKEN_OTHER;
	}

	/* Is the menu already active ? */
	if (!active_menuitem) {
		debug (RPT_DEBUG, "%s: Activating menu screen", __FUNCTION__);
		menuscreen_switch_item (main_menu);
		return;
	}

	res = menuitem_process_input (active_menuitem, token, key);

	switch (res) {
	  case MENURESULT_ERROR:
		report (RPT_ERR, "%s: Error from menu_handle_input", __FUNCTION__);
		break;
	  case MENURESULT_NONE:
		if (active_menuitem) {
			menuitem_update_screen (active_menuitem, menuscreen);
			/* No rebuild needed, only value can be changed */
		}
		/* Nothing extra to be done */
		break;
	  case MENURESULT_ENTER:
		/* Enter the selected menuitem
		 * Note: this is not for checkboxes etc that don't have their
		 *   own screen. The menu_handle_input function should do
		 *   things like toggling checkboxes !
		 */
		debug (RPT_DEBUG, "%s: Entering subitem", __FUNCTION__);
		menuscreen_switch_item (menu_get_current_item (active_menuitem));
		break;
	  case MENURESULT_CLOSE:
		debug (RPT_DEBUG, "%s: Closing item", __FUNCTION__);
		menuscreen_switch_item (active_menuitem->parent);
		break;
	  case MENURESULT_QUIT:
		debug (RPT_DEBUG, "%s: Closing menu screen", __FUNCTION__);
		menuscreen_switch_item (NULL);
		break;
	}
}

void menuscreen_create_menu ()
{
	Menu * options_menu;
	Menu *   driver_menu;
	MenuItem * checkbox;
	MenuItem * slider;
	Driver * driver;

	MenuItem * test_item;
	Menu * test_menu;

	debug (RPT_DEBUG, "%s()", __FUNCTION__);

	main_menu = menu_create ("mainmenu", NULL, "LCDproc Menu", NULL);

	options_menu = menu_create ("options", NULL, "Options", NULL);
	menu_add_item (main_menu, options_menu);

	screens_menu = menu_create ("screens", NULL, "Screens", NULL);
	menu_add_item (main_menu, screens_menu);

	checkbox = menuitem_create_checkbox ("heartbeat", heartbeat_handler, "Heartbeat", true, heartbeat);
	menu_add_item (options_menu, checkbox);

	checkbox = menuitem_create_checkbox ("backlight", backlight_handler, "Backlight", true, backlight);
	menu_add_item (options_menu, checkbox);

	for (driver = drivers_getfirst(); driver; driver = drivers_getnext()) {
		int contrast_avail = 0;
		int brightness_avail = 0;

		contrast_avail = (driver->get_contrast && driver->set_contrast);
		brightness_avail = (driver->get_brightness && driver->set_brightness);

		if (contrast_avail || brightness_avail) {
			driver_menu = menu_create (driver->name, NULL, driver->name, driver);
			menu_add_item (options_menu, driver_menu);
			if (contrast_avail) {
				slider = menuitem_create_slider ("contrast", contrast_handler, "Contrast", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);
			}
			if (brightness_avail) {
				slider = menuitem_create_slider ("onbrightness", brightness_handler, "On Brightness", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);

				slider = menuitem_create_slider ("offbrightness", brightness_handler, "Off Brightness", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);
			}
		}
	}
	test_menu = menu_create ("test", NULL, "Test menu", NULL);
	menu_add_item (main_menu, test_menu);

	test_item = menuitem_create_action ("", NULL, "Action", MENURESULT_NONE);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_action ("", NULL, "Action,closing", MENURESULT_CLOSE);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_action ("", NULL, "Action,quiting", MENURESULT_QUIT);
	menu_add_item (test_menu, test_item);

	test_item = menuitem_create_checkbox ("", NULL, "Checkbox", false, false);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_checkbox ("", NULL, "Checkbox, gray", true, false);
	menu_add_item (test_menu, test_item);

	test_item = menuitem_create_ring ("", NULL, "Ring", "ABC\tDEF\t01234567890\tOr a very long string that will not fit on any display", 1);
	menu_add_item (test_menu, test_item);

	test_item = menuitem_create_slider ("", NULL, "Slider", "mintext", "maxtext", -20, 20, 1, 0);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_slider ("", NULL, "Slider,step=5", "mintext", "maxtext", -20, 20, 5, 0);
	menu_add_item (test_menu, test_item);

	test_item = menuitem_create_numeric ("", NULL, "Numeric", 1, 365, 15);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_numeric ("", NULL, "Numeric,signed", -20, +20, 15);
	menu_add_item (test_menu, test_item);

	test_item = menuitem_create_alpha ("", NULL, "Alpha", 0, 3, 12, true, true, true, ".-+@", "LCDproc-v0.5");
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_alpha ("", NULL, "Alpha, caps only", 0, 3, 12, true, false, false, "-", "LCDPROC");
	menu_add_item (test_menu, test_item);
}

MenuEventFunc (heartbeat_handler)
{
	debug (RPT_DEBUG, "%s( item=%s, event=%d )", __FUNCTION__, item->id, event);

	if (event == MENUEVENT_UPDATE) {
		/* Set heartbeat setting */
		heartbeat = item->data.checkbox.value;
		report (RPT_INFO, "Menu: set heartbeat to %d",
				item->data.checkbox.value);
	}
	return 0;
}

MenuEventFunc (backlight_handler)
{
	debug (RPT_DEBUG, "%s( item=%s, event=%d )", __FUNCTION__, item->id, event);

	if (event == MENUEVENT_UPDATE)
	{
		/* Set backlight setting */
		backlight = item->data.checkbox.value;
		report (RPT_INFO, "Menu: set backlight to %d",
				item->data.checkbox.value);
	}
	return 0;
}

MenuEventFunc (contrast_handler)
{
	debug (RPT_DEBUG, "%s( item=%s, event=%d )", __FUNCTION__, item->id, event);

	/* This function can be called by one of several drivers that
	 * support contrast !
	 * We need to check the menu association to see which driver. */
	if (event == MENUEVENT_MINUS || event == MENUEVENT_PLUS) {

		/* Determinte the driver */
		Driver * driver = item->parent->data.menu.association;

		driver->set_contrast (driver, item->data.slider.value);
		//item->data.slider.value = driver->get_contrast (driver);
		report (RPT_INFO, "Menu: set contrast of [%.40s] to %d",
				item->data.checkbox.value);
	}
	return 0;
}

MenuEventFunc (brightness_handler)
{
	debug (RPT_DEBUG, "%s( item=%s, event=%d )", __FUNCTION__, item->id, event);

	/* This function can be called by one of several drivers that
	 * support contrast !
	 * We need to check the menu association to see which driver. */
	if (event == MENUEVENT_MINUS || event == MENUEVENT_PLUS) {

		/* Determinte the driver */
		Driver * driver = item->parent->data.menu.association;

		if ( strcmp (item->id, "onbrightness") == 0) {
			driver->set_brightness (driver, BACKLIGHT_ON, item->data.slider.value);
		}
		else if ( strcmp (item->id, "offbrightness") == 0) {
			driver->set_brightness (driver, BACKLIGHT_OFF, item->data.slider.value);
		}
	}
	return 0;
}

void
menuscreen_add_screen (Screen * s)
{
	Menu * m;
	MenuItem * mi;

	debug (RPT_DEBUG, "%s( Screen=\"%s\" )", __FUNCTION__, s->id);

	if (!screens_menu)
		return;	/* When screens have not been created ... */

	/* Create a menu entry for the screen */
	m = menu_create (s->id, NULL, s->name?s->name:s->id, s);
	menu_add_item (screens_menu, m);

	/* And add some items for it... */
	mi = menuitem_create_action ("", NULL, "(don't work yet)", MENURESULT_NONE);
	menu_add_item (m, mi);

	mi = menuitem_create_action ("", NULL, "To Front", MENURESULT_QUIT);
	menu_add_item (m, mi);

	mi = menuitem_create_checkbox ("", NULL, "Visible", false, true);
	menu_add_item (m, mi);

	mi = menuitem_create_numeric ("", NULL, "Duration", 2, 3600, s->duration);
	menu_add_item (m, mi);

	mi = menuitem_create_numeric ("", NULL, "Priority", 0, 255, s->priority);
	menu_add_item (m, mi);
}

void
menuscreen_remove_screen (Screen * s)
{
	Menu * m;

	debug (RPT_DEBUG, "%s( Screen=\"%s\" )", __FUNCTION__, s->id);

	m = menu_find_item (screens_menu, s->id, false);
	menu_remove_item (screens_menu, m);
	menuitem_destroy (m);
}
