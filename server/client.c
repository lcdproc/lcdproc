/*
 * client_data.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "config.h"

#include "client.h"
#include "screenlist.h"
#include "render.h"
#include "shared/report.h"
#include "shared/LL.h"

Client * client_create (int sock)
{
	Client *c;

	debug (RPT_DEBUG, "new_client(%i)", sock);

	/* Allocate new client...*/
	c = malloc (sizeof (Client));
	if (!c) {
		report (RPT_ERR, "client_create: Error allocating");
		return NULL;
	}
	/* Init struct members*/
	c->sock = sock;
	c->messages = NULL;
	c->backlight = BACKLIGHT_OPEN;
	c->heartbeat = HEARTBEAT_OPEN;

	/*Set up message list...*/
	c->messages = LL_new ();
	if (!c->messages) {
		report (RPT_ERR, "client_create: Error allocating");
		free (c);
		return NULL;
	}

	c->ack = 0;
	c->name = NULL;
	c->client_keys = NULL;

	c->screenlist = LL_new();

	if (!c->screenlist) {
		report( RPT_ERR, "client_create: Error allocating");
		return NULL;
	}
	return c;
}

int
client_destroy (Client * c)
{
	//int err;
	Screen *s;

	debug (RPT_INFO, "client_destroy()");

	if (!c)
		return -1;

	client_close_sock (c);

	/*Free client's other data*/
	c->ack = 0;

	/* Clean up the name...*/
	if (c->name)
		free (c->name);

	/* Clean up the key list...*/
	//if (d->client_keys)
	//	free (d->client_keys);

	/* Clean up the screenlist...*/
	debug( RPT_DEBUG, "client_data_destroy: Cleaning screenlist");

	for( s=LL_GetFirst (c->screenlist); s; s=LL_GetNext(c->screenlist) ) {
		debug( RPT_DEBUG, "client_data_destroy: removing screen %s", s->id);

		/* FIXME? This shouldn't be handled here...
		 * Now, remove it from the screenlist...*/
		if (screenlist_remove_all (s) < 0) {
			/* Not a serious error..*/
			report( RPT_ERR, "client_data_destroy:  Error dequeueing screen");
			return 0;
		}
		/* Free its memory...*/
		screen_destroy (s);
		/* Note that the screen is not removed from the list because
		 * the list will be destroyed anyway...
		 */
	}
	LL_Destroy( c->screenlist);

	/* Remove structure */
	free (c);

	return 0;
}

void client_close_sock (Client * c)
{
	char *str;

	if (c->sock == EOF )
		return;

	/*Eat the rest of the incoming requests...*/
	debug (RPT_DEBUG, "client_destroy: get_messages");
	while ((str = client_get_message (c))) {
		if (str) {
			debug (RPT_DEBUG, "client_destroy: kill message %s", str);
			free (str);
		}
	}

	/*close socket...*/
	if (c->sock) {
		/*sock_send_string (c->sock, "bye\n");*/
		close(c->sock);
		report(RPT_NOTICE, "closed socket for #%d", c->sock);
	}
	c->sock = EOF;
}

/*Add and remove messages from the client's queue...*/
int
client_add_message (Client * c, char *message)
{
	int err = 0;
	char *dup;
	char *str, *cp;
	char delimiters[] = "\n\r\0";
/*  int len;*/

	debug(RPT_DEBUG, "client_add_message(%s)", message);

	if (!c)
		return -1;
	if (!message)
		return -1;

/*  len = strlen(message);
 *   if(len < 1) return 0;
 */

	/* Copy the string to avoid overwriting the original...*/
	dup = strdup (message);
	if (!dup) {
		report(RPT_ERR, "client_add_message: Error allocating");
		return -1;
	}
	/* Now split the string into lines and enqueue each one...*/
	for (str = strtok (dup, delimiters); str; str = strtok (NULL, delimiters)) {
		cp = strdup (str);
		debug (RPT_DEBUG, "client_add_message: %s", cp);
		err += LL_Enqueue (c->messages, (void *) cp);
	}

	/*debug(RPT_DEBUG, "client_add_message(%s): %i errors", message, err);*/
	free (dup);						  /* Fixed memory leak...*/

	/* Err is the number of errors encountered...*/
	return err;

}

/* Woo-hoo!  A simple function.  :)*/
char *
client_get_message (Client * c)
{
	char *str;

	debug(RPT_DEBUG, "client_get_message()");

	if (!c)
		return NULL;

	str = (char *) LL_Dequeue (c->messages);

	/*debug(RPT_DEBUG, "client_get_message:  \"%s\"", str);*/

	return str;
}


Screen *
client_find_screen (Client * c, char *id)
{
	Screen *s;

	if (!c)
		return NULL;
	if (!id)
		return NULL;

	debug (RPT_INFO, "client_find_screen(%s)", id);

	LL_Rewind (c->screenlist);
	do {
		s = LL_Get (c->screenlist);
		if ((s) && (0 == strcmp (s->id, id))) {
			debug (RPT_DEBUG, "client_find_screen: Found %s", id);
			return s;
		}
	} while (LL_Next (c->screenlist) == 0);

	return NULL;
}

int
client_add_screen (Client * c, Screen * s)
{
	/* TODO:  Check for errors here?*/
	LL_Push (c->screenlist, (void *) s);

	/* Now, add it to the screenlist...*/
	if (screenlist_add (s) < 0) {
		report (RPT_ERR, "client_add_screen: Error queueing new screen");
		return -1;
	}

	return 0;
}

int
client_remove_screen (Client * c, Screen * s)
{
	if (!c)
		return -1;
	if (!s)
		return -1;

	/* TODO:  Check for errors here?*/
	LL_Remove (c->screenlist, (void *) s);

	/* Now, remove it from the screenlist...*/
	if (screenlist_remove_all (s) < 0) {
		/* Not a serious error..*/
		report (RPT_ERR, "client_remove_screen: Error dequeueing screen");
		return 0;
	}
	return 0;
}

int client_screen_count (Client * c)
{
	return LL_Length(c->screenlist);
}