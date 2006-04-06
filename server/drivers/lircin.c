/*  This is the LCDproc driver for LIRC infrared devices (http://www.lirc.org)

    Copyright (C) 2000, Harald Klein
                  2002, Rene Wagner
    [Merged some stuff from a different lircin driver, so:]
                  1999, David Glaude

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
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <lirc/lirc_client.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "lcd.h"
#include "lircin.h"

#include "report.h"

typedef struct lircin_private_data {
	char *lircrc;
	char *prog;
	int lircin_fd;
	struct lirc_config *lircin_irconfig;
} PrivateData;


MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "lircin_";

/***********************************************************
 * init() should set up any device-specific stuff, and
 * point all the function pointers.
 */
MODULE_EXPORT int
lircin_init (Driver *drvthis)
{
	char s[256] = "";

	PrivateData *p;

	/* Alocate and store private data */
        p = (PrivateData *) malloc( sizeof( PrivateData) );
	if( ! p ) {
		report( RPT_ERR, "%s: Could not allocate private data.", drvthis->name );
	        return -1;
	}
	if( drvthis->store_private_ptr( drvthis, p ) ) {
		report( RPT_ERR, "%s: Could not store private data.", drvthis->name );
	        return -1;
	}

	/* Initialise the PrivateData structure */
	p->lircrc = NULL;
	p->prog = NULL;
	p->lircin_irconfig = NULL;
	p->lircin_fd = -1;

	/* READ CONFIG FILE:*/

	/* Get location of lircrc to be used */
	if (drvthis->config_get_string ( drvthis->name , "lircrc" , 0 , NULL) != NULL)
		strncpy(s, drvthis->config_get_string ( drvthis->name , "lircrc" , 0 , ""), sizeof(s));

	if (*s != '\0') {
		s[sizeof(s)-1] = '\0';
		p->lircrc = malloc(strlen(s) + 1);
		if( p->lircrc == NULL ) {
			report( RPT_ERR, "%s: Could not allocate new memory.", drvthis->name );
			return -1;
		}
		strcpy(p->lircrc, s);
		report  (RPT_INFO,"%s: Using lircrc: %s", drvthis->name, p->lircrc);
	}
	else {
		report (RPT_INFO,"%s: Using default lircrc: ~/.lircrc", drvthis->name);
	}

	/* Get program identifier "prog=..." to be used */
	strncpy(s, drvthis->config_get_string ( drvthis->name , "Prog" , 0 , LIRCIN_DEF_PROG), sizeof(s));

	p->prog = malloc(strlen(s) + 1);
	if( p->prog == NULL ) {
		report( RPT_ERR, "%s: Could not allocate new memory.", drvthis->name );
		return -1;
	}
	strcpy(p->prog, s);
	report  (RPT_INFO, "%s: Using prog: %s", drvthis->name, p->prog);

	/* End of config file parsing */

	/* open socket to lirc */
	if (-1 == (p->lircin_fd = lirc_init (p->prog, LIRCIN_VERBOSELY))) {
		report( RPT_ERR, "%s: Could not connect to lircd.", drvthis->name );

		lircin_close( drvthis);
		return -1;
	}

	if (0 != lirc_readconfig (p->lircrc, &p->lircin_irconfig, NULL)) {
		report( RPT_ERR, "%s: lirc_readconfig() failed.", drvthis->name );

		lircin_close( drvthis);
		return -1;
	}

	/* socket shouldn't block lcdd */
	if (fcntl(p->lircin_fd, F_SETFL, O_NONBLOCK) < 0){
                report(RPT_ERR, "%s: Unable to change lircin_fd(%d) to O_NONBLOCK: %s",
			drvthis->name, p->lircin_fd, strerror(errno));

		lircin_close( drvthis);
		return -1;
		}
	fcntl (p->lircin_fd, F_SETFD, FD_CLOEXEC);


	return 0;
}

/*********************************************************************
 * Closes the device
 */
MODULE_EXPORT void
lircin_close (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	if (p != NULL) {
		if (p->lircrc != NULL)
			free(p->lircrc);
		p->lircrc = NULL;

		if (p->prog != NULL)
			free(p->prog);
		p->prog = NULL;
		
		if (p->lircin_irconfig != NULL)
			lirc_freeconfig (p->lircin_irconfig);
		p->lircin_irconfig = NULL;
		
		if (p->lircin_fd >= 0) {
			lirc_deinit ();
			close (p->lircin_fd);
		}	
		p->lircin_fd = -1;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/*********************************************************************
 * Tries to read a character from an input device...
 *
 * Return NULL for "nothing available".
 */
MODULE_EXPORT const char *
lircin_get_key (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	char *code = NULL, *cmd = NULL;

	if ( (lirc_nextcode(&code) == 0) && (code != NULL) ) {
		if ( (lirc_code2char(p->lircin_irconfig,code,&cmd)==0) && (cmd!=NULL) ) {
			report (RPT_DEBUG, "%s: \"%s\"", drvthis->name, cmd);
		}
		free (code);
	}

	return cmd;
}
