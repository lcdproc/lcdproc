/*

  input.c

  Handles keypad (and other?) input from the user.

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
#include "shared/debug.h"

#include "drivers/lcd.h"

#include "client_data.h"
#include "clients.h"
#include "screen.h"
#include "widget.h"
#include "screenlist.h"
#include "menus.h"

#include "input.h"

int server_input (int key);

// FIXME!  The server tends to crash when "E" is pressed..  (?!)
// (but only when the joystick driver is the last one on the list...)

// Checks for keypad input, and dispatches it
int
handle_input ()
{
	char str[256];
	int key;
	screen *s;
	widget *w;
	client *c;

	key = lcd.getkey ();

	if (!key)
		return 0;

	debug ("handle_input(%c)\n", (char) key);

	if (key) {
		// TODO:  Interpret and translate keys!

		// Give current screen a shot at the key first
		s = screenlist_current ();
		w = widget_find( s, KEYS_WIDGETID );
		if( s->parent && w && w->text && strchr( w->text, key ) ) {
			// This screen wants this key.  Tell it we got one
			sprintf(str, "key %c\n", key);
			sock_send_string(s->parent->sock, str);
			// Nobody else gets this key
		} else {
			// Give key to clients who want it
			c = (client *)LL_GetFirst(clients);
			while(c) {
				// If the client should have this keypress...
				if( c->data->client_keys && strchr(c->data->client_keys, key) ) {
					// Send keypress to client
					sprintf(str, "key %c\n", key);
					sock_send_string(c->sock, str);
				};
				c = (client *)LL_GetNext(clients);
			} // while clients

			// Give server a shot at all keys
			server_input (key);
		}
	};

	return 0;
}

int
server_input (int key)
{
	debug ("server_input(%c)\n", (char) key);

	switch (key) {
	case 'A':
		if (screenlist_action == SCR_HOLD)
			screenlist_action = 0;
		else
			screenlist_action = SCR_HOLD;
		break;
	case 'B':
		screenlist_action = SCR_BACK;
		screenlist_prev ();
		break;
	case 'C':
		screenlist_action = SCR_SKIP;
		screenlist_next ();
		break;
	case 'D':
		debug ("got the menu key!\n");
		server_menu ();
		break;
	default:
		debug ("server_input: Unused key \"%c\" (%i)\n", (char) key, key);
		break;
	}

	return 0;
}
