/** \file server/menuscreens.c
 * Creates the server menu screen(s) and creates the menus that should be
 * displayed on this screen.
 * It also handles its key presses and converts them to menu tokens for
 * easier processing.
 *
 * \note
 * menuscreens.c does not know whether a menuitem is displayed INSIDE
 * a menu or on a separate SCREEN, for flexibility.
 */

/*-
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 */

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "screen.h"
#include "screenlist.h"
#include "menuscreens.h"
#include "shared/configfile.h"
#include "shared/report.h"
#include "input.h"
#include "driver.h"
#include "drivers.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Next include files are needed for settings that we can modify */
#include "render.h"


char *menu_key;
char *enter_key;
char *up_key;
char *down_key;
char *left_key;
char *right_key;
static unsigned int keymask;	/* mask of defined menu keys */

Screen *menuscreen = NULL;
MenuItem *active_menuitem = NULL;
/** the "real" main_menu */
Menu *main_menu = NULL;
/** customizable entry point into the menu system (see menuscreen_set_main()). */
Menu *custom_main_menu = NULL;
Menu *screens_menu = NULL;

/* Local prototypes */
static void handle_quit(void);
static void handle_close(void);
static void handle_none(void);
static void handle_enter(void);
static void handle_successor(void);
void menuscreen_switch_item(MenuItem *new_menuitem);
void menuscreen_create_menu(void);
#ifdef LCDPROC_TESTMENUS
void menuscreen_create_testmenu(void);
#endif
Menu *menuscreen_get_main(void);
MenuEventFunc(heartbeat_handler);
MenuEventFunc(backlight_handler);
MenuEventFunc(titlespeed_handler);
MenuEventFunc(contrast_handler);
MenuEventFunc(brightness_handler);

int
menuscreens_init(void)
{
	const char *tmp;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	/*
	 * Get keys from config file: MenuKey, EnterKey, UpKey, DownKey,
	 * LeftKey, RightKey. For a working menu at least 3 are necessary:
	 * MenuKey, EnterKey, UpKey/DownKey.
	 */
	keymask = 0;
	menu_key = enter_key = NULL;
	tmp = config_get_string("menu", "MenuKey", 0, NULL);
	if (tmp != NULL) {
		menu_key = strdup(tmp);
		keymask |= MENUTOKEN_MENU;
	}
	tmp = config_get_string("menu", "EnterKey", 0, NULL);
	if (tmp != NULL) {
		enter_key = strdup(tmp);
		keymask |= MENUTOKEN_ENTER;
	}

	up_key = down_key = NULL;
	tmp = config_get_string("menu", "UpKey", 0, NULL);
	if (tmp != NULL) {
		up_key = strdup(tmp);
		keymask |= MENUTOKEN_UP;
	}
	tmp = config_get_string("menu", "DownKey", 0, NULL);
	if (tmp != NULL) {
		down_key = strdup(tmp);
		keymask |= MENUTOKEN_DOWN;
	}

	left_key = right_key = NULL;
	tmp = config_get_string("menu", "LeftKey", 0, NULL);
	if (tmp != NULL) {
		left_key = strdup(tmp);
		keymask |= MENUTOKEN_LEFT;
	}
	tmp = config_get_string("menu", "RightKey", 0, NULL);
	if (tmp != NULL) {
		right_key = strdup(tmp);
		keymask |= MENUTOKEN_RIGHT;
	}

	/* Now reserve the keys that were defined */
	if (menu_key != NULL)
		input_reserve_key(menu_key, true, NULL);
	if (enter_key != NULL)
		input_reserve_key(enter_key, false, NULL);
	if (up_key != NULL)
		input_reserve_key(up_key, false, NULL);
	if (down_key != NULL)
		input_reserve_key(down_key, false, NULL);
	if (left_key != NULL)
		input_reserve_key(left_key, false, NULL);
	if (right_key != NULL)
		input_reserve_key(right_key, false, NULL);

	/* Create screen */
	menuscreen = screen_create("_menu_screen", NULL);
	if (menuscreen != NULL)
		menuscreen->priority = PRI_HIDDEN;
	active_menuitem = NULL;

	screenlist_add(menuscreen);

	/* Build menu */
	menuscreen_create_menu();

	return 0;
}


int
menuscreens_shutdown(void)
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	/* Program shutdown before completed startup */
	if (!menuscreen)
		return -1;

	/* Quit menu just to make sure */
	menuscreen_switch_item(NULL);

	/* Destroy the menuscreen */
	screenlist_remove(menuscreen);
	screen_destroy(menuscreen);
	menuscreen = NULL;

	/* Destroy all menus */
	menuitem_destroy(main_menu);
	main_menu = NULL;
	custom_main_menu = NULL;
	screens_menu = NULL;

	/* Forget menu's key reservations */
	input_release_client_keys(NULL);

	if (menu_key != NULL)
		free(menu_key);
	if (enter_key != NULL)
		free(enter_key);
	if (up_key != NULL)
		free(up_key);
	if (down_key != NULL)
		free(down_key);
	if (left_key != NULL)
		free(left_key);
	if (right_key != NULL)
		free(right_key);
	keymask = 0;

	return 0;
}


void
menuscreen_inform_item_destruction(MenuItem * item)
{
	MenuItem *i;

	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"));

	/* Are we currently in (a subitem of) the given item ? */
	for (i = active_menuitem; i != NULL; i = i->parent) {
		if (i == item) {
			menuscreen_switch_item(item->parent);
		}
	}
}

void
menuscreen_inform_item_modified(MenuItem * item)
{
	debug(RPT_DEBUG, "%s(item=[%s])", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"));

	if ((active_menuitem == NULL) || (item == NULL))
		return;

	/* Are we currently in the item or the parent of the item ? */
	if (active_menuitem == item || active_menuitem == item->parent) {
		menuitem_rebuild_screen(active_menuitem, menuscreen);
	}
}

bool
is_menu_key(const char *key)
{
	if ((menu_key != NULL) && (key != NULL) && (strcmp(key, menu_key) == 0))
		return true;
	else
		return false;
}

/** This function changes the menuitem to the given one, and does necessary
 * actions.
 * To leave the menu system, specify NULL for new_menuitem.
 * The item will not be reset when the new item is a child of the last one.
 */
void
menuscreen_switch_item(MenuItem * new_menuitem)
{
	MenuItem *old_menuitem = active_menuitem;

	debug(RPT_DEBUG, "%s(item=[%s]) from active_menuitem=[%s]", __FUNCTION__,
	      ((new_menuitem != NULL) ? new_menuitem->id : "(null)"),
	      ((old_menuitem != NULL) ? old_menuitem->id : "(null)"));

	/* First we do the switch */
	active_menuitem = new_menuitem;

	/* What was the state change ? */
	if (!old_menuitem && !new_menuitem) {
		/* Nothing to be done */
	}
	else if (old_menuitem && !new_menuitem) {
		/* leave menu system */
		menuscreen->priority = PRI_HIDDEN;
	}
	else if (!old_menuitem && new_menuitem) {
		/* Menu is becoming active */
		menuitem_reset(active_menuitem);
		menuitem_rebuild_screen(active_menuitem, menuscreen);

		menuscreen->priority = PRI_INPUT;
	}
	else {
		/* We're left with the usual case: a menu level switch */
		if (old_menuitem->parent != new_menuitem) {
			menuitem_reset(new_menuitem);
		}
		menuitem_rebuild_screen(active_menuitem, menuscreen);
	}

	if (old_menuitem && old_menuitem->event_func)
		old_menuitem->event_func(old_menuitem, MENUEVENT_LEAVE);
	if (new_menuitem && new_menuitem->event_func)
		new_menuitem->event_func(new_menuitem, MENUEVENT_ENTER);

	return;
}

static void
handle_quit(void)
{
	debug(RPT_DEBUG, "%s: Closing menu screen", __FUNCTION__);
	menuscreen_switch_item(NULL);
}

static void
handle_close(void)
{
	debug(RPT_DEBUG, "%s: Closing item", __FUNCTION__);
	menuscreen_switch_item(
			       (active_menuitem == menuscreen_get_main())
			       ? NULL
			       : active_menuitem->parent);
}

static void
handle_none(void)
{
	debug(RPT_DEBUG, "%s: Staying in item", __FUNCTION__);
	if (active_menuitem) {
		menuitem_update_screen(active_menuitem, menuscreen);
		/* No rebuild needed, only value can be changed */
	}
	/* Nothing extra to be done */
}

/** Enter the selected menuitem
 * Note: this is not for checkboxes etc that don't have their
 *   own screen. The menuitem_process_input function should do
 *   things like toggling checkboxes !
 */
static void
handle_enter(void)
{
	debug(RPT_DEBUG, "%s: Entering subitem", __FUNCTION__);
	menuscreen_switch_item(menu_get_current_item(active_menuitem));
}

static void
handle_predecessor(void)
{
	MenuItem *predecessor;
	MenuItem *item = (active_menuitem->type == MENUITEM_MENU)
		? menu_get_item_for_predecessor_check(active_menuitem)
		: active_menuitem;
	assert(item != NULL);
	debug(RPT_DEBUG, "%s: Switching to registered predecessor '%s' of '%s'.",
	      __FUNCTION__, item->predecessor_id, item->id);
	predecessor = menuitem_search(item->predecessor_id,
				      (Client *) active_menuitem->client);
	if (predecessor == NULL) {
		/*
		 * note: if _quit_, _close_, _none_ get here this would be an
		 * implementation error - they should have been handled via
		 * different MENURESULT codes.
		 */
		report(RPT_ERR, "%s: cannot find predecessor '%s' of '%s'.",
		       __FUNCTION__, item->predecessor_id, item->id);
		return;
	}
	switch (predecessor->type) {
	    case MENUITEM_ACTION:
	    case MENUITEM_CHECKBOX:
	    case MENUITEM_RING:
		if (active_menuitem != predecessor->parent)
			menuscreen_switch_item(predecessor->parent);
		/* this won't work for hidden subitems */
		menu_select_subitem(active_menuitem, item->predecessor_id);
		menuitem_update_screen(active_menuitem, menuscreen);
		break;
	    default:
		if ((predecessor->parent != NULL) &&
		    (predecessor->parent->type == MENUITEM_MENU)) {
			/* update parent menu too */
			menu_select_subitem(predecessor->parent, predecessor->id);
		}
		menuscreen_switch_item(predecessor);
		break;
	}
}

static void
handle_successor(void)
{
	MenuItem *successor;
	MenuItem *item = (active_menuitem->type == MENUITEM_MENU)
		? menu_get_item_for_successor_check(active_menuitem)
		: active_menuitem;
	assert(item != NULL);
	debug(RPT_DEBUG, "%s: Switching to registered successor '%s' of '%s'.",
	      __FUNCTION__, item->successor_id, item->id);
	successor = menuitem_search(item->successor_id,
				    (Client *) active_menuitem->client);
	if (successor == NULL) {
		/*
		 * note: if _quit_, _close_, _none_ get here this would be an
		 * implementation error - they should have been handled via
		 * different MENURESULT codes.
		 */
		report(RPT_ERR, "%s: cannot find successor '%s' of '%s'.",
		       __FUNCTION__, item->successor_id, item->id);
		return;
	}
	switch (successor->type) {
	    case MENUITEM_ACTION:
	    case MENUITEM_CHECKBOX:
	    case MENUITEM_RING:
		if (active_menuitem != successor->parent)
			menuscreen_switch_item(successor->parent);
		/* this won't work for hidden subitems */
		menu_select_subitem(active_menuitem, item->successor_id);
		menuitem_update_screen(active_menuitem, menuscreen);
		break;
	    default:
		if ((successor->parent != NULL) &&
		    (successor->parent->type == MENUITEM_MENU)) {
			/* update parent menu too */
			menu_select_subitem(successor->parent, successor->id);
		}
		menuscreen_switch_item(successor);
		break;
	}
}

void
menuscreen_key_handler(const char *key)
{
	MenuToken token = MENUTOKEN_NONE;
	MenuResult res;

	debug(RPT_DEBUG, "%s(\"%s\")", __FUNCTION__, key);

	if ((menu_key != NULL) && (strcmp(key, menu_key) == 0)) {
		token = MENUTOKEN_MENU;
	}
	else if ((enter_key != NULL) && (strcmp(key, enter_key) == 0)) {
		token = MENUTOKEN_ENTER;
	}
	else if ((up_key != NULL) && (strcmp(key, up_key) == 0)) {
		token = MENUTOKEN_UP;
	}
	else if ((down_key != NULL) && (strcmp(key, down_key) == 0)) {
		token = MENUTOKEN_DOWN;
	}
	else if ((left_key != NULL) && (strcmp(key, left_key) == 0)) {
		token = MENUTOKEN_LEFT;
	}
	else if ((right_key != NULL) && (strcmp(key, right_key) == 0)) {
		token = MENUTOKEN_RIGHT;
	}
	else {
		token = MENUTOKEN_OTHER;
	}

	/* Is the menu already active ? */
	if (!active_menuitem) {
		debug(RPT_DEBUG, "%s: Activating menu screen", __FUNCTION__);
		menuscreen_switch_item(menuscreen_get_main());
		return;
	}

	res = menuitem_process_input(active_menuitem, token, key, keymask);
	switch (res) {
	    case MENURESULT_ERROR:
		report(RPT_ERR, "%s: Error from menuitem_process_input", __FUNCTION__);
		break;
	    case MENURESULT_NONE:
		handle_none();
		break;
	    case MENURESULT_ENTER:
		handle_enter();
		break;
	    case MENURESULT_CLOSE:
		handle_close();
		break;
	    case MENURESULT_QUIT:
		handle_quit();
		break;
	    case MENURESULT_PREDECESSOR:
		handle_predecessor();
		break;
	    case MENURESULT_SUCCESSOR:
		handle_successor();
		break;
	    default:
		assert(!"unexpected menuresult");
		break;
	}
}

void
menuscreen_create_menu(void)
{
	Menu *options_menu;
	Menu *driver_menu;
	MenuItem *checkbox;
	MenuItem *slider;
	Driver *driver;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	main_menu = menu_create("mainmenu", NULL, "LCDproc Menu", NULL);
	if (main_menu == NULL) {
		report(RPT_ERR, "%s: Cannot create main menu", __FUNCTION__);
		return;
	}

	options_menu = menu_create("options", NULL, "Options", NULL);
	if (options_menu == NULL) {
		report(RPT_ERR, "%s: Cannot create options menu", __FUNCTION__);
		return;
	}
	menu_add_item(main_menu, options_menu);

#ifdef LCDPROC_TESTMENUS
	/*
	 * TODO: Menu items in the screens menu currently have no functions
	 * assigned. Therefore only enable the menu for testing. If functions
	 * are available, this code should be outside the #ifdef.
	 */
	screens_menu = menu_create("screens", NULL, "Screens", NULL);
	if (screens_menu == NULL) {
		report(RPT_ERR, "%s: Cannot create screens menu", __FUNCTION__);
		return;
	}
	menu_add_item(main_menu, screens_menu);

	menuscreen_create_testmenu();
#endif

	/*
	 * add option menu contents: menu's client is NULL since we're in the
	 * server
	 */
	checkbox = menuitem_create_checkbox("heartbeat", heartbeat_handler, "Heartbeat", NULL, true, heartbeat);
	menu_add_item(options_menu, checkbox);

	checkbox = menuitem_create_checkbox("backlight", backlight_handler, "Backlight", NULL, true, backlight);
	menu_add_item(options_menu, checkbox);

	slider = menuitem_create_slider("titlespeed", titlespeed_handler,
					"TitleSpeed", NULL, "0", "10", TITLESPEED_NO, TITLESPEED_MAX, 1, titlespeed);
	menu_add_item(options_menu, slider);

	/*
	 * add driver specific option menus for each driver: menu's client is
	 * NULL since we're in the server
	 */
	for (driver = drivers_getfirst(); driver; driver = drivers_getnext()) {
		int contrast_avail = (driver->get_contrast && driver->set_contrast) ? 1 : 0;
		int brightness_avail = (driver->get_brightness && driver->set_brightness) ? 1 : 0;

		if (contrast_avail || brightness_avail) {
			/* menu's client is NULL since we're in the server */
			driver_menu = menu_create(driver->name, NULL, driver->name, NULL);
			if (driver_menu == NULL) {
				report(RPT_ERR, "%s: Cannot create menu for driver %s",
				       __FUNCTION__, driver->name);
				continue;
			}
			menu_set_association(driver_menu, driver);
			menu_add_item(options_menu, driver_menu);
			if (contrast_avail) {
				int contrast = driver->get_contrast(driver);

				/* menu's client is NULL since we're in the server */
				slider = menuitem_create_slider("contrast", contrast_handler, "Contrast",
								NULL, "min", "max", 0, 1000, 25, contrast);
				menu_add_item(driver_menu, slider);
			}
			if (brightness_avail) {
				int onbrightness = driver->get_brightness(driver, BACKLIGHT_ON);
				int offbrightness = driver->get_brightness(driver, BACKLIGHT_OFF);

				slider = menuitem_create_slider("onbrightness", brightness_handler, "On Brightness",
								NULL, "min", "max", 0, 1000, 25, onbrightness);
				menu_add_item(driver_menu, slider);

				slider = menuitem_create_slider("offbrightness", brightness_handler, "Off Brightness",
								NULL, "min", "max", 0, 1000, 25, offbrightness);
				menu_add_item(driver_menu, slider);
			}
		}
	}
}

#ifdef LCDPROC_TESTMENUS
void
menuscreen_create_testmenu(void)
{
	MenuItem *test_item;
	Menu *test_menu;

	char testiso[] = {
		'D', 'e', 'm', 'o', '\t',
		/* #160 */
		160, 161, 162, 163, 164, 165, 166, 167, '\t',
		168, 169, 170, 171, 172, 173, 174, 175, '\t',
		176, 177, 178, 179, 180, 181, 182, 183, '\t',
		184, 185, 186, 187, 188, 189, 190, 191, '\t',
		/* #192 */
		192, 193, 194, 195, 196, 197, 198, 199, '\t',
		200, 201, 202, 203, 204, 205, 206, 207, '\t',
		208, 209, 210, 211, 212, 213, 214, 215, '\t',
		216, 217, 218, 219, 220, 221, 222, 223, '\t',
		/* #224 */
		224, 225, 226, 227, 228, 229, 230, 231, '\t',
		232, 233, 234, 235, 236, 237, 238, 239, '\t',
		240, 241, 242, 243, 244, 245, 246, 247, '\t',
		248, 249, 250, 251, 252, 253, 254, 255, '\0'
	};

	test_menu = menu_create("test", NULL, "Test menu", NULL);
	if (test_menu == NULL) {
		report(RPT_ERR, "%s: Cannot create test menu", __FUNCTION__);
		return;
	}
	menu_add_item(main_menu, test_menu);

	/* menu's client is NULL since we're in the server */
	test_item = menuitem_create_action("", NULL, "Action", NULL, MENURESULT_NONE);
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_action("", NULL, "Action,closing", NULL, MENURESULT_CLOSE);
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_action("", NULL, "Action,quitting", NULL, MENURESULT_QUIT);
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_checkbox("", NULL, "Checkbox", NULL, false, false);
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_checkbox("", NULL, "Checkbox, gray", NULL, true, false);
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_ring("", NULL, "Ring", NULL, "ABC\tDEF\t01234567890\tOr a very long string that will not fit on any display", 1);
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_slider("", NULL, "Slider", NULL, "mintext", "maxtext", -20, 20, 1, 0);
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_slider("", NULL, "Slider,step=5", NULL, "mintext", "maxtext", -20, 20, 5, 0);
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_numeric("", NULL, "Numeric", NULL, 1, 365, 15);
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_numeric("", NULL, "Numeric,signed", NULL, -20, +20, 15);
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_alpha("", NULL, "Alpha", NULL, 0, 3, 12, true, true, true, ".-+@", "LCDproc-v0.5");
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_alpha("", NULL, "Alpha, caps only", NULL, 0, 3, 12, true, false, false, "-", "LCDPROC");
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_ip("", NULL, "IPv4", NULL, false, "192.168.1.245");
	menu_add_item(test_menu, test_item);
	test_item = menuitem_create_ip("", NULL, "IPv6", NULL, true, "1080:0:0:0:8:800:200C:417A");
	menu_add_item(test_menu, test_item);

	test_item = menuitem_create_ring("", NULL, "Charset", NULL, testiso, 0);
	menu_add_item(test_menu, test_item);
}
#endif /* LCDPROC_TESTMENUS */

MenuEventFunc(heartbeat_handler)
{
	debug(RPT_DEBUG, "%s(item=[%s], event=%d)", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"), event);

	if ((item != NULL) && (event == MENUEVENT_UPDATE)) {
		/* Set heartbeat setting */
		heartbeat = item->data.checkbox.value;
		report(RPT_INFO, "Menu: set heartbeat to %d", heartbeat);
	}
	return 0;
}

MenuEventFunc(backlight_handler)
{
	debug(RPT_DEBUG, "%s(item=[%s], event=%d)", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"), event);

	if ((item != NULL) && (event == MENUEVENT_UPDATE)) {
		/* Set backlight setting */
		backlight = item->data.checkbox.value;
		report(RPT_INFO, "Menu: set backlight to %d", backlight);
	}
	return 0;
}

MenuEventFunc(titlespeed_handler)
{
	debug(RPT_DEBUG, "%s(item=[%s], event=%d)", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"), event);

	if ((item != NULL) && ((event == MENUEVENT_MINUS) || (event == MENUEVENT_PLUS))) {
		/* set titlespeed setting */
		titlespeed = item->data.slider.value;
		report(RPT_INFO, "Menu: set titlespeed to %d", titlespeed);
	}
	return 0;
}

MenuEventFunc(contrast_handler)
{
	debug(RPT_DEBUG, "%s(item=[%s], event=%d)", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"), event);

	/*
	 * This function can be called by one of several drivers that support
	 * contrast
	 */
	if ((item != NULL) && ((event == MENUEVENT_MINUS) || (event == MENUEVENT_PLUS))) {
		/* Determine the driver by following the menu's association */
		Driver *driver = item->parent->data.menu.association;

		if (driver != NULL) {
			driver->set_contrast(driver, item->data.slider.value);
			report(RPT_INFO, "Menu: set contrast of [%.40s] to %d",
			       driver->name, item->data.slider.value);
		}
	}
	return 0;
}

MenuEventFunc(brightness_handler)
{
	debug(RPT_DEBUG, "%s(item=[%s], event=%d)", __FUNCTION__,
	      ((item != NULL) ? item->id : "(null)"), event);

	/*
	 * This function can be called by one of several drivers that support
	 * brightness !
	 */
	if ((item != NULL) && ((event == MENUEVENT_MINUS) || (event == MENUEVENT_PLUS))) {
		/* Determine the driver by following the menu's association */
		Driver *driver = item->parent->data.menu.association;

		if (driver != NULL) {
			if (strcmp(item->id, "onbrightness") == 0) {
				driver->set_brightness(driver, BACKLIGHT_ON, item->data.slider.value);
			}
			else if (strcmp(item->id, "offbrightness") == 0) {
				driver->set_brightness(driver, BACKLIGHT_OFF, item->data.slider.value);
			}
		}
	}
	return 0;
}

void
menuscreen_add_screen(Screen *s)
{
	Menu *m;
	MenuItem *mi;

	debug(RPT_DEBUG, "%s(s=[%s])", __FUNCTION__,
	      ((s != NULL) ? s->id : "(null)"));

	/* screens have not been created or no screen given ... */
	if ((screens_menu == NULL) || (s == NULL))
		return;

	/* Create a menu entry for the screen */
	m = menu_create(s->id, NULL, ((s->name != NULL) ? s->name : s->id), s->client);
	if (m == NULL) {
		report(RPT_ERR, "%s: Cannot create menu", __FUNCTION__);
		return;
	}
	menu_set_association(m, s);
	menu_add_item(screens_menu, m);

	/* And add some items for it... */
	mi = menuitem_create_action("", NULL, "(don't work yet)", s->client, MENURESULT_NONE);
	menu_add_item(m, mi);

	mi = menuitem_create_action("", NULL, "To Front", s->client, MENURESULT_QUIT);
	menu_add_item(m, mi);

	mi = menuitem_create_checkbox("", NULL, "Visible", s->client, false, true);
	menu_add_item(m, mi);

	mi = menuitem_create_numeric("", NULL, "Duration", s->client, 2, 3600, s->duration);
	menu_add_item(m, mi);

	mi = menuitem_create_ring("", NULL, "Priority", s->client,
	       "Hidden\tBackground\tForeground\tAlert\tInput", s->priority);
	menu_add_item(m, mi);
}


void
menuscreen_remove_screen(Screen *s)
{
	debug(RPT_DEBUG, "%s(s=[%s])", __FUNCTION__,
	      (s != NULL) ? s->id : "(NULL)");

	/* allow to remove the menuscreen itself */
	if ((s == NULL) || (s == menuscreen))
		return;

	if (screens_menu) {
		Menu *m = menu_find_item(screens_menu, s->id, false);

		menu_remove_item(screens_menu, m);
		menuitem_destroy(m);
	}
}

int
menuscreen_goto(Menu * menu)
{
	debug(RPT_DEBUG, "%s(m=[%s]): active_menuitem=[%s]",
	      __FUNCTION__, (menu != NULL) ? menu->id : "(NULL)",
	      (active_menuitem != NULL) ? active_menuitem->id : "(NULL)");
	menuscreen_switch_item(menu);
	return 0;
}

/** sets custom main menu. Use NULL pointer to reset it to the "real" main
 * menu. */
int
menuscreen_set_main(Menu * menu)
{
	debug(RPT_DEBUG, "%s(m=[%s])",
	      __FUNCTION__, (menu != NULL) ? menu->id : "(NULL)");
	custom_main_menu = menu;
	return 0;
}

Menu *
menuscreen_get_main(void)
{
	return custom_main_menu ? custom_main_menu : main_menu;
}
