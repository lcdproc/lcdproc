/* 	This is the LCDproc driver for various serial VFD Devices

	Copyright (C) 2006 Stefan Herdler

	This driver is based on wirz-sli.c, hd44780.c, drv_base.c and NoritakeVFD
	driver.
	It may contain parts of other drivers of this package too.

	2006-05-16 Version 0.3: everything should work (not all hardware tested!)

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



	The driver should operate most of NEC, Futaba and Noritake 7x5 dot VFDs with
	serial(rs232) and/or parallel interface. See /docs/lcdproc-user/serialvfd-howto.html
	for further information


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
icon		Implemented.
cursor		NOT IMPLEMENTED: Is it really used?
set_char	Implemented.
get_free_chars	Implemented.
cellwidth	Implemented.
cellheight	Implemented.
get_contrast	Not implemented, no software control.
set_contrast	Not implemented, no software control.
get_brightness	Implemented.
set_brightness	Implemented.
backlight	Implemented.
output		Not implemented.
get_key		Not implemented, no keys.
get_info	Implemented.

*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "lcd.h"
#include "serialVFD.h"

#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#define DEFAULT_OFF_BRIGHTNESS	300
#define DEFAULT_ON_BRIGHTNESS	1000

#define pos1_cursor	4 //moves cursor to top left character.
#define mv_cursor	5 //moves cursor to position specified by the next byte.
#define reset		6 //reset
#define init_cmds	7 //commands needed to initialize the display.
#define set_user_char	8 //set user character.
#define hor_tab		9 //moves cursor 1 chr right
#define LPTPORT 0x378

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "serialVFD_";

/* Internal functions */
static void serialVFD_init_vbar (Driver *drvthis);
static void serialVFD_init_hbar (Driver *drvthis);
static void serialVFD_put_char (Driver *drvthis, int n);

// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
serialVFD_init (Driver *drvthis)
{
	int tmp, w, h;
	char size[200] = DEFAULT_SIZE;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;
	p->ccmode = CCMODE_STANDARD;
	p->ISO_8859_1 = 1;
	p->refresh_timer = 480;
	p->hw_brightness = 0;

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis );
	
/* Read config file */

	p->use_parallel	= drvthis->config_get_bool( drvthis->name, "use_parallel", 0, 0 );
	
		/* Which device should be used */
		strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
		p->device[sizeof(p->device)-1] = '\0';
		report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	if (p->use_parallel) {
		p->port	= drvthis->config_get_int( drvthis->name, "port", 0, LPTPORT );
	}
	else {

		/* Which speed */
		tmp = drvthis->config_get_int (drvthis->name, "Speed", 0, DEFAULT_SPEED);
		if ((tmp != 1200) && (tmp != 2400) && (tmp != 9600) && (tmp != 19200) && (tmp != 115200)) {
			report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200. Using default %d.\n",
				drvthis->name, DEFAULT_SPEED);
			tmp = DEFAULT_SPEED;
		}
		if (tmp == 1200) p->speed = B1200;
		else if (tmp == 2400) p->speed = B2400;
		else if (tmp == 9600) p->speed = B9600;
		else if (tmp == 19200) p->speed = B19200;
		else if (tmp == 115200) p->speed = B115200;
	}
//	report(RPT_ERR, "%s: Port: %X\n", __FUNCTION__, p->port, strerror (errno));

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot parse size: %s; using default %s.",
			drvthis->name, size, DEFAULT_SIZE);
		sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
	}
	p->width = w;
	p->height = h;

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_ON_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_ON_BRIGHTNESS);
		tmp = DEFAULT_ON_BRIGHTNESS;
	}
	p->on_brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFF_BRIGHTNESS);
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_OFF_BRIGHTNESS);
		tmp = DEFAULT_OFF_BRIGHTNESS;
	}
	p->off_brightness = tmp;

	/* ISO 8859 1 */
	p->ISO_8859_1 = drvthis->config_get_bool(drvthis->name, "ISO_8859_1", 0, 1);

	/* Which displaytype */
	tmp = drvthis->config_get_int(drvthis->name, "Type", 0, DEFAULT_DISPLAYTYPE);
	if ((tmp < 0) || (tmp > 3)) {
		report(RPT_WARNING, "%s: Type must be between 0 and 3; using default %d",
			drvthis->name, DEFAULT_DISPLAYTYPE);
		tmp = DEFAULT_DISPLAYTYPE;
	}
	p->display_type = tmp;

	/* Number of custom characters */
	tmp = drvthis->config_get_int (drvthis->name, "Custom-Characters", 0, -83);
	if ((tmp < 0) || (tmp > 99)) {
		report(RPT_WARNING, "%s: The number of Custom-Characters must be between 0 and 99. Using default.",
			drvthis->name, 0);
		tmp = -83;
	}
	p->customchars = tmp;


//	report (RPT_ERR, "%s: Port: %X\n", __FUNCTION__, p->port, strerror (errno));

// Do connection type specific io-port init
	if (Port_Function[p->use_parallel].init_fkt (drvthis) == -1) {
		report(RPT_ERR, "%s: unable to initialize io-port.", drvthis->name);
		return -1;
	}

// setup frame buffer and backing store
	/* make sure the frame buffer is there... */
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer.", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->width * p->height);

	/* make sure the framebuffer backing store is there... */
	p->backingstore = (unsigned char *) malloc(p->width * p->height);
	if (p->backingstore == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer backing store.", drvthis->name);
		return -1;
	}
	memset(p->backingstore, 0, p->width * p->height);

//setup displayspecific data
	serialVFD_load_display_data(drvthis);

//	report (RPT_ERR, "%s: Port: %X\n", drvthis->name, p->port, strerror (errno));

//initialise display
	Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[reset][1],p->hw_cmd[reset][0]);
	Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);
	serialVFD_backlight(drvthis, 1);
	report(RPT_DEBUG, "%s: init() done", drvthis->name);
	return 0;

}

/*
 * Retrieves brightness (in promille)
 */

MODULE_EXPORT int
serialVFD_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->on_brightness : p->off_brightness;
}


/*
 * Sets on/off brightness (in promille)
 */
MODULE_EXPORT void
serialVFD_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if (promille < 0 || promille > 1000)
		return;

	/* store the software value since there is not get */
	if (state == BACKLIGHT_ON) {
		p->on_brightness = promille;
	}
	else {
		p->off_brightness = promille;
	}
}


/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 */
MODULE_EXPORT void
serialVFD_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->on_brightness
			     : p->off_brightness;

	// map range [0, 1000] -> [0, 4] that the hardware understands
	//(4 steps 0-250, 251-500, 501-750, 751-1000)
	hardware_value /= 251;
	if(hardware_value != p->hw_brightness){
		p->hw_brightness=hardware_value;
		Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[p->hw_brightness][1],\
		p->hw_cmd[p->hw_brightness][0]);
		}

}



/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
serialVFD_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->customchars >= p->cellheight || p->predefined_vbar == 1){
		serialVFD_init_vbar(drvthis);
		lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, p->vbar_cc_offset);
	}
	else{
		lib_vbar_static(drvthis, x, y, len, promille, options, 2, 0x5E);
	}
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//

MODULE_EXPORT void
serialVFD_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->customchars >= p->cellwidth || p->predefined_hbar == 1){
		serialVFD_init_hbar(drvthis);
		lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, p->hbar_cc_offset);
	}
	else{
		lib_hbar_static(drvthis, x, y, len, promille, options, 2, 0x2C);
	}
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, value 1 mean "on" and 0 is "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
serialVFD_set_char (Driver *drvthis, int n, unsigned char *dat)
{	//set char in p->custom_char
	PrivateData *p = drvthis->private_data;
	unsigned int byte, bit;

	if (n < 0 || n > p->customchars-1)
		return;
	if (!dat)
		return;

	for (byte = 0; byte < p->usr_chr_dot_assignment[0]; byte++) {
		int letter = 0;

		for (bit = 0; bit < 8; bit++) {
			int pos = (int) p->usr_chr_dot_assignment[bit+8*byte+1];

			if (pos > 0) {
				int posbyte = (pos-1) / 5;
				int posbit = 4 - ((pos-1) % 5);

				letter |= ((dat[posbyte] >> posbit) & 1) << bit;;
			}
		}
		p->custom_char[n][byte] = letter;
	}

}

static void
serialVFD_put_char (Driver *drvthis, int n)
{	// put char in display
	PrivateData *p = drvthis->private_data;

	Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[set_user_char][1],\
		p->hw_cmd[set_user_char][0]);// substitute and select Character to overwrite
	Port_Function[p->use_parallel].write_fkt (drvthis, (char*)&p->usr_chr_mapping[n], 1);
	Port_Function[p->use_parallel].write_fkt (drvthis, &p->custom_char[n][0], p->usr_chr_dot_assignment[0]);// overwrite selected Character
}



/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized serialVFD->width*serialVFD->height
//
MODULE_EXPORT void
serialVFD_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int i, j, last_chr = -10;
	char custom_char_changed[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


	for (i = 0; i < p->customchars; i++) {
		for (j = 0; j < p->usr_chr_dot_assignment[0]; j++) {
			if (p->custom_char[i][j] != p->custom_char_store[i][j]) {
				custom_char_changed[i]=1;
//				report (RPT_ERR, "%s: Char: %X   %i\n", __FUNCTION__, i, j, strerror (errno));
				}
			p->custom_char_store[i][j]=p->custom_char[i][j];
		}
	}

	if (p->refresh_timer > 500) { // Do a full refresh every 500 refreshs.
	// With this it is possible to switch display on and off while lcdproc is running
		Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[init_cmds][1],p->hw_cmd[init_cmds][0]);

		Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[p->hw_brightness][1],\
		p->hw_cmd[p->hw_brightness][0]); // restore brightness

		memset(p->backingstore, 0, p->width * p->height); // clear Backing-store

		for(i=0;i<p->customchars;i++) // refresh all customcharacters
			custom_char_changed[i]=1;
		p->refresh_timer = 0;
	}

	p->refresh_timer++;

	if (p->display_type != 1) { //not KD Rev 2.1
		for(i=0;i<p->customchars;i++) // set customcharacters
			if(custom_char_changed[i])
				serialVFD_put_char (drvthis, i);
		}

	if(custom_char_changed[p->last_custom])
		p->last_custom=-10;

	for (i = 0; i < (p->height * p->width); i++) {

		/* Backing-store implementation.  If it's already
		 * on the screen, don't put it there again
		 */

		if(p->framebuf[i] != p->backingstore[i] || (p->framebuf[i] <=30 && custom_char_changed[(int)p->framebuf[i]])) {
			if (last_chr < i-1){ // if not last char written cursor has to be moved.
				if(last_chr < i-2-p->hw_cmd[mv_cursor][0]) {
					Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[mv_cursor][1],\
						p->hw_cmd[mv_cursor][0]);
					Port_Function[p->use_parallel].write_fkt (drvthis, (char*)&i, 1);
					}
				else {
					for (j = last_chr; j < (i-1); j++)
						Port_Function[p->use_parallel].write_fkt (drvthis, &p->hw_cmd[hor_tab][1], p->hw_cmd[hor_tab][0]);
				}
			}

			if(p->framebuf[i] <= 30) { // custom character
				if (p->display_type == 1) { // KD Rev 2.1 only
					if (p->last_custom != p->framebuf[i]){
						Port_Function[p->use_parallel].write_fkt (drvthis, "\x1A\xDB", 2);		// substitute and select character to overwrite (237)
						Port_Function[p->use_parallel].write_fkt (drvthis, &p->custom_char[(int)p->framebuf[i]][0], 7);// overwrite selected character
						}
					Port_Function[p->use_parallel].write_fkt (drvthis, "\xDB", 1);			// write character
					p->last_custom = p->framebuf[i];
				}
				else {	// all other displays
					Port_Function[p->use_parallel].write_fkt (drvthis, (char*)&p->usr_chr_mapping[(int)p->framebuf[i]], 1);
				}
			}
			else if(p->framebuf[i] > 127 && (p->ISO_8859_1 != 0)) { // ISO_8859_1 translation for 129 ... 255
				Port_Function[p->use_parallel].write_fkt (drvthis, &p->charmap[p->framebuf[i] - 128], 1);
			}
			else {
				Port_Function[p->use_parallel].write_fkt (drvthis, &p->framebuf[i], 1);
			}

			last_chr = i;
		}
	}

	if (last_chr != -10) // update backingstore if something changed
		memcpy(p->backingstore, p->framebuf, p->height * p->width);
}


MODULE_EXPORT void
serialVFD_num( Driver * drvthis, int x, int num )
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if (p->ccmode != CCMODE_BIGNUM) { // Are the customcharacters set up correctly? If not:
		do_init = 1;	// Lib_adv_bignum has to set the customcharacters.
		p->ccmode = CCMODE_BIGNUM; // Switch customcharactermode to bignum.
	}
	// Lib_adv_bignum does everything needed to show the bignumbers.
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}




/*
 * Places an icon on screen
 */
MODULE_EXPORT int
serialVFD_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
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

	// Yes we know, this is a VERY BAD implementation :-)
	switch (icon) {
		case ICON_BLOCK_FILLED:
			serialVFD_chr(drvthis, x, y, 127);
			break;
		case ICON_HEART_FILLED:
			if (p->customchars > 0){
		        	p->ccmode = CCMODE_STANDARD;
				serialVFD_set_char(drvthis, 0, heart_filled);
				serialVFD_chr(drvthis, x, y, 0);
			}
			else
				serialVFD_icon(drvthis, x, y, ICON_BLOCK_FILLED);
			break;
		case ICON_HEART_OPEN:
			if (p->customchars > 0){
				p->ccmode = CCMODE_STANDARD;
				serialVFD_set_char(drvthis, 0, heart_open);
				serialVFD_chr(drvthis, x, y, 0);
			}
			else
				serialVFD_chr(drvthis, x, y, 0x23);
			break;
		default:
			return -1; // Let the core do other icons
	}
	return 0;
}



/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before serialVFD->vbar()
//
static void
serialVFD_init_vbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != CCMODE_VBAR) {
		unsigned char vBar[p->cellheight];
		int i;

		p->ccmode = CCMODE_VBAR;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 1; i < p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i] = 0xFF;
			serialVFD_set_char(drvthis, i, vBar);
		}
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
serialVFD_init_hbar (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != CCMODE_HBAR) {
		unsigned char hBar[p->cellheight];
		int i;

		p->ccmode = CCMODE_HBAR;

		for (i = 1; i < p->cellwidth; i++) {
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			serialVFD_set_char(drvthis, i, hBar);
		}
	}
}





/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
serialVFD_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);

}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
serialVFD_string (Driver *drvthis, int x, int y, char string[])
{
	PrivateData *p = drvthis->private_data;
	int i;

	x--;
	y--;
	for (i = 0; string[i] != '\0'; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y * p->width) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
serialVFD_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p != NULL) {
		Port_Function[p->use_parallel].close_fkt (drvthis);
		if (p->framebuf)
			free(p->framebuf);
		if (p->backingstore)
			free(p->backingstore);
		free(p);
	}

	drvthis->store_private_ptr(drvthis, NULL);
}

/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
serialVFD_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
serialVFD_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}

/////////////////////////////////////////////////////////////////
// Returns the maximum number of custom char slots (not how many
// are free at a moment, maybe this isn't even needed...
//
MODULE_EXPORT int
serialVFD_get_free_chars (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->customchars;
}
/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
serialVFD_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
serialVFD_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
serialVFD_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;
	if ((x > p->width) || (y > p->height))
		return;
	y--;
	x--;

	p->framebuf[(y * p->width) + x ] = c;
}


/////////////////////////////////////////////////////////////////
// provides some info about this driver
//
MODULE_EXPORT const char *
serialVFD_get_info (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	strcpy(p->info, "Driver for many serialVFDs from NEC(all FIPC based), Noritake, Futaba and the \"KD Rev2.1\"VFD.");
	return p->info;
}
