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
void input_internal_key (KeyReservation * kr);

LinkedList * keylist;


int init_input()
{
	report (RPT_INFO, "%s()", __FUNCTION__ );

	keylist = LL_new();

	return 0;
}

int
handle_input ()
{
	char * key;
	Client * c;
	KeyReservation * kr;

	report (RPT_INFO, "%s()", __FUNCTION__ );

	c = screenlist_current()->client;

	/* Handle all keypresses */
	while ((key = drivers_get_key ()) != NULL ) {

		/* Find what client wants the key */
		kr = input_find_key (key, c);
		if (kr) {
			/* A hit ! */
			if (kr->client == NULL) {
				report (RPT_DEBUG, "%s: key for internal client: [%.40s]", __FUNCTION__, key );
				input_internal_key (kr);
			}
			else {
				/* It's an external client */
				report (RPT_DEBUG, "%s: key for external client: [%.40s]", __FUNCTION__, key );
				input_send_to_client (c, key);
			}
		}
		else {
			/* What do we do with left-over keys ? */
			report (RPT_INFO, "%s: left over key: [%.40s]", __FUNCTION__, key );

			/* Well... nothing ! */
		}
	}
	return 0;
}

void input_send_to_client (Client * c, char * key)
{
	char * s;

	debug (RPT_DEBUG, "%s( client=%p, key=\"%.40s\" )", __FUNCTION__, c, key);

	/* Allocate just as much as we need */
	s = malloc (strlen(key) + strlen("key \n") + 1);
	sprintf(s, "key %s\n", key);
	sock_send_string(c->sock, s);
	free (s);
}

void
input_internal_key (KeyReservation * kr)
{
	if (kr->exclusive || screenlist_current() == menuscreen) {
		menuscreen_key_handler (kr->key);
	}
	else {
		/* TODO: give keys to server screen */
	}
}


/* TODO: REPLACE THE FOLLOWING FUNCTION BY SOMETHING NEW */
int
server_input (int key)
{
	report(RPT_INFO, "%s( key='%c' )", __FUNCTION__, (char) key);

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

	debug (RPT_DEBUG, "%s( key=\"%.40s\", exclusive=%d, client=%p )", __FUNCTION__, key, exclusive, client);

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

	report (RPT_INFO, "%s: key [%.40s] is now reserved in %s mode", __FUNCTION__, key, (exclusive?"exclusive":"shared"));

	return 0;
}

void input_release_key (char * key, Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( key=\"%.40s\", client=%p )", __FUNCTION__, key, client);

	for (kr=LL_GetFirst(keylist); kr; kr=LL_GetNext(keylist)) {
		if (kr->client == client
		&& strcmp (kr->key, key) == 0) {
			report (RPT_INFO, "%s: key [%.40s] is being released from %s mode", __FUNCTION__, key, (kr->exclusive?"exclusive":"shared"));
			free (kr->key);
			free (kr);
			LL_DeleteNode (keylist);
			return;
		}
	}
}

void input_release_client_keys (Client * client)
{
	KeyReservation * kr;

	debug (RPT_DEBUG, "%s( client=%p )", __FUNCTION__, client);

	kr=LL_GetFirst(keylist);
	while (kr) {
		if (kr->client == client) {
			report (RPT_INFO, "%s: key [%.40s] is now released from %s mode", __FUNCTION__, kr->key, (kr->exclusive?"exclusive":"shared"));
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

	debug (RPT_DEBUG, "%s( key=\"%.40s\", client=%p )", __FUNCTION__, key, client);

	for (kr=LL_GetFirst(keylist); kr; kr=LL_GetNext(keylist)) {
		if (strcmp (kr->key, key) == 0) {
			if (kr->exclusive || client==kr->client) {
				return kr;
			}
		}
	}
	return NULL;
}
