/** \file server/client.c
 * Define all the client data and actions.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "client.h"
#include "screen.h"
#include "screenlist.h"
#include "render.h"
#include "input.h"
#include "menuscreens.h"
#include "shared/report.h"
#include "shared/LL.h"

Client *client_create(int sock)
{
	Client *c;

	debug(RPT_DEBUG, "%s(sock=%i)", __FUNCTION__, sock);

	/* Allocate new client...*/
	c = malloc(sizeof(Client));
	if (!c) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}
	/* Init struct members*/
	c->sock = sock;
	c->messages = NULL;
	c->backlight = BACKLIGHT_OPEN;
	c->heartbeat = HEARTBEAT_OPEN;

	/*Set up message list...*/
	c->messages = LL_new();
	if (!c->messages) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		free(c);
		return NULL;
	}

	c->state = NEW;
	c->name = NULL;
	c->menu = NULL;

	c->screenlist = LL_new();

	if (!c->screenlist) {
		report(RPT_ERR, "%s: Error allocating", __FUNCTION__);
		return NULL;
	}
	return c;
}

int
client_destroy(Client *c)
{
	Screen *s;
	Menu *m;
	char *str;

	if (!c)
		return -1;

	debug(RPT_DEBUG, "%s(c=[%d])", __FUNCTION__, c->sock);

	/* Close the socket */
	close(c->sock);

	/* Eat messages */
	while ((str = client_get_message(c))) {
		free(str);
	}
	LL_Destroy(c->messages);

	/* Clean up the screenlist...*/
	debug(RPT_DEBUG, "%s: Cleaning screenlist", __FUNCTION__);

	for (s = LL_GetFirst(c->screenlist); s; s = LL_GetNext(c->screenlist)) {
		/* Free its memory...*/
		screen_destroy(s);
		/* Note that the screen is not removed from the list because
		 * the list will be destroyed anyway...
		 */
	}
	LL_Destroy(c->screenlist);

	m = (Menu *) c->menu;
	/* Destroy the client's menu, if it exists */
	if (m) {
		menuscreen_inform_item_destruction(m);
		menu_remove_item(m->parent, m);
		menuscreen_inform_item_modified(m->parent);
		menuitem_destroy(m);
	}

	/* Forget client's key reservations */
	input_release_client_keys(c);

	/* Free client's other data */
	c->state = GONE;

	/* Clean up the name...*/
	if (c->name)
		free(c->name);

	/* Remove structure */
	free(c);

	debug(RPT_DEBUG, "%s: Client data removed", __FUNCTION__);
	return 0;
}

/*Add and remove messages from the client's queue...*/
int
client_add_message(Client *c, char *message)
{
	int err = 0;

	if (!c)
		return -1;
	if (!message)
		return -1;

	if (strlen(message) > 0) {
		debug(RPT_DEBUG, "%s(c=[%d], message=\"%s\")", __FUNCTION__,
			c->sock, message);
		err = LL_Enqueue(c->messages, (void *) message);
	}

	return err;
}

/* Woo-hoo!  A simple function.  :)*/
char *
client_get_message(Client *c)
{
	char *str;

	debug(RPT_DEBUG, "%s(c=[%d])", __FUNCTION__, c->sock);

	if (!c)
		return NULL;

	str = (char *) LL_Dequeue(c->messages);

	return str;
}


Screen *
client_find_screen(Client *c, char *id)
{
	Screen *s;

	if (!c)
		return NULL;
	if (!id)
		return NULL;

	debug(RPT_DEBUG, "%s(c=[%d], id=\"%s\")", __FUNCTION__, c->sock, id);

	LL_Rewind(c->screenlist);
	do {
		s = LL_Get(c->screenlist);
		if ((s) && (0 == strcmp(s->id, id))) {
			debug(RPT_DEBUG, "%s: Found %s", __FUNCTION__, id);
			return s;
		}
	} while (LL_Next(c->screenlist) == 0);

	return NULL;
}

int
client_add_screen(Client *c, Screen *s)
{
	if (!c)
		return -1;
	if (!s)
		return -1;

	debug(RPT_DEBUG, "%s(c=[%d], s=[%s])", __FUNCTION__, c->sock, s->id);

	LL_Push(c->screenlist, (void *) s);

	/* Now, add it to the screenlist...*/
	screenlist_add(s);

	return 0;
}

int
client_remove_screen(Client *c, Screen *s)
{
	if (!c)
		return -1;
	if (!s)
		return -1;

	debug(RPT_DEBUG, "%s(c=[%d], s=[%s])", __FUNCTION__, c->sock, s->id);

	/* TODO:  Check for errors here?*/
	LL_Remove(c->screenlist, (void *) s, NEXT);

	/* Now, remove it from the screenlist...*/
	screenlist_remove(s);

	return 0;
}

int client_screen_count(Client *c)
{
	return LL_Length(c->screenlist);
}
