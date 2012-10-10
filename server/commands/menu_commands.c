/** \file server/commands/menu_commands.c
 * Implements handlers for client commands concerning menus.
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 *               2004, F5 Networks, Inc. - IP-address input
 *               2005, Peter Marschall - error checks, ...
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "shared/report.h"
#include "shared/sockets.h"

#include "client.h"
#include "menuitem.h"
#include "menu.h"
#include "menuscreens.h"
#include "menu_commands.h"

/* Local functions */
MenuEventFunc(menu_commands_handler);
int set_predecessor(MenuItem *item, char *itemid, Client *client);
int set_successor(MenuItem *item, char *itemid, Client *client);


/** small utility for debug output of command line. */
static char *argv2string(int argc, char **argv)
{
	char *rtn = NULL;
	unsigned int len;
	unsigned int i;
	for (i = len = 0; i < argc; i++)
		len += strlen(argv[i]) + 1;
	rtn = malloc(len + 1);
	if (rtn != NULL) {
		rtn[0] = '\0';
		for (i = 0; i < argc; i++) {
			strcat(rtn, argv[i]);
			strcat(rtn, " ");
		}
	}
	return rtn;
}


/**
 * Adds an item to a menu.
 *
 *\verbatim
 * Usage: menu_add_item <menuid> <newitemid> <type> [<text>] {<option>}+
 *\endverbatim
 *
 * You should use "" as id for the client's main menu. This menu will be
 * created automatically when you add an item to it the first time.
 *
 * You cannot create a menu in the main level yourself, unless you replace the
 * main menu with the client's menu.
 * The names you use for items should be unique for your client.
 * The text is the visible text for the item.
 *
 * The following types are available:
 * - menu
 * - action
 * - checkbox
 * - ring (a kind of listbox of one line)
 * - slider
 * - numeric
 * - alpha
 * - ip
 *
 * For the list of supported options see menu_set_item_func.
 */
int
menu_add_item_func(Client *c, int argc, char **argv)
{
	char *menu_id;
	char *item_id;
	char *text = NULL;
	Menu *menu = NULL;
	MenuItem *item;
	MenuItemType itemtype;

	debug(RPT_DEBUG, "%s(Client [%d], %s, %s)",
	       __FUNCTION__, c->sock, argv[1], argv[2]);
	if (c->state != ACTIVE)
		return 1;

	if (c->name == NULL) {
		sock_send_error(c->sock, "You need to give your client a name first\n");
		return 0;
	}

	if (argc < 4) {
		sock_send_error(c->sock, "Usage: menu_add_item <menuid> <newitemid> <type> [<text>] [<option>]+\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	/* Does the client have a menu already ? */
	if (c->menu == NULL) {
		/* We need to create it */
		report(RPT_INFO, "Client [%d] is using the menu", c->sock);
		c->menu = menu_create("_client_menu_", menu_commands_handler, c->name, c);
		if (c->menu == NULL) {
			sock_send_error(c->sock, "Cannot create menu\n");
			return 1;
		}
		menu_add_item(main_menu, c->menu);
	}

	/* use either the given menu or the client's main menu if none was specified */
	menu = (menu_id[0] != '\0')
	       ? menu_find_item(c->menu, menu_id, true)
	       : c->menu;
	if (menu == NULL) {
		sock_send_error(c->sock, "Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item(c->menu, item_id, true);
	if (item != NULL) {
		sock_send_error(c->sock, "Item id already in use\n");
		return 0;
	}

	/* Find menuitem type */
	itemtype = menuitem_typename_to_type(argv[3]);
	if (itemtype == MENUITEM_INVALID) {
		sock_send_error(c->sock, "Invalid menuitem type\n");
		return 0;
	}

	/* Is a text given (options don't count)? */
	if ((argc >= 5) && (argv[4][0] != '-')) {
		text = argv[4];
	}
	else {
		text = "";
	}

	/* Create the menuitem */
	switch (itemtype) {
	  case MENUITEM_MENU:
		item = menu_create(item_id, menu_commands_handler, text, c);
		break;
	  case MENUITEM_ACTION:
		item = menuitem_create_action(item_id, menu_commands_handler, text, c,
				MENURESULT_NONE);
		break;
	  case MENUITEM_CHECKBOX:
		item = menuitem_create_checkbox(item_id, menu_commands_handler, text, c,
				false, false);
		break;
	  case MENUITEM_RING:
		item = menuitem_create_ring(item_id, menu_commands_handler, text, c,
				"", 0);
		break;
	  case MENUITEM_SLIDER:
		item = menuitem_create_slider(item_id, menu_commands_handler, text, c,
				 "", "", 0, 100, 1, 25);
		break;
	  case MENUITEM_NUMERIC:
		item = menuitem_create_numeric(item_id, menu_commands_handler, text, c,
				0, 100, 0);
		break;
	  case MENUITEM_ALPHA:
		item = menuitem_create_alpha(item_id, menu_commands_handler, text, c,
				0, 0, 10, true, false, true, "-./", "");
		break;
	  case MENUITEM_IP:
		item = menuitem_create_ip(item_id, menu_commands_handler, text, c,
				0, "192.168.1.245");
		break;
	 default:
	   assert(!"unexpected menuitem type");
	}
	menu_add_item(menu, item);
	menuscreen_inform_item_modified(menu);

	/* call menu_set_item() with a temporarily allocated argv
	 * to process the remaining options */
	if ((argc > 5) || ((argc == 5) && (argv[4][0] == '-'))) {
		// menu_add_item <menuid> <newitemid> <type> [<text>] [<option>]+
		// menu_set_item <menuid> <itemid> {<option>}+
		int i, j;
		char **tmp_argv = malloc(argc * sizeof(char *));

		assert(tmp_argv);
		tmp_argv[0] = "menu_set_item";
		for (i = j = 1; i < argc; i++) {
			/* skip "type" */
			if (i == 3)
				continue;
			/* skip "text" */
			if ((i == 4) && (argv[4][0] != '-'))
				continue;

			tmp_argv[j++] = argv[i];
		}
		// call menu_set_item() and let it send result to client
		menu_set_item_func(c, j, tmp_argv);
		free(tmp_argv);
	}
	else	// make sure the client gets informed
		sock_send_string(c->sock, "success\n");

	return 0;
}

/**
 * Deletes an item from a menu
 *
 *\verbatim
 * Usage: menu_del_item <menuid> <itemid>
 *\endverbatim
 *
 * The given item in the given menu will be deleted. If you have deleted all
 * the items from your client menu, that menu will automatically be removed.
 */
int
menu_del_item_func(Client *c, int argc, char **argv)
{
	Menu *menu;
	MenuItem *item;
	char *menu_id;
	char *item_id;

	if (c->state != ACTIVE)
		return 1;

	if (argc != 3) {
		sock_send_error(c->sock, "Usage: menu_del_item <menuid> <itemid>\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	/* Does the client have a menu already ? */
	if (c->menu == NULL) {
		sock_send_error(c->sock, "Client has no menu\n");
		return 0;
	}

	/* use either the given menu or the client's main menu if none was specified */
	menu = (menu_id[0] != '\0')
	       ? menu_find_item(c->menu, menu_id, true)
	       : c->menu;
	if (menu == NULL) {
		sock_send_error(c->sock, "Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item(c->menu, item_id, true);
	if (item == NULL) {
		sock_send_error(c->sock, "Cannot find item\n");
		return 0;
	}
	menuscreen_inform_item_destruction(item);
	menu_remove_item(menu, item);
	menuscreen_inform_item_modified(item->parent);
	menuitem_destroy(item);

	/* Was it the last item in the client's menu ? */
	if (menu_getfirst_item(c->menu) == NULL) {
		menuscreen_inform_item_destruction(c->menu);
		menu_remove_item(main_menu, c->menu);
		menuscreen_inform_item_modified(main_menu);
		menu_destroy(c->menu);
		c->menu = NULL;
	}
	sock_send_string(c->sock, "success\n");
	return 0;
}

/**
 * Sets the info about a menu item.
 *
 * For example, text displayed, value, etc...
 *
 *\verbatim
 * Usage: menu_set_item <menuid> <itemid> {<option>}+
 *
 * The following parameters can be set per item:
 * (you should include the - in the option)
 *
 * For all types:
 * -text "text"			("")
 *	Sets the visible text.
 * -is_hidden false|true	(false)
 *	If the item currently should not appear in a menu.
 * -prev id			()
 *	Sets the predecessor of this item (what happens after "Escape")
 *
 * For all except menus:
 * -next id			()
 *	Sets the successor of this item (what happens after "Enter")
 *
 * action:
 * -menu_result none|close|quit	(none)
 *	Sets what to do with the menu when this action is selected:
 *	- none: the menu stays as it is.
 *	- close: the menu closes and returns to a higher level.
 *	- quit: quits the menu completely so you can foreground your app.
 *
 * checkbox:
 * -value off|on|gray		(off)
 *	Sets its current value.
 * -allow_gray false|true	(false)
 *	Sets if a grayed checkbox is allowed.
 *
 * ring:
 * -value <int>			(0)
 *	Sets the index in the stringlist that is currently selected.
 * -strings <string>		(empty)
 *	The subsequent strings that can be selected. They should be
 *	tab-separated in ONE string.
 *
 * slider:
 * -value <int>			(0)
 *	Sets its current value.
 * -mintext <string>		("")
 * -maxtex <string>		("")
 *	Text at the minimal and maximal side. On small displays these might
 *	not be displayed.
 * -minvalue <int>		(0)
 * -maxvalue <int>		(100)
 *	The minimum and maximum value of the slider.
 * -stepsize <int>		(1)
 *	The stepsize of the slider. If you use 0, you can control it yourself
 *	completely.
 *
 * numeric:
 * -value <int>			(0)
 *	Sets its current value.
 * -minvalue <int>		(0)
 * -maxvalue <int>		(100)
 *	The minimum and maximum value that are allowed. If you make one of
 *	them negative, the user will be able to enter negative numbers too.
 * Maybe floats will work too in the future.
 *
 * alpha:
 * -value <string>
 *	Sets its current value.	("")
 * -password_char <char>	(none)
 * -minlength <int>		(0)
 * -maxlength <int>		(10)
 *	Set the minimum and maximum allowed length.
 * -allow_caps false|true	(true)
 * -allow_noncaps false|true	(false)
 * -allow_numbers false|true	(true)
 *	Allows these groups of characters.
 * -allowed_extra <string>	("")
 *	The chars in this string are also allowed.
 *
 * ip:
 * -value <string>
 *	Sets its current value.	("")
 * -v6 false|true
 *\endverbatim
 *
 * Hmm, this is getting very big. We might need a some real parser after all.
 */
int
menu_set_item_func(Client *c, int argc, char **argv)
{
	typedef enum AttrType { NOVALUE, BOOLEAN, CHECKBOX_VALUE, SHORT, INT,
				FLOAT, STRING } AttrType;

	/* This table generalizes the options.
	 * The table lists which options can exist for which menu items,
	 * what kind of parameter they should have and where this scanned
	 * parameter should be stored.
	 */
	struct OptionTable {
		MenuItemType menuitem_type;	/* For what MenuItem type is
						   the option ?
						   Use -1 for ALL types. */
		char *name;			/* The option concerned */
		AttrType attr_type;		/* Type of value */
		int attr_offset;		/* Where to put the value
						   in the structure.
						   Use -1 to process it
						   yourself. */
		/* Watch out with STRING, it will free() the current value
		 * and reallocate for the new value !! If you don't want
		 * that, use -1 for offset, to process it yourself. */
	} option_table[] = {
		{ -1,			"text",		STRING,		offsetof(MenuItem,text) },
		{ -1,			"is_hidden",	BOOLEAN,	offsetof(MenuItem,is_hidden) },
		{ -1,			"prev",		STRING,		-1 },
		{ -1,			"next",		STRING,		-1 },
		{ MENUITEM_ACTION,	"menu_result",	STRING,		-1 },
		{ MENUITEM_CHECKBOX,	"value",	CHECKBOX_VALUE,	offsetof(MenuItem,data.checkbox.value) },
		{ MENUITEM_CHECKBOX,	"allow_gray",	BOOLEAN,	offsetof(MenuItem,data.checkbox.allow_gray) },
		{ MENUITEM_RING,	"value",	SHORT,		offsetof(MenuItem,data.ring.value) },
		{ MENUITEM_RING,	"strings",	STRING,		-1 },
		{ MENUITEM_SLIDER,	"value",	INT,		offsetof(MenuItem,data.slider.value) },
		{ MENUITEM_SLIDER,	"minvalue",	INT,		offsetof(MenuItem,data.slider.minvalue) },
		{ MENUITEM_SLIDER,	"maxvalue",	INT,		offsetof(MenuItem,data.slider.maxvalue) },
		{ MENUITEM_SLIDER,	"stepsize",	INT,		offsetof(MenuItem,data.slider.stepsize) },
		{ MENUITEM_SLIDER,	"mintext",	STRING,		offsetof(MenuItem,data.slider.mintext) },
		{ MENUITEM_SLIDER,	"maxtext",	STRING,		offsetof(MenuItem,data.slider.maxtext) },
		{ MENUITEM_NUMERIC,	"value",	INT,		offsetof(MenuItem,data.numeric.value) },
		{ MENUITEM_NUMERIC,	"minvalue",	INT,		offsetof(MenuItem,data.numeric.minvalue) },
		{ MENUITEM_NUMERIC,	"maxvalue",	INT,		offsetof(MenuItem,data.numeric.maxvalue) },
		/*{ MENUITEM_NUMERIC,	"allow_decimals",BOOLEAN,	offsetof(MenuItem,data.numeric.allow_decimals) },*/
		{ MENUITEM_ALPHA,	"value",	STRING,		-1 /*offsetof(MenuItem,data.alpha.value)*/ },
		{ MENUITEM_ALPHA,	"minlength",	SHORT,		offsetof(MenuItem,data.alpha.minlength) },
		{ MENUITEM_ALPHA,	"maxlength",	SHORT,		offsetof(MenuItem,data.alpha.maxlength) },
		{ MENUITEM_ALPHA,	"password_char",STRING,		-1 },
		{ MENUITEM_ALPHA,	"allow_caps",	BOOLEAN,	offsetof(MenuItem,data.alpha.allow_caps) },
		{ MENUITEM_ALPHA,	"allow_noncaps",BOOLEAN,	offsetof(MenuItem,data.alpha.allow_noncaps) },
		{ MENUITEM_ALPHA,	"allow_numbers",BOOLEAN,	offsetof(MenuItem,data.alpha.allow_numbers) },
		{ MENUITEM_ALPHA,	"allowed_extra",STRING,		offsetof(MenuItem,data.alpha.allowed_extra) },
		{ MENUITEM_IP,		"v6",		BOOLEAN,	offsetof(MenuItem,data.ip.v6) },
		{ MENUITEM_IP,		"value",	STRING,		-1 /*offsetof(MenuItem,data.ip.value)*/ },
		{ -1,			NULL,		-1,		-1 }
	};

	bool bool_value = false;
	CheckboxValue checkbox_value = CHECKBOX_OFF;
	short short_value = 0;
	int int_value = 0;
	float float_value = 0;
	char *string_value = NULL;

	Menu *menu;
	MenuItem *item;
	char *menu_id;
	char *item_id;
	char *tmp_argv;
	int argnr;

	tmp_argv = argv2string(argc, argv);
	if (tmp_argv != NULL) {
		debug(RPT_DEBUG, "%s(Client [%d]: %s)",
		       __FUNCTION__, c->sock, tmp_argv);
		free(tmp_argv);
	}
	if (c->state != ACTIVE)
		return 1;

	if (argc < 4) {
		sock_send_error(c->sock, "Usage: menu_set_item <menuid> <itemid> {<option>}+\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	/* use either the given menu or the client's main menu if none was specified */
	menu = (menu_id[0] != '\0')
	       ? menu_find_item(c->menu, menu_id, true)
	       : c->menu;
	if (menu == NULL) {
		sock_send_error(c->sock, "Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item(c->menu, item_id, true);
	if (item == NULL) {
		sock_send_error(c->sock, "Cannot find item\n");
		return 0;
	}

	/* Scan all arguments */
	for (argnr = 3; argnr < argc; argnr++) {
		int option_nr = -1;
		int found_option_name = 0;
		int error = 0;
		void *location;
		char *p;

		/* Find the option in the table */
		if (argv[argnr][0] == '-') {
			int i;

			for (i = 0; option_table[i].name != NULL; i++) {
				if (strcmp(argv[argnr]+1, option_table[i].name) == 0) {
					found_option_name = 1;
					if (item->type == option_table[i].menuitem_type
					    || option_table[i].menuitem_type == -1) {
						option_nr = i;
					}
				}
			}
		}
		else {
			sock_printf_error(c->sock, "Found non-option: \"%.40s\"\n", argv[argnr]);
			continue; /* Skip to next arg */
		}
		if (option_nr == -1) {
			if (found_option_name) {
				sock_printf_error(c->sock, "Option not valid for menuitem type: \"%.40s\"\n", argv[argnr]);
			}
			else {
				sock_printf_error(c->sock, "Unknown option: \"%.40s\"\n", argv[argnr]);
			}
			continue; /* Skip to next arg */
		}

		/* OK, we now know we have an option that is valid for the item type. */

		/* Check for value */
		if (option_table[option_nr].attr_type != NOVALUE) {
			if (argnr + 1 >= argc) {
				sock_printf_error(c->sock, "Missing value at option: \"%.40s\"\n", argv[argnr]);
				continue; /* Skip to next arg (probably is not existing :) */
			}
		}
		/* Process the value that goes with the option */
		location = (void *) item + option_table[option_nr].attr_offset;
		switch (option_table[option_nr].attr_type) {
		  case NOVALUE:
			break;
		  case BOOLEAN:
			if (strcmp(argv[argnr+1], "false") == 0) {
				bool_value = false;
			}
			else if (strcmp(argv[argnr+1], "true") == 0) {
				bool_value = true;
			}
			else {
				error = 1;
				break;
			}
			if (option_table[option_nr].attr_offset != -1) {
				*(bool *) location = bool_value;
			}
			break;
		  case CHECKBOX_VALUE:
			if (strcmp(argv[argnr+1], "off") == 0) {
				checkbox_value = CHECKBOX_OFF;
			}
			else if (strcmp(argv[argnr+1], "on") == 0) {
				checkbox_value = CHECKBOX_ON;
			}
			else if (strcmp(argv[argnr+1], "gray") == 0) {
				checkbox_value = CHECKBOX_GRAY;
			}
			else {
				error = 1;
				break;
			}
			if (option_table[option_nr].attr_offset != -1) {
				*(CheckboxValue *) location = checkbox_value;
			}
			break;
		  case SHORT:
			short_value = strtol(argv[argnr+1], &p, 0);
			if ((argv[argnr+1][0] == '\0') || (*p != '\0')) {
				error = 1;
				break;
			}
			if (option_table[option_nr].attr_offset != -1) {
				*(short *) location = short_value;
			}
			break;
		  case INT:
			int_value = strtol(argv[argnr+1], &p, 0);
			if ((argv[argnr+1][0] == '\0') || (*p != '\0')) {
				error = 1;
				break;
			}
			if (option_table[option_nr].attr_offset != -1) {
				*(int *) location = int_value;
			}
			break;
		  case FLOAT:
			float_value = strtod(argv[argnr+1], &p);
			if ((argv[argnr+1][0] == '\0') || (*p != '\0')) {
				error = 1;
				break;
			}
			if (option_table[option_nr].attr_offset != -1) {
				*(float *) location = float_value;
			}
			break;
		  case STRING:
		  	string_value = argv[argnr+1];
			if (option_table[option_nr].attr_offset != -1) {
				free(*(char **) location);
				*(char **) location = strdup(string_value);
			}
			else if (strcmp(argv[argnr], "-prev") == 0) {
				set_predecessor(item, string_value, c);
			}
			else if (strcmp(argv[argnr], "-next") == 0) {
				set_successor(item, string_value, c);
			}
			break;
		}
		switch (error) {
		  case 1:
			sock_printf_error(c->sock, "Could not interpret value at option: \"%.40s\"\n", argv[argnr]);
			argnr ++;
			continue; /* Skip current option and the invalid value */
		}

		/* And at last process extra things for certain options.
		 * Most useful for the attr_offset==-1 stuff. */
		switch (item->type) {
		  case MENUITEM_ACTION:
			if (strcmp(argv[argnr]+1, "menu_result") == 0) {
				if (strcmp(argv[argnr+1], "none") == 0) {
					set_successor(item, "_none_", c);
				}
				else if (strcmp(argv[argnr+1], "close") == 0) {
					set_successor(item, "_close_", c);
				}
				else if (strcmp(argv[argnr+1], "quit") == 0) {
					set_successor(item, "_quit_", c);
				}
				else {
					error = 1;
				}
			}
			break;
		  case MENUITEM_SLIDER:
			if (item->data.slider.value < item->data.slider.minvalue) {
				item->data.slider.value = item->data.slider.minvalue;
			}
			else if (item->data.slider.value > item->data.slider.maxvalue) {
				item->data.slider.value = item->data.slider.maxvalue;
			}
			break;
		  case MENUITEM_RING:
		  	if (strcmp(argv[argnr]+1, "strings") == 0) {
		  		free(item->data.ring.strings);
		  		item->data.ring.strings = tablist2linkedlist(string_value);
		  	}
			item->data.ring.value %= LL_Length(item->data.ring.strings);
			break;
		  case MENUITEM_NUMERIC:
			menuitem_reset(item);
			break;
		  case MENUITEM_ALPHA:
			if (strcmp(argv[argnr]+1, "password_char") == 0) {
				item->data.alpha.password_char = string_value[0];
			}
			else if (strcmp(argv[argnr]+1, "maxlength") == 0) {
				char * new_buf;
				if ((short_value < 0) || (short_value > 1000)) {
					error = 2;
					break;
				}
				new_buf = malloc(short_value + 1);
				strncpy(new_buf, item->data.alpha.value, short_value);
				new_buf[short_value] = '\0'; /* terminate */
				free(item->data.alpha.value);
				item->data.alpha.value = new_buf;
				free(item->data.alpha.edit_str);
				item->data.alpha.edit_str = malloc(short_value + 1);
				item->data.alpha.edit_str[0] = '\0';
			}
			else if (strcmp(argv[argnr]+1, "value") == 0) {
				strncpy(item->data.alpha.value, string_value, item->data.alpha.maxlength);
				item->data.alpha.value[ item->data.alpha.maxlength ] = 0; /* terminate */
			}
			menuitem_reset(item);
			break;
		  case MENUITEM_IP:
			if (strcmp(argv[argnr]+1, "v6") == 0) {
				char * new_buf;
                		/* set max lenth depending ob boolean option v6 */
				item->data.ip.maxlength = (bool_value == 0) ? 15 : 39;

				new_buf = malloc(item->data.ip.maxlength + 1);
				strncpy(new_buf, item->data.ip.value, item->data.ip.maxlength);

				new_buf[item->data.ip.maxlength] = '\0'; /* terminate */
				free(item->data.ip.value);
				item->data.ip.value = new_buf;
				free(item->data.ip.edit_str);
				item->data.ip.edit_str = malloc(item->data.ip.maxlength +1);
				item->data.ip.edit_str[0] = '\0';
			}
			else if (strcmp(argv[argnr]+1, "value") == 0) {
				strncpy(item->data.ip.value, string_value, item->data.ip.maxlength);
				item->data.ip.value[item->data.ip.maxlength] = '\0'; /* terminate */
			}
			menuitem_reset(item);
			break;
		  default:
			break;
		}
		switch (error) {
		  case 1:
			sock_printf_error(c->sock, "Could not interpret value at option: \"%.40s\"\n", argv[argnr]);
			continue; /* Skip to next arg and retry it as an option */
		  case 2:
			sock_printf_error(c->sock, "Value out of range at option: \"%.40s\"\n", argv[argnr]);
			argnr ++;
			continue; /* Skip current option and the invalid value */
		}
		menuscreen_inform_item_modified(item);
		if (option_table[option_nr].attr_type != NOVALUE) {
			/* Skip the now used argument */
			argnr ++;
		}
	}
	sock_send_string(c->sock, "success\n");
	return 0;
}

/**
 * Requests the menu system to display the given menu screen.
 *
 * Depending on
 * the setting of the LCDPROC_PERMISSIVE_MENU_GOTO it is impossible
 * to go to a menu of another client (or the server menus). Same
 * restriction applies to the optional predecessor_id
 *
 *\verbatim
 * Usage: menu_goto <id> [<predecessor_id>]
 *\endverbatim
 */
int
menu_goto_func(Client * c, int argc, char **argv)
{
	char *menu_id;
	Menu *menu;

	debug(RPT_DEBUG, "%s(Client [%d], %s, %s)",
	       __FUNCTION__, c->sock, ((argc > 1) ? argv[1] : "<null>"),
	       ((argc > 2) ? argv[2] : "<null>"));
	if (c->state != ACTIVE)
		return 1;

	if ((argc < 2) || (argc > 3)) {
		sock_send_error(c->sock, "Usage: menu_goto <menuid> [<predecessor_id>]\n");
		return 0;
	}

	menu_id = argv[1];

	/* use either the given menu or the client's main menu if none was specified */
	menu = (menu_id[0] != '\0')
	       ? menuitem_search(menu_id, c)
	       : c->menu;
	if (menu == NULL) {
		sock_send_error(c->sock, "Cannot find menu id\n");
		return 0;
	}

	if (argc > 2)
		set_predecessor(menu, argv[2], c);

	menuscreen_goto(menu);
	/* Failure is not returned (Robijn) */
	sock_send_string(c->sock, "success\n");
	return 0;
}

/** Sets the predecessor of a Menuitem item to itemid (for wizzards).
 * For example the menuitem to go to after hitting "Enter" on item.
 *
 * \return 0 on success and -1 otherwise
 */
int set_predecessor(MenuItem *item, char *itemid, Client *c)
{
	// no sense to call this function on a null item
	assert(item != NULL);
	debug(RPT_DEBUG, "%s(%s, %s, %d)", __FUNCTION__,
	      item->id, itemid, c->sock);

	// handle these special
	if ((strcmp("_quit_", itemid) != 0) &&
	    (strcmp("_close_", itemid) != 0) &&
	    (strcmp("_none_", itemid) != 0)) {
		MenuItem *predecessor = menuitem_search(itemid, c);

		if (predecessor == NULL) {
			sock_printf_error(c->sock, "Cannot find predecessor '%s'"
				 " for item '%s'\n", itemid, item->id);
			return -1;
		}
	}
	debug(RPT_DEBUG, "%s(Client [%d], ...)"
	      " setting '%s's predecessor from '%s' to '%s'",
	      __FUNCTION__, c->sock, item->id,
	      item->predecessor_id, itemid);
	if (item->predecessor_id != NULL)
		free(item->predecessor_id);
	item->predecessor_id = strdup(itemid);
	return 0;
}

/** Sets the successor of a Menuitem item to itemid (for wizzards). For example the
 * menuitem to go to after hitting "Enter" on item. Checks that a matching
 * menu item can be found. Checks if item is not a menu. (If you would
 * redefine the meaning of "Enter" for a menu it would not be useful
 * anymore.)
 *
 * \return 0 on success and -1 otherwise
 */
int set_successor(MenuItem *item, char *itemid, Client *c)
{
	// no sense to call this function on a null item
	assert(item != NULL);
	debug(RPT_DEBUG, "%s(%s, %s, %d)", __FUNCTION__,
	      item->id, itemid, c->sock);

	// handle these special
	if ((strcmp("_quit_", itemid) != 0) &&
	    (strcmp("_close_", itemid) != 0) &&
	    (strcmp("_none_", itemid) != 0)) {
		MenuItem *successor = menuitem_search(itemid, c);

		if (successor == NULL) {
			sock_printf_error(c->sock, "Cannot find successor '%s'"
				 " for item '%s'\n", itemid, item->id);
			return -1;
		}
	}
	if (item->type == MENUITEM_MENU) {
		sock_printf_error(c->sock, "Cannot set successor of '%s':"
			    " wrong type '%s'\n", item->id,
			    menuitem_type_to_typename(item->type));
		return -1;
	}
	debug(RPT_DEBUG, "%s(Client [%d], ...)"
	       " setting '%s's successor from '%s' to '%s'",
	       __FUNCTION__, c->sock, item->id,
	       item->successor_id, itemid);
	if (item->successor_id != NULL)
		free(item->successor_id);
	item->successor_id = strdup(itemid);
	return 0;
}

/**
 * Requests the menu system to set the entry point into the menu system.
 *
 *\verbatim
 * Usage: menu_set_main <id>
 *\endverbatim
 */
int
menu_set_main_func(Client *c, int argc, char **argv)
{
	char *menu_id;
	Menu *menu;

	debug(RPT_DEBUG, "%s(Client [%d], %s, %s)",
	       __FUNCTION__, c->sock, ((argc > 1) ? argv[1] : "<null>"),
	       ((argc > 2) ? argv[2] : "<null>"));
	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: menu_set_main <menuid>\n");
		return 0;
	}

	menu_id = argv[1];

	if (menu_id[0] == '\0') {
		/* No menu specified = client's main menu */
		menu = c->menu;
	}
	else if (strcmp(menu_id, "_main_") == 0) {
		menu = NULL;
	}
	else {
		/* A specified menu */
		menu = menu_find_item(c->menu, menu_id, true);
		if (menu == NULL) {
			sock_send_error(c->sock, "Cannot find menu id\n");
			return 0;
		}
	}

	menuscreen_set_main(menu);

	sock_send_string(c->sock, "success\n");
	return 0;
}

/**
 * This function catches the event for the menus that have been
 * created on behalf of the clients. It informs the client with
 * an event message.
 */
MenuEventFunc(menu_commands_handler)
{
	Client *c;

	/* Where should the message go to ? */
	c = menuitem_get_client(item);
	if (c == NULL) {
		report(RPT_ERR, "%s: Could not find client of item \"%s\"",
		       __FUNCTION__, item->id);
		return -1;
	}

	/* Compose & send message */
	if ((event == MENUEVENT_UPDATE) ||
	    (event == MENUEVENT_MINUS) ||
	    (event == MENUEVENT_PLUS)) {
		switch (item->type) {
		  case MENUITEM_CHECKBOX:
			sock_printf(c->sock, "menuevent %s %.40s %s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, ((char *[]) {"off","on","gray"})[item->data.checkbox.value]);
			break;
		  case MENUITEM_SLIDER:
			sock_printf(c->sock, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.slider.value);
			break;
		  case MENUITEM_RING:
			sock_printf(c->sock, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.ring.value);
			break;
		  case MENUITEM_NUMERIC:
			sock_printf(c->sock, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.numeric.value);
			break;
		  case MENUITEM_ALPHA:
			sock_printf(c->sock, "menuevent %s %.40s %.40s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.alpha.value);
			break;
		  case MENUITEM_IP:
			sock_printf(c->sock, "menuevent %s %.40s %.40s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.ip.value);
			break;
		  default:
			sock_printf(c->sock, "menuevent %s %.40s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id);
		}
	}
	else if ((event == MENUEVENT_ENTER) ||
		 (event == MENUEVENT_LEAVE)) {
		sock_printf(c->sock, "menuevent %s %.40s\n",
			menuitem_eventtype_to_eventtypename(event),
			item->id);
	}
	else {
		sock_printf(c->sock, "menuevent %s %.40s\n",
			menuitem_eventtype_to_eventtypename(event),
			item->id);
	}

	return 0;
}
