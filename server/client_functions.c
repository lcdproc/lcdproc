/*
  client_functions.c
  
  This contains definitions for all the functions which clients can run.
  The functions here are to be called only from parse.c's interpreter.
  
  The client's available function set is defined here, as is the syntax
  for each command.
  
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "../shared/debug.h"
#include "../shared/sockets.h"

#include "drivers/lcd.h"

#include "main.h"
#include "render.h"
#include "clients.h"
#include "parse.h"
#include "screen.h"
#include "widget.h"
#include "client_functions.h"

client_function commands[] = {
	{"test_func", test_func_func},
	{"hello", hello_func},
	{"client_set", client_set_func},
	{"client_add_key", client_add_key_func},
	{"client_del_key", client_del_key_func},
	{"screen_add_key", screen_add_key_func},
	{"screen_del_key", screen_del_key_func},
	{"screen_add", screen_add_func},
	{"screen_del", screen_del_func},
	{"screen_set", screen_set_func},
	{"widget_add", widget_add_func},
	{"widget_del", widget_del_func},
	{"widget_set", widget_set_func},
	{"menu_add", menu_add_func},
	{"menu_del", menu_del_func},
	{"menu_set", menu_set_func},
	// TODO:  Adhere these to the naming convention?
	{"menu_item_add", menu_item_add_func},
	{"menu_item_del", menu_item_del_func},
	{"menu_item_set", menu_item_set_func},
	// Misc stuff...
	{"backlight", backlight_func},
	{"output", output_func},
	{NULL, NULL},
};

// TODO:  Maybe more error-checking for "->"'s?

///////////////////////////////////////////////////////////////////////////
// Debugging only..  prints out a list of arguments it receives
//
int
test_func_func (client * c, int argc, char **argv)
{
	int i;
	char str[256];

	for (i = 0; i < argc; i++) {
		sprintf (str, "test_func_func:  %i -> %s\n", i, argv[i]);
		printf (str);
		sock_send_string (c->sock, str);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// The client must say "hello" before doing anything else.
//
// It returns a string of info about the server to the client
//
int
hello_func (client * c, int argc, char **argv)
{
	char str[256];

	// TODO:  Give *real* info about the server/lcd...

	debug ("Hello!\n");

	sprintf (str, "connect LCDproc %s protocol %s lcd wid %i hgt %i cellwid %i cellhgt %i\n", version, protocol_version, lcd.wid, lcd.hgt, lcd.cellwid, lcd.cellhgt);
	sock_send_string (c->sock, str);

	if (c->data)
		c->data->ack = 1;

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// sets info about the client, such as its name
//
int
client_set_func (client * c, int argc, char **argv)
{
	int i;

	if (!c->data->ack)
		return 1;

	for (i = 1; i < argc; i++) {
		// Handle the "name" parameter
		if (0 == strcmp (argv[i], "-name")) {
			if (argc > i + 1) {
				i++;
				debug ("client_set: name=\"%s\"\n", argv[i]);

				// set the name...
				if (c->data->name)
					free (c->data->name);
				c->data->name = strdup (argv[i]);
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? name requires a parameter\n");
			}
		} else {
			sock_send_string (c->sock, "huh? invalid parameter\n");
		}
	}

	// If there were no parameters...
	if (argc == 1)
		sock_send_string (c->sock, "huh? What do you want to set?\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Tells the server the client would like to accept keypresses
// of a particular type
//
int
client_add_key_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.  Try using screen_add_key.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Tells the server the client would NOT like to accept keypresses
// of a particular type
//
int
client_del_key_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.  Try using screen_del_key.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Tells the server the client would like to accept keypresses
// of a particular type when the given screen is active on the display
//
int
screen_add_key_func (client * c, int argc, char **argv)
{
	widget *  w ;  /* Keys are stored on a WID_KEYS widget */
	screen *  s ;  /* Attached to a specific screen */
	char *  id ;  /* Screen ID */
	char *  keys ;  /* Keys wanted */

	if (!c->data->ack)
		return 1;

	if (argc < 2) {
		sock_send_string (c->sock, "huh? You must specify a screen id\n");
		return 0;
	}
	if (argc < 3) {
		sock_send_string (c->sock, "huh? You must specify a key list\n");
		return 0;
	}
	if (argc > 3) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}
	id = argv[1];
	keys = argv[2];
	debug ("screen_add_key: Adding key(s) %s to screen %s\n", keys, id);

	// Find the screen
	s = screen_find (c, id);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	// Find the keys widget
	w = widget_find( s, KEYS_WIDGETID );
	if (!w) {
		int  err ;
		// No keys widget, create a new one
		err = widget_add (s, KEYS_WIDGETID, types[WID_KEYS], NULL, c->sock);
		if (err < 0) {
			fprintf (stderr, "screen_add_key: Error creating keys widget\n");
		} else {
			w = widget_find (s, KEYS_WIDGETID );
		}
	};

	if (w) {
		// Have a widget
		if (!w->text) {
			// Save supplied key list
			w->text = strdup( keys );
        } else {
			// Add supplied keys to existing list
			// NOTE: There could be duplicates in the resulting list
			//    That's OK, it's the existence of the key in the list
			//    that's important.  We'll be more careful in the delete
			//    key function.
			char *  new ;
			new = realloc( w->text, strlen(w->text) + strlen(keys) +1 );
			if( new ) {
				w->text = new ;
				strcat( new, keys );
			}
		}
	}

	if (!w) {
		sock_send_string(c->sock, "huh? failed\n");
	} else
		sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Tells the server the client would NOT like to accept keypresses
// of a particular type when the given screen is active on the display
//
int
screen_del_key_func (client * c, int argc, char **argv)
{
	widget *  w ;  /* Keys are stored on a WID_KEYS widget */
	screen *  s ;  /* Attached to a specific screen */
	char *  id ;  /* Screen ID */
	char *  keys ;  /* Keys wanted */

	if (!c->data->ack)
		return 1;

	if (argc < 2) {
		sock_send_string (c->sock, "huh? You must specify a screen id\n");
		return 0;
	}
	if (argc < 3) {
		sock_send_string (c->sock, "huh? You must specify a key list\n");
		return 0;
	}
	if (argc > 3) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}
	id = argv[1] ;
	keys = argv[2] ;
	debug ("screen_del_key: Deleting key(s) %s from screen %s\n", keys, id);
	// Find the screen
	s = screen_find (c, id);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	// Find the keys widget
	w = widget_find( s, KEYS_WIDGETID );
	if (w && w->text) {
		// Got the widget, remove keys from the text member
		// NOTE: We let malloc/realloc remember the length
		//    of the allocated storage.  If keys are later
		//    added, realloc (in add_key above) will make
		//    sure there is enough space at w->text
		char *  from ;
		char *  to ;

		to = from = w->text ;
		while( *from ) {
			//  Is this key to be deleted from the list?
			if( strchr( keys, *from ) ) {
				// Yes, skip it
				++from ;
			} else {
				// No, save it
				*to++ = *from++ ;
			}
		}
	}

	sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Tells the server the client has another screen to offer
//
int
screen_add_func (client * c, int argc, char **argv)
{
	int err = 0;

	if (!c->data->ack)
		return 1;

	if (argc < 2) {
		sock_send_string (c->sock, "huh?  Specify a screen #id\n");
		return 0;
	}
	if (argc > 2) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	debug ("screen_add: Adding screen %s\n", argv[1]);
	err = screen_add (c, argv[1]);
	if (err < 0) {
		fprintf (stderr, "screen_add_func:  Error adding screen\n");
		sock_send_string (c->sock, "huh? failed to add screen id#\n");
	}
	if (err > 0)
		sock_send_string (c->sock, "huh? You already have a screen with that id#\n");
	if (err == 0)
		sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Client requests that the server forget about a screen
//
int
screen_del_func (client * c, int argc, char **argv)
{
	int err = 0;

	if (!c->data->ack)
		return 1;

	if (argc < 2) {
		sock_send_string (c->sock, "huh?  Specify a screen #id\n");
		return 0;
	}
	if (argc > 2) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	debug ("screen_del: Deleting screen %s\n", argv[1]);
	err = screen_remove (c, argv[1]);
	if (err < 0) {
		fprintf (stderr, "screen_del_func:  Error removing screen\n");
		sock_send_string(c->sock, "huh? failed to remove screen\n");
	}
	if (err > 0)
		sock_send_string (c->sock, "huh? You don't have a screen with that id#\n");
	if ( err == 0 )
		sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Configures info about a particular screen, such as its
//  name, priority, or duration
//
int
screen_set_func (client * c, int argc, char **argv)
{
	int i;

	int number;
	char *id;
	screen *s;

	if (!c->data->ack)
		return 1;

	// If there weren't enough parameters...
	if (argc < 2) {
		sock_send_string (c->sock, "huh? You must specify a screen id\n");
		return 0;
	}

	if (argc == 2) {
		sock_send_string (c->sock, "huh? What do you want to set?\n");
		return 0;
	}

	id = argv[1];
	s = screen_find (c, id);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	// Handle the rest of the parameters
	for (i = 2; i < argc; i++) {
		// Handle the "name" parameter
		if (0 == strcmp (argv[i], "-name")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: name=\"%s\"\n", argv[i]);

				// set the name...
				if (s->name)
					free (s->name);
				s->name = strdup (argv[i]);
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -name requires a parameter\n");
			}
		}
		// Handle the "priority" parameter
		else if (0 == strcmp (argv[i], "-priority")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: priority=\"%s\"\n", argv[i]);

				// set the priority...
				number = atoi (argv[i]);
				if (number > 0)
					s->priority = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -priority requires a parameter\n");
			}
		}
		// Handle the "duration" parameter
		else if (0 == strcmp (argv[i], "-duration")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: duration=\"%s\"\n", argv[i]);

				// set the duration...
				number = atoi (argv[i]);
				if (number > 0)
					s->duration = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -duration requires a parameter\n");
			}
		}
		// Handle the "heartbeat" parameter
		else if (0 == strcmp (argv[i], "-heartbeat")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: heartbeat=\"%s\"\n", argv[i]);

				// set the heartbeat type...
				if (0 == strcmp (argv[i], "on"))
					s->heartbeat = 1;
				if (0 == strcmp (argv[i], "heart"))
					s->heartbeat = 1;
				if (0 == strcmp (argv[i], "normal"))
					s->heartbeat = 1;
				if (0 == strcmp (argv[i], "default"))
					s->heartbeat = 1;
				if (0 == strcmp (argv[i], "off"))
					s->heartbeat = 0;
				if (0 == strcmp (argv[i], "none"))
					s->heartbeat = 0;
				if (0 == strcmp (argv[i], "slash"))
					s->heartbeat = 2;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -heartbeat requires a parameter\n");
			}
		}
		// Handle the "wid" parameter
		else if (0 == strcmp (argv[i], "-wid")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: wid=\"%s\"\n", argv[i]);

				// set the duration...
				number = atoi (argv[i]);
				if (number > 0)
					s->wid = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -wid requires a parameter\n");
			}
		}
		// Handle the "hgt" parameter
		else if (0 == strcmp (argv[i], "-hgt")) {
			if (argc > i + 1) {
				i++;
				debug ("screen_set: hgt=\"%s\"\n", argv[i]);

				// set the duration...
				number = atoi (argv[i]);
				if (number > 0)
					s->hgt = number;
				sock_send_string(c->sock, "success\n");
			} else {
				sock_send_string (c->sock, "huh? -hgt requires a parameter\n");
			}
		} else {
			sock_send_string (c->sock, "huh? invalid parameter\n");
		}
	}									  // done checking argv

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Adds a widget to a screen, but doesn't give it a value
//
int
widget_add_func (client * c, int argc, char **argv)
{
	int err;

	char *type;
	char *sid;
	char *wid;
	char *in = NULL;
	screen *s;

	if (!c->data->ack)
		return 1;

	// If there weren't enough parameters...
	if (argc < 2) {
		sock_send_string (c->sock, "huh? You must specify a screen id\n");
		return 0;
	}
	if (argc < 3) {
		sock_send_string (c->sock, "huh? You must specify a widget id\n");
		return 0;
	}
	if (argc < 4) {
		sock_send_string (c->sock, "huh? You must specify a widget type\n");
		return 0;
	}
	if (argc > 6) {
		sock_send_string (c->sock, "huh? Too many parameters\n");
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}

	type = argv[3];

	// Check for additional flags...
	if (argc > 4) {
		// Handle the "-in" flag to place widgets in a container...
		if (0 == strcmp (argv[4], "-in")) {
			if (argc < 6) {
				sock_send_string (c->sock, "huh? Specify a frame to place widget in\n");
				return 0;
			}
			in = argv[5];
		}
	}
	// Add the widget and set its type...
	err = widget_add (s, wid, type, in, c->sock);
	if (err < 0) {
		fprintf (stderr, "widget_add_func:  Error adding widget\n");
		sock_send_string(c->sock, "huh? failed\n");
	}
	else
		sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Removes a widget from a screen, and forgets about it
//
int
widget_del_func (client * c, int argc, char **argv)
{
	int err = 0;

	char *sid;
	char *wid;
	screen *s;

	if (!c->data->ack)
		return 1;

	// Check the number of parameters...
	if (argc < 2) {
		sock_send_string (c->sock, "huh?  Specify a screen #id\n");
		return 0;
	}
	if (argc < 3) {
		sock_send_string (c->sock, "huh?  Specify a widget #id\n");
		return 0;
	}
	if (argc > 3) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	sid = argv[1];
	wid = argv[2];

	debug ("screen_del: Deleting widget %s.%s\n", sid, wid);

	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
	}

	err = widget_remove (s, wid, c->sock);
	if (err < 0) {
		fprintf (stderr, "widget_del_func:  Error removing widget\n");
		sock_send_string(c->sock, "huh? failed\n");
	}
	else
		sock_send_string(c->sock, "success\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Configures information about a widget, such as its size, shape,
//  contents, position, speed, etc...
//
// Ack!  This is long!
int
widget_set_func (client * c, int argc, char **argv)
{
	int i;

	int x, y;
	int left, top, right, bottom;
	int length, direction;
	int width, height;
	int speed;
	char *sid, *wid;
	screen *s;
	widget *w;

	if (!c->data->ack)
		return 1;

	// If there weren't enough parameters...
	if (argc < 2) {
		sock_send_string (c->sock, "huh? You must specify a screen id\n");
		return 0;
	}
	if (argc < 3) {
		sock_send_string (c->sock, "huh? You must specify a widget id\n");
		return 0;
	}
	if (argc < 4) {
		sock_send_string (c->sock, "huh? You must send some widget data\n");
		return 0;
	}

	sid = argv[1];
	wid = argv[2];
	s = screen_find (c, sid);
	if (!s) {
		sock_send_string (c->sock, "huh? Invalid screen id\n");
		return 0;
	}
	w = widget_find (s, wid);
	if (!w) {
		sock_send_string (c->sock, "huh? Invalid widget id\n");
		// Client Debugging...
		{
			int i;
			fprintf (stderr, "huh? Invalid widget id (%s)\n", wid);
			for (i = 0; i < argc; i++)
				fprintf (stderr, "%s ", argv[i]);
			fprintf (stderr, "\n");
		}
		return 0;
	}
	// FIXME?  Shouldn't this be handled in widget.c?
	i = 3;
	switch (w->type) {
	case WID_STRING:				  // String takes "x y text"
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else					  // Set all the data...
			{
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
				if (w->text)
					free (w->text);
				w->text = strdup (argv[i + 2]);
				if (!w->text) {
					fprintf (stderr, "widget_set_func: Error allocating string\n");
					return -1;
				}
				debug ("Widget %s set to %s\n", wid, w->text);
				sock_send_string(c->sock, "success\n");
			}
		}
		break;
	case WID_HBAR:				  // Hbar takes "x y length"
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				length = atoi (argv[i + 2]);
				w->x = x;
				w->y = y;
				w->length = length;
			}
			debug ("Widget %s set to %i\n", wid, w->length);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_VBAR:				  // Vbar takes "x y length"
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				length = atoi (argv[i + 2]);
				w->x = x;
				w->y = y;
				w->length = length;
			}
			debug ("Widget %s set to %i\n", wid, w->length);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_ICON:				  // Icon takes "x y binary_data"
		if (argc != i + 3)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
				// TODO:  Parse binary data and copy it to widget's data...
			}
		}
		sock_send_string (c->sock, "huh? Widget type not yet implemented\n");
		break;
	case WID_TITLE:				  // title takes "text"
		if (argc != i + 1)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if (w->text)
				free (w->text);
			w->text = strdup (argv[i]);
			if (!w->text) {
				fprintf (stderr, "widget_set_func: Error allocating string\n");
				return -1;
			}
			debug ("Widget %s set to %s\n", wid, w->text);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_SCROLLER:			  // Scroller takes "left top right bottom direction speed text"
		if (argc != i + 7) {
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		} else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0])) || (!isdigit (argv[i + 2][0])) || (!isdigit (argv[i + 3][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				left = atoi (argv[i]);
				//debug("left: %d\n",left);
				top = atoi (argv[i + 1]);
				//debug("top: %d\n",top);
				right = atoi (argv[i + 2]);
				//debug("right: %d\n",right);
				bottom = atoi (argv[i + 3]);
				//debug("bottom: %d\n",bottom);
				direction = (int) (argv[i + 4][0]);
				//debug("dir: %c\n",(char)direction);
				speed = atoi (argv[i + 5]);
				//debug("speed: %d\n",speed);
				// Direction must be v or h
				if (((char) direction != 'h') && ((char) direction != 'v')) {
					sock_send_string (c->sock, "huh? Invalid direction\n");
				} else {
					w->left = left;
					w->top = top;
					w->right = right;
					w->bottom = bottom;
					w->length = direction;
					w->speed = speed;
					if (w->text)
						free (w->text);
					w->text = strdup (argv[i + 6]);
					if (!w->text) {
						sock_send_string(c->sock, "huh? out of memory\n");
						fprintf (stderr, "widget_set_func: Error allocating string\n");
						return -1;
					}
					debug ("Widget %s set to %s\n", wid, w->text);
					sock_send_string(c->sock, "success\n");
				}
			}
		}
		break;
	case WID_FRAME:				  // Frame takes "left top right bottom wid hgt direction speed"
		if (argc != i + 8) {
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		} else {
			if ((!isdigit (argv[i][0])) || (!isdigit (argv[i + 1][0])) || (!isdigit (argv[i + 2][0])) || (!isdigit (argv[i + 3][0])) || (!isdigit (argv[i + 4][0])) || (!isdigit (argv[i + 5][0]))) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else {
				left = atoi (argv[i]);
				//debug("left: %d\n",left);
				top = atoi (argv[i + 1]);
				//debug("top: %d\n",top);
				right = atoi (argv[i + 2]);
				//debug("right: %d\n",right);
				bottom = atoi (argv[i + 3]);
				//debug("bottom: %d\n",bottom);
				width = atoi (argv[i + 4]);
				//debug("right: %d\n",right);
				height = atoi (argv[i + 5]);
				//debug("bottom: %d\n",bottom);
				direction = (int) (argv[i + 6][0]);
				//debug("dir: %c\n",(char)direction);
				speed = atoi (argv[i + 7]);
				//debug("speed: %d\n",speed);
				// Direction must be v or h
				if (((char) direction != 'h') && ((char) direction != 'v')) {
					sock_send_string (c->sock, "huh? Invalid direction\n");
				} else {
					w->left = left;
					w->top = top;
					w->right = right;
					w->bottom = bottom;
					w->wid = width;
					w->hgt = height;
					w->length = direction;
					w->speed = speed;
					debug ("Widget %s set to (%i,%i)-(%i,%i) %ix%i\n", wid, left, top, right, bottom, width, height);
					sock_send_string(c->sock, "success\n");
				}
			}
		}
		break;
	case WID_NUM:					  // Num takes "x num"
		if (argc != i + 2)
			sock_send_string (c->sock, "huh? Wrong number of arguments\n");
		else {
			if (!isdigit (argv[i][0])) {
				sock_send_string (c->sock, "huh? Invalid coordinates\n");
			} else if (!isdigit (argv[i + 1][0])) {
				sock_send_string (c->sock, "huh? Invalid number\n");
			} else {
				x = atoi (argv[i]);
				y = atoi (argv[i + 1]);
				w->x = x;
				w->y = y;
			}
			debug ("Widget %s set to %i\n", wid, w->y);
			sock_send_string(c->sock, "success\n");
		}
		break;
	case WID_NONE:
	default:
		sock_send_string (c->sock, "huh? Widget has no type\n");
		break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Adds a menu to the client; handled by the server...
//
int
menu_add_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Removes a client's menu and all contents from the server
//
int
menu_del_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Sets info about a menu, but not its items
//
// For example, should the menu be top-level, or buried somewhere?
//
int
menu_set_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Adds an item to a menu
//
int
menu_item_add_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Deletes an item from a menu
//
int
menu_item_del_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Sets the info about a menu item
//
// For example, text displayed, widget type, value, etc...
//
int
menu_item_set_func (client * c, int argc, char **argv)
{

	if (!c->data->ack)
		return 1;

	sock_send_string (c->sock, "huh?  Not implemented yet.\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Toggles the backlight, if enabled.
//
int
backlight_func (client * c, int argc, char **argv)
{
	if (!c->data->ack)
		return 1;

	// Check the number of parameters...
	if (argc < 2) {
		sock_send_string (c->sock, "huh?  Specify a screen #id\n");
		return 0;
	}

	if (argc > 2) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	debug ("backlight(%s)\n", argv[1]);

	switch (backlight) {
	case BACKLIGHT_OPEN:
		if (0 == strcmp ("on", argv[1])) {
			backlight_state = BACKLIGHT_ON;
		}
		if (0 == strcmp ("off", argv[1])) {
			backlight_state = BACKLIGHT_OFF;
		}
		if (0 == strcmp ("toggle", argv[1])) {
			if (backlight_state == BACKLIGHT_ON)
				backlight_state = BACKLIGHT_OFF;
			else if (backlight_state == BACKLIGHT_OFF)
				backlight_state = BACKLIGHT_ON;
		}
		if (0 == strcmp ("blink", argv[1])) {
			backlight_state |= BACKLIGHT_BLINK;
		}
		if (0 == strcmp ("flash", argv[1])) {
			backlight_state |= BACKLIGHT_FLASH;
		}
		break;
	case BACKLIGHT_VIS:
		break;
	}
	sock_send_string(c->sock, "success\n");

	return 0;

}

////////////////////////////////////////////////////////////////////////////
// Sets the output port on MtxOrb LCDs
int
output_func (client * c, int argc, char **argv)
{
	int rc = 0;
	if (argc != 2) {
		sock_send_string (c->sock, "huh?  Too many parameters...\n");
		rc = 1;
	} else {
		if (0 == strcmp (argv[1], "on"))
			/* switch all ouputs on */
			output_state = -1;
	        else if (0 == strcmp (argv[1], "off"))
			/* switch all ouputs off */
	                output_state = 0;
		else {
			long out;
			char *endptr;

			out = strtol(argv[1], &endptr, 0);
			if ( (out == 0) && (errno != 0) ) {
				sock_send_string (c->sock,
					 "huh?  invalid parameter...\n");
				rc = 1;
			}
			else {
				output_state = out;
			}
		}
	}

	if ( rc == 0 ) {
		sock_send_string(c->sock, "success\n");
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
