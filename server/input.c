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

#include "../shared/sockets.h"
#include "../shared/debug.h"

#include "drivers/lcd.h"

#include "client_data.h"
#include "clients.h"
#include "screen.h"
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
	client *c;

	key = lcd.getkey ();

	if (!key)
		return 0;

	debug ("handle_input(%c)\n", (char) key);

	if (key) {
		s = screenlist_current ();
		if (s) {
			c = s->parent;
			if (c) {
				// TODO:  Interpret and translate keys!
				// If the client should have this keypress...
				// Send keypress to client
				if (key >= 'E' && key <= 'Z') {
					// TODO:  Implement client "acceptable key" lists
					sprintf (str, "key %c\n", key);
					sock_send_string (c->sock, str);
				}
				// Otherwise, tell the server about it.
				else {
					server_input (key);
				}
			} else {
				// If no parent, it means we're on the server screen.
				// so, the server gets all keypresses there.
				server_input (key);
			}
		}
	}

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
		debug ("server_input: Invalid key \"%c\" (%i)\n", (char) key, key);
		break;
	}

	return 0;
}
