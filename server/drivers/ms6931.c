/** \file server/drivers/ms6931.c
 * LCDd \c ms6931 driver for MSI-6931 displays in 1U rack servers by MSI.
 */

/*  This is the LCDproc driver for MSI-6931 displays
	as found in the following 1U rack servers by MSI:
		MS-9202
		MS-9205
		MS-9211

    Copyright (C) 2003, Marcel Pommer <marsellus at users dot sourceforge dot net>

	The code is derived from the CFontz driver

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "lcd.h"
#include "ms6931.h"
#include "report.h"
#include "lcd_lib.h"

/** private data for the \c ms6931 driver */
typedef struct ms6931_private_data {
	char device[200];
	int fd;
	unsigned char *framebuf;
	unsigned char heartbeatCharacter;
	int width;
	int height;
} PrivateData;

// mandatory symbols
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT char *symbol_prefix = "ms6931_";


/////////////////////////////////////////////////////////////////
// character conversion table
//
static char charTable[] =
{
/*   0 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/*  16 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/*  32 */	' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
/*  48 */	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
/*  64 */	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  ' ',  ']',  '^',  '_',
/*  96 */	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
/* 112 */	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '_',  ' ',
/* 128 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 144 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 160 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 176 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 192 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 208 */	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
/* 224 */	224,  225,  226,  227,  228,  229,  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  236,  ' ',  238,  239,
/* 240 */	' ',  ' ',  242,  243,  244,  245,  246,  247,  ' ',  ' ',  ' ',  ' ',  ' ',  253,  ' ',  ' '
};

/////////////////////////////////////////////////////////////////
// output functions
//
static int
ms6931_write(int fd, void *str, int len)
{
	return write(fd, str, len);
}

static int
ms6931_setpos(int fd, int pos)
{
	static char out[3] = { '~', 0x24, 0 };
	pos &= 0xFF;
	out[2] = (char) pos;
	return ms6931_write(fd, out, 3);
}

static int
ms6931_attn(int fd, int len)
{
	static char out[3] = { '~', 0x26, 0 };
	len &= 0xFF;
	out[2] = (char) len;
	return ms6931_write(fd, out, 3);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized width*height
//
static void
ms6931_draw_frame (Driver *drvthis, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	int i;

	if (!dat)
		return;

	// we do this already in ms6931_chr() and ms6931_string}():
	//char *cvt;
	//for (cvt = dat; cvt < (dat + p->width * p->height); cvt++)
	//	*cvt = charTable[(unsigned char) *cvt];

	for (i = 0; i < p->height; i++) {
		unsigned char *row = dat + (p->width * i);

		ms6931_setpos(p->fd, p->width * i);
		ms6931_attn(p->fd, p->width);
		ms6931_write(p->fd, row, p->width);
	}
}

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
ms6931_init (Driver *drvthis)
{
	PrivateData *p;
	struct termios portset;
	char size[20];
	int w, h;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->fd = -1;
	p->framebuf = NULL;


	debug(RPT_INFO, "ms6931_init: init(%p)", drvthis);

	/*Which serial device should be used*/
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, MS6931_DEF_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO,"%s: using Device %s", drvthis->name, p->device);

	/*Which size*/
	strncpy(size, drvthis->config_get_string(drvthis->name , "Size", 0, MS6931_DEF_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, size, MS6931_DEF_SIZE);
		sscanf(MS6931_DEF_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* get the character to use for heartbeat */
	p->heartbeatCharacter = (unsigned char)(drvthis->config_get_int(drvthis->name, "HeartbeatCharacter", 0, (int)'*') & 0xFF);
	if ((p->heartbeatCharacter == '\0')
	    || (p->heartbeatCharacter > 127)
	    || (charTable[p->heartbeatCharacter] == ' ')) {
		p->heartbeatCharacter = '*';
	}

	/* Set up io port correctly, and open it...*/
	debug(RPT_DEBUG, "%s: Opening serial device: %s", drvthis->name, p->device);
	p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}
	fcntl(p->fd, F_SETOWN, getpid());
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, p->device);

	// set terminal
	tcgetattr(p->fd, &portset);
#ifdef HAVE_CFMAKERAW
	cfmakeraw(&portset);
#else
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	cfsetospeed(&portset, B9600);
//	cfsetispeed(&portset, B0);

	tcsetattr(p->fd, TCSANOW, &portset);

	// set display to comunications mode
	ms6931_write(p->fd, "~\040", 2);
	sleep(1);

	// create framebuffer and clear display
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	ms6931_clear(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
ms6931_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if ((p->fd >= 0) && (p->framebuf != NULL)) {
			ms6931_clear(drvthis);
			ms6931_flush(drvthis);
			ms6931_backlight (drvthis, BACKLIGHT_OFF);
		}

		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf != NULL)
			free(p->framebuf);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

	report(RPT_DEBUG, "%s: close() done", drvthis->name);
}

/////////////////////////////////////////////////////////////////
//
MODULE_EXPORT void
ms6931_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	ms6931_draw_frame(drvthis, p->framebuf);
}

/////////////////////////////////////////////////////////////////
// Returns the display width/height
//
MODULE_EXPORT int
ms6931_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

MODULE_EXPORT int
ms6931_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
ms6931_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = charTable[(unsigned char) c];
}

/////////////////////////////////////////////////////////////////
// switches the backlight on/off
//
MODULE_EXPORT void
ms6931_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	static int saved_state = -1;
	static char out[3] = { '~', 0x01, 0 };

	if (on != saved_state) {
		switch (on) {
			case BACKLIGHT_OFF:
				out[2] = 0x00;
				break;
			case BACKLIGHT_ON:
			default:
				out[2] = 0x01;
		}
		ms6931_write(p->fd, out, 3);
		report(RPT_DEBUG, "%s: backlight: switched to %d", drvthis->name, on);
	}
	saved_state = on;
}

/////////////////////////////////////////////////////////////////
// sets the cursor
//
MODULE_EXPORT void
ms6931_cursor (Driver *drvthis, int x, int y, int state)
{
	PrivateData *p = drvthis->private_data;
	static int saved_state = -1;
	static char out[3] = { '~', 0x23, 0 };

	ms6931_setpos(p->fd, y * p->width + x);

	if (state != saved_state) {
		switch (state) {
			case CURSOR_OFF:
				out[2] = 0;
				break;
			case CURSOR_UNDER:
				out[2] = 2;
				break;
			case CURSOR_DEFAULT_ON:
			case CURSOR_BLOCK:
			default:
				out[2] = 3;
		}
		ms6931_write(p->fd, out, 3);
		report(RPT_DEBUG, "%s: cursor: switched to %d", drvthis->name, state);
	}
	saved_state = state;
}

/////////////////////////////////////////////////////////////////
// Clears the framebuffer and the display
//
MODULE_EXPORT void
ms6931_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	//ms6931_write("~\042", 2);
	memset(p->framebuf, ' ', p->width * p->height);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
ms6931_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		unsigned char c = (unsigned char) string[i];

		if (x >= 0)
			p->framebuf[(y * p->width) + x] = charTable[c];
	}
}

/////////////////////////////////////////////////////////////
// draw a horizontal bar (vertical makes no sense on ms-6931
//
MODULE_EXPORT void
ms6931_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	PrivateData *p = drvthis->private_data;
	char bar[17];
	int max = p->width - x;
	int size;

	if (len > max)
		len = max;
	if (len < 1)
		return;

	size = len * promille / 1000;
	if ((len * promille) % 1000 > 500)
		size++;

	report(RPT_DEBUG, "%s: hbar: len=%d, size=%d, promile=%d",
			drvthis->name, len, size, promille);

	memset(bar, ' ', len);
	memset(bar, '*', size);
	bar[len] = '\0';

	ms6931_string(drvthis, x, y, bar);
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
MODULE_EXPORT void
ms6931_heartbeat (Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;
	static int timer = 0;
	static int saved_state = HEARTBEAT_ON;

	report(RPT_DEBUG, "%s: heartbeat: state=%d", drvthis->name, state);

	if (state)
		saved_state = state;
	if (state == HEARTBEAT_ON) {
		char ch = ((timer + 4) & 5) ? p->heartbeatCharacter : ' ';

		ms6931_chr(drvthis, p->width, 1, ch);
		ms6931_flush(drvthis);
	}
	timer++;
	timer &= 0x0F;
}

/////////////////////////////////////////////////////////////
// controls the keys
//

MODULE_EXPORT const char *
ms6931_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int ret;
	char buf;
	const char *key = NULL;
	static struct timeval selectTimeout = { 0, 0 };
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(p->fd, &fdset);

	if ((ret = select(FD_SETSIZE, &fdset, NULL, NULL, &selectTimeout)) < 0) {
		report(RPT_DEBUG, "%s: get_key: select() failed (%s)",
				drvthis->name, strerror(errno));
		return NULL;
	}
	if (!ret) {
		FD_SET(p->fd, &fdset);
		return NULL;
	}

	if (!FD_ISSET(p->fd, &fdset))
		return NULL;

	if ((ret = read(p->fd, &buf, 1)) < 0) {
		report(RPT_DEBUG, "%s: get_key: read() failed (%s)",
				drvthis->name, strerror(errno));
		return NULL;
	}
	if (ret == 1) {
		switch (buf) {
		case 'L':
			key = "Escape";
			break;
		case 'M':
			key = "Enter";
			break;
		case 'R':
			key = "Down";
			break;
		default:
			report(RPT_DEBUG, "%s get_key: illegal key 0x%02X",
					drvthis->name, buf);
			return NULL;
		}

		report(RPT_DEBUG, "%s: get_key: returns %s", drvthis->name, key);
		return key;
	}

	return NULL;
}

