#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "shared/debug.h"

#include "drivers/lcd.h"

#include "clients.h"
#include "screen.h"
#include "screenlist.h"
#include "widget.h"

#include "serverscreens.h"

screen *server_screen;
char *id = "ClientList";
char *name = "Client List";

char title[256] = "LCDproc Server";
char one[256] = "";
char two[256] = "";
char three[256] = "";

#define WidgetXPos(w,a) (w)->x = (a)
#define WidgetYPos(w,a) (w)->y = (a)
#define WidgetText(w,a) (w)->text = (a)
#define WidgetString(w,a,b,t)	{WidgetXPos(w,a);WidgetYPos(w,b);WidgetText(w,t);}

int
server_screen_init ()
{
	widget *w;

	debug ("server_screen_init\n");

	server_screen = screen_create ();

	if (!server_screen) {
		fprintf (stderr, "server_screen_init: Error allocating screen\n");
		return -1;
	}

	server_screen->id = id;
	server_screen->name = name;
	server_screen->duration = 8; // 1 second, instead of 4...

	if (widget_add (server_screen, "title", "title", NULL, 1) != 0) {
		fprintf (stderr, "server_screen_init: internal error: could not add title widget\n");
	}
	if (widget_add (server_screen, "one", "string", NULL, 1) != 0) {
		fprintf (stderr, "server_screen_init: internal error: could not add title widget\n");
	}
	if (widget_add (server_screen, "two", "string", NULL, 1) != 0) {
		fprintf (stderr, "server_screen_init: internal error: could not add title widget\n");
	}
	if (widget_add (server_screen, "three", "string", NULL, 1) != 0) {
		fprintf (stderr, "server_screen_init: internal error: could not add title widget\n");
	}

	// Now, initialize all the widgets...
	if ((w = widget_find (server_screen, "title")) != NULL) {
		WidgetText(w,title);
	} else
		fprintf (stderr, "server_screen_init: Can't find title\n");

	if ((w = widget_find (server_screen, "one")) != NULL)
		WidgetString(w,1,2,one)
	else
		fprintf (stderr, "server_screen_init: Can't find widget one\n");

	if ((w = widget_find (server_screen, "two")) != NULL)
		WidgetString(w,1,3,two)
	else
		fprintf (stderr, "server_screen_init: Can't find widget two\n");

	if ((w = widget_find (server_screen, "three")) != NULL)
		WidgetString(w,1,4,three)
	else
		fprintf (stderr, "server_screen_init: Can't find widget three\n");

	// And enqueue the screen
	screenlist_add (server_screen);

	debug ("server_screen_init done\n");

	return 0;
}

static int
screen_count (client *c) {
	int n;

	n = 0;
	LL_Rewind (c->data->screenlist);
	do {
		if (LL_Get (c->data->screenlist) != NULL)
			n++;
	} while (LL_Next (c->data->screenlist) == 0);

	return n;
}

int
update_server_screen (int timer)
{
	client *c;
	int num_clients;
	screen *s;
	int num_screens;

	// Draw a title...
	//strcpy(title, "LCDproc Server");

	// Now get info on the number of connected clients...
	num_clients = 0;
	num_screens = 0;
	LL_Rewind (clients);
	do {
		c = LL_Get (clients);
		if (c) {
			num_clients++;
			num_screens += screen_count(c);
//			LL_Rewind (c->data->screenlist);
//			do {
//				s = LL_Get (c->data->screenlist);
//				if (s) {
//					num_screens++;
//				}
//			} while (LL_Next (c->data->screenlist) == 0);
		}
	} while (LL_Next (clients) == 0);

	// Format strings for the appropriate size display...
	//
	if (lcd_ptr->hgt >= 3) {
		snprintf (one, sizeof(one), "Clients: %i", num_clients);
		snprintf (two, sizeof(two), "Screens: %i", num_screens);
	} else {
		if (lcd_ptr->wid >= 20)
			snprintf (one, sizeof(one), "%i Client%s, %i Screen%s", num_clients,
				(num_clients == 1) ? "" : "s", num_screens,
				(num_screens == 1) ? "" : "s");
		else							  // 16x2 size
			snprintf (one, sizeof(one), "%i Cli%s, %i Scr%s", num_clients,
				(num_clients == 1) ? "" : "s", num_screens,
				(num_screens == 1) ? "" : "s");
	}

	return 0;
}

int
no_screen_screen (int timer)
{

	lcd_ptr->clear ();
	lcd_ptr->string (1, 1, "Error:  No screen!");
	lcd_ptr->flush ();

	return 0;
}

int
goodbye_screen ()
{
	char *b20 = "                    ";
	char *t20 = "  Thanks for using  ";
#ifdef LINUX
	char *l20 = " LCDproc and Linux! ";
#else
	char *l20 = "      LCDproc!      ";
#endif

	char *b16 = "                ";
	char *t16 = "Thanks for using";
#ifdef LINUX
	char *l16 = " LCDproc+Linux! ";
#else
	char *l16 = "    LCDproc!    ";
#endif

	lcd_ptr->clear ();

	if (lcd_ptr->hgt >= 4) {
		if (lcd_ptr->wid >= 20) {
			lcd_ptr->string (1, 1, b20);
			lcd_ptr->string (1, 2, t20);
			lcd_ptr->string (1, 3, l20);
			lcd_ptr->string (1, 4, b20);
		} else {
			lcd_ptr->string (1, 1, b16);
			lcd_ptr->string (1, 2, t16);
			lcd_ptr->string (1, 3, l16);
			lcd_ptr->string (1, 4, b16);
		}
	} else {
		if (lcd_ptr->wid >= 20) {
			lcd_ptr->string (1, 1, t20);
			lcd_ptr->string (1, 2, l20);
		} else {
			lcd_ptr->string (1, 1, t16);
			lcd_ptr->string (1, 2, l16);
		}
	}

	lcd_ptr->flush ();

	return 0;
}
