/** \file server/drivers/MD8800.c
 * LCDd \c MD8800 driver for the CFD used in Medion MD8800 PCs.
 */

/*
    Copyright (C) 2006 Stefan Herdler in collaboration with Martin Møller.

    This source Code is based on the NoritakeVFD, the serialVFD and the
    CFontzPacket Driver of this package.

    2006-05-07 Version 0.1: mostly everything should work

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


    ==============================================================================

    Known hardware-commands of the MD8800 display
    discovered by Martin Møller (listed in octal notation):

    \33\0abcdef (abcdef can be anything, it seems) - set clock, I think.
	   	The mapping is: bb:aa dd.cc.eeff
	   	Date values are entered in hex, e.g. :
    		0x18 0x08 0x05 0x04 0x20 0x06 for 8:18 04.05.2006

    \33\1 - 24 hour clock
    \33\2 - 12 hour clock (AM/PM)
    \33\3 - Stop clock from moving around
    \33\4 - Moving clock
    \33\5 - Show clock

    \33\40 - 2-line mode, clearing at end of last line.
    \33\41 - Only write to line1, no clearing,
             automatic scroll to the left when display is full.
    \33\42 - Only write to line2, no clearing,
             automatic scroll to the left when display is full.

      (d is in the range 0-4. Anything above 4 is full brightness, b = 0/1):
    ==============================================================================
    \33\60\00\d - brightness of 'hdd'   icon. \33\60\00\0 clears the 'hdd'   icon.
    \33\60\01\d - brightness of '1394'  icon. \33\60\01\0 clears the '1394'  icon.
    \33\60\02\d - brightness of 'cdrom' icon. \33\60\02\0 clears the 'cdrom' icon.
    \33\60\03\d - brightness of 'usb'   icon. \33\60\03\0 clears the 'usb'   icon.
    \33\60\04\d - brightness of 'movie' icon. \33\60\04\0 clears the 'movie' icon.
    \33\60\05\d - brightness of 'tv'    icon. \33\60\05\0 clears the 'tv'    icon.
    \33\60\06\d - brightness of 'music' icon. \33\60\06\0 clears the 'music' icon.
    \33\60\07\d - brightness of 'photo' icon. \33\60\07\0 clears the 'photo' icon.
    \33\60\10\d - recording light on/off.
    \33\60\11\b - email icon on/off.
    \33\60\12\b - email icon red on/off.
    \33\60\13\b - volume bar (least) on/off.
    \33\60\14\b - volume bar on/off.
    \33\60\15\b - volume bar on/off.
    \33\60\16\b - volume bar on/off.
    \33\60\17\b - volume bar on/off.
    \33\60\20\b - volume bar on/off.
    \33\60\21\b - volume bar (most) on/off.
    \33\60\22\b - red light beneath volume bars on/off (ie max volume)
    \33\60\23\b - speaker icon on/off
    \33\60\24\b - muted speaker icon on/off (they are adjacent, not overlapping)
    \33\60\25\b - red wifi/sound bar (least) on/off.
    \33\60\26\b - red wifi/sound bar on/off.
    \33\60\27\b - red wifi/sound bar (most) on/off.
    \33\60\30\b - (de)activate bounding box around first 4 icons (hdd,1394,cd,usb)
    \33\60\31\b - (de)activate bounding box around second 4 icons (movie,tv,music,photos)
    \33\60\32\b - (de)activate bounding box around record and 'play/pause/whatever' icon.
    \33\60\33\b - (de)activate bounding box around email icon.
    \33\60\34\b - (de)activate bounding box around volume icons.

    \33\62x - Don't know yet.
    \33\63x - Don't know yet.
    \33\61abcdefghi - Show graphics in 'multimedia' icon based on bitmap represented by 'abcdefghi'.
                      Graphics are 9 7-bit bitmaps. Values are given from the right-up, moving towards the left.
                      See examples in MD8800.c.

    \33\100\d - Brightness. d=0-5, i.e. 6 levels, from black to 'shiny'.  :)
    \33\120   - reset again (soft?). Acts as a 'clear' command. Doesn't unset some things.
    \33\121   - pos1.
    \33\122   - turn on display.
    \33\123   - turn off display (you can still write to it and '\33\122' will show the result.
    \33\124   - demo condition. The display is 'raining' and the icons are dancing  :) . \33\120 cannot reset it. \37 can.
    \33\360   - vertical bars in each character slot.
    \33\361   - horizontal bars in each character slot.
    \33\365   - Product and version! : SG-1788 ver 1.0

    \37 - Reset display (yay!)




    Commands availible to the client via output command:

    Commandnumber off+0 on+50 (50 that should be enough space for unknown commands ;-))
    |(decimal notation)
    |
    v      (d is in the range 0-4. Anything above 4 is full brightness, b = 0/1):
    ==============================================================================
    1	(de)activate 'hdd'   icon.
    2	(de)activate '1394'  icon.
    3	(de)activate 'cdrom' icon.
    4	(de)activate 'usb'   icon.
    5	(de)activate 'movie' icon.
    6	(de)activate 'tv'    icon.
    7	(de)activate 'music' icon.
    8	(de)activate 'photo' icon.
    9	recording light on/off.
    10 	(de)activate bounding box around first 4 icons (hdd,1394,cd,usb)
    11 	(de)activate bounding box around second 4 icons (movie,tv,music,photos)
    12 	(de)activate bounding box around record and 'play/pause/whatever' icon.
    13 	(de)activate bounding box around email icon.
    14 	(de)activate bounding box around volume icons.
    15	(de)activate all bounding boxes

    Commandnumber(decimal notation):
    |
    v
    110 off
    111 email icon on
    112 email icon and red icon on

    120 off
    121 volume bar least on
    122 volume bar least 2 on
    123 volume bar least 3 on
    124 volume bar least 4 on
    125 volume bar least 5 on
    126 volume bar least 6 on
    127 volume bar all 7 on
    128 volume bar all 7 and red light beneath  on
	That should make it easy for the client:
	Just calculate a volumelevel between 0 and 8 then add 120.

    130 off
    131 muted speaker icon on
    132 speaker icon on

    140 off
    141 red wifi/sound bar least on
    142 red wifi/sound bar least 2 on
    143 red wifi/sound bar all 3 on
    144 red wifi/sound bar "scanning"
	See volume bar.

	Predefined 'multimedia' icons:
    150 clear mm-icon
    151 play
    152 stop
    153 pause
    154 ffwd
    155 rwnd
    156 heart
    157 heart outline



    The display is running at:
    9600 baud 8n*2* no flow control

*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "MD8800.h"

#include "report.h"

/* Constants for userdefchar_mode */
#define NUM_CCs		0 /* max. number of custom characters */
#define DEFAULT_OFF_BRIGHTNESS	300
#define DEFAULT_ON_BRIGHTNESS	1000

/** private data for the \c MD8800 driver */
typedef struct MD8800_private_data {
	char device[200];
	int fd;
	int speed;
	/* dimensions */
	int width, height;
	int cellwidth, cellheight;
	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;
	/* definable characters */
	int on_brightness;
	int off_brightness;
	int hw_brightness;
	int last_command;
	int wifi_scan;
	char info[255];
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "MD8800_";



// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
MD8800_init (Driver *drvthis)
{
	struct termios portset;
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
	p->fd = -1;
	p->cellwidth = DEFAULT_CELL_WIDTH;
	p->cellheight = DEFAULT_CELL_HEIGHT;
        p->wifi_scan = 1;
	p->hw_brightness = 6;

	debug(RPT_INFO, "%s(%p)", __FUNCTION__, drvthis);

	/* Read config file */
	/* Which device should be used */
	strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(p->device));
	p->device[sizeof(p->device)-1] = '\0';
	report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

	/* Which size */
	strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
	size[sizeof(size)-1] = '\0';
	if ((sscanf(size, "%dx%d", &w, &h) != 2)
	    || (w <= 0) || (w > LCD_MAX_WIDTH)
	    || (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report(RPT_WARNING, "%s: cannot parse Size: %s; using default %s",
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


	/* Which speed */
	tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);
	if ((tmp != 1200) && (tmp != 2400) && (tmp != 9600) && (tmp != 19200) && (tmp != 115200)) {
		report(RPT_WARNING, "%s: Speed must be 1200, 2400, 9600, 19200 or 115200; using default %d",
			drvthis->name, DEFAULT_SPEED);
		tmp = DEFAULT_SPEED;
	}
	if (tmp == 1200) p->speed = B1200;
	else if (tmp == 2400) p->speed = B2400;
	else if (tmp == 9600) p->speed = B9600;
	else if (tmp == 19200) p->speed = B19200;
	else if (tmp == 115200) p->speed = B115200;




	/* Set up io port correctly, and open it...*/
	debug(RPT_DEBUG, "%s: Opening device: %s", __FUNCTION__, p->device);
	p->fd = open (p->device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (p->fd == -1) {
		report(RPT_ERR, "%s: open() of %s failed (%s)", drvthis->name, p->device, strerror(errno));
		return -1;
	}

	tcgetattr(p->fd, &portset);

	// We use RAW mode
#ifdef HAVE_CFMAKERAW
	// The easy way
	cfmakeraw(&portset);
#else
	// The hard way
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
   	                   | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL;
#endif
	portset.c_cflag |= CSTOPB;
	// Set port speed
	cfsetospeed(&portset, p->speed);
	cfsetispeed(&portset, B0);

	// Do it...
	tcsetattr(p->fd, TCSANOW, &portset);

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

	MD8800_backlight(drvthis, 1);

	//init
	write(p->fd, "\x1F", 1);
	write(p->fd, "\x1B", 1);
	write(p->fd, "\x20", 1);

	//boot screen
	write(p->fd, "\x1B\x51", 2);// medion \33\121 oct
	write(p->fd, "MD8800driver forlcdproc  (C) by:", 32);
	sleep(1);
	write(p->fd, " Stefan Herdler & Martin M\xF6ller ", 32);
	sleep(1);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;

}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
MD8800_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);

		if (p->backingstore)
			free(p->backingstore);
		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
//        MD8800_set_brightness (drvthis, 0, p->off_brightness);
}

/*
 * Returns the display width in characters
 */
MODULE_EXPORT int
MD8800_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/*
 * Returns the display height in characters
 */
MODULE_EXPORT int
MD8800_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/*
 * Returns the width of a character in pixels
 */
MODULE_EXPORT int
MD8800_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/*
 * Returns the height of a character in pixels
 */
MODULE_EXPORT int
MD8800_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}

/*
 * Retrieves brightness (in promille)
 */
MODULE_EXPORT int
MD8800_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->on_brightness : p->off_brightness;
}


/*
 * Sets on/off brightness (in promille)
 */
MODULE_EXPORT void
MD8800_set_brightness(Driver *drvthis, int state, int promille)
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
	 //MD8800_backlight(drvthis,state);
}


/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 */
MODULE_EXPORT void
MD8800_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int hardware_value = (on == BACKLIGHT_ON)
			     ? p->on_brightness
			     : p->off_brightness;

	/* map range [0, 1000] -> [0, 6] that the hardware understands */
	hardware_value /= 167;
	if (hardware_value != p->hw_brightness) {
		p->hw_brightness = hardware_value;
		write(p->fd, "\x1B\x40", 2);
		write(p->fd, &p->hw_brightness, 1);
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
MD8800_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	y--;
	x--;
	if ((x > p->width) || (y > p->height))
		return;

	p->framebuf[(y * p->width) + x ] = c;
}




/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized MD8800->width*MD8800->height
//



MODULE_EXPORT void
MD8800_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (memcmp(p->backingstore, p->framebuf, p->width*p->height) == 0)
		return;

	/* Backing-store implementation.  If it's already
	 * on the screen, don't put it there again
	 */

       	/* else, write out the entire row */
	memcpy(p->backingstore, p->framebuf, p->width*p->height);

	write(p->fd, "\x1B\x51", 2);// medion \33\121 oct
//	usleep(300);
//	write(p->fd, "\x51", 1);// medion \33\121 oct
//	usleep(300);

//	write(p->fd, "\x0D", 1); // nec
	write(p->fd, p->framebuf, p->width*p->height);


}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
MD8800_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);
}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
MD8800_string (Driver *drvthis, int x, int y, const char string[])
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

MODULE_EXPORT int
MD8800_icon (Driver *drvthis, int x, int y, int icon)
{
	switch (icon) {
		case ICON_BLOCK_FILLED:
			MD8800_chr(drvthis, x, y, 127);
			break;
		default:
			return -1; // Let the core do other icons
	}
	return 0;
}

////////////////////////////////////////////////////////////////////
// Controls the custom icons.
//
//
MODULE_EXPORT void
MD8800_output (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (on == 144) {
		switch (p->wifi_scan){
			case 1:
				write(p->fd, "\x1B\x30\x15\x01", 4);
			//	write(p->fd, "\x1B\x30\x16\x00", 4);
				write(p->fd, "\x1B\x30\x17\x00", 4);
			break;
			case 4:
				write(p->fd, "\x1B\x30\x15\x00", 4);
				write(p->fd, "\x1B\x30\x16\x01", 4);
			//	write(p->fd, "\x1B\x30\x17\x00", 4);
			break;
			case 7:
			//	write(p->fd, "\x1B\x30\x15\x00", 4);
				write(p->fd, "\x1B\x30\x16\x00", 4);
				write(p->fd, "\x1B\x30\x17\x01", 4);
			break;
			case 10:
				p->wifi_scan=0;
			break;
			}
		p->wifi_scan++;
	}

	if (on == p->last_command)
		return;
	p->last_command = on;

	report(RPT_ERR, "commmand send is: %i", on);
	switch (on) {
	// +0 -> off
		case 1: // 'hdd'
			write(p->fd, "\x1B\x30\x00\x00", 4);
		break;
		case 2: // '1394'
			write(p->fd, "\x1B\x30\x01\x00", 4);
		break;
		case 3: // 'cdrom'
			write(p->fd, "\x1B\x30\x02\x00", 4);
		break;
		case 4: // 'usb'
			write(p->fd, "\x1B\x30\x03\x00", 4);
		break;
		case 5: // 'movie'
			write(p->fd, "\x1B\x30\x04\x00", 4);
		break;
		case 6: // 'tv'
			write(p->fd, "\x1B\x30\x05\x00", 4);
		break;
		case 7: // 'music'
			write(p->fd, "\x1B\x30\x06\x00", 4);
		break;
		case 8: // 'photo'
			write(p->fd, "\x1B\x30\x07\x00", 4);
		break;
		case 9: // 'recording light'
			write(p->fd, "\x1B\x30\x08\x00", 4);
		break;
		case 10: // bounding box (hdd,1394,cd,usb)
			write(p->fd, "\x1B\x30\x18\x00", 4);
		break;
		case 11: // bounding box (movie,tv,music,photos)
			write(p->fd, "\x1B\x30\x19\x00", 4);
		break;
		case 12: // bounding box (play/pause/whatever)
			write(p->fd, "\x1B\x30\x1A\x00", 4);
		break;
		case 13: // bounding box (email)
			write(p->fd, "\x1B\x30\x1B\x00", 4);
		break;
		case 14: // bounding box (volume)
			write(p->fd, "\x1B\x30\x1C\x00", 4);
		break;
		case 15: // all bounding boxes
			write(p->fd, "\x1B\x30\x18\x00", 4);
			write(p->fd, "\x1B\x30\x19\x00", 4);
			write(p->fd, "\x1B\x30\x1A\x00", 4);
			write(p->fd, "\x1B\x30\x1B\x00", 4);
			write(p->fd, "\x1B\x30\x1C\x00", 4);
		break;

	// +50 -> on
		case 51: // 'hdd'
			write(p->fd, "\x1B\x30\x00", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 52: // '1394'
			write(p->fd, "\x1B\x30\x01", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 53: // 'cdrom'
			write(p->fd, "\x1B\x30\x02", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 54: // 'usb'
			write(p->fd, "\x1B\x30\x03", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 55: // 'movie'
			write(p->fd, "\x1B\x30\x04", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 56: // 'tv'
			write(p->fd, "\x1B\x30\x05", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 57: // 'music'
			write(p->fd, "\x1B\x30\x06", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 58: // 'photo'
			write(p->fd, "\x1B\x30\x07", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 59: // 'recording light'
			write(p->fd, "\x1B\x30\x08", 3);
			write(p->fd, &p->hw_brightness, 1);
		break;
		case 60: // bounding box (hdd,1394,cd,usb)
			write(p->fd, "\x1B\x30\x18\x01", 4);
		break;
		case 61: // bounding box (movie,tv,music,photos)
			write(p->fd, "\x1B\x30\x19\x01", 4);
		break;
		case 62: // bounding box (play/pause/whatever)
			write(p->fd, "\x1B\x30\x1A\x01", 4);
		break;
		case 63: // bounding box (email)
			write(p->fd, "\x1B\x30\x1B\x01", 4);
		break;
		case 64: // bounding box (volume)
			write(p->fd, "\x1B\x30\x1C\x01", 4);
		break;
		case 65: // all bounding boxes
			write(p->fd, "\x1B\x30\x18\x01", 4);
			write(p->fd, "\x1B\x30\x19\x01", 4);
			write(p->fd, "\x1B\x30\x1A\x01", 4);
			write(p->fd, "\x1B\x30\x1B\x01", 4);
			write(p->fd, "\x1B\x30\x1C\x01", 4);
		break;
			//email icons
		case 100:
			write(p->fd, "\x1B\x30\x09\x00", 4);
			write(p->fd, "\x1B\x30\x0A\x00", 4);
		break;
		case 101:

			write(p->fd, "\x1B\x30\x09\x01", 4);
			write(p->fd, "\x1B\x30\x0A\x00", 4);
		break;
		case 102:

			write(p->fd, "\x1B\x30\x09\x01", 4);
			write(p->fd, "\x1B\x30\x0A\x01", 4);
		break;
			// volume bar
		case 120:
			write(p->fd, "\x1B\x30\x0B\x00", 4);
			write(p->fd, "\x1B\x30\x0C\x00", 4);
			write(p->fd, "\x1B\x30\x0D\x00", 4);
			write(p->fd, "\x1B\x30\x0E\x00", 4);
			write(p->fd, "\x1B\x30\x0F\x00", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 121:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x00", 4);
			write(p->fd, "\x1B\x30\x0D\x00", 4);
			write(p->fd, "\x1B\x30\x0E\x00", 4);
			write(p->fd, "\x1B\x30\x0F\x00", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 122:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x00", 4);
			write(p->fd, "\x1B\x30\x0E\x00", 4);
			write(p->fd, "\x1B\x30\x0F\x00", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 123:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x00", 4);
			write(p->fd, "\x1B\x30\x0F\x00", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 124:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x01", 4);
			write(p->fd, "\x1B\x30\x0F\x00", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 125:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x01", 4);
			write(p->fd, "\x1B\x30\x0F\x01", 4);
			write(p->fd, "\x1B\x30\x10\x00", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 126:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x01", 4);
			write(p->fd, "\x1B\x30\x0F\x01", 4);
			write(p->fd, "\x1B\x30\x10\x01", 4);
			write(p->fd, "\x1B\x30\x11\x00", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 127:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x01", 4);
			write(p->fd, "\x1B\x30\x0F\x01", 4);
			write(p->fd, "\x1B\x30\x10\x01", 4);
			write(p->fd, "\x1B\x30\x11\x01", 4);
			write(p->fd, "\x1B\x30\x12\x00", 4);
		break;
		case 128:
			write(p->fd, "\x1B\x30\x0B\x01", 4);
			write(p->fd, "\x1B\x30\x0C\x01", 4);
			write(p->fd, "\x1B\x30\x0D\x01", 4);
			write(p->fd, "\x1B\x30\x0E\x01", 4);
			write(p->fd, "\x1B\x30\x0F\x01", 4);
			write(p->fd, "\x1B\x30\x10\x01", 4);
			write(p->fd, "\x1B\x30\x11\x01", 4);
			write(p->fd, "\x1B\x30\x12\x01", 4);
		break;
		case 130:
			write(p->fd, "\x1B\x30\x13\x00", 4);
			write(p->fd, "\x1B\x30\x14\x00", 4);
		break;
		case 131:
			write(p->fd, "\x1B\x30\x13\x00", 4);
			write(p->fd, "\x1B\x30\x14\x01", 4);
		break;
		case 132:
			write(p->fd, "\x1B\x30\x13\x01", 4);
			write(p->fd, "\x1B\x30\x14\x00", 4);
		break;


			// red wifi/sound bar
		case 140:
			write(p->fd, "\x1B\x30\x15\x00", 4);
			write(p->fd, "\x1B\x30\x16\x00", 4);
			write(p->fd, "\x1B\x30\x17\x00", 4);
		break;
		case 141:
			write(p->fd, "\x1B\x30\x15\x01", 4);
			write(p->fd, "\x1B\x30\x16\x00", 4);
			write(p->fd, "\x1B\x30\x17\x00", 4);
		break;
		case 142:
			write(p->fd, "\x1B\x30\x15\x01", 4);
			write(p->fd, "\x1B\x30\x16\x01", 4);
			write(p->fd, "\x1B\x30\x17\x00", 4);
		break;
		case 143:
			write(p->fd, "\x1B\x30\x15\x01", 4);
			write(p->fd, "\x1B\x30\x16\x01", 4);
			write(p->fd, "\x1B\x30\x17\x01", 4);
		break;
			// multimedia icon
		case 150: // clear
			write(p->fd, "\x1B\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00", 11);
		break;
		case 151: // play
			write(p->fd, "\x1B\x31\x00\x00\x08\x1C\x3E\x7F\x00\x00\x00", 11);
		break;
		case 152: // stop
			write(p->fd, "\x1B\x31\x00\x3E\x3E\x3E\x3E\x3E\x00\x00\x00", 11);
		break;
		case 153: // pause
			write(p->fd, "\x1B\x31\x00\x3E\x3E\x00\x3E\x3E\x00\x00\x00", 11);
		break;
		case 154: // ffwd
			write(p->fd, "\x1B\x31\x00\x08\x1C\x3E\x08\x1C\x3E\x00\x00", 11);
		break;
		case 155: // rwnd
			write(p->fd, "\x1B\x31\x00\x3E\x1C\x08\x3E\x1C\x08\x00\x00", 11);
		break;
		case 156: // heart
			write(p->fd, "\x1B\x31\x00\x0C\x1E\x3E\x7C\x3E\x1E\x0C\x00", 11);
		break;
		case 157: // heart outline
			write(p->fd, "\x1B\x31\x0C\x12\x21\x41\x02\x41\x21\x12\x0C", 11);
		break;

		default:
			return;
	}

}

/*
 * Provide some info about this driver
 */
MODULE_EXPORT const char *
MD8800_get_info(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	snprintf(p->info, sizeof(p->info)-1, "Medion MD8800 Driver");
	return p->info;
}

/* EOF */
