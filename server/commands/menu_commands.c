/*
 * menu_commands.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 *
 * This particular file defines actions concerning client supplied menus.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>

#include "shared/report.h"
#include "shared/sockets.h"

#include "menu.h"
#include "menuitem.h"
#include "menuscreens.h"
#include "client.h"

MenuEventFunc(menu_commands_handler);

/*************************************************************************
 * menu_add_item_func
 *
 * Adds an item to a menu
 *
 * Usage: menu_add_item <menuid> <newitemid> <type> [<text>]
 * You should use "" as id for the client's main menu. This menu will be
 * created automatically when you add an item to it the first time.
 * You (currently?) cannot create a menu in the main level yourself.
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
 */

int
menu_add_item_func (Client * c, int argc, char **argv)
{
	char * menu_id;
	char * item_id;
	char * text = NULL;
	Menu * menu = NULL;
	MenuItem * item;
	MenuItemType itemtype;

	if (!c->ack)
		return 1;

	if (!c->name) {
		sock_send_string (c->sock, "huh?  You need to give your client a name first\n");
		return 0;
	}

	if ((argc < 4 )) {
		sock_send_string (c->sock, "huh?  Usage: menu_add_item <menuid> <newitemid> <type> [<text>]\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	/* Does the client have a menu already ? */
	if (!c->menu) {
		/* We need to create it */
		c->menu = menu_create ("_client_menu_", menu_commands_handler, c->name, c);
		menu_add_item (main_menu, c->menu);
	}

	if ( menu_id[0] == 0 ) {
		/* No menu specified = client's main menu */
		menu = c->menu;
	} else {
		/* A specified menu */
		menu = menu_find_item (c->menu, menu_id, true);
	}
	if (!menu) {
		sock_send_string (c->sock, "huh?  Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item (c->menu, item_id, true);
	if (item) {
		sock_send_string (c->sock, "huh?  Item id already in use\n");
		return 0;
	}

	/* Find menuitem type */
	itemtype = menuitem_typename_to_type (argv[3]);
	if (itemtype == -1) {
		sock_send_string (c->sock, "huh?  Invalid menuitem type\n");
		return 0;
	}

	/* Is a text given ? */
	if (argc >= 5) {
		text = argv[4];
	}
	else {
		text = "";
	}

	/* Create the menuitem */
	switch (itemtype) {
	  case MENUITEM_MENU:
		item = menu_create (item_id, menu_commands_handler, text, c);
		break;
	  case MENUITEM_ACTION:
		item = menuitem_create_action (item_id, menu_commands_handler, text,
				MENURESULT_NONE);
		break;
	  case MENUITEM_CHECKBOX:
		item = menuitem_create_checkbox (item_id, menu_commands_handler, text,
				false, false);
		break;
	  case MENUITEM_RING:
		item = menuitem_create_ring (item_id, menu_commands_handler, text,
				"", 0);
		break;
	  case MENUITEM_SLIDER:
		item = menuitem_create_slider (item_id, menu_commands_handler, text,
				 "", "", 0, 100, 1, 25);
		break;
	  case MENUITEM_NUMERIC:
		item = menuitem_create_numeric (item_id, menu_commands_handler, text,
				0, 100, 0);
		break;
	  case MENUITEM_ALPHA:
		item = menuitem_create_alpha (item_id, menu_commands_handler, text,
				0, 0, 10, true, false, true, "-./", "");
		break;
	}
	menu_add_item (menu, item);
	menuscreen_inform_item_modified (menu);
	sock_send_string(c->sock, "success\n");
	return 0;
}

/*************************************************************************
 * menu_del_item_func
 *
 * Deletes an item from a menu
 *
 * Usage: menu_del_item <menuid> <itemid>
 * The given item in the given menu will be deleted. If you have deleted all
 * the items from your client menu, that menu will automatically be removed.
 */
int
menu_del_item_func (Client * c, int argc, char **argv)
{
	Menu * menu;
	MenuItem * item;
	char * menu_id;
	char * item_id;

	if (!c->ack)
		return 1;

	if ((argc < 3 )) {
		sock_send_string (c->sock, "huh?  Usage: menu_del_item <menuid> <itemid>\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	/* Does the client have a menu already ? */
	if (!c->menu) {
		sock_send_string (c->sock, "huh?  Client has no menu\n");
		return 0;
	}

	if ( menu_id[0] == 0 ) {
		/* No menu specified = client's main menu */
		menu = c->menu;
	} else {
		/* A specified menu */
		menu = menu_find_item (c->menu, menu_id, true);
	}
	if (!menu) {
		sock_send_string (c->sock, "huh?  Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item (c->menu, item_id, true);
	if (!item) {
		sock_send_string (c->sock, "huh?  Cannot find item\n");
		return 0;
	}
	menuscreen_inform_item_destruction (item);
	menu_remove_item (menu, item);
	menuscreen_inform_item_modified (item->parent);
	menuitem_destroy (item);

	/* Was it the last item in the client's menu ? */
	if (menu_getfirst_item(c->menu) == NULL) {
		menuscreen_inform_item_destruction (c->menu);
		menu_remove_item (main_menu, c->menu);
		menuscreen_inform_item_modified (main_menu);
		menu_destroy (c->menu);
		c->menu = NULL;
	}
	sock_send_string(c->sock, "success\n");
	return 0;
}

/**************************************************************************
 * menu_set_item_func
 * Sets the info about a menu item
 *
 * For example, text displayed, value, etc...
 *
 * Usage: menu_set_item <menuid> <itemid> {<option>}+
 * The following parameters can be set per item:
 * (you should include the - in the option)
 *
 * For all types:
 * -text "text"			("")
 *	Sets the visible text.
 *
 * menu:
 * (has no extra settable options)
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
 * Hmm, this is getting very big. We might need a some real parser after all.
 */

int
menu_set_item_func (Client * c, int argc, char **argv)
{
	typedef enum AttrType { NOVALUE, BOOLEAN, CHECKBOX_VALUE, SHORT, INT,
				FLOAT, STRING } AttrType;

	struct OptionTable {
		MenuItemType menuitem_type;	/* For what MenuItem type is
						   the option ?
						   Use -1 for ALL types. */
		char * name;			/* The option concerned */
		AttrType attr_type;		/* Type of value */
		int attr_offset;		/* Where to put it the value
						   in the structure.
						   Use -1 to process it
						   yourself. */
		/* Watch out with STRING, it will free() the current value
		 * and reallocate for the new value !! If you don't want
		 * that, use -1 for offset, to process it yourself. */
	} option_table[] = {
		{ -1,			"text",		STRING,		offsetof(MenuItem,text) },
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
		{ MENUITEM_ALPHA,	"value",	STRING,		offsetof(MenuItem,data.alpha.value) },
		{ MENUITEM_ALPHA,	"minlength",	SHORT,		offsetof(MenuItem,data.alpha.minlength) },
		{ MENUITEM_ALPHA,	"maxlength",	SHORT,		offsetof(MenuItem,data.alpha.maxlength) },
		{ MENUITEM_ALPHA,	"password_char",STRING,		-1 },
		{ MENUITEM_ALPHA,	"allow_caps",	BOOLEAN,	offsetof(MenuItem,data.alpha.allow_caps) },
		{ MENUITEM_ALPHA,	"allow_noncaps",BOOLEAN,	offsetof(MenuItem,data.alpha.allow_noncaps) },
		{ MENUITEM_ALPHA,	"allow_numbers",BOOLEAN,	offsetof(MenuItem,data.alpha.allow_numbers) },
		{ MENUITEM_ALPHA,	"allowed_extra",STRING,		offsetof(MenuItem,data.alpha.allowed_extra) },
		{ -1,			NULL,		-1,		-1 }
	};

	bool bool_value = false;
	CheckboxValue checkbox_value = CHECKBOX_OFF;
	short short_value = 0;
	int int_value = 0;
	float float_value = 0;
	char * string_value = NULL;

	Menu * menu;
	MenuItem * item;
	char * menu_id;
	char * item_id;
	int argnr;
	char buf[80];

	if (!c->ack)
		return 1;

	if ((argc < 4 )) {
		sock_send_string (c->sock, "huh?  Usage: menu_set_item <menuid> <itemid> {<option>}+\n");
		return 0;
	}

	menu_id = argv[1];
	item_id = argv[2];

	if ( menu_id[0] == 0 ) {
		/* No menu specified = client's main menu */
		menu = c->menu;
	} else {
		/* A specified menu */
		menu = menu_find_item (c->menu, menu_id, true);
	}
	if (!menu) {
		sock_send_string (c->sock, "huh?  Cannot find menu id\n");
		return 0;
	}

	item = menu_find_item (c->menu, item_id, true);
	if (!item) {
		sock_send_string (c->sock, "huh?  Cannot find item\n");
		return 0;
	}

	/* Scan all arguments */
	for( argnr = 3; argnr < argc; argnr ++) {
		int option_nr = -1;
		int found_option_name = 0;
		int error = 0;
		void * location;
		char * p;

		/* Find the option in the table */
		if( argv[argnr][0] == '-' ) {
			int i;
			for( i=0; option_table[i].name; i++ ) {
				if( strcmp( argv[argnr]+1, option_table[i].name ) == 0 ) {
					found_option_name = 1;
					if( item->type == option_table[i].menuitem_type
					|| option_table[i].menuitem_type == -1 ) {
						option_nr = i;
					}
				}
			}
		}
		else {
			snprintf( buf, sizeof(buf), "huh?  Found non-option: \"%.40s\"\n", argv[argnr] );
			sock_send_string (c->sock, buf);
			continue; /* Skip to next arg */
		}
		if( option_nr == -1 ) {
			if( found_option_name ) {
				snprintf( buf, sizeof(buf), "huh?  Option not valid for menuitem type: \"%.40s\"\n", argv[argnr] );
			} else {
				snprintf( buf, sizeof(buf), "huh?  Unknown option: \"%.40s\"\n", argv[argnr] );
			}
			sock_send_string (c->sock, buf);
			continue; /* Skip to next arg */
		}

		/* OK, we now know we have an option that is valid for the item type. */

		/* Check for value */
		if( option_table[option_nr].attr_type != NOVALUE ) {
			if( argnr + 1 >= argc ) {
				snprintf( buf, sizeof(buf), "huh?  Missing value at option: \"%.40s\"\n", argv[argnr] );
				sock_send_string (c->sock, buf);
				continue; /* Skip to next arg (probably is not existing :) */
			}
		}
		/* Process the value that goes with the option */
		location = (void*)item + option_table[option_nr].attr_offset;
		switch( option_table[option_nr].attr_type ) {
		  case NOVALUE:
			break;
		  case BOOLEAN:
			if( strcmp( argv[argnr+1], "false" ) == 0 ) {
				bool_value = false;
			} else if( strcmp( argv[argnr+1], "true" ) == 0 ) {
				bool_value = true;
			} else {
				error = 1;
				break;
			}
			if( option_table[option_nr].attr_offset != -1 ) {
				*(bool *)location = bool_value;
			}
			break;
		  case CHECKBOX_VALUE:
			if( strcmp( argv[argnr+1], "off" ) == 0 ) {
				checkbox_value = CHECKBOX_OFF;
			} else if( strcmp( argv[argnr+1], "on" ) == 0 ) {
				checkbox_value = CHECKBOX_ON;
			} else if( strcmp( argv[argnr+1], "gray" ) == 0 ) {
				checkbox_value = CHECKBOX_GRAY;
			} else {
				error = 1;
				break;
			}
			if( option_table[option_nr].attr_offset != -1 ) {
				*(CheckboxValue *)location = checkbox_value;
			}
			break;
		  case SHORT:
			short_value = strtol( argv[argnr+1], &p, 0 );
			if( argv[argnr+1][0] == 0
			|| *p != 0 ) {
				error = 1;
				break;
			}
			if( option_table[option_nr].attr_offset != -1 ) {
				*(short*)location = short_value;
			}
			break;
		  case INT:
			int_value = strtol( argv[argnr+1], &p, 0 );
			if( argv[argnr+1][0] == 0
			|| *p != 0 ) {
				error = 1;
				break;
			}
			if( option_table[option_nr].attr_offset != -1 ) {
				*(int*)location = int_value;
			}
			break;
		  case FLOAT:
			float_value = strtod( argv[argnr+1], &p );
			if( argv[argnr+1][0] == 0
			|| *p != 0 ) {
				error = 1;
				break;
			}
			if( option_table[option_nr].attr_offset != -1 ) {
				*(float*)location = float_value;
			}
			break;
		  case STRING:
		  	string_value = argv[argnr+1];
			if( option_table[option_nr].attr_offset != -1 ) {
				free( *(char**)location );
				*(char**)location = strdup( string_value );
			}
			break;
		}
		switch( error ) {
		  case 1:
			snprintf( buf, sizeof(buf), "huh?  Could not interpret value at option: \"%.40s\"\n", argv[argnr] );
			sock_send_string (c->sock, buf);
			argnr ++;
			continue; /* Skip current option and the invalid value */
		}

		/* And at last process extra things for certain options.
		 * Most useful for the attr_offset==-1 stuff. */
		switch (item->type) {
		  case MENUITEM_ACTION:
			if( strcmp( argv[argnr]+1, "menu_result" ) == 0 ) {
				if( strcmp( argv[argnr+1], "none" ) == 0 ) {
					item->data.action.menu_result = MENURESULT_NONE;
				} else if( strcmp( argv[argnr+1], "close" ) == 0 ) {
					item->data.action.menu_result = MENURESULT_CLOSE;
				} else if( strcmp( argv[argnr+1], "quit" ) == 0 ) {
					item->data.action.menu_result = MENURESULT_QUIT;
				} else {
					error = 1;
				}
			}
			break;
		  case MENUITEM_SLIDER:
			if( item->data.slider.value < item->data.slider.minvalue ) {
				item->data.slider.value = item->data.slider.minvalue;
			} else if( item->data.slider.value > item->data.slider.maxvalue ) {
				item->data.slider.value = item->data.slider.maxvalue;
			}
			break;
		  case MENUITEM_RING:
		  	if( strcmp( argv[argnr]+1, "strings" ) == 0 ) {
		  		free (item->data.ring.strings);
		  		item->data.ring.strings = tablist2linkedlist (string_value);
		  	}
			item->data.ring.value %= LL_Length( item->data.ring.strings );
			break;
		  case MENUITEM_NUMERIC:
			menuitem_reset (item);
			break;
		  case MENUITEM_ALPHA:
			if( strcmp( argv[argnr]+1, "password_char" ) == 0 ) {
				item->data.alpha.password_char = string_value[0];
			}
			else if( strcmp( argv[argnr]+1, "maxlength" ) == 0 ) {
				char * new_buf;
				if( short_value < 0 || short_value > 1000 ) {
					error = 2;
					break;
				}
				new_buf = malloc( short_value + 1 );
				strncpy( new_buf, item->data.alpha.value, short_value );
				new_buf[short_value] = 0; /* terminate */
				free( item->data.alpha.value );
				item->data.alpha.value = new_buf;
				free( item->data.alpha.edit_str );
				item->data.alpha.edit_str = malloc( short_value + 1 );
				item->data.alpha.edit_str[0] = 0;
			}
			else if( strcmp( argv[argnr]+1, "value" ) == 0 ) {
				strncpy( item->data.alpha.value, string_value, item->data.alpha.maxlength );
				item->data.alpha.value[ item->data.alpha.maxlength ] = 0; /* terminate */
			}
			menuitem_reset (item);
			break;
		  default:
			break;
		}
		switch( error ) {
		  case 1:
			snprintf( buf, sizeof(buf), "huh?  Could not interpret value at option: \"%.40s\"\n", argv[argnr] );
			sock_send_string (c->sock, buf);
			continue; /* Skip to next arg and retry it as an option */
		  case 2:
			snprintf( buf, sizeof(buf), "huh?  Value out of range at option: \"%.40s\"\n", argv[argnr] );
			sock_send_string (c->sock, buf);
			argnr ++;
			continue; /* Skip current option and the invalid value */
		}
		menuscreen_inform_item_modified (item);
		if( option_table[option_nr].attr_type != NOVALUE ) {
			/* Skip the now used argument */
			argnr ++;
		}
	}
	sock_send_string(c->sock, "success\n");
	return 0;
}

MenuEventFunc (menu_commands_handler)
{
	char buf[80] = "";
	Client * c;
	MenuItem * i;

	/* Compose message */
	if( event == MENUEVENT_UPDATE
	|| event == MENUEVENT_MINUS
	|| event == MENUEVENT_PLUS ) {
		switch( item->type ) {
		  case MENUITEM_CHECKBOX:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s %s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, ((char*[]){"off","on","gray"})[item->data.checkbox.value] );
			break;
		  case MENUITEM_SLIDER:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.slider.value );
			break;
		  case MENUITEM_RING:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.ring.value );
			break;
		  case MENUITEM_NUMERIC:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s %d\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.numeric.value );
			break;
		  case MENUITEM_ALPHA:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s %.40s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id, item->data.alpha.value );
			break;
		  default:
			snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s\n",
				menuitem_eventtype_to_eventtypename(event),
				item->id);
		}
	}
	else {
		snprintf (buf, sizeof(buf)-1, "menuevent %s %.40s\n",
			menuitem_eventtype_to_eventtypename(event),
			item->id);
	}

	buf[sizeof(buf)-1] = 0;

	/* Where should the message go to ? */
	for( i = item; i && i->parent != main_menu; i = i->parent );
	c = (Client *) i->data.menu.association;
	if( !c ) {
		report( RPT_ERR, "%s: Could not find client of item \"%s\"", __FUNCTION__, item->id );
		return -1;
	}

	/* Send it */
	sock_send_string (c->sock, buf);
	return 0;
}
