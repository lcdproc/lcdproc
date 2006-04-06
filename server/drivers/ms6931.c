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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "ms6931.h"
#include "report.h"
#include "lcd_lib.h"
/*
#include "shared/str.h"
#include "server/configfile.h"
*/

static int fd;
static char *framebuf = NULL;
static unsigned char heartbeatCharacter;
static fd_set fdset;
static int width = 0;
static int height = 0;
static struct timeval selectTimeout = { 0, 0 };

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
ms6931_write(void *str, int len)
{
	return write(fd, str, len);
}

static int
ms6931_setpos(int pos)
{
	static char out[3] = { '~', 0x24, 0 };
	pos &= 0xff;
	out[2] = (char)pos;
	return ms6931_write(out, 3);
}

static int
ms6931_attn(int len)
{
	static char out[3] = { '~', 0x26, 0 };
	len &= 0xff;
	out[2] = (char)len;
	return ms6931_write(out, 3);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized width*height
//
void
ms6931_draw_frame (char *dat)
{
	char *row;
	int i;
	char *cvt;

	if (!dat)
		return;

	for (cvt = dat; cvt < (dat+width*height); cvt++)
		*cvt = charTable[(unsigned char)*cvt];

	for (i = 0; i < height; i++) {
		row = dat + (width * i);
		ms6931_setpos(width * i);
		ms6931_attn(width);
		ms6931_write(row, width);
	}
}

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
ms6931_init (Driver *drvthis)
{
	struct termios portset;
	int w, h;
	char device[200] = MS6931_DEF_DEVICE;
	char size[200] = MS6931_DEF_SIZE;

	debug(RPT_INFO, "ms6931_init: init(%p)", drvthis );

	/*Which serial device should be used*/
	strncpy(device, drvthis->config_get_string ( drvthis->name , "Device" , 0 , MS6931_DEF_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	report (RPT_INFO,"ms6931_init: Using device: %s", device);

	/*Which size*/
	strncpy(size, drvthis->config_get_string ( drvthis->name , "Size" , 0 , MS6931_DEF_SIZE),sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size , "%dx%d", &w, &h ) != 2
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report (RPT_WARNING, "ms6931_init: Cannot read size: %s. Using default value.", size);
		sscanf( MS6931_DEF_SIZE , "%dx%d", &w, &h );
	}
	width = w;
	height = h;

	/* get the character to use for heartbeat */
	heartbeatCharacter = (unsigned char)(drvthis->config_get_int (drvthis->name, "HeartbeatCharacter", 0, (int)'*') & 0xff);
	if (!heartbeatCharacter
		|| heartbeatCharacter > 127
		|| charTable[heartbeatCharacter] == ' ')
	{
		heartbeatCharacter = '*';
	}

	/* Set up io port correctly, and open it...*/
	debug( RPT_DEBUG, "ms6931_init: Opening serial device: %s", device);
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report (RPT_ERR, "ms6931_init: open() failed (%s)", strerror (errno));
		return -1;
	} else {
		fcntl(fd, F_SETOWN, getpid());
		report (RPT_INFO, "ms6931_init: opened display on %s", device);
	}

	FD_ZERO (&fdset);
	FD_SET (fd, &fdset);

	// set terminal
	tcgetattr (fd, &portset);
#ifdef HAVE_CFMAKERAW
	cfmakeraw( &portset );
#else
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
	cfsetospeed (&portset, B9600);
//	cfsetispeed (&portset, B0);

	tcsetattr (fd, TCSANOW, &portset);

	// set display to comunications mode 
	ms6931_write("~\040", 2);
	sleep(1);

	// create framebuffer and clear display
	framebuf = (unsigned char *) malloc (width * height);
	ms6931_clear(drvthis);

	selectTimeout.tv_sec = 0;
	selectTimeout.tv_usec = 0;

	report (RPT_DEBUG, "ms6931_init: done");

	return fd;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
ms6931_close (Driver *drvthis)
{
	ms6931_clear(drvthis);
	ms6931_flush(drvthis);
	ms6931_backlight (drvthis, BACKLIGHT_OFF);
       
	close (fd);

	if (framebuf) {
		free (framebuf);
		framebuf = NULL;
	}

	report (RPT_DEBUG, "ms6931_close: done");
}

/////////////////////////////////////////////////////////////////
//
MODULE_EXPORT void
ms6931_flush (Driver *drvthis)
{
	ms6931_draw_frame (framebuf);
}

/////////////////////////////////////////////////////////////////
// Returns the display width/height
//
MODULE_EXPORT int
ms6931_width (Driver *drvthis)
{
	return width;
}

MODULE_EXPORT int
ms6931_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
ms6931_chr (Driver *drvthis, int x, int y, char c)
{
	if (x>width || y>height)
		return;
	y--;
	x--;
	framebuf[(y * width) + x] = charTable[(unsigned char)c];
}

/////////////////////////////////////////////////////////////////
// switches the backlight on/off
//
MODULE_EXPORT void
ms6931_backlight (Driver *drvthis, int on)
{
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
		ms6931_write (out, 3);
		report (RPT_DEBUG, "ms6931_backlight: switched to %d", on);
	}
	saved_state = on;
}

/////////////////////////////////////////////////////////////////
// sets the cursor
//
MODULE_EXPORT void
ms6931_cursor (Driver *drvthis, int x, int y, int state)
{
	static int saved_state = -1;
	static char out[3] = { '~', 0x23, 0 };

	ms6931_setpos(y * width + x);

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
		ms6931_write (out, 3);
		report (RPT_DEBUG, "ms6931_cursor: switched to %d", state);
	}
	saved_state = state;
}

/////////////////////////////////////////////////////////////////
// Clears the framebuffer and the display
//
MODULE_EXPORT void
ms6931_clear (Driver *drvthis)
{
//	ms6931_write("~\042", 2);
	memset (framebuf, ' ', width * height);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
ms6931_string (Driver *drvthis, int x, int y, char string[])
{
	int i;

	x--;
	y--;

	for (i = 0; string[i]; i++) {
		if (string[i] == -1) {
			string[i] = ' ';
		}
		if ((y * width) + x + i > (width * height))
			break;
		framebuf[(y * width) + x + i] = charTable[(unsigned char)string[i]];
	}
}

/////////////////////////////////////////////////////////////
// draw a horizontal bar (vertical makes no sense on ms-6931
//
MODULE_EXPORT void
ms6931_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	char bar[17];
	int max = width - x;
	int size;

	if (len > max)
		len = max;
	if (len < 1)
		return;

	size = len * promille / 1000;
	if ((len * promille) % 1000 > 500)
		size++;

report(RPT_DEBUG, "ms6931_hbar: len=%d, size=%d, promile=%d", len, size, promille);

	memset (bar, ' ', len);
	memset (bar, '*', size);
	bar[len] = '\0';

	ms6931_string(drvthis, x, y, bar);
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
MODULE_EXPORT void
ms6931_heartbeat (Driver *drvthis, int state)
{
	static int timer = 0;
	char whichChar;
	static int saved_state = HEARTBEAT_ON;

	report (RPT_DEBUG, "ms6931_heartbeat: state=%d", state);

	if (state)
		saved_state = state;
	if (state == HEARTBEAT_ON) {
		whichChar = ((timer + 4) & 5) ? heartbeatCharacter : ' ';
		ms6931_chr (drvthis, width, 1, whichChar);
		ms6931_flush (drvthis);
	}
	timer++;
	timer &= 0x0f;
}

/////////////////////////////////////////////////////////////
// controls the keys
//

MODULE_EXPORT const char *
ms6931_get_key (Driver *drvthis)
{
	int ret;
	char buf;
	const char *key = NULL;

	if ((ret = select(FD_SETSIZE, &fdset, NULL, NULL, &selectTimeout)) < 0) {
		report(RPT_DEBUG, "ms6931_get_key: select() failed (%s)", strerror(errno));
		return NULL;
	}
	if (!ret) {
		FD_SET(fd, &fdset);
		return NULL;
	}
	
	if (!FD_ISSET(fd, &fdset))
		return NULL;

	if ((ret = read(fd, &buf, 1)) < 0) {
		report(RPT_DEBUG, "ms6931_get_key: read() failed (%s)", strerror(errno));
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
			report(RPT_DEBUG, "ms6931_get_key: illegal key 0x%02x", (int)buf);
			return NULL;
		}

		report(RPT_DEBUG, "ms6931_get_key: returning %s", key);
		return key;
	}

	return NULL;
}


