/*
 * menu.c
 * This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"

/* recursively read the menu hierarchy */
MenuEntry *menu_read(MenuEntry *parent, const char *name)
{
	static int id = 0;

	if (config_has_section(name)) {
		MenuEntry *me = calloc(1, sizeof(MenuEntry));
		
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
		if (me->name == NULL) {
			//menu_free(me);
			return NULL;
		}	

		me->entries = NULL;
		me->next = NULL;
		me->command = NULL;
		
		if (config_get_string(name, "Entry", 0, NULL) != NULL) {
			MenuEntry **addr = &me->entries;
			const char *entryname;
			int index = 0;
			
			// it is a sub-menu
			me->type = menu;
			
			while ((entryname = config_get_string(name, "Entry", index++, NULL)) != NULL) {
				MenuEntry *entry = menu_read(me, entryname);

				if (entry == NULL) {
					//menu_free(me);
					return NULL;
				}	

				*addr = entry;
				addr = &entry->next;
			}	
		}	
		else if (config_get_string(name, "Exec", 0, NULL) != NULL) {
			// it's a command to execute
			me->type = exec;

			me->command = strdup(config_get_string(name, "Exec", 0, ""));
			if (me->command == NULL) {
				//menu_free(me);
				return NULL;
			}	
		}
		else {
			//menu_free(me);
			return NULL;
		}	

		return me;
	}	

	return NULL;
}


/* create LCDproc commands for the menu entry hierarchy and send it to the server */
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

			case menu:
				// don't create a separate entry for the main menu
				if ((parent != NULL) && (me->id != 0)) {
					if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" menu \"%s\"\n",
							parent_id, me->id, me->displayname) < 0)
						return -1;
				}	
				
				// recursively do it for the menu's sub-menus
				for (entry = me->entries; entry != NULL; entry = entry->next) {
					if (menu_sock_send(entry, me, sock) < 0)
						return -1;
				}	
				break;
			case exec:
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" action \"%s\"\n",
						parent_id, me->id, me->displayname) < 0)
					return -1;
				if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -menu_result quit\n",
						parent_id, me->id) < 0)
					return -1;
				break;
			case unknown:
			default:
				return -1;
		}
		return 0;
	}	
	return -1;
}


/* find menu entry by its id */
MenuEntry *menu_find_by_id(MenuEntry *me, int id)
{
	if (me != NULL) {
		if (me->id == id)
			return me;

		if (me->type == menu) {
			MenuEntry *entry;
			
			for (entry = me->entries; entry != NULL; entry = entry->next) {
				MenuEntry *result = menu_find_by_id(entry, id);

				if (result != NULL)
					return result;
			}
		}	
	}
	return NULL;
}


/* return command of a menu entry */
const char *menu_command(MenuEntry *me)
{
	if ((me != NULL) && (me->type == exec))
		return me->command;

	return NULL;
}


/* free menu entry hierarchy */
void menu_free(MenuEntry *me)
{
	if (me != NULL) {
		switch (me->type) {
			MenuEntry *entry;

			case menu:
				for (entry = me->entries; entry != NULL; ) {
					MenuEntry *old = entry;

					entry = entry->next;
					old->next = NULL;
					menu_free(old);
				}	
				me->entries = NULL;
				break;
			case exec:
				if (me->command != NULL)
					free(me->command);
				me->command = NULL;
				break;
			case unknown:
			default:
				break;
		}

		if (me->name != NULL)
			free(me->name);
		me->name = NULL;
				
		if (me->displayname != NULL)
			free(me->displayname);
		me->displayname = NULL;

		me->type = unknown;

		free(me);
	}
}


#if defined(DEBUG)
/* dump menu entry hierarchy to screen */
void menu_dump(MenuEntry *me)
{
	if (me != NULL) {
		report(RPT_DEBUG, "# menu ID: %d", me->id);
		report(RPT_DEBUG, "[%s]", me->name);
		if (me->displayname != NULL)
			report(RPT_DEBUG, "DisplayName=\"%s\"", me->displayname);
		
		switch (me->type) {
			MenuEntry *entry;

			case menu:
				for (entry = me->entries; entry != NULL; entry = entry->next)
					report(RPT_DEBUG, "Entry=%s", entry->name);
				break;
			case exec:
				report(RPT_DEBUG, "Exec=\"%s\"", me->command);
				break;
			case unknown:
			default:
				report(RPT_DEBUG, "ERROR: unknown menu entry type");
				break;
		}
		report(RPT_DEBUG, "");

		// recursively walk through sub-menus
		if (me->type == menu) {
			MenuEntry *entry;

			for (entry = me->entries; entry != NULL; entry = entry->next)
				menu_dump(entry);
		}
	}
}
#endif

/* EOF */
