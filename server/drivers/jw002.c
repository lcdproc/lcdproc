/** \file server/drivers/jw002.c
 * This is the LCDproc driver for Hyundai HG12603A equipped with
 * PJRC firmware (http://www.pjrc.com/tech/mp3/pushbutton_info.html)
 * */

/*
    This is the LCDproc driver for Hyundai HG12603A equipped with
    PJRC firmware (http://www.pjrc.com/tech/mp3/pushbutton_info.html)

    The HG12603(A) is a 2"x4" LCD panel with a Samsung KS107B controller,
    driven by an on-board 8051-family microcontroller.  Normally, the
    KS107B would make this a graphical LCD, but the actual pixels on
    the glass are diced up into 8 lines of 24 5x7 character areas. 

    Applicable external links:
    - http://www.pjrc.com/tech/mp3/index.html
    - http://www.pjrc.com/store/mp3_display.html
    - http://www.pjrc.com/tech/mp3/firmware.html
    - http://www.pjrc.com/tech/mp3/lcd_protocol.html

    Copyright (C) 2008, 2009, 2019, 2020, Ethan Dicks

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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <sys/poll.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "jw002.h"
#include "adv_bignum.h"

#include "shared/report.h"


/* PJRC firmware supports an optional 3x4 keypad matrix */
#define MAX_KEY_MAP	12

// Don't forget to define key mappings in LCDd.conf: KeyMap_A=Enter, ...
// otherwise you will not get your keypad working.

/*
 * The JW-002 driver depends on the four standard firmware fonts
 * to implement vbar, hbar, and bignum.  None of the standard fonts
 * happen to have the standard LCDproc heartbeat chars, so we define
 * those ourselves.
 *
 * For the same reason, we don't use the hardware clear but rather
 * empty the internal frame buffer.  The frame buffer holds all
 * the requested changes until the server asks us to flush the
 * changes to the display.  This also permits us to do an
 * incremental update and reduce the number of characters to be
 * sent to the display across the serial link.
 *
 * In order to display graphic widgets, we define and use our own
 * custom characters.  To avoid multiple definitions of the same
 * custom characters, we use a caching mechanism that remembers
 * what is currently defined.  In order to avoid always redefining
 * the same custom character at the beginning of the table, we
 * rotate the beginning of the table.  This is supposed to reduce
 * the number of character redefinitions and make the caching more
 * effective.  The overall goal is to reduce the number of
 * characters sent to the display.
 *
 * (TODO: a totally custom RAM-resident font that gets loaded once)
 */


/* Constants for userdefchar_mode */
#define NUM_CCs		28   // max. number of custom characters
#define CC_OFFSET	192  // first custom char

//Type of characters currently stored in CGRAM. from lcd.h as info
//typedef enum {
//	standard,	/* only char 0 is used for heartbeat */
//	vbar,		/* vertical bars */
//	hbar,		/* horizontal bars */
//	custom,		/* custom settings */
//	bignum,		/* big numbers */
//	bigchar		/* big characters */
//} CGmode;

/** private data for the \c jw002 driver */
typedef struct jw002_private_data {
	int fd;			/* The LCD file descriptor */
	
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	int xoff, yoff;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* defineable characters */
	CGmode ccmode;
	int font;		/* font number */

	int output_state;	/* static data from jw002_output */
	int contrast;		/* static data from set/get_contrast */
	int brightness;
	int offbrightness;

	/* the keymap */
	char *keymap[MAX_KEY_MAP];
	int keys;
	int keypad_test_mode;

	char info[255];		/* static data from jw002_get_info */
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "jw002_";

static void jw002_hardware_clear(Driver *drvthis);
static void jw002_linewrap(Driver *drvthis, int on);
static void jw002_autoscroll(Driver *drvthis, int on);
//static void jw002_cursorblink(Driver *drvthis, int on);
static void jw002_cursor_goto(Driver *drvthis, int x, int y);
static void jw002_init_keys (Driver *drvthis);


/* Parse one key from the configfile */
static char
jw002_parse_keypad_setting (Driver *drvthis, char *keyname, char default_value)
{
	char return_val = 0;
	const char *s;
	char buf[255];

	s = drvthis->config_get_string(drvthis->name, keyname, 0, NULL);
	if (s != NULL) {
		strncpy(buf, s, sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		return_val = buf[0];
	} else {
		return_val = default_value;
	}
	return return_val;
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
jw002_init (Driver *drvthis)
{
	struct termios portset;

	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	int tmp, w, h, xoff, yoff;

        PrivateData *p;

	/* Alocate and store private data */
        p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
	        return -1;
	if (drvthis->store_private_ptr(drvthis, p))
	        return -1;

	/* Initialise the PrivateData structure */
	p->fd = -1;

	p->width      = JW002_DEFAULT_WIDTH;
	p->height     = JW002_DEFAULT_HEIGHT;
	p->cellwidth  = JW002_DEFAULT_CELLWIDTH;
	p->cellheight = JW002_DEFAULT_CELLHEIGHT;

	p->framebuf = NULL;
	p->backingstore = NULL;

	p->output_state = -1;	/* static data from jw002_output */
	p->keypad_test_mode = 0;

	debug(RPT_INFO, "jw002: init(%p)", drvthis);

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	/* Get display size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
				drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE , "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Get font number (0-3 are ROM fonts, 4-31 are RAM fonts) */
	tmp = drvthis->config_get_int(drvthis->name, "Font", 0, JW002_DEFAULT_FONT);
	if (tmp < 0 || tmp >= JW002_MAX_FONT) {
		report(RPT_WARNING, "%s: Font numbers range from 0-31.  Font number %d is invalid; using font 0",
				drvthis->name, tmp);

		tmp = 0;
	}
	p->font = tmp;

// jw-002 interface speed is compiled into the firmware - default is 19200
	/* Get speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	switch (tmp) {
		case 1200:
			speed = B1200;
			break;
		case 2400:
			speed = B2400;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;
			break;
		default:
			speed = B19200;
			report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600 or 19200; using default %d",
					drvthis->name, tmp);
	}

	/* Get keypad settings */

	/* Get offsets */
	xoff = drvthis->config_get_int(drvthis->name, "X_offset", 0, DEFAULT_XOFFSET);
	yoff = drvthis->config_get_int(drvthis->name, "Y_offset", 0, DEFAULT_YOFFSET);

	if (xoff < 0 || xoff + p->width > LCD_MAX_WIDTH) {
		report(RPT_WARNING, "%s:  X offset of %d invalid.  Ignoring X offset request",
				drvthis->name, xoff);
		xoff = 0;
	}

	if (yoff < 0 || yoff + p->height > LCD_MAX_HEIGHT) {
		report(RPT_WARNING, "%s:  Y offset of %d invalid.  Ignoring Y offset request",
				drvthis->name, xoff);
		yoff = 0;
	}
	p->xoff = xoff;
	p->yoff = yoff;

	/* keypad test mode? */
	if (drvthis->config_get_bool(drvthis->name, "keypad_test_mode", 0, 0)) {
		fprintf(stdout, "jw-002: Entering keypad test mode...\n");
		p->keypad_test_mode = 1;
		stay_in_foreground = 1;
	}

	if (!p->keypad_test_mode) {
		/* We don't send any chars to the server in keypad test mode.
		 * So there's no need to get them from the configfile in keypad
		 * test mode.
		 */

		int i;

		/* assume no mapped keys */
		p->keys = 0;

		/* read the keymap */
		for (i = 0; i < MAX_KEY_MAP; i++) {
			char buf[40];
			const char *s;

			/* First fill with NULL; */
			p->keymap[i] = NULL;

			/* Read config value */
			sprintf(buf, "KeyMap_%c", i+'A');
			s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

			/* Was a key specified in the config file ? */
			if (s != NULL) {
				p->keys++;
				p->keymap[i] = strdup(s);
				report(RPT_INFO, "%s: Key '%c' mapped to \"%s\"",
					drvthis->name, i+'A', s );
			}
		}
	}
	/* End of config file parsing */

	/* Set up io port correctly, and open it... */
//	p->fd = open(device, O_RDWR | O_NOCTTY);
	p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, device, strerror(errno));
		if (errno == EACCES)
			report(RPT_ERR, "%s: %s device could not be opened...", drvthis->name, device);
  		return -1;
	}
	report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

	tcgetattr(p->fd, &portset);

	// THIS ALL COMMENTED OUT BECAUSE WE NEED TO SET TIMEOUTS
	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW_NOT
	/* The easy way */
	cfmakeraw(&portset);
#else
	/* The hard way */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;
#endif

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) calloc(p->width * p->height, 1);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* make sure the framebuffer backing store is there... */
	p->backingstore = (unsigned char *) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store", drvthis->name);
		return -1;
	}
	memset(p->backingstore, ' ', p->width * p->height);

	/* set initial LCD configuration */
	jw002_init_keys(drvthis);
	jw002_hardware_clear(drvthis);  // clear screen and set font from p->font
	jw002_linewrap(drvthis, 0);  // turn off linewrap _and_ autoscroll

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
jw002_close (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->backingstore)
			free(p->backingstore);
		p->backingstore = NULL;

        	free(p);
	}	
	drvthis->store_private_ptr(drvthis, NULL);
	debug(RPT_DEBUG, "jw002: closed");
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
jw002_width (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

        return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
jw002_height (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
jw002_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
jw002_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
jw002_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	debug(RPT_DEBUG, "jw002: printing string at (%d,%d)", x, y);

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((y < 0) || (y >= p->height))
		return;

	for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
		/* Check for buffer overflows... */
		if (x >= 0)
			p->framebuf[(y * p->width) + x] = string[i];
	}

}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
jw002_clear (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	/* set hbar/vbar/bignum mode back to normal character display */
	p->ccmode = standard;

	/* replace all chars in framebuf with spaces */
	memset(p->framebuf, ' ', (p->width * p->height));

	/* make backing store differ from framebuf so it all gets cleared by jw002_flush */
	//memset(p->backingstore, 0xFE, (p->width * p->height));

	debug(RPT_DEBUG, "jw002: cleared screen");
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 *
 * The one possible trick is that we want to display chars 0-5 here (for
 * hbars), which could be interpreted as control chars.  The technique
 * for printing out low chars is...
 *
 * Display Byte 0 to 31
 *   Some control characters (ascii 0 to 31) have special functions, and if
 *   a PC terminal emulation program is also receiving the communication, it
 *   may respond to control characters. To cause the board to display
 *   characters 0 to 31, send a 0x5C followed by the control character plus 32.
 */

MODULE_EXPORT void
jw002_flush (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;
	int modified = 0;
	int i, j;

	for (i = 0; i < p->height; i++) {
		// set  pointers to start of the line in frame buffer & backing store
		unsigned char *sp = p->framebuf + (i * p->width);
		unsigned char *sq = p->backingstore + (i * p->width);

		// set  pointers to end of the line in frame buffer & backing store
		unsigned char *ep = sp + (p->width - 1);
		unsigned char *eq = sq + (p->width - 1);
		int length = 0;

		debug(RPT_DEBUG, "Framebuf: '%.*s'", p->width, sp);
		debug(RPT_DEBUG, "Backingstore: '%.*s'", p->width, sq);

		/* Strategy:
		 * - not more than one update command per line
		 * - leave out leading and trailing parts that are identical
		 */

		// skip over leading identical portions of the line
		for (j = 0; (sp <= ep) && (*sp == *sq); sp++, sq++, j++)
			;

		// skip over trailing identical portions of the line
		for (length = p->width - j; (length > 0) && (*ep == *eq); ep--, eq--, length--)
			;

		/* there are differences, ... */
		if (length > 0) {
			unsigned char out[length+2];
			unsigned char safeout[length+2 * 2]; // room to include escape char for out-of-band chars
			unsigned char rawchar;

			memcpy(out, sp, length);

			unsigned char *rawp, *safep;  // point to each of our two line buffers
			rawp = out;
			safep = safeout;
			while (rawp < out + length) {
				// grab a char and check to see if it's "printable" or not
				rawchar = *rawp++;
				// look for 0x5C, 0x00-0x1F, 0x80-0x9F
				if (rawchar == 0x5C || rawchar < 0x20 || (rawchar > 0x7F && rawchar < 0xA0) ) {
					*safep++ = 0x5C;  // throw in escape char
					if (rawchar < 0x20) {
						rawchar += 0x20;  // scoot low chars up to printable range
					}
					if (rawchar > 0x7F) {
						rawchar -= 0x20;  // scoot down high chars down to printable range
					}
				}
				*safep++ = rawchar;  // copy char to output buffer
			}

			debug(RPT_DEBUG, "%s: l=%d c=%d count=%d string='%.*s'",
			      __FUNCTION__, i, j, length, length, sp);

			jw002_cursor_goto(drvthis, j+1, i+1);

			length = safep - safeout;  // new length is however many chars we wrote to our OOB buffer
			write(p->fd, safeout, length);
			modified++;
		}      
	}	// i < p->height

	if (modified)
		memcpy(p->backingstore, p->framebuf, p->width * p->height);

	debug(RPT_DEBUG, "jw002: frame buffer flushed");
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
jw002_chr (Driver *drvthis, int x, int y, char c)
{
        PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
	                p->framebuf[(y * p->width) + x] = c;

}

/**
 * Set output port(s).
 *
 *  External input/output control (protocol version 1 and later only)
 *     Control extra I/O on CPU. All signals are active low, electrical
 *     ground is active '1', electrical 5V is inactive '0'. (refer to
 *     Tom's LCD Modification page for more information.
 *
 *     http://www.wiresncode.com/projects/pjrcmp3/display-mods/
 *
 *  Transmit:
 *      0x5C 0x4A <command> <state>
 *      command = '0' control LED on front panel
 *      command = '1' control pin 7 on CPU (use with care, may crash non-modified hardware)
 *      command = '2' control pin 8 on CPU (use with care, may crash non-modified hardware)
 *      command = '4' read LED on front panel (will turn led off and always read '0')
 *      command = '5' read pin 7 on CPU (will always read '0' on non-modified hardware)
 *      command = '6' read pin 8 on CPU (will always read '0' on non-modified hardware)
 *      state = '0' for inactive (electrical 5V)
 *      state = '1' for active (electrical ground)
 *      (state is ignored for read, but must be supplied)
 *
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
MODULE_EXPORT void
jw002_output (Driver *drvthis, int state)
{
        PrivateData *p = drvthis->private_data;
        unsigned char out[5] = { '\\', 'J', '0', '0' };  // pre-load with LED "off" msg

	/* only deal with main LED (bit 1) */
	if (state < 2) {
		// remember the state
		p->output_state = state;

		// set up the output command
		out[3] = (state & 1) ? '1' : '0';

		// send command to jw002 and log that we did it
        	debug(RPT_DEBUG, "jw002: setting LED to %d", state & 1);
		// //write(p->fd, out, 4);


	} else {
		/* throw a debug msg if they are trying to manipulate control pins 7 or 8 */
		debug(RPT_DEBUG, "jw002: output control requires a modified LCD module");
	}
}


/**
 * Clear the LCD using its hardware command
 * \param drvthis  Pointer to driver structure.
 */
static void
jw002_hardware_clear (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	unsigned char out[4] = { '\\', '@', ' ', '0' };

	out[2] = p->font + 32;  // font numbers must be shifted to printable chars

	write(p->fd, out, 4); // clear and select font 0

	debug(RPT_DEBUG, "jw002: cleared LCD and switched to font %d", p->font);
}


/**
 * Turn the LCD's built-in linewrapping feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
jw002_linewrap (Driver *drvthis, int on)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { '\\', 'A', 0, 0 };

	out[2] = (on) ? '3' : '0';  // wrap _and_ scroll on or off
	write(p->fd, out, 3);

	debug(RPT_DEBUG, "jw002: linewrap turned %s", (on) ? "on" : "off");
}


/**
 * Turn the LCD's built-in automatic scrolling feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
jw002_autoscroll (Driver *drvthis, int on)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { '\\', 'A', 0, 0 };

	out[2] = (on) ? '3' : '0';  // wrap _and_ scroll on or off
	write(p->fd, out, 3);

	debug(RPT_DEBUG, "jw002: autoscroll turned %s", (on) ? "on" : "off");
}


/**
 * Move cursor to position (x,y).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 */
static void
jw002_cursor_goto(Driver *drvthis, int x, int y)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[5] = { '\\', 'B', 0, 0 };

	x--; // turn 1-based coords back into 0-based coords
	y--;
	/* set cursor position */
	if ((x >= 0) && (x < p->width))
		out[2] = (unsigned char) x + p->xoff + 0x20;  // shift values above cntrl chars
	if ((y >= 0) && (y < p->height))
		out[3] = (unsigned char) y + p->yoff + 0x20;  // shift values above cntrl chars
	write(p->fd, out, 4);
}


/**
 * Provide general information about the LCD/VFD display.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
jw002_get_info (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "jw002: get_info");

	memset(p->info, '\0', sizeof(p->info));
	strcpy(p->info, "JW-002 Driver ");

	return p->info;
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
jw002_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
#define ROMBARS

#ifndef ROMBARS
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i <= p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i] = 0xFF;
			jw002_set_char(drvthis, i+CC_OFFSET, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, CC_OFFSET);

#else
	PrivateData *p = drvthis->private_data;

	int cellheight = p->cellheight; // quick hack to get hbars working

        int total_pixels  = ((long) 2 * len * cellheight + 1 ) * promille / 2000;
        int pos;

        for (pos = 0; pos < len; pos++ ) {

                int pixels = total_pixels - cellheight * pos;

                if ( pixels >= cellheight ) {
                        /* write a "full" block to the screen... */
                        drvthis->icon (drvthis, x, y - pos, ICON_BLOCK_FILLED);
                }
                else if ( pixels > 0 ) {
                        /* write a partial block... */
                        drvthis->chr (drvthis, x, y - pos, (cellheight - pixels) + 136);
                        break;
                }
                else {
                        ; /* write nothing (not even a space) */
                }
        }
#endif

}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 *
 * For the JW-002, the first 6 chars of Font 0 happen to contain vertical
 * lines of 0, 1, 2, 3, 4, and 5 pixels wide.  No custom-defined chars
 * required.
 *
 */
MODULE_EXPORT void
jw002_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
#define ROMBARS

#ifndef ROMBARS
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}
		p->ccmode = hbar;

		memset(hBar, 0x00, sizeof(hBar));

		for (i = 1; i <= p->cellwidth; i++) {
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			jw002_set_char(drvthis, i+CC_OFFSET, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, CC_OFFSET);
#else
	PrivateData *p = drvthis->private_data;

	int cellwidth = p->cellwidth; // quick hack to get hbars working

        int total_pixels  = ((long) 2 * len * cellwidth + 1 ) * promille / 2000;
        int pos;

        for (pos = 0; pos < len; pos++ ) {

                int pixels = total_pixels - cellwidth * pos;

                if ( pixels >= cellwidth ) {
                        /* write a "full" block to the screen... */
                        drvthis->icon (drvthis, x+pos, y, ICON_BLOCK_FILLED);
                }
                else if ( pixels > 0 ) {
                        /* write a partial block... */
                        drvthis->chr (drvthis, x+pos, y, pixels);
                        break;
                }
                else {
                        ; /* write nothing (not even a space) */
                }
        }
#endif
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
jw002_num (Driver *drvthis, int x, int num)
{

	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	debug(RPT_DEBUG, "BG jw002_num: ccmode = %d (bignum = %d)", p->ccmode, bignum);

	if (p->ccmode != bignum) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
					drvthis->name);
			return;
		}

		p->ccmode = bignum;

		do_init = 1;
	}

	debug(RPT_DEBUG, "BG  do_init = %d", do_init);

	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, CC_OFFSET, do_init);

}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
jw002_get_free_chars (Driver *drvthis)
{
//PrivateData *p = drvthis->private_data;

  return NUM_CCs;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8(=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 *
 * From http://www.pjrc.com/mp3/lcd_protocol.html
 *
 * Define Custom Character
 *   Download a custom character to RAM. The character number is sent as
 *   two byte ascii hex, followed by a font number (offset by 32), and
 *   an 8-byte bitmap. You may write to any font. User defined fonts
 *   should be placed in 8 through 31. Low numbered fonts may also be
 *   written, but this should generally only be done if the appearance
 *   of particular characters needs to be changed. See the LCD Fonts
 *   page for images of the fonts included in the display board's
 *   firmware. All downloaded font bitmaps are stored in volatile
 *   memory and are lost when the board is rebooted.
 *
 *  Transmit:
 *      0x5C 0x44 <char, 2 byte hex> <font+32> <bitmap, 8 bytes>
 *
 *      Example: Changing the appearance of capital "R" (R is ascii 0x52)
 *               in font #0:
 *      0x5C 0x44 0x35 0x32 0x20 0x3E 0x39 0x39 0x39 0x3E 0x3A 0x39 0x39
 */
MODULE_EXPORT void
jw002_set_char (Driver *drvthis, int n, unsigned char *dat)
{
//#define HEX2ASC(x) ((((x) & 0xF) + (((x) & 0xF) > 9) ? 48 : 55 ))
//#define HEX2ASC(x) ((x) & 15) + ((((x) & 15) > 9) ? 55 : 48)
#define HEX2ASC(x) ((x) + (((x) > 9) ? 55 : 48))

	PrivateData *p = drvthis->private_data;
	unsigned char out[13] = { '\\', 'D', 0,0, ' ', 0,0,0,0,0,0,0,0 };
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < CC_OFFSET) || (n >= NUM_CCs+CC_OFFSET))
		return;
	if (!dat)
		return;

	out[2] = HEX2ASC(n >> 4);  /* Custom char to define. xxx */
	out[3] = HEX2ASC(n & 0x0F);  /* Custom char to define. xxx */

	for (row = 0; row <= p->cellheight; row++) {
		out[row+5] = (dat[row] & mask) | 0x20; // data portion range 0x20 - 0x3F
	}

#ifdef DEBUG_CHARS
	debug(RPT_DEBUG, "Chardef: '%.*s'", 13, out);
#endif
	write(p->fd, out, 13);

}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \return  Information whether the icon is handled here or needs to be handled by the server core.
 */
MODULE_EXPORT int
jw002_icon (Driver *drvthis, int x, int y, int icon)
{

//	PrivateData *p = drvthis->private_data;


	static unsigned char heart_open[] = 
		{ b__XXXXX,
		  b__X_X_X,
		  b_______,
		  b_______,
		  b_______,
		  b__X___X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char heart_filled[] = 
		{ b__XXXXX,
		  b__X_X_X,
		  b___X_X_,
		  b___XXX_,
		  b___XXX_,
		  b__X_X_X,
		  b__XX_XX,
		  b__XXXXX };
	static unsigned char arrow_up[] = 
		{ b____X__,
		  b___XXX_,
		  b__X_X_X,
		  b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b_______ };
	static unsigned char arrow_down[] = 
		{ b____X__,
		  b____X__,
		  b____X__,
		  b____X__,
		  b__X_X_X,
		  b___XXX_,
		  b____X__,
		  b_______ };
	/*
	static unsigned char arrow_left[] = 
		{ b_______,
		  b____X__,
		  b___X___,
		  b__XXXXX,
		  b___X___,
		  b____X__,
		  b_______,
		  b_______ };
	static unsigned char arrow_right[] = 
		{ b_______,
		  b____X__,
		  b_____X_,
		  b__XXXXX,
		  b_____X_,
		  b____X__,
		  b_______,
		  b_______ };
	*/
	static unsigned char checkbox_off[] = 
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X___X,
		  b__X___X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_on[] = 
		{ b____X__,
		  b____X__,
		  b__XXX_X,
		  b__X_XX_,
		  b__X_X_X,
		  b__X___X,
		  b__XXXXX,
		  b_______ };
	static unsigned char checkbox_gray[] = 
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X_X_X,
		  b__XX_XX,
		  b__X_X_X,
		  b__XXXXX,
		  b_______ };
	/*
	static unsigned char selector_left[] = 
		{ b___X___,
		  b___XX__,
		  b___XXX_,
		  b___XXXX,
		  b___XXX_,
		  b___XX__,
		  b___X___,
		  b_______ };
	static unsigned char selector_right[] = 
		{ b_____X_,
		  b____XX_,
		  b___XXX_,
		  b__XXXX_,
		  b___XXX_,
		  b____XX_,
		  b_____X_,
		  b_______ };
	static unsigned char ellipsis[] = 
		{ b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b__X_X_X,
		  b_______ };
	static unsigned char block_filled[] = 
		{ b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };
	*/

	/* Yes we know, this is a VERY BAD implementation :-) */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			jw002_chr(drvthis, x, y, 136);  // Font 0 fill chr
			break;
		case ICON_HEART_FILLED:
			jw002_set_char(drvthis, CC_OFFSET, heart_filled);
			jw002_chr(drvthis, x, y, CC_OFFSET);
			break;
		case ICON_HEART_OPEN:
			jw002_set_char(drvthis, CC_OFFSET+1, heart_open);
			jw002_chr(drvthis, x, y, CC_OFFSET+1);
			break;
		case ICON_ARROW_UP:
			jw002_set_char(drvthis, CC_OFFSET+2, arrow_up);
			jw002_chr(drvthis, x, y, CC_OFFSET+2);
		case ICON_ARROW_DOWN:
			jw002_set_char(drvthis, CC_OFFSET+3, arrow_down);
			jw002_chr(drvthis, x, y, CC_OFFSET+3);
			break;
		case ICON_ARROW_LEFT:
			jw002_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			jw002_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
			jw002_set_char(drvthis, CC_OFFSET+4, checkbox_off);
			jw002_chr(drvthis, x, y, CC_OFFSET+4);
			break;
		case ICON_CHECKBOX_ON:
			jw002_set_char(drvthis, CC_OFFSET+5, checkbox_on);
			jw002_chr(drvthis, x, y, CC_OFFSET+5);
			break;
		case ICON_CHECKBOX_GRAY:
			jw002_set_char(drvthis, CC_OFFSET+6, checkbox_gray);
			jw002_chr(drvthis, x, y, CC_OFFSET+6);
			break;
		default:
			return -1; /* Let the core do other icons */
	}

	return 0;
}


/**
 * Set cursor position and state.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal cursor position (column).
 * \param y        Vertical cursor position (row).
 * \param state    New cursor state.
 */
MODULE_EXPORT void 
jw002_cursor (Driver *drvthis, int x, int y, int state)
{

	// jw-002 doesn't support visible (hardware-driven) cursors

	/* set cursor position */
	 jw002_cursor_goto(drvthis, x, y);
}

/**
 * Initialize keypad strings
 *
 * The standard PJRC firmware for the JW-002 sends back strings at
 * keydown, keyup, and repeat.  We don't really care about keyup,
 * and to us, repeat might as well be another keydown event.  Fortunately,
 * the firmware allows us to set arbitrary strings for all keys and
 * all events.
 *
 * To simplify things, we will set the keydown and repeat strings to 'A'-'L',
 * and the keyup strings to nothing (12 spaces).
 *
 * From http://www.pjrc.com/mp3/lcd_protocol.html
 *
 * Configure Pushbutton Message String
 *
 *  Transmit:
 *      0x5C 0x48 <button> <string, 12 bytes>
 *        button = 48 to 59, sets string when pressed (down)
 *        button = 64 to 75, sets strings when released (up)
 *        button = 80 to 91, sets strings for auto-repeat
 *
 *        string = 12 byte string to send, pad with spaces to 12 bytes,
 *        (all spaces suppresses printing of the string)
 *
 *  Receive:
 *      nothing 
 *
 *
 * \param drvthis  Pointer to driver structure.
 *
 */
#define KEYDOWN_STR 48
#define KEYUP_STR   64
#define KEYRPT_STR  80

void
jw002_init_keys (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int key;

	// The basic command structure is 0x5C 0x48 <key> <12 char return string>
        unsigned char out[15]      = { '\\', 'H', 0, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };

	// loop over all possible key values and set all three strings
	for (key = 0; key < MAX_KEY_MAP; key++) {

		// 'down' first
		out[2] = key + KEYDOWN_STR;
		out[3] = key + 'A'; // return 'A' through 'L'
		write(p->fd, out, 15);

		// 'repeat' next
		out[2] = key + KEYRPT_STR;
		out[3] = key + 'A'; // return 'A' through 'L'
		write(p->fd, out, 15);

		// 'up' last
		out[2] = key + KEYUP_STR;
		out[3] = ' ' ; // return nothing
		write(p->fd, out, 15);
	}

        debug(RPT_DEBUG, "jw002: initialized keypad return strings");
}

/**
 * Get key from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  String representation of the key.
 *
 */
MODULE_EXPORT const char *
jw002_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char key = 0;
	struct pollfd fds[1];

	/* don't query the keyboard if there are no mapped keys; see \todo above */
	if ((p->keys == 0) && (!p->keypad_test_mode))
		return NULL;

	/* poll for data or return */
	fds[0].fd = p->fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	poll(fds,1,0);
	if (fds[0].revents == 0)
		return NULL;

	(void) read(p->fd, &key, 1);
	report(RPT_DEBUG, "%s: get_key: key 0x%02X", drvthis->name, key);

	if (key == '\0')
		return NULL;

	if (!p->keypad_test_mode) {
		/* we assume standard key mapping here */
		if ((key >= 'A') && (key <= 'A' + MAX_KEY_MAP)) {
			return p->keymap[key-'A'];
		}
		if (key == 0x0d || key == 0x0a) {
			return NULL;	// ignore CR and LF from the jw002
		}
		else {
			report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
			return NULL;
		}
	}
	else {
		fprintf(stdout, "jw002: Received character %c\n", key);
		fprintf(stdout, "jw002: Press another key of your device.\n");
	}
	return NULL;
}

/* EOF */
