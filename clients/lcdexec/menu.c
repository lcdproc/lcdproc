/** \file clients/lcdexec/menu.c
 * Menu parsing and building functions for the \c lcdexec client
 */

/* This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006-7, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"


/* names for boolean and tristate values */
static char *boolValueName[] = { "false", "true" };
static char *triGrayValueName[] = { "off", "on", "gray" };


/** recursively read the menu hierarchy */
MenuEntry *menu_read(MenuEntry *parent, const char *name)
{
	static int id = 0;

	if ((name != NULL) && (config_has_section(name))) {
		MenuEntry *me = calloc(1, sizeof(MenuEntry)); // auto-NULL elements

		if (me == NULL)
			return NULL;
		// set common entries
		me->id = id++;
		me->name = strdup(name);
		if (me->name == NULL) {
			//menu_free(me);
			return NULL;
		}

		me->displayname = strdup(config_get_string(name, "DisplayName", 0, name));
		if (me->displayname == NULL) {
			//menu_free(me);
			return NULL;
		}

		me->parent = parent;
		me->next = NULL;
		me->children = NULL;
		me->numChildren = 0;

		if (config_get_string(name, "Entry", 0, NULL) != NULL) {
			MenuEntry **addr = &me->children;
			const char *entryname;

			// it is a sub-menu
			me->type = MT_MENU;

			// read menu entries
			while ((entryname = config_get_string(name, "Entry", me->numChildren, NULL)) != NULL) {
				MenuEntry *entry = menu_read(me, entryname);

				if (entry == NULL) {
					//menu_free(me);
					return NULL;
				}

				me->numChildren++;

				*addr = entry;
				addr = &entry->next;
			}
		}
		else if (config_get_string(name, "Exec", 0, NULL) != NULL) {
			MenuEntry **addr = &me->children;
			const char *entryname;

			// it's a command to execute
			me->type = MT_EXEC;

			me->data.exec.command = strdup(config_get_string(name, "Exec", 0, ""));
			if (me->data.exec.command == NULL) {
				//menu_free(me);
				return NULL;
			}
			me->data.exec.feedback = config_get_bool(name, "Feedback", 0, 0);

			// try to read parameters
			while ((entryname = config_get_string(name, "Parameter", me->numChildren, NULL)) != NULL) {
				MenuEntry *entry = menu_read(me, entryname);

				if (entry == NULL) {
					//menu_free(me);
					return NULL;
				}

				me->numChildren++;

				*addr = entry;
				addr = &entry->next;
			}

			// automagically add an "Apply ?" action
			if ((me->numChildren > 0) && (addr != NULL))
				*addr = menu_read(me, NULL);
		}
		else if (config_get_string(name, "Type", 0, NULL) != NULL) {
			// it's a command parameter
			const char *type;

			type = config_get_string(name, "Type", 0, "");

			if (strcasecmp(type, "slider") == 0) {
				char buf[35];

				me->type = MT_ARG_SLIDER;

				me->data.slider.value = config_get_int(name, "Value", 0, 0);
				me->data.slider.minval = config_get_int(name, "MinValue", 0, 0);
				me->data.slider.maxval = config_get_int(name, "MaxValue", 0, 1000);

				sprintf(buf, "%d", me->data.slider.minval);
				me->data.slider.mintext = strdup(config_get_string(name, "MinText", 0, buf));
				sprintf(buf, "%d", me->data.slider.maxval);
				me->data.slider.maxtext = strdup(config_get_string(name, "MaxText", 0, buf));

				me->data.slider.stepsize = config_get_int(name, "StepSize", 0, 1);
			}
			else if (strcasecmp(type, "ring") == 0) {
				const char *tmp;
				int numStrings = 0;
				int i = 0;

				me->type = MT_ARG_RING;

				me->data.ring.value = config_get_int(name, "Value", 0, 0);
				numStrings = config_has_key(name, "String");
				me->data.ring.strings = calloc(sizeof(char *), numStrings+1);
				me->data.ring.strings[numStrings] = NULL;

				while ((tmp = config_get_string(name, "String", i, NULL)) != NULL) {
					me->data.ring.strings[i] = strdup(tmp);
					i++;
				}
				me->data.ring.strings[i] = NULL;
			}
			else if (strcasecmp(type, "numeric") == 0) {
				me->type = MT_ARG_NUMERIC;

				me->data.numeric.value = config_get_int(name, "Value", 0, 0);
				me->data.numeric.minval = config_get_int(name, "MinValue", 0, 0);
				me->data.numeric.maxval = config_get_int(name, "MaxValue", 0, 1000);
			}
			else if (strcasecmp(type, "alpha") == 0) {
				me->type = MT_ARG_ALPHA;

				me->data.alpha.value = strdup(config_get_string(name, "Value", 0, ""));
				me->data.alpha.minlen = config_get_int(name, "MinLength", 0, 0);
				me->data.alpha.maxlen = config_get_int(name, "MaxLength", 0, 100);
				me->data.alpha.allowed = strdup(config_get_string(name, "AllowedChars", 0,
										  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
			}
			else if (strcasecmp(type, "ip") == 0) {
				me->type = MT_ARG_IP;

				me->data.ip.value = strdup(config_get_string(name, "Value", 0, ""));
				me->data.ip.v6 = config_get_bool(name, "Value", 0, 0);
			}
			else if (strcasecmp(type, "checkbox") == 0) {
				const char *tmp;

				me->type = MT_ARG_CHECKBOX;

				me->data.checkbox.allow_gray = config_get_bool(name, "AllowGray", 0, 0);
				me->data.checkbox.value = (me->data.checkbox.allow_gray)
				                          ? config_get_tristate(name, "Value", 0, "gray", 0)
				                          : config_get_bool(name, "Value", 0, 0);
				// get replacement strings for different values
				tmp = config_get_string(name, "OffText", 0, NULL);
				me->data.checkbox.map[0] = (tmp != NULL) ? strdup(tmp) : NULL;
				tmp = config_get_string(name, "OnText", 0, NULL);
				me->data.checkbox.map[1] = (tmp != NULL) ? strdup(tmp) : NULL;
				tmp = config_get_string(name, "GrayText", 0, NULL);
				me->data.checkbox.map[2] = (tmp != NULL) ? strdup(tmp) : NULL;
			}
			else {
				report(RPT_DEBUG, "illegal parameter type");
				//menu_free(me);
				return NULL;
			}
		}
		else {
			report(RPT_DEBUG, "unknown menu entry type");
			//menu_free(me);
			return NULL;
		}

		return me;
	}
	else {
		/* the magic stuff: if name is NULL and parent is an EXEC entry,
		 * then generate an Action entry with the name "Apply" */
		if ((name == NULL) && (parent != NULL) && (parent->type = MT_EXEC)) {
			MenuEntry *me = calloc(1, sizeof(MenuEntry)); // auto-NULL elements

			if (me == NULL)
				return NULL;
			// set common entries
			me->id = id++;
			me->name = malloc(strlen(parent->name) + 10);
			if (me->name == NULL) {
				//menu_free(me);
				return NULL;
			}
			strcpy(me->name, "Apply_");
			strcat(me->name, parent->name);

			me->displayname = strdup("Apply!");
			if (me->displayname == NULL) {
				//menu_free(me);
				return NULL;
			}

			me->parent = parent;
			me->next = NULL;
			me->children = NULL;
			me->numChildren = 0;
			me->type = MT_ARG_ACTION | MT_AUTOMATIC;

			return me;
		}
	}

	return NULL;
}


/** create LCDproc commands for the menu entry hierarchy and send it to the server */
int menu_sock_send(MenuEntry *me, MenuEntry *parent, int sock)
{
	if ((me != NULL) && (sock > 0)) {
		char parent_id[12];

		// set parent_id depending on the parent given
		if ((parent != NULL) && (parent->id != 0))
			sprintf(parent_id, "%d", parent->id);
		else
			strcpy(parent_id, "");

		switch (me->type) {
			MenuEntry *entry;

			case MT_MENU:
				// don't create a separate entry for the main menu
				if ((parent != NULL) && (me->id != 0)) {
					if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" menu \"%s\"\n",
							parent_id, me->id, me->displayname) < 0)
						return -1;
				}

				// recursively do it for the menu's sub-menus
				for (entry = me->children; entry != NULL; entry = entry->next) {
					if (menu_sock_send(entry, me, sock) < 0)
						return -1;
				}
				break;
			case MT_EXEC:
				if (me->children == NULL) {
					if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" action \"%s\"\n",
							parent_id, me->id, me->displayname) < 0)
						return -1;

					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -menu_result quit\n",
							parent_id, me->id) < 0)
						return -1;
				}
				else {
					if ((parent != NULL) && (me->id != 0)) {
						if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" menu \"%s\"\n",
								parent_id, me->id, me->displayname) < 0)
							return -1;
					}

					// (recursively) do it for the entry's parameters
					for (entry = me->children; entry != NULL; entry = entry->next) {
						if (menu_sock_send(entry, me, sock) < 0)
							return -1;
					}
				}
				break;
			case MT_ARG_SLIDER:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" slider -text \"%s\""
						      " -value %d -minvalue %d -maxvalue %d"
						      " -mintext \"%s\" -maxtext \"%s\" -stepsize %d\n",
						      parent_id, me->id, me->displayname,
						      me->data.slider.value,
						      me->data.slider.minval,
						      me->data.slider.maxval,
						      me->data.slider.mintext,
						      me->data.slider.maxtext,
						      me->data.slider.stepsize) <0)
					return -1;

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_ARG_RING:
				{
					int i;
					char *tmp = strdup("");

					// join all strings with TAB as separator
					for (i = 0; me->data.ring.strings[i] != NULL; i++) {
						tmp = realloc(tmp, strlen(tmp) + 1 +
								   strlen(me->data.ring.strings[i]) + 1);
						if (tmp[0] != '\0')
							strcat(tmp, "\t");
						strcat(tmp, me->data.ring.strings[i]);
					}

					if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" ring -text \"%s\""
							      " -value %d -strings \"%s\"\n",
							      parent_id, me->id, me->displayname,
							      me->data.ring.value,
							      tmp) < 0)
						return -1;
				}

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_ARG_NUMERIC:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" numeric -text \"%s\""
						      " -value %d -minvalue %d -maxvalue %d\n",
						      parent_id, me->id, me->displayname,
						      me->data.numeric.value,
						      me->data.numeric.minval,
						      me->data.numeric.maxval) < 0)
					return -1;

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_ARG_ALPHA:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" alpha -text \"%s\""
						      " -value \"%s\" -minlength %d -maxlength %d"
						      " -allow_caps false -allow_noncaps false"
						      " -allow_numbers false -allowed_extra \"%s\"\n",
						      parent_id, me->id, me->displayname,
						      me->data.alpha.value,
						      me->data.alpha.minlen,
						      me->data.alpha.maxlen,
						      me->data.alpha.allowed) <0)
					return -1;

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_ARG_IP:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" ip -text \"%s\""
						      " -value \"%s\" -v6 %s\n",
						      parent_id, me->id, me->displayname,
						      me->data.ip.value,
						      boolValueName[me->data.ip.v6]) < 0)
					return -1;

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_ARG_CHECKBOX:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" checkbox -text \"%s\""
						      " -value %s -allow_gray %s\n",
						      parent_id, me->id, me->displayname,
						      triGrayValueName[me->data.checkbox.value],
						      boolValueName[me->data.checkbox.allow_gray]) < 0)
					return -1;

				if (me->next == NULL) {
					if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
							parent_id, me->id) < 0)
						return -1;
				}
				break;
			case MT_AUTOMATIC:
			case MT_ARG_ACTION:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" action \"%s\"\n",
						parent_id, me->id, me->displayname) < 0)
					return -1;

				if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -menu_result quit\n",
						parent_id, me->id) < 0)
					return -1;
				break;
			default:
				return -1;
		}
		return 0;
	}
	return -1;
}


/** find menu entry by its id */
MenuEntry *menu_find_by_id(MenuEntry *me, int id)
{
	if (me != NULL) {
		if (me->id == id)
			return me;

		if ((me->type == MT_MENU) || (me->type == MT_EXEC)) {
			MenuEntry *entry;

			for (entry = me->children; entry != NULL; entry = entry->next) {
				MenuEntry *result = menu_find_by_id(entry, id);

				if (result != NULL)
					return result;
			}
		}
	}
	return NULL;
}


/** return command of a menu entry */
const char *menu_command(MenuEntry *me)
{
	if ((me != NULL) && (me->type == MT_EXEC))
		return me->data.exec.command;

	return NULL;
}


/** free menu entry hierarchy */
void menu_free(MenuEntry *me)
{
	if (me != NULL) {
		MenuEntry *entry;
		int i;

		switch (me->type) {
			case MT_EXEC:
				if (me->data.exec.command != NULL)
					free(me->data.exec.command);
				me->data.exec.command = NULL;
				/* fall through */
			case MT_MENU:
				for (entry = me->children; entry != NULL; ) {
					MenuEntry *old = entry;

					entry = entry->next;
					old->next = NULL;
					menu_free(old);
				}
				me->children = NULL;
				break;
			case MT_ARG_SLIDER:
				if (me->data.slider.mintext != NULL)
					free(me->data.slider.mintext);
				me->data.slider.mintext = NULL;
				if (me->data.slider.maxtext != NULL)
					free(me->data.slider.maxtext);
				me->data.slider.maxtext = NULL;
				break;
			case MT_ARG_RING:
				if (me->data.ring.strings != NULL) {
					int i;

					for (i = 0; me->data.ring.strings[i] != NULL; i++)
						free(me->data.ring.strings[i]);

					free(me->data.ring.strings);
					me->data.ring.strings = NULL;
				}
				break;
			case MT_ARG_ALPHA:
				if (me->data.alpha.value != NULL)
					free(me->data.alpha.value);
				me->data.alpha.value = NULL;
				if (me->data.alpha.allowed != NULL)
					free(me->data.alpha.allowed);
				me->data.alpha.allowed = NULL;
				break;
			case MT_ARG_IP:
				if (me->data.ip.value != NULL)
					free(me->data.ip.value);
				me->data.ip.value = NULL;
				break;
			case MT_ARG_CHECKBOX:
				for (i = 0; i < sizeof(me->data.checkbox.map)/sizeof(me->data.checkbox.map[0]); i++) {
					if (me->data.checkbox.map[i] != NULL) {
						free(me->data.checkbox.map[i]);
						me->data.checkbox.map[i] = NULL;
					}
				}
			default:
				break;
		}

		if (me->name != NULL)
			free(me->name);
		me->name = NULL;

		if (me->displayname != NULL)
			free(me->displayname);
		me->displayname = NULL;

		me->type = MT_UNKNOWN;

		free(me);
	}
}


#if defined(DEBUG)
/** dump menu entry hierarchy to screen */
void menu_dump(MenuEntry *me)
{
	if (me != NULL) {
		/* the quick way out */
		if (me->type & MT_AUTOMATIC)
			return;

		report(RPT_DEBUG, "# menu ID: %d", me->id);
		report(RPT_DEBUG, "[%s]", me->name);
		if (me->displayname != NULL)
			report(RPT_DEBUG, "DisplayName=\"%s\"", me->displayname);

		switch (me->type) {
			MenuEntry *entry;

			case MT_MENU:
				// dump menu entry references
				for (entry = me->children; entry != NULL; entry = entry->next)
					report(RPT_DEBUG, "Entry=%s", entry->name);
				report(RPT_DEBUG, "");

				// recursively walk through sub-menus
				for (entry = me->children; entry != NULL; entry = entry->next)
					menu_dump(entry);
				break;
			case MT_EXEC:
				report(RPT_DEBUG, "Exec=\"%s\"", me->data.exec.command);
				report(RPT_DEBUG, "Feedback=%s", boolValueName[me->data.exec.feedback]);

				// dump entry's parameter referencess
				for (entry = me->children; entry != NULL; entry = entry->next)
					report(RPT_DEBUG, "Parameter=%s", entry->name);
				report(RPT_DEBUG, "");

				// dump entry's parameters
				for (entry = me->children; entry != NULL; entry = entry->next)
					menu_dump(entry);
				break;
			case MT_ARG_SLIDER:
				report(RPT_DEBUG, "Type=slider");
				report(RPT_DEBUG, "Value=%d", me->data.slider.value);
				report(RPT_DEBUG, "MinValue=%d", me->data.slider.minval);
				report(RPT_DEBUG, "MaxValue=%d", me->data.slider.maxval);
				report(RPT_DEBUG, "Stepsize=%d", me->data.slider.stepsize);
				report(RPT_DEBUG, "MinText=%s", me->data.slider.mintext);
				report(RPT_DEBUG, "MaxText=%s", me->data.slider.maxtext);
				report(RPT_DEBUG, "");
				break;
			case MT_ARG_RING:
				report(RPT_DEBUG, "Type=ring");
				report(RPT_DEBUG, "Value: %d", me->data.ring.value);
				{
					int i;

					for (i = 0; me->data.ring.strings[i] != NULL; i++)
						report(RPT_DEBUG, "String=\"%s\"", me->data.ring.strings[i]);
				}
				report(RPT_DEBUG, "");
				break;
			case MT_ARG_NUMERIC:
				report(RPT_DEBUG, "Type=numeric");
				report(RPT_DEBUG, "Value=%d", me->data.numeric.value);
				report(RPT_DEBUG, "MinValue=%d", me->data.numeric.minval);
				report(RPT_DEBUG, "MaxValue=%d", me->data.numeric.maxval);
				report(RPT_DEBUG, "");
				break;
			case MT_ARG_ALPHA:
				report(RPT_DEBUG, "Type:=lpha");
				report(RPT_DEBUG, "Value=\"%s\"", me->data.alpha.value);
				report(RPT_DEBUG, "AllowedChars=\"%s\"", me->data.alpha.allowed);
				report(RPT_DEBUG, "");
				break;
			case MT_ARG_IP:
				report(RPT_DEBUG, "Type=ip");
				report(RPT_DEBUG, "Value=\"%s\"", me->data.ip.value);
				report(RPT_DEBUG, "V6=%s", boolValueName[me->data.ip.v6]);
				report(RPT_DEBUG, "");
				break;
			case MT_ARG_CHECKBOX:
				report(RPT_DEBUG, "Type=ip");
				report(RPT_DEBUG, "Value=%s", triGrayValueName[me->data.checkbox.value]);
				report(RPT_DEBUG, "AllowGray=%s", boolValueName[me->data.checkbox.allow_gray]);
				if (me->data.checkbox.map[0] != NULL)
					report(RPT_DEBUG, "OffText=%s", me->data.checkbox.map[0]);
				if (me->data.checkbox.map[1] != NULL)
					report(RPT_DEBUG, "OnText=%s", me->data.checkbox.map[1]);
				if (me->data.checkbox.map[2] != NULL)
					report(RPT_DEBUG, "GrayText=%s", me->data.checkbox.map[2]);
				report(RPT_DEBUG, "");
				break;
			default:
				report(RPT_DEBUG, "ERROR: unknown menu entry type");
				break;
		}

	}
}
#endif

/* EOF */
