/*
 * input.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2003, Joris Robijn
 *
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>

/* Accepts and uses keypad input while displaying screens... */
int handle_input ();

/* These defines should be used by drivers for version 0.4.3 of LCDproc
 * as return values for _getkey().
 * You should not change these values, as some drivers still return
 * A, B, C; D directly without using these defines!
 */
#define INPUT_PAUSE_KEY         'A'
#define INPUT_BACK_KEY          'B'
#define INPUT_FORWARD_KEY       'C'
#define INPUT_MAIN_MENU_KEY     'D'

#ifndef bool
# define bool short
# define true 1
# define false 0
#endif

typedef struct KeyReservation {
	char * key;
	bool exclusive;
	Client * client;	/* NULL for internal clients */
} KeyReservation;


int input_init();
	/* Init the input handling system */

int input_shutdown();
	/* Shut it down */

int input_reserve_key (const char * key, bool exclusive, Client * client);
	/* Reserves a key for a client */
	/* Return -1 if reservation of key is not possible */

void input_release_key (const char * key, Client * client);
	/* Releases a key reservation */

void input_release_client_keys (Client * client);
	/* Releases all key reservations for a given client */

KeyReservation * input_find_key (const char * key, Client * client);
	/* Finds if a key reservation causes a 'hit'.
	 * If the key was reserved exclusively, the client will be ignored.
	 * If the key was reserved shared, the client must match.
	 */

#endif
