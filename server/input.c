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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/sockets.h"
#include "shared/report.h"

#include "drivers.h"

#include "client.h"
#include "screenlist.h"
#include "menuscreens.h"

#include "input.h"

int server_input (int key);
void input_send_to_client (Client * c, char * key);
void input_internal_key (char * key);

LinkedList * keylist;


int init_input()
{
	debug (RPT_DEBUG, "%s()", __FUNCTION__ );

	keylist = LL_new();

	return 0;
}

int
handle_input ()
{
	char * key;
	Client * current_client;
	Client * target;
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s()", __FUNCTION__ );

	current_client = screenlist_current()->client;

	/* Handle all keypresses */
	while ((key = drivers_get_key ()) != NULL ) {

		/* Find what client wants the key */
		kr = input_find_key (key, current_client);
		if (kr) {
			/* A hit ! */
			report (RPT_DEBUG, "%s: reserved key: \"%.40s\"", __FUNCTION__, key );
			target = kr->client;
		} else {
			report (RPT_DEBUG, "%s: left over key: \"%.40s\"", __FUNCTION__, key );
			/*target = current_client;*/
			target = NULL; /* left-over keys are always for internal client */
		}
		if (target == NULL) {
			report (RPT_DEBUG, "%s: key is for internal client", __FUNCTION__ );
			input_internal_key (key);
		} else {
			/* It's an external client */
			report (RPT_DEBUG, "%s: key is for external client on socket %d", __FUNCTION__, target->sock );
			input_send_to_client (current_client, key);
		}
	}
	return 0;
}

void input_send_to_client (Client * c, char * key)
{
	char * s;

	debug (RPT_DEBUG, "%s( client=[%d], key=\"%.40s\" )", __FUNCTION__, c->sock, key);

	/* Allocate just as much as we need */
	s = malloc (strlen(key) + strlen("key \n") + 1);
	sprintf(s, "key %s\n", key);
	sock_send_string(c->sock, s);
	free (s);
}

void
input_internal_key (char * key)
{
	if (is_menu_key(key) || screenlist_current() == menuscreen) {
		menuscreen_key_handler (key);
	}
	else {
		/* TODO: give keys to server screen */
	}
}


/* TODO: REPLACE THE FOLLOWING FUNCTION BY SOMETHING NEW */
int
server_input (int key)
{
	debug(RPT_DEBUG, "%s( key='%c' )", __FUNCTION__, (char) key);

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
			debug (RPT_DEBUG, "%s: got the menu key!", __FUNCTION__);
			//server_menu ();
			report (RPT_ERR, "MENU TEMPORATY DISABLED");
			break;
		default:
			debug (RPT_DEBUG, "%s: Unused key \"%c\" (%i)", __FUNCTION__, (char) key, key);
			break;
	}

	return 0;
}

int input_reserve_key (char * key, bool exclusive, Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( key=\"%.40s\", exclusive=%d, client=[%d] )", __FUNCTION__, key, exclusive, (client?client->sock:-1));

	/* Find out if this key is already reserved in a way that interferes
	 * with the new reservation.
	 */
	for (kr=LL_GetFirst(keylist); kr; kr=LL_GetNext(keylist)) {
		if (strcmp (kr->key, key) == 0) {
			if (kr->exclusive || exclusive) {
				/* Sorry ! */
				return -1;
			}
		}
	}

	/* We can now safely add it ! */
	kr = malloc (sizeof(KeyReservation));
	kr->key = strdup (key);
	kr->exclusive = exclusive;
	kr->client = client;
	LL_Push(keylist, kr);

	report (RPT_INFO, "Key \"%.40s\" is now reserved in %s mode by client [%d]", key, (exclusive?"exclusive":"shared"), (client?client->sock:-1));

	return 0;
}

void input_release_key (char * key, Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( key=\"%.40s\", client=[%d] )", __FUNCTION__, key, (client?client->sock:-1));

	for (kr=LL_GetFirst(keylist); kr; kr=LL_GetNext(keylist)) {
		if (kr->client == client
		&& strcmp (kr->key, key) == 0) {
			free (kr->key);
			free (kr);
			LL_DeleteNode (keylist);
			report (RPT_INFO, "Key \"%.40s\" was reserved in %s mode by client [%d] and is now released", key, (kr->exclusive?"exclusive":"shared"), (client?client->sock:-1));
			return;
		}
	}
}

void input_release_client_keys (Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( client=[%d] )", __FUNCTION__, (client?client->sock:-1));

	kr=LL_GetFirst(keylist);
	while (kr) {
		if (kr->client == client) {
			report (RPT_INFO, "Key \"%.40s\" was reserved in %s mode by client [%d] and is now released", kr->key, (kr->exclusive?"exclusive":"shared"), (client?client->sock:-1));
			free (kr->key);
			free (kr);
			LL_DeleteNode (keylist);
			kr = LL_Get (keylist);
		} else {
			kr = LL_GetNext(keylist);
		}
	}
}

KeyReservation * input_find_key (char * key, Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( key=\"%.40s\", client=[%d] )", __FUNCTION__, key, (client?client->sock:-1));

	for (kr=LL_GetFirst(keylist); kr; kr=LL_GetNext(keylist)) {
		if (strcmp (kr->key, key) == 0) {
			if (kr->exclusive || client==kr->client) {
				return kr;
			}
		}
	}
	return NULL;
}
