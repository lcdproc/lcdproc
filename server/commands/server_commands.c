/*
 * server_commands.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *               2002, Joris Robijn
 *
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 *
 * This particular file defines actions concerning the server settings.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "shared/report.h"
#include "shared/sockets.h"

#include "client.h"
#include "render.h"

/****************************************************************************
 * Sets the state of the output port (such as on MtxOrb LCDs)
 *
 * usage: output <on|off|int>
 */
#define ALL_OUTPUTS_ON -1
#define ALL_OUTPUTS_OFF 0

int
output_func (Client * c, int argc, char **argv)
{
	/*int rc = 0;*/
	char str[128];

	if (argc != 2) {
		if (argc == 1)
			sock_send_string (c->sock, "huh?  usage: output <on|off|num> -- num may be decimal, hex, or octal\n");
		else
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	if (0 == strcmp (argv[1], "on"))
		output_state = ALL_OUTPUTS_ON;
	else if (0 == strcmp (argv[1], "off"))
		output_state = ALL_OUTPUTS_OFF;
	else {
		long out;
		char *endptr, *p;

		/* Note that there is no valid range set for
		 * output_state; thus a value in the 12 digits
		 * is not considered out of range.
		 */

		errno = 0;

		/* errno is set here, because if strtol does not result in
		 * ERANGE (out of range error) it will not set errno (!).
		 * At least, this is the case with glibc 2.1.3 ...
		 */

		p = argv[1];
		out = strtol(p, &endptr, 0);

		/* From the man page for strtol(3)
		 *
		 * In particular, if *nptr is not `\0' but **endptr is
		 * `\0' on return, the entire string is valid.
		 *
		 * In this case, argv[1] is *nptr, and &endptr is **endptr.
		 */

		if (errno) {
			int space;

			strcat(str, "huh? number argument: ");
			space = sizeof(str) - 3 - strlen(str);

			strncat(str, strerror(errno), space);
			strcat(str, "\n");

			sock_send_string (c->sock, str);
			return 0;
		} else if (*p != '\0' && *endptr == '\0') {
			output_state = out;
		} else {
			sock_send_string (c->sock, "huh?  invalid parameter...\n");
			return 0;
		}
	}

	sock_send_string(c->sock, "success\n");

	/* Makes sense to me to set the output immediately;
	 * however, the outputs are currently set in
	 * draw_screen(screen * s, int timer)
	 * Whatever for? */

	/* drivers_output (output_state); */

	report(RPT_NOTICE, "output states changed");
	return 0;
}

/*******************************************************************************
 * sleep_func
 *
 * usage: sleep <seconds>
 */
int
sleep_func (Client * c, int argc, char **argv)
{
	int secs;
	long out;
	char *endptr, *p;
	char str[120];

#define MAX_SECS 60
#define MIN_SECS 1

	if (argc != 2) {
		if (argc == 1)
			sock_send_string (c->sock, "huh?  usage: sleep <secs>\n");
		else
			sock_send_string (c->sock, "huh?  Too many parameters...\n");
		return 0;
	}

	errno = 0;

	/* errno is set here, because if strtol does not result in
	 * ERANGE (out of range error) it will not set errno (!).
	 * At least, this is the case with glibc 2.1.3 ...
	 */

	p = argv[1];
	out = strtol(p, &endptr, 0);

	/* From the man page for strtol(3)
	 *
	 * In particular, if *nptr is not `\0' but **endptr is
	 * `\0' on return, the entire string is valid.
	 *
	 * In this case, argv[1] is *nptr, and &endptr is **endptr.
	 */

	if (errno) {
		int space;

		strcat(str, "huh? number argument: ");
		space = sizeof(str) - 3 - strlen(str);

		strncat(str, strerror(errno), space);
		strcat(str, "\n");

		sock_send_string (c->sock, str);
		return 0;
	} else if (*p != '\0' && *endptr == '\0') {
		secs = out;
		out = out > MAX_SECS ? MAX_SECS : out;
		out = out < MIN_SECS ? MIN_SECS : out;
	} else {
		sock_send_string (c->sock, "huh?  invalid parameter...\n");
		return 0;
	}

	/* Repeat until no more remains - should normally be zero
	 * on exit the first time...*/
	snprintf(str, sizeof(str), "sleeping %d seconds\n", secs);
	sock_send_string (c->sock, str);

	/* whoops.... if this takes place as planned, ALL screens
	 * will "freeze" for the alloted time...
	 *
	 * while ((secs = sleep(secs)) > 0)
	 */	;

	sock_send_string (c->sock, "huh? ignored (not fully implemented)\n");
	return 0;
}

/*****************************************************************************
 * Does nothing, returns "noop complete" message
 *
 * This is useful for shell scripts or programs that want to talk
 *    with LCDproc and not get deadlocked.  Send a noop after each
 *    command and look for the "noop complete" message.
 */
int
noop_func (Client * c, int argc, char **argv)
{
	sock_send_string (c->sock, "noop complete\n");
	return 0;
}

