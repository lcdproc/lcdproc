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

// These are the keys for a (likely) properly functioning LK202-25...
// #define KEY_UP    'I'
// #define KEY_DOWN  'J'
// #define KEY_LEFT  'O'
// #define KEY_RIGHT 'E'
// #define KEY_F1    'N'
// #define KEY_F2    'M'
// #define KEY_ENTER 'H'

// These are the keys for my (possibly) broken LK202-25...
#define KEY_UP    'I'
#define KEY_DOWN  'F'
#define KEY_LEFT  'K'
#define KEY_RIGHT 'A'
#define KEY_F1    'N'
// #define KEY_F2    'M'
// #define KEY_ENTER 'H'

// TODO: Generalize these into each driver...
//
// Really, what this comes down to, is different settings for
// different displays.  Unless you want to recompile for all
// the defaults, these key settings must be in the driver.
//
// But then, which driver is active and which screen takes which
// input and... (sigh).
//
#define PAUSE_KEY	KEY_F1
#define BACK_KEY	KEY_LEFT
#define FORWARD_KEY	KEY_RIGHT
#define MAIN_MENU_KEY	KEY_DOWN

// This seems somewhat arbitrary, but it IS the original settings:
//
// #define PAUSE_KEY	'A'
// #define BACK_KEY	'B'
// #define FORWARD_KEY	'C'
// #define MAIN_MENU_KEY	'D'

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

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

#define KeyWanted(a,b)	((a) && strchr((a), (b)))
#define CurrentScreen	screenlist_current
#define FirstClient(a)	(client *)LL_GetFirst(a);
#define NextClient(a)	(client *)LL_GetNext(a);

int server_input (int key);

// FIXME!  The server tends to crash when "E" is pressed..  (?!)
// (but only when the joystick driver is the last one on the list...)

// Checks for keypad input, and dispatches it
int
handle_input ()
{
	char str[15];
	int key;
	screen *s;
	//widget *w;
	client *c;

	if ((key = lcd_ptr->getkey ()) == 0)
		return 0;

	//debug ("handle_input(%c)\n", (char) key);

	// Sequence:
	// 	Does the current screen want the key?
	// 	IfTrue: handle and quit
	// 	IfFalse:
	// 	    Let ALL clients handle it if they want
	// 	    Let Server handle it, too
	//
	// This leads to a unique situation:
	//     First: multiple clients may handle the same key in multiple ways
	//     Second: the server may handle the key differently yet
	//
	// Solution: Only the current screen can handle the key press.
	// Alternately, only one client can handle the key press.

	// TODO:  Interpret and translate keys!

	// Give current screen a shot at the key first
	s = CurrentScreen ();

	if (KeyWanted(s->keys, key)) {
		// This screen wants this key.  Tell it we got one
		snprintf(str, sizeof(str), "key %c\n", key);
		sock_send_string(s->parent->sock, str);
		// Nobody else gets this key
	}

	// if the current screen doesn't want it,
	// let the server have it...

	else {
		// Give key to clients who want it

		c = FirstClient(clients);

		while (c) {
			// If the client should have this keypress...
			if(KeyWanted(c->data->client_keys,key)) {
				// Send keypress to client
				snprintf(str, sizeof(str), "key %c\n", key);
				sock_send_string(c->sock, str);
				break;	// first come, first serve
			};
			c = NextClient(clients);
		} // while clients

		// Give server a shot at all keys
		server_input (key);
	}

	return 0;
}

int
server_input (int key)
{
	debug ("server_input(%c)\n", (char) key);
	syslog(LOG_DEBUG, "key %d pressed on device", key);

	switch ((char) key) {
		case PAUSE_KEY:
			if (screenlist_action == SCR_HOLD)
				screenlist_action = 0;
			else
				screenlist_action = SCR_HOLD;
			break;
		case BACK_KEY:
			screenlist_action = SCR_BACK;
			screenlist_prev ();
			break;
		case FORWARD_KEY:
			screenlist_action = SCR_SKIP;
			screenlist_next ();
			break;
		case MAIN_MENU_KEY:
			debug ("got the menu key!\n");
			server_menu ();
			break;
		default:
			debug ("server_input: Unused key \"%c\" (%i)\n", (char) key, key);
			break;
	}

	return 0;
}
