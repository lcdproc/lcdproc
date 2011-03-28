/** \file clients/lcdvc/lcd_link.c
 * Functions to build and update the display and to handle server input.
 */

/*-
 * This file is part of lcdvc, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 *               2006-2008, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "lcd_link.h"
#include "vc_link.h"
#include "lcdvc.h"
#include "shared/report.h"
#include "shared/str.h"
#include "shared/sockets.h"

char *address = UNSET_STR;
int port = UNSET_INT;
short autoscroll = 1;

int sock;
short listening = 0;
short scroll_x = 0, scroll_y = 0;
short lcd_cursor_x, lcd_cursor_y;
short lcd_width = 0, lcd_height = 0;
char *lcd_buf = NULL;

short last_vc_cursor_y = 0;
short last_vc_cursor_x = 0;
short last_lcd_cursor_x = 0;
short last_lcd_cursor_y = 0;

static int read_connect_string(void);
static int split(char *str, char delim, char *parts[], int maxparts);


int setup_connection(void)
{
	char buf[200];
	int i;
	int e = 0;
	short line;

	report(RPT_INFO, "Connecting to %s:%d", address, port);

	sock = sock_connect(address, port);
	if (sock < 0) {
		report(RPT_ERR, "Connecting to %s:%d failed", address, port);
		return -1;
	}
	/* Create our menu */
	sock_send_string(sock, "hello\n");
	if (read_connect_string() < 0) {
		return -1;
	}

	snprintf(buf, sizeof(buf)-1, "client_set -name \"%s\"\n", progname);
	sock_send_string(sock, buf);

	/* Create screen */
	CHAIN(e, sock_send_string(sock, "screen_add console\n"));
	for (line = 0; line < lcd_height; line++) {
		snprintf(buf, sizeof(buf)-1, "widget_add console line%d string\n", line);
		buf[sizeof(buf)-1] = 0;
		CHAIN(e, sock_send_string(sock, buf));
	}
	/* Add menu items */
	CHAIN(e, sock_send_string(sock, "menu_add_item \"\" autoscroll checkbox \"Auto scroll\"\n"));
	CHAIN(e, sock_send_string(sock, "menu_set_item \"\" autoscroll -value on\n"));

	/* Reserve keys */

	for (i = 0; i < 4; i++) {
		snprintf(buf, sizeof(buf)-1, "client_add_key \"%s\"\n", keys[i]);
		CHAIN(e, sock_send_string(sock, buf));
	}

	if (e < 0) {
		report(RPT_ERR, "Could not send to to LCDd");
		return -1;
	}

	return 0;
}


int teardown_connection(void)
{
	sock_close(sock);

	return 0;
}


static int read_connect_string(void)
{
	char buf[8192];
	int len = 0;
	int a;
	char *argv[20];
	int argc;
	short received = 0;
	short timeout = 50; /* Give the server 5 secs to respond */

	buf[0] = '\0';

	while (!received && timeout > 0) {
		len = sock_recv_string(sock, buf, sizeof(buf));
		if (len == 0) {
			usleep(100000);
			timeout --;
		} else {
			received = 1;
		}
	}
	report(RPT_INFO, "Received: %s", buf);

	if (len <= 0) {
		report(RPT_ERR, "Did not receive LCDd connect response.");
		return -1;
	}

	argc = split(buf, ' ', argv, 20);
	for (a = 1; a < argc; a++) {
		if (0 == strcmp(argv[a], "wid"))
			lcd_width = atoi(argv[++a]);
		else if (0 == strcmp(argv[a], "hgt"))
			lcd_height = atoi(argv[++a]);
	}
	if (argc <= 0 || strcmp(argv[0], "connect") != 0
	|| lcd_width == 0 || lcd_width == 0) {
		report(RPT_ERR, "Received invalid LCDd connect response.");
		return -1;
	}
	return 0;
}


/**
 * Splits a string into parts, to which pointers will be returned in &parts.
 * The return value is the number of parts.
 * maxparts is the maximum number of parts returned. If more parts exist
 * they are (unsplit) in the last part.
 * The parts are split at the character delim.
 * No new space will be allocated, the string str will be mutated !
 */
static int split(char *str, char delim, char *parts[], int maxparts)
{
	char *p1 = str;
	char *p2;
	int part_nr = 0;

	/* Find the delim char to end the current part */
	while (part_nr < maxparts - 1 && (p2 = strchr(p1, delim))) {

		/* subsequent parts... */
		*p2 = 0;
		parts[part_nr] = p1;

		p1 = p2 + 1; /* Just after the delim char */
		part_nr ++;
	}
	/* and the last part... */
	parts[part_nr] = p1;
	part_nr ++;

	return part_nr;
}


int read_response(char *buf, int maxsize)
{
	return sock_recv_string(sock, buf, maxsize);
}


int process_response(char *str)
{
	char *argv[10];
	int argc;
	//int i;
	//char *p;
	char *str2 = strdup(str); /* get_args modifies str2 */

	report(RPT_DEBUG, "Server said: \"%s\"", str);

	/* Check what the server just said to us... */
	argc = get_args(argv, str2, 10);
	if (argc < 1) {
		free(str2);
		return 0;
	}

	if (strcmp(argv[0], "listen") == 0) {
		listening = 1;
	}
	else if (strcmp(argv[0], "ignore") == 0) {
		listening = 0;
	}
	else if (strcmp(argv[0], "menuevent") == 0) {
		/* Ah, this is what we were waiting for ! */
		if (argc < 2) {
			report(RPT_WARNING, "Server gave invalid response");
			free(str2);
			return -1;
		}
		else if (strcmp(argv[1], "update") == 0) {
			if (argc < 4) {
				report(RPT_WARNING, "Server gave invalid response");
				free(str2);
				return -1;
			}
			if (strcmp(argv[2], "autoscroll") == 0) {
				if (strcmp(argv[3], "on") == 0) {
					autoscroll = 1;
				} else if (strcmp(argv[3], "off") == 0) {
					autoscroll = 0;
				}
				report(RPT_INFO, "Autoscroll set to %d", autoscroll);
			}
		}
		else {
			; /* Ignore other menuevents */
		}
	}
	else if (strcmp(argv[0], "key") == 0) {
		if (argc < 2) {
			report(RPT_WARNING, "Server gave invalid response");
			free(str2);
			return -1;
		}
		if (strcmp(argv[1], keys[0]) == 0) {
			if (scroll_y > 0) scroll_y --;
		} else if (strcmp(argv[1], keys[1]) == 0) {
			if (scroll_y + lcd_height < vc_height) scroll_y ++;
		} else if (strcmp(argv[1], keys[2]) == 0) {
			if (scroll_x > 0) scroll_x --;
		} else if (strcmp(argv[1], keys[3]) == 0) {
			if (scroll_x + lcd_width < vc_width) scroll_x ++;
		}
	}
	else if (strcmp(argv[0], "huh?") == 0) {
		/* Report errors */
		report(RPT_WARNING, "Server said: \"%s\"", str);
	}
	else {
		; /* Ignore all other responses */
	}
	free(str2);
	return 0;

}


int update_display(void)
{
	//int bytes_read;
	short line;
	int e = 0;
	char buf[80];
	char *str_buf;
	short num_lines;
	num_lines = min(lcd_height, vc_height);

	if (!listening)
		return 0;

	if (!lcd_buf) {
		/* Not yet allocated */
		lcd_buf = malloc(lcd_width * lcd_height);
		memset(lcd_buf, ' ', lcd_width * lcd_height);
	}

	if (autoscroll
	&& (last_vc_cursor_x != vc_cursor_x || last_vc_cursor_y != vc_cursor_y)) {
		last_vc_cursor_x = vc_cursor_x;
		last_vc_cursor_y = vc_cursor_y;

		/* Adjust scroll position */
		if (scroll_x > vc_cursor_x)
			scroll_x = vc_cursor_x;
		if (scroll_x < vc_cursor_x - lcd_width + 1)
			scroll_x = vc_cursor_x - lcd_width + 1;
		if (scroll_y > vc_cursor_y)
			scroll_y = vc_cursor_y;
		if (scroll_y < vc_cursor_y - lcd_height + 1)
			scroll_y = vc_cursor_y - lcd_height + 1;
	}
	lcd_cursor_x = vc_cursor_x - scroll_x + 1;
	lcd_cursor_y = vc_cursor_y - scroll_y + 1;
	if (lcd_cursor_x != last_lcd_cursor_x || lcd_cursor_y != last_lcd_cursor_y) {
		last_lcd_cursor_x = lcd_cursor_x;
		last_lcd_cursor_y = lcd_cursor_y;

		/* New scroll positions, send the cursor command */
		if (lcd_cursor_x < 1 || lcd_cursor_x > lcd_width
		|| lcd_cursor_y < 1 || lcd_cursor_y > lcd_height) {
			snprintf(buf, sizeof(buf)-1, "screen_set console -cursor off\n");
		} else {
			snprintf(buf, sizeof(buf)-1, "screen_set console -cursor on -cursor_x %d -cursor_y %d\n",
					lcd_cursor_x, lcd_cursor_y);
		}
		buf[sizeof(buf)-1] = 0;
		CHAIN(e, sock_send_string(sock, buf));
	}

	/* Send all (changed) lines */
	str_buf = malloc(80 + 2 * lcd_width);
	for (line = 0; line < num_lines; line++) {

		char *vc_p;
		char *lcd_p;
		short line_width;

		line_width = min(lcd_width, vc_width);

		/* Where does the data for this line come from ? */
		vc_p = vc_buf + vc_width * (scroll_y + line) + scroll_x;
		lcd_p = lcd_buf + lcd_width * line;

		/* Has the line data changed ? */
		if (memcmp(vc_p, lcd_p, line_width) != 0) {
			/* Yes, so send it */
			char *a;
			short pos;

			/* Format/escape the data */
			snprintf(str_buf, 80, "widget_set console line%d 1 %d \"", line, line+1);

			a = str_buf + strlen(str_buf); /* start just after the " */
			for (pos = 0; pos < line_width; pos++) {
				if (vc_p[pos] == '\\' || vc_p[pos] == '\"') {
					*a++ = '\\'; /* add escape char */
				}
				if ((signed char) vc_p[pos] >= 32) {
					*a++ = vc_p[pos]; /* add char */
				} else {
					*a++ = '?';
				}
			}
			*a++ = '\"'; /* end string */
			*a++ = '\n'; /* newline */
			*a = 0; /* terminate */
			CHAIN(e, sock_send_string(sock, str_buf));

			/* And store the new data */
			memcpy(lcd_p, vc_p, line_width);

		}
	}
	free(str_buf);

	if (e < 0) {
		report(RPT_ERR, "Error while sending data to LCDd");
		return -1;
	}

	return 0;
}


int send_nop(void)
{
	return sock_send_string(sock, "\n");
}
