/* 	This is the LCDproc driver for Point Of Sale ("POS") devices using 
	various protocols.  While it currently only supports AEDEX, 
	it can be extended to provide support for many POS emulation types.

	Copyright (C) 2006 Eric Pooch

	This driver is based on MtxOrb.c driver and is subject to its copyrights.
	
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


List of driver entry point:

init		Implemented.
close		Implemented.
width		Implemented.
height		Implemented.
clear		Implemented by space filling no custom char info.
flush		Implemented.
string		Implemented.
chr		Implemented.
vbar		Implemented.
hbar		Implemented.
num		Implemented.
heartbeat	Implemented.
icon		NOT IMPLEMENTED: not part of POS protocol
cursor		NOT IMPLEMENTED: not part of AEDEX protocol
set_char	NOT IMPLEMENTED: not part of AEDEX protocol
get_free_chars	Implemented.
cellwidth	Implemented.
cellheight	Implemented.
get_contrast	NOT IMPLEMENTED: not part of AEDEX protocol
set_contrast	NOT IMPLEMENTED: not part of AEDEX protocol
get_brightness	NOT IMPLEMENTED: not part of AEDEX protocol
set_brightness	NOT IMPLEMENTED: not part of AEDEX protocol
backlight	NOT IMPLEMENTED: not part of AEDEX protocol
output		Not implemented.
get_key		Not implemented, no keys.
get_info	Implemented.
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
#include "serialPOS.h"
#include "adv_bignum.h"

#include "report.h"


/*
 * The serialPOS driver does not use a lot of hardware features.
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
	POS_IEE = 0,
	POS_AEDEX,
	POS_Epson,
	POS_Emax,
	POS_IBM,
	POS_LogicControls,
	POS_Ultimate
} POS_EmulationType;


#define AEDEXPrefix "!#"

typedef enum {
	AEDEXLine1Display = 1,	/*	upper line display	*/
	AEDEXLine2Display,		/*	bottom line display	*/
	AEDEXLine3Display,		/*	not sure what this code really is	*/
	AEDEXContinuousScroll,	/*	upper line message scroll continuously	*/
	AEDEXDisplayTime,		/*	"hh':'mm" h,m='0'-'9' display time	*/
	AEDEXSingleScroll,		/*	upper line message scroll one pass	*/
	AEDEXAllScroll,			/*	not sure what this code really is	*/
	AEDEXChangeCode,		/*	change attention code	*/
	AEDEXAllLineDisplay		/*	two line display	*/
} AEDEXCommands;


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
	
	/* feature enable */
	int hardwrap;
	int hardscroll;

	POS_EmulationType emulation_mode;	/* The emulation type  */
	int output_state;	/* static data from serialPOS_output */

	char info[255];		/* static data from serialPOS_get_info */
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "serialPOS_";

static void serialPOS_hardware_clear(Driver *drvthis);
static void serialPOS_linewrap(Driver *drvthis, int on);
static void serialPOS_autoscroll(Driver *drvthis, int on);
static void serialPOS_cursorblink(Driver *drvthis, int on);
static void serialPOS_cursor_goto(Driver *drvthis, int x, int y);


/* Parse one key from the configfile */
static char
serialPOS_parse_keypad_setting (Driver *drvthis, char *keyname, char default_value)
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
serialPOS_init (Driver *drvthis)
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

	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	p->framebuf = NULL;
	p->backingstore = NULL;

	p->output_state = -1;	/* static data from serialPOS_output */

	p->hardwrap = 0;
	p->hardscroll = 0;

	p->emulation_mode = POS_AEDEX;
	
	debug(RPT_INFO, "serialPOS: init(%p)", drvthis);

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

	/* Get emulation type */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Type", 0, DEFAULT_TYPE), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	if (strncasecmp(buf, "IEE", 3) == 0) {
		p->emulation_mode = POS_IEE;
	} else if (strncasecmp(buf, "AED", 3) == 0) {
		p->emulation_mode = POS_AEDEX;
	} else if (strncasecmp(buf, "Eps", 3) == 0) {
		p->emulation_mode = POS_Epson;
	} else if (strncasecmp(buf, "Ema", 3) == 0) {
		p->emulation_mode = POS_Emax;
	} else if (strncasecmp(buf, "Log", 3) == 0) {
		p->emulation_mode = POS_LogicControls;
	} else if (strncasecmp(buf, "IBM", 3) == 0) {
		p->emulation_mode = POS_IBM;
	} else if (strncasecmp(buf, "Ult", 3) == 0) {
		p->emulation_mode = POS_Ultimate;
	} else {
		report(RPT_ERR, "%s: unknown display Type %s; must be one of IEE, AEDEX, Epson, Emax, Logic Controls or Ultimate",
				drvthis->name, buf);
		return -1;
	}

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

	/* Get speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	switch (tmp) {
		case 1200:
			speed = B1200;
			break;
		case 2400:
			speed = B2400;
			break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		default:
			speed = B9600;
			report(RPT_WARNING, "%s: Speed must be 1200, 2400, 4800 or 9600; using default %d",
					drvthis->name, tmp);
	}

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
	serialPOS_hardware_clear(drvthis);
	serialPOS_linewrap(drvthis, DEFAULT_LINEWRAP);
	serialPOS_autoscroll(drvthis, DEFAULT_AUTOSCROLL);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 1;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_close (Driver *drvthis)
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
	debug(RPT_DEBUG, "serialPOS: closed");
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
serialPOS_width (Driver *drvthis)
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
serialPOS_height (Driver *drvthis)
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
 * \param x	Horizontal character position (column).
 * \param y	Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
serialPOS_string (Driver *drvthis, int x, int y, const char string[])
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

	report(RPT_DEBUG, "serialPOS: printed string at (%d,%d)", x, y);
}


/**
 * Clear the framebuffer.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	/* replace all chars in framebuf with spaces */
	memset(p->framebuf, ' ', (p->width * p->height));

	report(RPT_DEBUG, "serialPOS: cleared framebuffer");
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
serialPOS_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	int modified = 0;
	int i;

	for (i = 0; i < p->height; i++) {
		// set  pointers to start of the line in frame buffer & backing store
		unsigned char *sp = p->framebuf + (i * p->width);
		unsigned char *sq = p->backingstore + (i * p->width);

		unsigned int length = p->width+5;

		char out[length];

		debug(RPT_DEBUG, "Framebuf: '%.*s'", p->width, sp);
		debug(RPT_DEBUG, "Backingstore: '%.*s'", p->width, sq);

		/* Strategy:
		 * - not more than one update command per line
		 * - skip lines that are identical
		 */

		// skip over identical lines
		if ( memcmp(sp, sq, p->width) == 0) {
			/* The lines are the same. */
			continue;
		}

		/* there are differences, ... */
		report(RPT_DEBUG, "%s: l=%d string='%.*s'", __FUNCTION__, i, p->width, sp);

		if (p->emulation_mode == POS_AEDEX) {
			int command = i+1;
			if ( i == 0 && p->hardscroll == 1 )
				command = AEDEXContinuousScroll;
				
			snprintf(out, length, "%s%d%.*s%c", AEDEXPrefix, command, p->width, sp, 13);
			debug(RPT_DEBUG, "%s%d%.*s%c", AEDEXPrefix, command, p->width, sp, 13);
		}

 		write(p->fd, out, sizeof(out));

		modified++;

	}

	if (modified)
		memcpy(p->backingstore, p->framebuf, p->width * p->height);

	
	report(RPT_DEBUG, "serialPOS: frame buffer flushed");
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal character position (column).
 * \param y	Vertical character position (row).
 * \param c	Character that gets written.
 */
MODULE_EXPORT void
serialPOS_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	/* Convert 1-based coords to 0-based... */
	x--;
	y--;

	/* The # character might interfere with the AEDEX command set */
	if (p->emulation_mode == POS_AEDEX && c == '#')
		c = '%';

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
			p->framebuf[(y * p->width) + x] = c;

	report(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
}


/**
 * Set output port(s).
 * Displays with keypad have 6 outputs whereas the others only have one.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
#ifdef NOTUSED
MODULE_EXPORT void
serialPOS_output (Driver *drvthis, int state)
{
	/* I don't get what this does.  Many POS displays have a pass-through function. */
	PrivateData *p = drvthis->private_data;

}
#endif


/**
 * Clear the LCD using its hardware command
 * \param drvthis  Pointer to driver structure.
 */
static void
serialPOS_hardware_clear (Driver *drvthis)
{
	/* There is no hardware clear in the POS command sets,
		but most have an easy software clear. */
	/*PrivateData *p = drvthis->private_data;

	char out[6];

	if (p->emulation_mode == POS_AEDEX) {
		snprintf(out, 6, "%s%c %d", AEDEXPrefix, AEDEXAllLineDisplay, 13);
	}
	
 	write(p->fd, out, 5);*/
	debug(RPT_DEBUG, "serialPOS: cleared LCD");
}


/**
 * Turn the POS's built-in linewrapping feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
serialPOS_linewrap (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	
	p->hardwrap = on;

	debug(RPT_DEBUG, "serialPOS: linewrap turned %s", (on) ? "on" : "off");
}


/**
 * Turn the POS's built-in automatic scrolling feature on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 */
static void
serialPOS_autoscroll (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	p->hardscroll = on;

	debug(RPT_DEBUG, "serialPOS: autoscroll turned %s", (on) ? "on" : "off");
}


/**
 * Turn POS's hardware cursor blinking on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New state.
 *
 */
static void
serialPOS_cursorblink (Driver *drvthis, int on)
{
	//PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "serialPOS: cursorblink turned %s", (on) ? "on" : "off");
}


/**
 * Move cursor to position (x,y).
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal character position (column).
 * \param y	Vertical character position (row).
 */
static void
serialPOS_cursor_goto(Driver *drvthis, int x, int y)
{
	//PrivateData *p = drvthis->private_data;
}


/**
 * Provide general information about the LCD/VFD display.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
serialPOS_get_info (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	strcpy(p->info, "Driver for Point of Sale Displays.");
	return p->info;
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal character position (column) of the starting point.
 * \param y	Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
serialPOS_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	// map
	char ascii_map[] = { ' ', ' ', '-', '-', '=', '=', '%', '%' };
	char *map = ascii_map;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;
	int pos;


	if ((x <= 0) || (y <= 0) || (x > p->width))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	for (pos = 0; pos < len; pos++) {

		if (y - pos <= 0)
			return;

		if (pixels >= p->cellheight) {
			/* write a "full" block to the screen... */
			serialPOS_chr(drvthis, x, y-pos, '%');
		}
		else if (pixels > 0) {
			// write a partial block...
			serialPOS_chr(drvthis, x, y-pos, map[len-1]);
			break;
		}
		else {
			; // write nothing (not even a space)
		}

		pixels -= p->cellheight;
	}
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal character position (column) of the starting point.
 * \param y	Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
serialPOS_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;
	int pos;

	if ((x <= 0) || (y <= 0) || (y > p->height))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	for (pos = 0; pos < len; pos++) {

		if (x + pos > p->width)
			return;

		if (pixels >= p->cellwidth * 2/3) {
			/* write a "full" block to the screen... */
			serialPOS_chr(drvthis, x+pos, y, '=');
		}
		else if (pixels > p->cellwidth * 1/3) {
			/* write a partial block... */
			serialPOS_chr(drvthis, x+pos, y, '-');
			break;
		}
		else {
			; // write nothing (not even a space)
		}

		pixels -= p->cellwidth;
	}
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
serialPOS_num (Driver *drvthis, int x, int num)
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
	//lib_adv_bignum(drvthis, x, num, 0, do_init);
}

#ifdef NOTUSED
/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
serialPOS_get_free_chars (Driver *drvthis)
{
	//PrivateData *p = drvthis->private_data;

	return NUM_CCs;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n	Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8(=cellheight) bytes, each representing a pixel row
 *		 starting from the top to bottom.
 *		 The bits in each byte represent the pixels where the LSB
 *		 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
serialPOS_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
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
 * \param x	Horizontal character position (column).
 * \param y	Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \return  Information whether the icon is handled here or needs to be handled by the server core.
 */
MODULE_EXPORT int
serialPOS_icon (Driver *drvthis, int x, int y, int icon)
{
	//PrivateData *p = drvthis->private_data;

	return 0;
}


/**
 * Set cursor position and state.
 * \param drvthis  Pointer to driver structure.
 * \param x	Horizontal cursor position (column).
 * \param y	Vertical cursor position (row).
 * \param state    New cursor state.
 */
MODULE_EXPORT void 
serialPOS_cursor (Driver *drvthis, int x, int y, int state)
{
	//hPrivateData *p = (PrivateData *) drvthis->private_data;

	/* set cursor state */
	 //serialPOS_cursor_goto(drvthis, x, y);
}

/**
 * Get key from the LCD/VFD.
 * \param drvthis  Pointer to driver structure.
 * \return  String representation of the key.
 *
 */
MODULE_EXPORT const char *
serialPOS_get_key (Driver *drvthis)
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
		fprintf(stdout, "serialPOS: Received character %c\n", key);
		fprintf(stdout, "serialPOS: Press another key of your device.\n");
	}
	return NULL;
}

#endif

/* EOF */
