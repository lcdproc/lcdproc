/** \file server/commands/command_list.c
 * Implements the dispatcher for handlers dealing with the client commands.
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *		 2003, Joris Robijn
 */

#include <stdlib.h>
#include <string.h>

#include "command_list.h"
#include "server_commands.h"
#include "client_commands.h"
#include "screen_commands.h"
#include "widget_commands.h"
#include "menu_commands.h"

static client_function commands[] = {
	{ "test_func",      test_func_func      },
	{ "hello",          hello_func          },
	{ "client_set",     client_set_func     },
	{ "client_add_key", client_add_key_func },
	{ "client_del_key", client_del_key_func },
/*	{ "screen_add_key", screen_add_key_func }, */
/*	{ "screen_del_key", screen_del_key_func }, */
	{ "screen_add",     screen_add_func     },
	{ "screen_del",     screen_del_func     },
	{ "screen_set",     screen_set_func     },
	{ "widget_add",     widget_add_func     },
	{ "widget_del",     widget_del_func     },
	{ "widget_set",     widget_set_func     },
	{ "menu_add_item",  menu_add_item_func  },
	{ "menu_del_item",  menu_del_item_func  },
	{ "menu_set_item",  menu_set_item_func  },
	{ "menu_goto",      menu_goto_func      },
	{ "menu_set_main",  menu_set_main_func  },
	/* Misc stuff...*/
	{ "backlight",      backlight_func      },
	{ "output",         output_func         },
	{ "noop",           noop_func           },
	{ "info",           info_func           },
	{ "sleep",          sleep_func          },
	{ "bye",            bye_func            },
	{ NULL,             NULL},
};

/**
 * Looks up a function for a command sent by the client.
 * \param cmd  Command to look up as string.
 * \return  Pointer to the implementing function.
 */
CommandFunc get_command_function(char *cmd)
{
	int i;

	if (cmd == NULL)
		return NULL;

	for (i = 0; commands[i].keyword != NULL; i++) {
		if (0 == strcmp(cmd, commands[i].keyword))
			return commands[i].function;
	}

	return NULL;
}

