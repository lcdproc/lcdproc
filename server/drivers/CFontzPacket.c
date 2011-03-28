/** \file server/drivers/CFontzPacket.c
 * LCDd \c CFontzPacket driver for the CFA533, CFA631, CFA633 & CFA635 display
 * series by CrystalFontz, Inc which use a packet based protocol.
 *
 * Applicable Data Sheets:
 *  \li http://www.crystalfontz.com/products/533/data_sheet/data_sheet.html
 *  \li http://www.crystalfontz.com/products/631/data_sheet/data_sheet.html
 *  \li http://www.crystalfontz.com/products/633/data_sheet/data_sheet.html
 *  \li http://www.crystalfontz.com/products/635/data_sheet/data_sheet.html
 *
 * \todo Add a cache for the custom characters.
 */

/*-
 *  Copyright (c) 2002 David GLAUDE
 *                2005 Peter Marschall
 *                2005 Nicolas Croiset <ncroiset@vdldiffusion.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
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

#include "lcd.h"
#include "CFontzPacket.h"
#include "CFontz633io.h"
#include "report.h"
#include "lcd_lib.h"
#include "CFontz-charmap.h"
#include "adv_bignum.h"

/* LEDs dispatch */
#define CF635_NUM_LEDs	8

/* Constants for userdefchar_mode */
#define NUM_CCs		8 /* max. number of custom characters */


/** private data for the \c CFontzPacket driver */
typedef struct CFontzPacket_private_data {
	char device[200];

	int fd;

	int model;
	int oldfirmware;
	int usb;
	int speed;

	CFA_Model *model_desc;

	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* definable characters */
	CGmode ccmode;

	int contrast;
	int brightness;
	int offbrightness;
	unsigned int LEDstate;

	char info[255];
} PrivateData;

/** List of known models and their default settings and features */
static CFA_Model CFA_ModelList[] = {
	{533, "16x2", 5, 19200 , HD44780_charmap, CFA_HAS_TEMP | CFA_HAS_4_TEMP_SLOTS},
	{631, "20x2", 5, 115200, CFontz_charmap , CFA_HAS_FAN | CFA_HAS_TEMP |
						CFA_HAS_KS0073 | CFA_HAS_4_TEMP_SLOTS},
	{633, "16x2", 5, 19200 , HD44780_charmap, CFA_HAS_FAN | CFA_HAS_TEMP},
	{635, "20x4", 5, 115200, CFontz_charmap , CFA_HAS_KS0073},
	{0, NULL, 0, 0, NULL, 0}
};

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "CFontzPacket_";

/* Internal functions */
static void CFontzPacket_hidecursor (Driver *drvthis);
static void CFontzPacket_reboot (Driver *drvthis);
static void CFontzPacket_no_fan_report (Driver *drvthis);
static void CFontzPacket_no_live_report (Driver *drvthis);
static void CFontzPacket_no_temp_report (Driver *drvthis);
static void CFontzPacket_hardware_clear (Driver *drvthis);
static void CFontzPacket_raw_chr (Driver *drvthis, int x, int y, unsigned char c);

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
CFontzPacket_init (Driver *drvthis)
{
	struct termios portset;
	int tmp, w, h, i;
	int cf_reboot = 0;
	char size[200] = "";

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */
	p->fd = -1;
	p->cellheight = DEFAULT_CELL_HEIGHT;
	p->ccmode = standard;
	p->LEDstate = 0xFFFF;

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis);

	EmptyKeyRing(&keyring);
	EmptyReceiveBuffer(&receivebuffer);

	/* Read config file */

	/* Try to find a matching model from our list of known modules */
	tmp = drvthis->config_get_int(drvthis->name, "Model", 0, 633);
	debug(RPT_INFO, "%s: Model (in config) is '%d'", __FUNCTION__, tmp);
	i = 0;
	while ((CFA_ModelList[i].model != 0) && (CFA_ModelList[i].model != tmp)) {
		i++;
	}
	if (CFA_ModelList[i].model == 0) {
		report(RPT_ERR, "%s: Invalid model configured", drvthis->name);
		return -1;
	}
	p->model = tmp;
	p->model_desc = &(CFA_ModelList[i]);
	report(RPT_INFO, "%s: Found configuration for %d", __FUNCTION__, p->model_desc->model);
	debug(RPT_INFO, "%s: Flags are 0x%04X", __FUNCTION__, p->model_desc->flags);

	/* Which device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* Size setting */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, p->model_desc->size), sizeof(size));
	size[sizeof(size)-1] = '\0';
	debug(RPT_INFO, "%s: Size (in config) is '%s'", __FUNCTION__, size);
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot parse Size: %s; using default %s",
			drvthis->name, size, p->model_desc->size);
		sscanf(p->model_desc->size, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;
	debug(RPT_INFO, "%s: Size used: %dx%d", __FUNCTION__, p->width, p->height);

	/* Cellwidth cannot be set by user */
	p->cellwidth = p->model_desc->cell_width;

	/* Which contrast */
	tmp = drvthis->config_get_int(drvthis->name, "Contrast", 0, DEFAULT_CONTRAST);
	debug(RPT_INFO, "%s: Contrast (in config) is '%d'", __FUNCTION__, tmp);
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

	/* Get speed setting. */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, p->model_desc->speed);
	debug(RPT_INFO, "%s: Speed (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp != 19200) && (tmp != 115200)) {
		report(RPT_WARNING, "%s: Speed must be 19200 or 115200; using default %d",
			drvthis->name, p->model_desc->speed);
		tmp = p->model_desc->speed;
	}
	p->speed = (tmp == 19200) ? B19200 : B115200;

	/* Does the display has an old firmware (<= 0.6)? */
	p->oldfirmware = drvthis->config_get_bool(drvthis->name, "OldFirmware", 0, 0);

	/* Reboot display? */
	cf_reboot = drvthis->config_get_bool(drvthis->name, "Reboot", 0, 0);

	/* Am I USB or not? */
	p->usb = drvthis->config_get_bool(drvthis->name, "USB", 0, 0);
	if (p->usb)
		report(RPT_INFO, "%s: USB is indicated (in config)", drvthis->name);

	/* Set up io port correctly, and open it... */
	debug(RPT_INFO, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open(p->device, (p->usb) ? (O_RDWR | O_NOCTTY) : (O_RDWR | O_NOCTTY | O_NDELAY));
	if (p->fd == -1) {
		report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr (p->fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
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
#endif
	if (p->usb) {
		portset.c_cc[VMIN] = 0;
		portset.c_cc[VTIME] = 0;
	}

	/* Set port speed */
	cfsetospeed (&portset, p->speed);
	cfsetispeed (&portset, B0);

	/* Do it... */
	tcsetattr (p->fd, TCSANOW, &portset);

	/* make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
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

	/* Set display-specific stuff.. */
	if (cf_reboot) {
		report(RPT_INFO, "%s: rebooting LCD...", drvthis->name);
		CFontzPacket_reboot(drvthis);
		cf_reboot = 0;
		debug(RPT_DEBUG, "%s: reboot done", __FUNCTION__);
	}

	CFontzPacket_hidecursor(drvthis);
	CFontzPacket_set_contrast(drvthis, p->contrast);
	CFontzPacket_no_live_report(drvthis);
	CFontzPacket_hardware_clear(drvthis);

	/* turn LEDs off on a CF635 */
	CFontzPacket_output(drvthis, 0);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontzPacket_close (Driver *drvthis)
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
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
CFontzPacket_width (Driver *drvthis)
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
CFontzPacket_height (Driver *drvthis)
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
CFontzPacket_cellwidth (Driver *drvthis)
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
CFontzPacket_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontzPacket_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int modified = 0;
	int i, j;

	if ((p->model == 633) && p->oldfirmware) {
		/*
		 * Older CFA-633 HW/FW types only support updates of full or
		 * partial line starting from pos 0.
		 */
		unsigned char *xp = p->framebuf;
		unsigned char *xq = p->backingstore;

		for (i = 0; i < p->width; i++) {
			if (*xp++ != *xq++) {
				send_bytes_message(p->fd, CF633_Set_LCD_Contents_Line_One, 16, p->framebuf);
				memcpy(p->backingstore, p->framebuf, p->width);
				modified++;
				break;
			}
		}

		xp = p->framebuf + p->width;
		xq = p->backingstore + p->width;

		for (i = 0; i < p->width; i++) {
			if (*xp++ != *xq++) {
				send_bytes_message(p->fd, CF633_Set_LCD_Contents_Line_Two, 16, p->framebuf + p->width);
				memcpy(p->backingstore + p->width, p->framebuf + p->width, p->width);
				modified++;
				break;
			}
		}
	}
	else {
		/*
		 * The current protocol is more flexible and we can do real
		 * delta update.
		 */

		for (i = 0; i < p->height; i++) {
			/*-
			 * Strategy:
			 * - not more than one update command per line
			 * - leave out leading and trailing parts that
			 *   are identical
			 */

			/* set pointers to start of the line in frame buffer & backing store */
			unsigned char *sp = p->framebuf + (i * p->width);
			unsigned char *sq = p->backingstore + (i * p->width);

			/* set pointers to end of the line in frame buffer & backing store */
			unsigned char *ep = sp + (p->width - 1);
			unsigned char *eq = sq + (p->width - 1);
			int length = 0;

			debug(RPT_DEBUG, "Framebuf: '%.*s'", p->width, sp);
			debug(RPT_DEBUG, "Backingstore: '%.*s'", p->width, sq);

			/* skip over leading identical portions of the line */
			for (j = 0; (sp <= ep) && (*sp == *sq); sp++, sq++, j++)
			  ;

			/* skip over trailing identical portions of the line */
			for (length = p->width - j; (length > 0) && (*ep == *eq); ep--, eq--, length--)
			  ;

			/* there are differences, ... */
			if (length > 0) {
				unsigned char out[length + 3];

				/* ... send then to the LCD */
				out[0] = j;	/* column */
				out[1] = i;	/* line */

				debug(RPT_DEBUG, "%s: l=%d c=%d count=%d string='%.*s'",
				      __FUNCTION__, out[0], out[1], length, length, sp);

				memcpy(&out[2], sp, length);
				send_bytes_message(p->fd, CF633_Send_Data_to_LCD, length + 2, out);
				modified++;
			}
		}		/* i < p->height */

		if (modified)
			memcpy(p->backingstore, p->framebuf, p->width * p->height);
	}

	/* send something to the LCD to allow keys to be received */
	if (!modified)
		send_zerobyte_message(p->fd, CF633_Ping_Command);
}


/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
CFontzPacket_get_key (Driver *drvthis)
{
	unsigned char key = GetKeyFromKeyRing(&keyring);

	switch (key) {
		case CFP_KEY_UL_PRESS:
		case CFP_KEY_UP:
			return "Up";
			break;
		case CFP_KEY_LL_PRESS:
		case CFP_KEY_DOWN:
			return "Down";
			break;
		case CFP_KEY_LEFT:
			return "Left";
			break;
		case CFP_KEY_RIGHT:
			return "Right";
			break;
		case CFP_KEY_UR_PRESS:
		case CFP_KEY_ENTER:
			return "Enter";
			break;
		case CFP_KEY_LR_PRESS:
		case CFP_KEY_ESCAPE:
			return "Escape";
			break;
		case CFP_KEY_UP_RELEASE:
		case CFP_KEY_DOWN_RELEASE:
		case CFP_KEY_LEFT_RELEASE:
		case CFP_KEY_RIGHT_RELEASE:
		case CFP_KEY_ENTER_RELEASE:
		case CFP_KEY_ESCAPE_RELEASE:
		case CFP_KEY_UL_RELEASE:
		case CFP_KEY_UR_RELEASE:
		case CFP_KEY_LL_RELEASE:
		case CFP_KEY_LR_RELEASE:
			/* Key release events are ignored */
			return NULL;
			break;
		default:
			if (key != '\0')
				report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
			return NULL;
			break;
	}
	/* NOTREACHED */
	return NULL;
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
CFontzPacket_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: x=%d, y=%d, ch=0x%02X", __FUNCTION__, x, y, c);

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = p->model_desc->charmap[(unsigned char) c];
}


/**
 * Print a character on the screen at position (x,y) bypassing the character
 * mapping table.
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
static void
CFontzPacket_raw_chr (Driver *drvthis, int x, int y, unsigned char c)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: x=%d, y=%d, ch=0x%02X", __FUNCTION__, x, y, c);

	y--;
	x--;

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = c;
}


/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille.
 */
MODULE_EXPORT int
CFontzPacket_get_contrast (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Change LCD contrast.
 * \param drvthis  Pointer to driver structure.
 * \param promille New contrast value in promille.
 */
MODULE_EXPORT void
CFontzPacket_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	int hardware_contrast;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	p->contrast = promille;

	/* map range [0, 1000] to a range that the hardware understands */
	/* on CF633: [0, 50], on CF631 & CF635: [0, 255] */
	hardware_contrast = ((p->model == 633) || (p->model == 533))
			    ? (p->contrast / 20)
			    : ((p->contrast * 255) / 1000);

	send_onebyte_message(p->fd, CF633_Set_LCD_Contrast, hardware_contrast);
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
CFontzPacket_get_brightness(Driver *drvthis, int state)
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
CFontzPacket_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
	}
	else {
		p->offbrightness = promille;
	}
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
CFontzPacket_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->brightness
			     : p->offbrightness;

	/* map range [0, 1000] -> [0, 100] that the hardware understands */
	hardware_value /= 10;
	send_onebyte_message(p->fd, CF633_Set_LCD_And_Keypad_Backlight, hardware_value);
}


/**
 * Get rid of the blinking cursor.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_hidecursor (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 0);
}


/**
 * Stop live reporting of temperature and fan RPM.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_no_live_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[2] = { 0, 0 };
	int slots = 8;

	if ((p->model_desc->flags & CFA_HAS_FAN) || (p->model_desc->flags & CFA_HAS_TEMP)) {
		if (p->model_desc->flags & CFA_HAS_4_TEMP_SLOTS)
			slots = 4;

		for (out[0] = 0; out[0] < slots; out[0]++)
			send_bytes_message(p->fd, CF633_Set_Up_Live_Fan_or_Temperature_Display, 2, out);
	}
}


/**
 * Stop the reporting of any fan.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_no_fan_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->model_desc->flags & CFA_HAS_FAN)
		send_onebyte_message(p->fd, CF633_Set_Up_Fan_Reporting, 0);
}


/**
 * Stop the reporting of any temperature.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_no_temp_report (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[4] = { 0, 0, 0, 0 };

	if (p->model_desc->flags & CFA_HAS_TEMP)
		send_bytes_message(p->fd, CF633_Set_Up_Temperature_Reporting, 4, out);
}


/**
 * Reset the LCD display.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_reboot (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[3] = { 8, 18, 99 };

	send_bytes_message(p->fd, CF633_Reboot, 3, out);
	sleep(2);
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
CFontzPacket_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			/*
			 * add pixel line per pixel line ...
			 * NOTE: cellwidth != bar width:
			 * 0x1F = 0xFF & ((1 << (p->cellwidth - 1)) - 1)
			 */
			vBar[p->cellheight - i] = 0x1F;
			CFontzPacket_set_char(drvthis, i, vBar);
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
CFontzPacket_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
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
			/* fill pixel columns from left to right. */
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			CFontzPacket_set_char(drvthis, i, hBar);
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
CFontzPacket_num(Driver *drvthis, int x, int num)
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

	/* Lib_adv_bignum does everything needed to show the bignumbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
CFontzPacket_get_free_chars (Driver *drvthis)
{
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
CFontzPacket_set_char (Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char out[9];
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	if ((n < 0) || (n >= NUM_CCs) || (!dat))
		return;

	/*
	 * Models with a KS0073 (631, 635) have a seamless display. Clear the
	 * last line on those to avoid the custom characters banging together
	 * with other characters (except if big numbers are to be shown). This
	 * actually makes vBars not seamless. This is supposed to be a feature,
	 * not a bug.
	 */
	if ((p->model_desc->flags & CFA_HAS_KS0073) && (p->ccmode != bignum))
		dat[p->cellheight-1] = 0;

	out[0] = n;	/* Custom char to define. */
	for (row = 0; row < p->cellheight; row++) {
		out[row+1] = dat[row] & mask;
	}
	send_bytes_message(p->fd, CF633_Set_LCD_Special_Character_Data, 9, out);
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
CFontzPacket_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s: x=%d, y=%d, icon=0x%02X", __FUNCTION__, x, y, icon);

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

	switch (icon) {
		case ICON_BLOCK_FILLED:
			if (p->model_desc->flags & CFA_HAS_KS0073) {
				CFontzPacket_raw_chr(drvthis, x, y, 214);
			}
			else
				CFontzPacket_raw_chr(drvthis, x, y, 255);
			break;
		case ICON_HEART_FILLED:
			CFontzPacket_set_char(drvthis, 0, heart_filled);
			CFontzPacket_chr(drvthis, x, y, 0);
			break;
		case ICON_HEART_OPEN:
			CFontzPacket_set_char(drvthis, 0, heart_open);
			CFontzPacket_chr(drvthis, x, y, 0);
			break;
		case ICON_ARROW_UP:
			if (p->model_desc->flags & CFA_HAS_KS0073)
				CFontzPacket_raw_chr(drvthis, x, y, 0xDE);
			else {
				CFontzPacket_set_char(drvthis, 1, arrow_up);
				CFontzPacket_chr(drvthis, x, y, 1);
			}
			break;
		case ICON_ARROW_DOWN:
			if (p->model_desc->flags & CFA_HAS_KS0073)
				CFontzPacket_raw_chr(drvthis, x, y, 0xE0);
			else {
				CFontzPacket_set_char(drvthis, 2, arrow_down);
				CFontzPacket_chr(drvthis, x, y, 2);
			}
			break;
		case ICON_ARROW_LEFT:
			if (p->model_desc->flags & CFA_HAS_KS0073)
				CFontzPacket_raw_chr(drvthis, x, y, 0xE1);
			else
				CFontzPacket_raw_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			if (p->model_desc->flags & CFA_HAS_KS0073)
				CFontzPacket_raw_chr(drvthis, x, y, 0xDF);
			else
				CFontzPacket_raw_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
			CFontzPacket_set_char(drvthis, 3, checkbox_off);
			CFontzPacket_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			CFontzPacket_set_char(drvthis, 4, checkbox_on);
			CFontzPacket_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			CFontzPacket_set_char(drvthis, 5, checkbox_gray);
			CFontzPacket_chr(drvthis, x, y, 5);
			break;
		case ICON_SELECTOR_AT_LEFT:
			if (!(p->model_desc->flags & CFA_HAS_KS0073))
				return -1;
			CFontzPacket_raw_chr(drvthis, x, y, 0x10);
			break;
		case ICON_SELECTOR_AT_RIGHT:
			if (!(p->model_desc->flags & CFA_HAS_KS0073))
				return -1;
			CFontzPacket_raw_chr(drvthis, x, y, 0x11);
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
CFontzPacket_cursor (Driver *drvthis, int x, int y, int state)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (!p->oldfirmware) {
		unsigned char cpos[2] = { 0, 0 };

		/* set cursor state */
		switch (state) {
			case CURSOR_OFF:	/* no cursor */
				send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 0);
				break;
			case CURSOR_UNDER:	/* underline cursor */
				send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 2);
				break;
			case CURSOR_BLOCK:	/* inverting blinking block */
				if (p->model == 631 || p->model == 635)
					send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 4);
				break;
			case CURSOR_DEFAULT_ON:	/* blinking block */
				/* FALLTHROUGH */
			default:
				send_onebyte_message(p->fd, CF633_Set_LCD_Cursor_Style, 1);
				break;
		}

		/* set cursor position */
		if ((x > 0) && (x <= p->width))
			cpos[0] = x - 1;
		if ((y > 0) && (y <= p->height))
			cpos[1] = y - 1;
		send_bytes_message(p->fd, CF633_Set_LCD_Cursor_Position, 2, cpos);
	}
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CFontzPacket_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
	p->ccmode = standard;
}


/**
 * Clear the LCD using hardware commands.
 * \param drvthis  Pointer to driver structure.
 */
static void
CFontzPacket_hardware_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	send_zerobyte_message(p->fd, CF633_Clear_LCD_Screen);
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
CFontzPacket_string (Driver *drvthis, int x, int y, const char string[])
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
		if (x >= 0) {
			p->framebuf[(y * p->width) + x] =
				p->model_desc->charmap[(unsigned char) string[i]];
		}
	}
}


/**
 * Set output port: output values using the LEDs of a CF635.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing LED states.
 */
MODULE_EXPORT void
CFontzPacket_output(Driver *drvthis, int state)
{
	static const unsigned char CFontz635_LEDs[CF635_NUM_LEDs] = {
		11, 9, 7, 5,	/* Green LEDs first, Top first */
		12,10, 8, 6,	/* Red LEDs next, Top first */
	};
	PrivateData *p = drvthis->private_data;
	unsigned char out[2];
	int lednum;

	if (p->model != 635)
		return;

	for (lednum = 0; lednum < CF635_NUM_LEDs; lednum++) {
		unsigned int mask = (1 << lednum);
		int on_off = (state & mask);

		if ((p->LEDstate & mask) != on_off) {
			out[0] = CFontz635_LEDs[lednum];
			out[1] = (on_off == 0) ? 0 : 100;
			send_bytes_message(p->fd, CF633_Set_GPIO_Pin, 2, out);
		}
	}
	p->LEDstate = state;
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return  Constant string with information.
 */
MODULE_EXPORT const char *
CFontzPacket_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	snprintf(p->info, sizeof(p->info)-1, "CrystalFontz Driver: CFA-%d",
		 p->model);
	return p->info;
}

/* EOF */
