/*  This is the LCDproc driver for Matrix Orbital devices
    (http://www.matrixorbital.com)

    For the Matrix Orbital LCD* LKD* VFD* and VKD* displays

    Applicable Data Sheets:
    - http://www.matrixorbital.ca/manuals/

    NOTE: GLK displays have a different driver.

    Copyright (C) 1999, William Ferrell and Scott Scriven
		  2001, Andre Breiler
		  2001, Philip Pokorny
		  2001, David Douthitt
		  2001, David Glaude
		  2001, Joris Robijn
		  2001, Rene Wagner
		  2006, Peter Marschall
		  2006, Ethan Dicks

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
#include "MtxOrb.h"
#include "adv_bignum.h"

#include "report.h"


/* MO displays allow 25 keys that map by default to 'A' - 'Y' */
#define MAX_KEY_MAP	25
// Don't forget to define key mappings in LCDd.conf: KeyMap_A=Enter, ...
// otherwise you will not get your keypad working.

#define IS_LCD_DISPLAY	(p->MtxOrb_type == MTXORB_LCD)
#define IS_LKD_DISPLAY	(p->MtxOrb_type == MTXORB_LKD)
#define IS_VFD_DISPLAY	(p->MtxOrb_type == MTXORB_VFD)
#define IS_VKD_DISPLAY	(p->MtxOrb_type == MTXORB_VKD)

/*
 * The MtxOrb driver does not use a lot of hardware features.
 * We try to replace them with more flexible software alternatives.
 * That's why vbar/hbar/bignum are using locally-defined functions;
 * it permits simultaneous use of those features and custom chars.
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
 */


typedef enum {
	MTXORB_LCD,
	MTXORB_LKD,
	MTXORB_VFD,
	MTXORB_VKD
} MtxOrb_type_type;


/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */

typedef enum {
	standard,	/* only char 0 is used for heartbeat */
	vbar,		/* vertical bars */
	hbar,		/* horizontal bars */
	custom,		/* custom settings */
	bignum,		/* big numbers */
	bigchar		/* big characters */
} CGmode;


typedef struct {
	int fd;			/* The LCD file descriptor */
	
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* defineable characters */
	CGmode ccmode;

	int output_state;	/* static data from MtxOrb_output */
	int contrast;		/* static data from set/get_contrast */
	int brightness;
	int offbrightness;

	MtxOrb_type_type MtxOrb_type;

	/* the keymap */
	char *keymap[MAX_KEY_MAP];
	int keys;
	int keypad_test_mode;

	char info[255];		/* static data from MtxOrb_get_info */
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "MtxOrb_";

static void MtxOrb_hardware_clear(Driver *drvthis);
static void MtxOrb_linewrap(Driver *drvthis, int on);
static void MtxOrb_autoscroll(Driver *drvthis, int on);
static void MtxOrb_cursorblink(Driver *drvthis, int on);
static void MtxOrb_cursor_goto(Driver *drvthis, int x, int y);


/* Parse one key from the configfile */
static char
MtxOrb_parse_keypad_setting (Driver *drvthis, char *keyname, char default_value)
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
MtxOrb_init (Driver *drvthis)
{
	struct termios portset;

	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	char buf[256] = "";
	int tmp, w, h;

        PrivateData *p;

	/* Alocate and store private data */
        p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
	        return -1;
	if (drvthis->store_private_ptr(drvthis, p))
	        return -1;

	/* Initialise the PrivateData structure */
	p->fd = -1;
	p->MtxOrb_type = MTXORB_LKD;  /* Assume it's an LCD w/keypad */

	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	p->framebuf = NULL;
	p->backingstore = NULL;

	p->output_state = -1;	/* static data from MtxOrb_output */
	p->keypad_test_mode = 0;

	debug(RPT_INFO, "MtxOrb: init(%p)", drvthis);

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

	/* Get contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Contrast must be between 0 and 1000; using default %d",
				drvthis->name, DEFAULT_CONTRAST);
		tmp = DEFAULT_CONTRAST;
	}
	p->contrast = tmp;

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_OFFBRIGHTNESS);
		tmp = DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

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

	/* Get display type */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Type", 0, DEFAULT_TYPE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	if (strncasecmp(buf, "lcd", 3) == 0) {
		p->MtxOrb_type = MTXORB_LCD;
	} else if (strncasecmp(buf, "lkd", 3) == 0) {
		p->MtxOrb_type = MTXORB_LKD;
	} else if (strncasecmp(buf, "vfd", 3) == 0) {
		p->MtxOrb_type = MTXORB_VFD;
	} else if (strncasecmp(buf, "vkd", 3) == 0) {
		p->MtxOrb_type = MTXORB_VKD;
	} else {
		report(RPT_ERR, "%s: unknown display Type %s; must be one of lcd, lkd, vfd, or vkd",
				drvthis->name, buf);
		return -1;
	}

	/* Get keypad settings */

	/* keypad test mode? */
	if (drvthis->config_get_bool(drvthis->name, "keypad_test_mode", 0, 0)) {
		fprintf( stdout, "MtxOrb: Entering keypad test mode...\n");
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
	p->fd = open(device, O_RDWR | O_NOCTTY);
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
	MtxOrb_hardware_clear(drvthis);
	MtxOrb_linewrap(drvthis, DEFAULT_LINEWRAP);
	MtxOrb_autoscroll(drvthis, DEFAULT_AUTOSCROLL);
	MtxOrb_cursorblink(drvthis, DEFAULT_CURSORBLINK);
	MtxOrb_set_contrast(drvthis, p->contrast);
	MtxOrb_backlight(drvthis, DEFAULT_BACKLIGHT);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
MtxOrb_close (Driver *drvthis)
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
	debug(RPT_DEBUG, "MtxOrb: closed");
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
MtxOrb_width (Driver *drvthis)
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
MtxOrb_height (Driver *drvthis)
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
MtxcOrb_cellwidth (Driver *drvthis)
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
MtxcOrb_cellheight (Driver *drvthis)
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
MtxOrb_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

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

	debug(RPT_DEBUG, "MtxOrb: printed string at (%d,%d)", x, y);
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
MtxOrb_clear (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	/* set hbar/vbar/bignum mode back to normal character display */
	p->ccmode = standard;

	/* replace all chars in framebuf with spaces */
	memset(p->framebuf, ' ', (p->width * p->height));

	/* make backing store differ from framebuf so it all gets cleared by MtxOrb_flush */
	//memset(p->backingstore, 0xFE, (p->width * p->height));

	debug(RPT_DEBUG, "MtxOrb: cleared screen");
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
MtxOrb_flush (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;
	int modified = 0;
	int i, j;

	for (i = 0; i < p->height; i++) {
		// set  pointers to start of the line in frame buffer & backing store
		unsigned char *sp = p->framebuf + (i * p->width);
		unsigned char *sq = p->backingstore + (i * p->width);

		debug(RPT_DEBUG, "Framebuf: '%.*s'", p->width, sp);
		debug(RPT_DEBUG, "Backingstore: '%.*s'", p->width, sq);

		/* Strategy:
		 * - not more than one update command per line
		 * - leave out leading and trailing parts that are identical
		 */

		// set  pointers to end of the line in frame buffer & backing store
		unsigned char *ep = sp + (p->width - 1);
		unsigned char *eq = sq + (p->width - 1);
		int length = 0;

		// skip over leading identical portions of the line
		for (j = 0; (sp <= ep) && (*sp == *sq); sp++, sq++, j++)
			;

		// skip over trailing identical portions of the line
		for (length = p->width - j; (length > 0) && (*ep == *eq); ep--, eq--, length--)
			;

		/* there are differences, ... */
		if (length > 0) {
			unsigned char out[length+2];
			unsigned char *byte;

			memcpy(out, sp, length);
			// replace command character \xFE by space
			while ((byte = memchr(out, '\xFE', length)) != NULL)
				*byte = ' ';

			debug(RPT_DEBUG, "%s: l=%d c=%d count=%d string='%.*s'",
			      __FUNCTION__, i, j, length, length, sp);

			MtxOrb_cursor_goto(drvthis, j+1, i+1);
			write(p->fd, out, length);
			modified++;
		}      
	}	// i < p->height

	if (modified)
		memcpy(p->backingstore, p->framebuf, p->width * p->height);

	debug(RPT_DEBUG, "MtxOrb: frame buffer flushed");
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
MtxOrb_chr (Driver *drvthis, int x, int y, char c)
{
        PrivateData *p = drvthis->private_data;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
	                p->framebuf[(y * p->width) + x] = c;

	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
}


/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille.
 */

MODULE_EXPORT int
MtxOrb_get_contrast (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change LCD contrast.
 * Whis only works on LCD displays where the
 * HW supports values from 0 to 255 with 140 looking OK.
 * Is it better to use the brightness for VFD/VKD displays ?
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
MtxOrb_set_contrast (Driver *drvthis, int promille)
{
        PrivateData *p = drvthis->private_data;
	int real_contrast = (int) ((long) promille * 255 / 1000);

	/* Check it */
	if ((promille < 0) || (promille > 1000))
		return;

	/* Store it */
	p->contrast = promille;

	/* And do it */
	if (IS_LCD_DISPLAY || IS_LKD_DISPLAY) {
		unsigned char out[4] = { '\xFE', 'P', 0 };

		out[2] = (unsigned char) real_contrast;
		write(p->fd, out, 3);

		report(RPT_DEBUG, "%s: contrast set to %d",
				drvthis->name, real_contrast);
	} else {
		report(RPT_DEBUG, "%s: contrast not set to %d - not LCD or LKD display",
				drvthis->name, real_contrast);
	}
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
MtxOrb_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
 */
MODULE_EXPORT void
MtxOrb_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is no get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
		MtxOrb_backlight(drvthis, BACKLIGHT_ON);
	}
	else {
		p->offbrightness = promille;
		MtxOrb_backlight(drvthis, BACKLIGHT_OFF);
	}
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
MtxOrb_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int promille = (on == BACKLIGHT_ON)
			     ? p->brightness
			     : p->offbrightness;

	if (IS_VKD_DISPLAY) {
		unsigned char out[5] = { '\xFE', '\x89', 0 };

		/* map range [0, 1000] -> [0, 3] that the hardware understands */
		out[2] = (unsigned char) ((long) promille * 3 / 1000);

		write(p->fd, out, 3);
	}
	else {
		unsigned char out[5] = { '\xFE', '\x99', 0 };

		/* map range [0, 1000] -> [0, 255] that the hardware understands */
		out[2] = (unsigned char) ((long) promille * 255 / 1000);

		write(p->fd, out, 3);
	}

	debug(RPT_DEBUG, "MtxOrb: changed brightness to %d", promille);
}


/**
 * Set output port(s).
 * Displays with keypad have 6 outputs whereas the others only have one.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
MODULE_EXPORT void
MtxOrb_output (Driver *drvthis, int state)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[5] = { '\xFE', 0, 0 };

	state &= 0x3F;	/* strip to six bits */

	p->output_state = state;

	debug(RPT_DEBUG, "MtxOrb: output pins set: %04X", state);

	if (IS_LCD_DISPLAY || IS_VFD_DISPLAY) {
		/* LCD and VFD displays only have one output port */
		out[1] = (state) ? 'W' : 'V';
		write(p->fd, out, 2);
	}
	else {
		int i;

		/* Other displays have six output ports;
		 * the value "on" is a binary value determining which
		 * ports are turned on (1) and off (0).
		 */
		for (i = 0; i < 6; i++) {
			out[1] = (state & (1 << i)) ? 'W' : 'V';
			out[2] = i+1;
			write(p->fd, out, 3);
		}
	}
}


/**
 * Clear the LCD using ints hardware command
 * \param drvthis  Pointer to driver structure.
 */
static void
MtxOrb_hardware_clear (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

	write(p->fd, "\xFE" "X", 2);

	debug(RPT_DEBUG, "MtxOrb: cleared LCD");
}


/**
 * Turn the LCD's built-in linewrapping feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
MtxOrb_linewrap (Driver *drvthis, int on)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { '\xFE', 0 };

	out[1] = (on) ? 'C' : 'D';
	write(p->fd, out, 2);

	debug(RPT_DEBUG, "MtxOrb: linewrap turned %s", (on) ? "on" : "off");
}


/**
 * Turn the LCD's built-in automatic scrolling feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
MtxOrb_autoscroll (Driver *drvthis, int on)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { '\xFE', 0 };

	out[1] = (on) ? 'Q' : 'R';
	write(p->fd, out, 2);

	debug(RPT_DEBUG, "MtxOrb: autoscroll turned %s", (on) ? "on" : "off");
}


/**
 * Turn LCD's hardware cursor blinking on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 *
 * \todo  Make sure this doesn't mess up non-VFD displays.
 */
static void
MtxOrb_cursorblink (Driver *drvthis, int on)
{
        PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { '\xFE', 0 };

	out[1] = (on) ? 'S' : 'T';
	write(p->fd, out, 2);

	debug(RPT_DEBUG, "MtxOrb: cursorblink turned %s", (on) ? "on" : "off");
}


/**
 * Move cursor to position (x,y).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 */
static void
MtxOrb_cursor_goto(Driver *drvthis, int x, int y)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[5] = { '\xFE', 'G', 0, 0 };

	/* set cursor position */
	if ((x > 0) && (x <= p->width))
		out[2] = (unsigned char) x;
	if ((y > 0) && (y <= p->height))
		out[3] = (unsigned char) y;
	write(p->fd, out, 4);
}


/**
 * Provide general information about the LCD/VFD display.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
MtxOrb_get_info (Driver *drvthis)
{
	char in = 0;
	char tmp[255], buf[64];
	/* int i = 0; */
        PrivateData *p = drvthis->private_data;

	fd_set rfds;

	struct timeval tv;
	int retval;

	debug(RPT_DEBUG, "MtxOrb: get_info");

	memset(p->info, '\0', sizeof(p->info));
	strcpy(p->info, "Matrix Orbital Driver ");

	/*
	 * Read type of display
	 */

	write(p->fd, "\x0FE" "7", 2);

	/* Watch fd to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(p->fd, &rfds);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		/* seconds */
	tv.tv_usec = 500;	/* microseconds */

/*	retval = select(p->fd+1, &rfds, NULL, NULL, &tv); */
	retval = select(p->fd+1, &rfds, NULL, NULL, NULL);

	if (retval) {
		if (read (p->fd, &in, 1) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			switch (in) {
				case '\x01': strcat(p->info, "LCD0821 "); break;
				case '\x03': strcat(p->info, "LCD2021 "); break;
				case '\x04': strcat(p->info, "LCD1641 "); break;
				case '\x05': strcat(p->info, "LCD2041 "); break;	/* VK202-25 ? */
				case '\x06': strcat(p->info, "LCD4021 "); break;
				case '\x07': strcat(p->info, "LCD4041 "); break;
				case '\x08': strcat(p->info, "LK202-25 "); break;
				case '\x09': strcat(p->info, "LK204-25 "); break;
				case '\x0A': strcat(p->info, "LK404-55 "); break;
				case '\x0B': strcat(p->info, "VFD2021 "); break;
				case '\x0C': strcat(p->info, "VFD2041 "); break;
				case '\x0D': strcat(p->info, "VFD4021 "); break;
				case '\x0E': strcat(p->info, "VK202-25 "); break;
				case '\x0F': strcat(p->info, "VK204-25 "); break;
				case '\x10': strcat(p->info, "GLC12232 "); break;
				case '\x11': strcat(p->info, "GLC12864 "); break;
				case '\x12': strcat(p->info, "GLC128128 "); break;	/* ??? */
				case '\x13': strcat(p->info, "GLC24064 "); break;
				case '\x14': strcat(p->info, "GLK12864-25 "); break;	/* ??? */
				case '\x15': strcat(p->info, "GLK24064-25 "); break;
				case '\x21': strcat(p->info, "GLK128128-25 "); break;	/* ??? */
				case '\x22': strcat(p->info, "GLK12232-25-WBL "); break;
				case '\x24': strcat(p->info, "GLK12232-25-SM "); break;
				case '\x31': strcat(p->info, "LK404-AT "); break;
				case '\x32': strcat(p->info, "VFD1621 "); break;	/* MOS-AV-162A ? */
				case '\x33': strcat(p->info, "LK402-12 "); break;
				case '\x34': strcat(p->info, "LK162-12 "); break;
				case '\x35': strcat(p->info, "LK204-25PC "); break;
				case '\x36': strcat(p->info, "LK202-24-USB "); break;
				case '\x37': strcat(p->info, "VK202-24-USB "); break;
				case '\x38': strcat(p->info, "LK204-24-USB "); break;
				case '\x39': strcat(p->info, "VK204-24-USB "); break;
				case '\x3A': strcat(p->info, "PK162-12 "); break;
				case '\x3B': strcat(p->info, "VK162-12 "); break;
				case '\x3C': strcat(p->info, "MOS-AP-162A "); break;
				case '\x3D': strcat(p->info, "PK202-25 "); break;
				case '\x3E': strcat(p->info, "MOS-AL-162A "); break;
				case '\x40': strcat(p->info, "MOS-AV-202A "); break;
				case '\x41': strcat(p->info, "MOS-AP-202A "); break;
				case '\x42': strcat(p->info, "PK202-24-USB "); break;
				case '\x43': strcat(p->info, "MOS-AL-082 "); break;
				case '\x44': strcat(p->info, "MOS-AL-204 "); break;
				case '\x45': strcat(p->info, "MOS-AV-204 "); break;
				case '\x46': strcat(p->info, "MOS-AL-402 "); break;
				case '\x47': strcat(p->info, "MOS-AV-402 "); break;
				case '\x48': strcat(p->info, "LK082-12 "); break;
				case '\x49': strcat(p->info, "VK402-12 "); break;
				case '\x4A': strcat(p->info, "VK404-55 "); break;
				case '\x4B': strcat(p->info, "LK402-25 "); break;
				case '\x4C': strcat(p->info, "VK402-25 "); break;
				default: /*snprintf(tmp, sizeof(tmp), "Unknown (%X) ", in); strcat(info, tmp); */
					     break;
			}
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device type");

	/*
	 * Read serial number of display
	 */

	memset(tmp, '\0', sizeof(tmp));
	write(p->fd, "\x0FE" "5", 2);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		/* seconds */
	tv.tv_usec = 500;	/* microseconds */

/*	retval = select(p->fd+1, &rfds, NULL, NULL, &tv); */
	retval = select(p->fd+1, &rfds, NULL, NULL, NULL);

	if (retval) {
		if (read(p->fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Serial No: %ld ", (long int) tmp);
			strcat(p->info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device serial number");

	/*
	 * Read firmware revision number
	 */

	memset(tmp, '\0', sizeof(tmp));
	write(p->fd, "\x0FE" "6", 2);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		/* seconds */
	tv.tv_usec = 500;	/* microseconds */

/*	retval = select(p->fd+1, &rfds, NULL, NULL, &tv); */
	retval = select(p->fd+1, &rfds, NULL, NULL, NULL);

	if (retval) {
		if (read(p->fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Firmware Rev. %ld ", (long int) tmp);
			strcat(p->info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device firmware revision");

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
MtxOrb_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
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

		for (i = 1; i < p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i] = 0xFF;
			MtxOrb_set_char(drvthis, i, vBar);
		}
	}

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
MtxOrb_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
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
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar)-1);
			MtxOrb_set_char(drvthis, i, hBar);
		}
	}

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
MtxOrb_num (Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

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

	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
MtxOrb_get_free_chars (Driver *drvthis)
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
 */
MODULE_EXPORT void
MtxOrb_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[12] = { '\xFE', 'N', 0, 0,0,0,0,0,0,0,0 };;
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (!dat)
		return;

	out[2] = n;	/* Custom char to define. xxx */

	for (row = 0; row < p->cellheight; row++) {
		out[row+3] = dat[row] & mask;
	}
	write(p->fd, out, 11);
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
MtxOrb_icon (Driver *drvthis, int x, int y, int icon)
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
			MtxOrb_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
			MtxOrb_set_char(drvthis, 0, heart_filled);
			MtxOrb_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			MtxOrb_set_char(drvthis, 0, heart_open);
			MtxOrb_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
			MtxOrb_set_char(drvthis, 1, arrow_up);
			MtxOrb_chr(drvthis, x, y, 1);
		case ICON_ARROW_DOWN:
			MtxOrb_set_char(drvthis, 2, arrow_down);
			MtxOrb_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_LEFT:
			MtxOrb_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			MtxOrb_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
			MtxOrb_set_char(drvthis, 3, checkbox_off);
			MtxOrb_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			MtxOrb_set_char(drvthis, 4, checkbox_on);
			MtxOrb_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			MtxOrb_set_char(drvthis, 5, checkbox_gray);
			MtxOrb_chr(drvthis, x, y, 5);
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
MtxOrb_cursor (Driver *drvthis, int x, int y, int state)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	/* set cursor state */
	switch (state) {
		case CURSOR_OFF:	// no cursor
			write(p->fd, "\xFE" "K", 2);
			break;
		case CURSOR_UNDER:	// underline cursor
		case CURSOR_BLOCK:	// inverting blinking block
		case CURSOR_DEFAULT_ON:	// blinking block
		default:
			write(p->fd, "\xFE" "J", 2);
			break;
	}

	/* set cursor position */
	 MtxOrb_cursor_goto(drvthis, x, y);
}


/**
 * Get key from the LCD/VFD.
 * \param drvthis  Pointer to driver structure.
 * \return  String representation of the key.
 *
 * \todo Recover the code for I2C connectivity to MtxOrb
 * and don't query the LCD if it does not support keypad.
 * Otherwise crash of the LCD and/or I2C bus.
 */
MODULE_EXPORT const char *
MtxOrb_get_key (Driver *drvthis)
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
		else {
			report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
			return NULL;
		}
	}
	else {
		fprintf(stdout, "MtxOrb: Received character %c\n", key);
		fprintf(stdout, "MtxOrb: Press another key of your device.\n");
	}
	return NULL;
}


/*	
	static unsigned char play[] =
		{ b___X___,
     		  b___XX__,
     		  b___XXX_,
     		  b___XXXX,
     		  b___XXX_,
     		  b___XX__,
     		  b___X___,
     		  b_______ };
	static unsigned char fforward[] =
		{ b_______,
     		  b__X_X__,
     		  b__XX_X_,
     		  b__XXX_X,
     		  b__XX_X_,
     		  b__X_X__,
     		  b_______,
     		  b_______ };
	static unsigned char frewind[] =
     		{ b_______,
     		  b____X_X,
     		  b___X_XX,
     		  b__X_XXX,
     		  b___X_XX,
     		  b____X_X,
     		  b_______,
     		  b_______ };
*/		  

/* EOF */
