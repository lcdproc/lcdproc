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
#include "configfile.h"
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


void menuscreen_create_menu ();
MenuEventFunc (menuscreen_heartbeat);
MenuEventFunc (menuscreen_backlight);
MenuEventFunc (menuscreen_contrast);
MenuEventFunc (menuscreen_brightness);
MenuEventFunc (menuscreen_screens);

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

bool is_menu_key (char * key)
{
	if (strcmp (key, menu_key) == 0)
		return true;
	else
		return false;
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
		active_menuitem = main_menu;
		menuitem_build_screen (active_menuitem, menuscreen);
		menuitem_reset (active_menuitem);
		menuitem_update_screen (active_menuitem, menuscreen);

		if (screenlist_add (menuscreen) < 0) {
			report (RPT_ERR, "%s: Error queueing menu screen", __FUNCTION__);
		}
		/* TODO: raise it ! */

		return;
	}

	res = menuitem_handle_input (active_menuitem, token, key);

	switch (res) {
	  case MENURESULT_ERROR:
		report (RPT_ERR, "%s: Error from menu_handle_input", __FUNCTION__);
		break;
	  case MENURESULT_OK:
		/* Nothing extra to be done */
		break;
	  case MENURESULT_ENTER:
	  	/* Enter the selected menuitem
	  	 * Note: this is not for checkboxes etc that don't have their
	  	 *   own screen. The menu_handle_input function should do
	  	 *   things like toggling checkboxes !
	  	 */
		debug (RPT_DEBUG, "%s: Entering subitem", __FUNCTION__);
	  	active_menuitem = menu_get_current_item (active_menuitem);
	  	menuitem_build_screen (active_menuitem, menuscreen);
		menuitem_reset (active_menuitem);
		break;
	  case MENURESULT_CLOSE:
		debug (RPT_DEBUG, "%s: Closing item", __FUNCTION__);
		active_menuitem = menuitem_get_parent (active_menuitem);
		if (active_menuitem) {
			/* We were in at least second level menu */
		  	menuitem_build_screen (active_menuitem, menuscreen);
			break;
		}
		/* If first level menu, quit menu now
		 * Therefor no break; now.
		 */
	  case MENURESULT_QUIT:
		debug (RPT_DEBUG, "%s: Closing menu screen", __FUNCTION__);
		active_menuitem = NULL;
		/* TODO: send menu to backgr */
		if (screenlist_remove (menuscreen) < 0) {
			report (RPT_ERR, "%s: Error unqueueing menu screen", __FUNCTION__);
		}
		break;
	}
	if (active_menuitem) {
		menuitem_update_screen (active_menuitem, menuscreen);
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

	screens_menu = menu_create ("screens", menuscreen_screens, "Screens", NULL);
	menu_add_item (main_menu, screens_menu);

	checkbox = menuitem_create_checkbox ("heartbeat", menuscreen_heartbeat, "Heartbeat", true, heartbeat);
	menu_add_item (options_menu, checkbox);

	checkbox = menuitem_create_checkbox ("backlight", menuscreen_backlight, "Backlight", true, backlight);
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
				slider = menuitem_create_slider ("contrast", menuscreen_contrast, "Contrast", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);
			}
			if (brightness_avail) {
				slider = menuitem_create_slider ("onbrightness", menuscreen_brightness, "On Brightness", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);

				slider = menuitem_create_slider ("offbrightness", menuscreen_brightness, "Off Brightness", "min", "max", 0, 1000, 100, 500);
				menu_add_item (driver_menu, slider);
			}
		}
	}
	test_menu = menu_create ("test", NULL, "Test menu", NULL);
	menu_add_item (main_menu, test_menu);

	test_item = menuitem_create_action ("", NULL, "Action", false, false);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_action ("", NULL, "Action,closing", true, false);
	menu_add_item (test_menu, test_item);
	test_item = menuitem_create_action ("", NULL, "Action,quiting", true, true);
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

MenuEventFunc (menuscreen_heartbeat)
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

MenuEventFunc (menuscreen_backlight)
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

MenuEventFunc (menuscreen_contrast)
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

MenuEventFunc (menuscreen_brightness)
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

MenuEventFunc (menuscreen_screens)
{
	debug (RPT_DEBUG, "%s( item=%s, event=%d )", __FUNCTION__, item->id, event);

	if (event == MENUEVENT_ENTER) {
		/* We are entering this menu */
		Screen * s;
		Menu * m;
		MenuItem * mi;

		/* Clean the screens menu */
		menu_destroy_all_items (screens_menu);

		/* Read list of screens */
		for (s = LL_GetFirst(screenlist_getlist()); s; s = LL_GetNext(screenlist_getlist())) {

			/* TODO: don't display screens that have _ at start of id */

			m = menu_create (s->id, NULL, s->name?s->name:s->id, s);
			menu_add_item (screens_menu, m);

			mi = menuitem_create_action ("", NULL, "To Front", true, true);
			menu_add_item (m, mi);

			mi = menuitem_create_checkbox ("", NULL, "Visible", false, true);
			menu_add_item (m, mi);

			mi = menuitem_create_numeric ("", NULL, "Duration", 2, 3600, s->duration);
			menu_add_item (m, mi);

			mi = menuitem_create_numeric ("", NULL, "Priority", -1, 4, s->priority);
			menu_add_item (m, mi);
		}
	}
	return 0;
}
