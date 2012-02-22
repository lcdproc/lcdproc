/** \file server/input.c
 * Handles keypad (and other?) input from the user.
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
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/LL.h"

#include "drivers.h"

#define INC_TYPES_ONLY 1
#include "client.h"
#include "screen.h"
#undef INC_TYPES_ONLY
#include "screenlist.h"
#include "menuscreens.h"
#include "input.h"
#include "render.h" /* For server_msg* */


LinkedList *keylist;
char *toggle_rotate_key;
char *prev_screen_key;
char *next_screen_key;
char *scroll_up_key;
char *scroll_down_key;

/* Local functions */
int server_input(int key);
void input_send_to_client(Client *c, const char *key);
void input_internal_key(const char *key);


int input_init(void)
{
	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	keylist = LL_new();

	/* Get rotate/scroll keys from config file */
	toggle_rotate_key = strdup(config_get_string("server", "ToggleRotateKey", 0, "Enter"));
	prev_screen_key = strdup(config_get_string("server", "PrevScreenKey", 0, "Left"));
	next_screen_key = strdup(config_get_string("server", "NextScreenKey", 0, "Right"));
	scroll_up_key = strdup(config_get_string("server", "ScrollUpKey", 0, "Up"));
	scroll_down_key = strdup(config_get_string("server", "ScrollDownKey", 0, "Down"));

	return 0;
}


int input_shutdown()
{
	if (!keylist) {
		/* Program shutdown before completed startup */
		return -1;
	}

	free(keylist);

	free(toggle_rotate_key);
	free(prev_screen_key);
	free(next_screen_key);
	free(scroll_up_key);
	free(scroll_down_key);

	return 0;
}


int
handle_input(void)
{
	const char *key;
	Screen *current_screen;
	Client *current_client;
	Client *target;
	KeyReservation *kr;

	debug(RPT_DEBUG, "%s()", __FUNCTION__);

	current_screen = screenlist_current();
	if (current_screen)
		current_client = current_screen->client;
	else
		current_client = NULL;

	/* Handle all keypresses */
	while ((key = drivers_get_key()) != NULL) {

		/* Find what client wants the key */
		kr = input_find_key(key, current_client);
		if (kr) {
			/* A hit ! */
			report(RPT_DEBUG, "%s: reserved key: \"%.40s\"", __FUNCTION__, key);
			target = kr->client;
		} else {
			report(RPT_DEBUG, "%s: left over key: \"%.40s\"", __FUNCTION__, key);
			/*target = current_client;*/
			target = NULL; /* left-over keys are always for internal client */
		}
		if (target == NULL) {
			report(RPT_DEBUG, "%s: key is for internal client", __FUNCTION__);
			input_internal_key(key);
		} else {
			/* It's an external client */
			report(RPT_DEBUG, "%s: key is for external client on socket %d", __FUNCTION__, target->sock);
			input_send_to_client(target, key);
		}
	}
	return 0;
}


void input_send_to_client(Client *c, const char *key)
{
	char *s;
	size_t size = strlen(key) + sizeof("key %s\n"); // this is large enough

	debug(RPT_DEBUG, "%s(client=[%d], key=\"%.40s\")", __FUNCTION__, c->sock, key);

	/* Allocate just as much as we need */
	s = calloc(1, size);
	if (s != NULL) {
		snprintf(s, size, "key %s\n", key);
		sock_send_string(c->sock, s);
		free(s);
	}
	else
		report(RPT_ERR, "%s: malloc failure", __FUNCTION__);
}


void
input_internal_key(const char *key)
{
	if (is_menu_key(key) || screenlist_current() == menuscreen) {
		menuscreen_key_handler(key);
	}
	else {
		/* Keys are for scrolling or rotating */
		if (strcmp(key, toggle_rotate_key) == 0) {
			autorotate = !autorotate;
			if (autorotate) {
				server_msg("Rotate", 4);
			} else {
				server_msg("Hold", 4);
			}
		}
		else if (strcmp(key, prev_screen_key) == 0) {
			screenlist_goto_prev();
			server_msg("Prev", 4);
		}
		else if (strcmp(key, next_screen_key) == 0) {
			screenlist_goto_next();
			server_msg("Next", 4);
		}
		else if (strcmp(key, scroll_up_key) == 0) {
		}
		else if (strcmp(key, scroll_down_key) == 0) {
		}
	}
}

int input_reserve_key(const char *key, bool exclusive, Client *client)
{
	KeyReservation *kr;

	debug(RPT_DEBUG, "%s(key=\"%.40s\", exclusive=%d, client=[%d])",
		__FUNCTION__, key, exclusive, (client?client->sock:-1));

	/* Find out if this key is already reserved in a way that interferes
	 * with the new reservation.
	 */
	for (kr = LL_GetFirst(keylist); kr != NULL; kr = LL_GetNext(keylist)) {
		if (strcmp(kr->key, key) == 0) {
			if (kr->exclusive || exclusive) {
				/* Sorry ! */
				return -1;
			}
		}
	}

	/* We can now safely add it ! */
	kr = malloc(sizeof(KeyReservation));
	kr->key = strdup(key);
	kr->exclusive = exclusive;
	kr->client = client;
	LL_Push(keylist, kr);

	report(RPT_INFO, "Key \"%.40s\" is now reserved %s by client [%d]",
		key, (exclusive ? "exclusively" : "shared"), (client ? client->sock : -1));

	return 0;
}

void input_release_key(const char *key, Client *client)
{
	KeyReservation *kr;

	debug(RPT_DEBUG, "%s(key=\"%.40s\", client=[%d])", __FUNCTION__, key, (client ? client->sock : -1));

	for (kr = LL_GetFirst(keylist); kr != NULL; kr = LL_GetNext(keylist)) {
		if ((kr->client == client) && (strcmp(kr->key, key) == 0)) {
			report(RPT_INFO, "Key \"%.40s\" reserved %s by client [%d] and is now released",
				key, (kr->exclusive ? "exclusively" : "shared"), (client ? client->sock : -1));
			free(kr->key);
			free(kr);
			LL_DeleteNode(keylist, NEXT);
			return;
		}
	}
}

void input_release_client_keys(Client *client)
{
	KeyReservation *kr;

	debug(RPT_DEBUG, "%s(client=[%d])", __FUNCTION__, (client ? client->sock : -1));

	for (kr = LL_GetFirst(keylist); kr != NULL; kr = LL_GetNext(keylist)) {
		if (kr->client == client) {
			report(RPT_INFO, "Key \"%.40s\" reserved %s by client [%d] and is now released",
				kr->key, (kr->exclusive ? "exclusively" : "shared"), (client ? client->sock : -1));
			free(kr->key);
			free(kr);
			// jump to node before deleted one to not miss any
			LL_DeleteNode(keylist, PREV);
		}
	}
}

KeyReservation *input_find_key(const char *key, Client *client)
{
	KeyReservation *kr;

	debug(RPT_DEBUG, "%s(key=\"%.40s\", client=[%d])", __FUNCTION__, key, (client?client->sock:-1));

	for (kr = LL_GetFirst(keylist); kr != NULL; kr = LL_GetNext(keylist)) {
		if (strcmp(kr->key, key) == 0) {
			if (kr->exclusive || client == kr->client) {
				return kr;
			}
		}
	}
	return NULL;
}
