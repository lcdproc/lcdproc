/** \file server/commands/server_commands.c
 * Implements handlers for client commands concerning the server settings.
 *
 * This contains definitions for all the functions which clients can run.
 * The functions here are to be called only from parse.c's interpreter.
 *
 * The client's available function set is defined here, as is the syntax
 * for each command.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Selene Scriven
 *               2002, Joris Robijn
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
#include "server_commands.h"

#define ALL_OUTPUTS_ON -1
#define ALL_OUTPUTS_OFF 0

/**
 * Sets the state of the output port (such as on MtxOrb LCDs)
 *
 *\verbatim
 * Usage: output <on|off|int>
 *\endverbatim
 */
int
output_func(Client *c, int argc, char **argv)
{
	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: output {on|off|<num>}\n");
		return 0;
	}

	if (0 == strcmp(argv[1], "on"))
		output_state = ALL_OUTPUTS_ON;
	else if (0 == strcmp(argv[1], "off"))
		output_state = ALL_OUTPUTS_OFF;
	else {
		long out;
		char *endptr;

		/* Note that there is no valid range set for
		 * output_state; thus a value in the 12 digits
		 * is not considered out of range.
		 */

		/* set errno to be able to detect errors in strtol() */
		errno = 0;

		out = strtol(argv[1], &endptr, 0);

		if (errno) {
			sock_printf_error(c->sock, "number argument: %s\n", strerror(errno));
			return 0;
		}
		else if ((*argv[1] != '\0') && (*endptr == '\0')) {
			output_state = out;
		}
		else {
			sock_send_error(c->sock, "invalid parameter...\n");
			return 0;
		}
	}

	sock_send_string(c->sock, "success\n");

	/* Makes sense to me to set the output immediately;
	 * however, the outputs are currently set in
	 * draw_screen(screen *s, int timer)
	 * Whatever for? */

	/* drivers_output(output_state); */

	report(RPT_NOTICE, "output states changed");
	return 0;
}

/**
 * The sleep_func was intended to make the server sleep for some seconds.
 * This function is currently ignored as making the server sleep actually
 * stalls it and disrupts other clients.
 *
 *\verbatim
 * Usage: sleep <seconds>
 *\endverbatim
 */
int
sleep_func(Client *c, int argc, char **argv)
{
	int secs;
	long out;
	char *endptr;

#define MAX_SECS 60
#define MIN_SECS 1

	if (c->state != ACTIVE)
		return 1;

	if (argc != 2) {
		sock_send_error(c->sock, "Usage: sleep <secs>\n");
		return 0;
	}

	/* set errno to be able to detect errors in strtol() */
	errno = 0;

	out = strtol(argv[1], &endptr, 0);

	/* From the man page for strtol(3)
	 *
	 * In particular, if *nptr is not `\0' but **endptr is
	 * `\0' on return, the entire string is valid.
	 *
	 * In this case, argv[1] is *nptr, and &endptr is **endptr.
	 */

	if (errno) {
		sock_printf_error(c->sock, "number argument: %s\n", strerror(errno));
		return 0;
	}
	else if ((*argv[1] != '\0') && (*endptr == '\0')) {
		/* limit seconds to range: MIN_SECS - MAX_SECS */
		out = (out > MAX_SECS) ? MAX_SECS : out;
		out = (out < MIN_SECS) ? MIN_SECS : out;
		secs = out;
	}
	else {
		sock_send_error(c->sock, "invalid parameter...\n");
		return 0;
	}

	/* Repeat until no more remains - should normally be zero
	 * on exit the first time...*/
	sock_printf(c->sock, "sleeping %d seconds\n", secs);

	/* whoops.... if this takes place as planned, ALL screens
	 * will "freeze" for the alloted time...
	 *
	 * while ((secs = sleep(secs)) > 0)
	 */	;

	sock_send_error(c->sock, "ignored (not fully implemented)\n");
	return 0;
}

/**
 * Does nothing, returns "noop complete" message.
 *
 * This is useful for shell scripts or programs that want to talk
 *    with LCDproc and not get deadlocked.  Send a noop after each
 *    command and look for the "noop complete" message.
 */
int
noop_func(Client *c, int argc, char **argv)
{
	if (c->state != ACTIVE)
		return 1;

	sock_send_string(c->sock, "noop complete\n");
	return 0;
}
