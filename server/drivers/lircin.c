/*  This is the LCDproc driver for LIRC infrared devices (http://www.lirc.org)

    Copyright (C) 2000, Harald Klein
		  2002, Rene Wagner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
*/


#include <stdlib.h>
#include <fcntl.h>
#include <lirc/lirc_client.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "lcd.h"
#include "lircin.h"

#include "report.h"

char *progname = "lircin";

static int fd;

static struct lirc_config *config;

MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "lircin_";

/***********************************************************
 * init() should set up any device-specific stuff, and
 * point all the function pointers.
 */
MODULE_EXPORT int
lircin_init (Driver *drvthis, char *args)
{

/* open socket to lirc */

	if (-1 == (fd = lirc_init ("lcdd", LIRCIN_VERBOSELY))) {
		report( RPT_ERR, "lirc: Could not connect to lircd." );
		return -1;
	}

	if (0 != lirc_readconfig (NULL, &config, NULL)) {
		lirc_deinit ();
		report( RPT_ERR, "lirc: lirc_readconfig() failed." );
		return -1;
	}
	fcntl (fd, F_SETFL, O_NONBLOCK);
	fcntl (fd, F_SETFD, FD_CLOEXEC);

/* socket shouldn block lcdd */

	fcntl (fd, F_SETFL, O_NONBLOCK);
	fcntl (fd, F_SETFD, FD_CLOEXEC);

	return 0;
}

/*********************************************************************
 * Closes the device
 */
MODULE_EXPORT void
lircin_close (Driver *drvthis)
{
	lirc_freeconfig (config);
	lirc_deinit ();
}

/*********************************************************************
 * Tries to read a character from an input device...
 *
 * Return NULL for "nothing available".
 */
MODULE_EXPORT char *
lircin_get_key (Driver *drvthis)
{
	char *code=NULL, *cmd=NULL;

	if ( (lirc_nextcode(&code)==0) && (code!=NULL) ) {
		if ( (lirc_code2char(config,code,&cmd)==0) && (cmd!=NULL) ) {
			report (RPT_DEBUG, "lirc: \"%s\"", cmd);
		}
		free (code);
	}
	return cmd;
}
