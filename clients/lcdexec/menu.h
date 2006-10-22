/*
 * lcdexec.c
 * This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "shared/str.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"


typedef enum {
	unknown = 0,
	menu = 1,
	exec = 2,
} MenuType;	


typedef struct menu_entry {
	char *name;
	char *displayname;
	int id;
	MenuType type;

	// variables necessary for type menu
	struct menu_entry *entries;
	struct menu_entry *next;

	// variables necessary for type exec
	char *command;
} MenuEntry;


MenuEntry *menu_read(MenuEntry *parent, const char *name);
int menu_sock_send(MenuEntry *me, MenuEntry *parent, int sock);
MenuEntry *menu_find_by_id(MenuEntry *me, int id);
const char *menu_command(MenuEntry *me);
void menu_free(MenuEntry *me);
void menu_dump(MenuEntry *me);

