/*
 * client_data.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *
 *
 * Creates and destroys a client's data structures.  These are mainly
 * its name, screen list, and menu list.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared/report.h"

#include "client_data.h"
#include "screen.h"
#include "screenlist.h"

#define ResetScreenList(a)	LL_Rewind(a)
#define NewScreen		LL_new
#define NextScreen(a)		(screen *)LL_Get(a)
#define MoreScreens(a)		(LL_Next(a) == 0)
#define DestroyScreenList(a)	LL_Destroy(a)

int
client_data_init (client_data * d)
{
	if (!d)
		return -1;

	d->ack = 0;
	d->name = NULL;
	d->client_keys = NULL;

	d->screenlist = NewScreen();
	if (!d->screenlist) {
		report( RPT_ERR, "client_data_init: Error allocating screenlist");
		return -1;
	}
	/* TODO:  this section...  (client menus)
	   d->menulist = LL_new();
	   if(!d->menulist)
	   {
	   report( RPT_ERR, "client_data_init: Error allocating menulist");
	   return -1;
	   }
	 */
	return 0;
}

int
client_data_destroy (client_data * d)
{
	screen *s;

	report( RPT_INFO, "client_data_destroy");

	if (!d)
		return -1;

	d->ack = 0;

	/* Clean up the name...*/
	if (d->name)
		free (d->name);

	/* Clean up the key list...*/
	if (d->client_keys)
		free (d->client_keys);

	/* Clean up the screenlist...*/
	debug( RPT_DEBUG, "client_data_destroy: Cleaning screenlist");
	ResetScreenList (d->screenlist);
	do {
		s = NextScreen(d->screenlist);
		if (s) {
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
		}
	} while (MoreScreens(d->screenlist));
	DestroyScreenList(d->screenlist);

	/* TODO:  clean up the rest of the data...*/

	return 0;
}
