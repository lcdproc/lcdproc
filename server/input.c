/*
 * input.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Handles keypad (and other?) input from the user.
 */

/*

  Currently, the keys are as follows:

  Context     Key      Function
  -------     ---      --------
  Normal               "normal" context is handled in this source file.
              A        Pause/Continue
              B        Back(Go to previous screen)
              C        Forward(Go to next screen)
              D        Open main menu
              E-Z      Sent to client, if any; ignored otherwise

 (menu keys are not handled here, but in the menu code)
  Menu
              A        Enter/select
              B        Up/Left
              C        Down/Right
              D        Exit/Cancel
              E-Z      Ignored
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "drivers.h"

#include "client_data.h"
#include "clients.h"
#include "screen.h"
#include "widget.h"
#include "screenlist.h"
#include "menus.h"

#include "input.h"

#define KeyWanted(a,b)	((a) && strchr((a), (b)))
#define CurrentScreen	screenlist_current
#define FirstClient(a)	(client *)LL_GetFirst(a);
#define NextClient(a)	(client *)LL_GetNext(a);

int server_input (int key);

/* FIXME!  The server tends to crash when "E" is pressed..  (?!)
 * (but only when the joystick driver is the last one on the list...)
 */

/* Checks for keypad input, and dispatches it */
int
handle_input ()
{
	char str[15];
	int key;
	screen *s;
	/*widget *w; */
	client *c;

	report (RPT_INFO, "handle_input()" );

	if ((key = drivers_getkey ()) == 0)
		return 0;

	debug (RPT_INFO, "handle_input got key: '%c'", key);

	/* Sequence:
	 * 	Does the current screen want the key?
	 * 	IfTrue: handle and quit
	 * 	IfFalse:
	 * 	    Let ALL clients handle it if they want
	 * 	    Let Server handle it, too
	 *
	 * This leads to a unique situation:
	 *     First: multiple clients may handle the same key in multiple ways
	 *     Second: the server may handle the key differently yet
	 *
	 * Solution: Only the current screen can handle the key press.
	 * Alternately, only one client can handle the key press.
	 */

	/* TODO:  Interpret and translate keys! */

	/* Give current screen a shot at the key first */
	s = CurrentScreen ();

	if (KeyWanted(s->keys, key)) {
		/* This screen wants this key.  Tell it we got one */
		snprintf(str, sizeof(str), "key %c\n", key);
		sock_send_string(s->parent->sock, str);
		/* Nobody else gets this key */
	}

	/* if the current screen doesn't want it,
	 * let the server have it...
	 */

	else {
		/* Give key to clients who want it */

		c = FirstClient(clients);

		while (c) {
			/* If the client should have this keypress... */
			if(KeyWanted(c->data->client_keys,key)) {
				/* Send keypress to client */
				snprintf(str, sizeof(str), "key %c\n", key);
				sock_send_string(c->sock, str);
				break;	/* first come, first serve */
			};
			c = NextClient(clients);
		} /* while clients */

		/* Give server a shot at all keys */
		server_input (key);
	}

	return 0;
}

int
server_input (int key)
{
	report(RPT_INFO, "server_input( key='%c' )", (char) key);

	switch ((char) key) {
		case INPUT_PAUSE_KEY:
			if (screenlist_action == SCR_HOLD)
				screenlist_action = 0;
			else
				screenlist_action = SCR_HOLD;
			break;
		case INPUT_BACK_KEY:
			screenlist_action = SCR_BACK;
			screenlist_prev ();
			break;
		case INPUT_FORWARD_KEY:
			screenlist_action = SCR_SKIP;
			screenlist_next ();
			break;
		case INPUT_MAIN_MENU_KEY:
			debug (RPT_DEBUG, "got the menu key!");
			server_menu ();
			break;
		default:
			debug (RPT_DEBUG, "server_input: Unused key \"%c\" (%i)", (char) key, key);
			break;
	}

	return 0;
}
