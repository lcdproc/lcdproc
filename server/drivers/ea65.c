/** \file server/drivers/ea65.c
 * LCDd \c ea65 driver for the VFD used in the Aopen XC Cube-AV EA65 media barebone.
 *
 * \todo  Implement API functions to change brightness on-the-fly
 */

/*  This is the LCDproc driver for the vfd on the Aopen EA65, based on
    the Crystal Fontz driver.

    Copyright (C) 2004 ????

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

/*Initial modification of CFontz driver by Kent Williams (c) 2005*/
/*some additions and updates by Karsten Festag (c) 2007*/

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
#include "ea65.h"
#include "report.h"
#include "lcd_lib.h"


/** private data for the \c ea65 driver */
typedef struct EA65_private_data {
        int fd;
        int brightness;
        int offbrightness;
        int width;
        int height;
        unsigned char *framebuf;
} PrivateData;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "EA65_";

/**
 * Initialize the driver.
 * Opens com port and sets baud correctly.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
EA65_init (Driver *drvthis)
{
        struct termios portset;
        /* device is fixed */
        char device[] = "/dev/ttyS1";
        /*speed is fixed at 9600*/
        int speed = B9600;

        /* Allocate and store private data */
        PrivateData *p;

        debug(RPT_INFO, "EA65: init(%p)", drvthis);

        p = (PrivateData *) malloc(sizeof(PrivateData));
        if (p == NULL)
                return -1;
        if (drvthis->store_private_ptr(drvthis, p))
                return -1;

        //// initialize private data
        // Width and Height are fixed
        p->width = 9;
        p->height = 1;
        // Make sure the frame buffer is there...
        p->framebuf = (unsigned char *) malloc (p->width * p->height);
        memset (p->framebuf, ' ', p->width * p->height);

        //// Read config file
        // Which backlight brightness
        p->brightness = drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS);
        if (0 <= p->brightness && p->brightness <= 1000) {
                if (p->brightness < 300) {
                        p->brightness = 0x00;           // command char that turns button LEDs off
                } else if (p->brightness >= 700 ) {
                        p->brightness = 0x01;           // command char that turns button LEDs on full
                } else {
                        p->brightness = 0x02;           // command char that turns button LEDs on half
                }
        } else {
                report (RPT_WARNING, "%s: Brightness must be between 0 and 1000. Using default %d",
				drvthis->name, DEFAULT_BRIGHTNESS);
                p->brightness = DEFAULT_BRIGHTNESS;
        }

        // Which backlight-off "brightness"
        p->offbrightness = drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS);
        if (0 <= p->offbrightness && p->offbrightness <= 1000) {
                if (p->offbrightness < 300) {
                        p->offbrightness = 0x00;           // command char that turns button LEDs off
                } else if (p->offbrightness >= 700) {
                        p->offbrightness = 0x01;           // command char that turns button LEDs on full
                } else {
                        p->offbrightness = 0x02;           // command char that turns button LEDs on half
                }
        } else {
                report (RPT_WARNING, "%s: OffBrightness must be between 0 and 1000. Using default %d",
				drvthis->name, DEFAULT_OFFBRIGHTNESS);
                p->offbrightness = DEFAULT_OFFBRIGHTNESS;
        }

        // Set up io port correctly, and open it...
        debug( RPT_DEBUG, "EA65: Opening serial device: %s", device);
        p->fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
        if (p->fd == -1) {
                report (RPT_ERR, "EA65_init: failed (%s)", strerror (errno));
                return -1;
        }
        tcgetattr (p->fd, &portset);

#ifdef HAVE_CFMAKERAW
        // The easy way
        cfmakeraw( &portset );
#else
        // The hard way
        portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON );
        portset.c_oflag &= ~OPOST;
        portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
        portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
        portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

        // Set port speed
        cfsetospeed (&portset, speed);
        cfsetispeed (&portset, B0);

        // Do it...
        tcsetattr (p->fd, TCSANOW, &portset);

        report (RPT_DEBUG, "EA65_init: done");

        return 0;
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
EA65_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

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
EA65_width (Driver *drvthis)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        return p->width;
}

/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
EA65_height (Driver *drvthis)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        return p->height;
}

/**
 * Flushes all output to the lcd.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
EA65_flush (Driver *drvthis)
{
        char out[6];
        int i;
        PrivateData *p = (PrivateData *) drvthis->private_data;

        // Need to check for and deal with unsupported chars or the display gets a little upset
        for (i=0; i < p->width * p->height; i++) {
                if ( (p->framebuf[i] >= 224 && p->framebuf[i] <= 253) ||     // is a lowercase accented letter
                        (p->framebuf[i] >= 97 && p->framebuf[i] <= 122) ) {   // is a lowercase letter
                        p->framebuf[i] -= 32;              // -> convert to upper case
                }
                if (p->framebuf[i] >= 48 && p->framebuf[i] <= 57) { }           // is a number -> do nothing
                else if (p->framebuf[i] >= 65 && p->framebuf[i] <= 90) { }    // is a uppercase letter -> do nothing
                else if (p->framebuf[i] == 42 || p->framebuf[i] == 43 ||
                         p->framebuf[i] == 45 || p->framebuf[i] == 47 ) { }   // is +,-,/,* -> do nothing
                  // Now lets replace some accented characters with remotely similar looking ones
                else if (p->framebuf[i] == 223) p->framebuf[i] = 83; // use an "S" instead of "ß"
                else if (p->framebuf[i] >= 192 && p->framebuf[i] <= 197)   // use an "A"
                        p->framebuf[i] = 65;
                else if (p->framebuf[i] >= 200 && p->framebuf[i] <= 203)   // use an "E"
                        p->framebuf[i] = 69;
                else if (p->framebuf[i] >= 204 && p->framebuf[i] <= 207)   // use an "I"
                        p->framebuf[i] = 73;
                else if (p->framebuf[i] >= 210 && p->framebuf[i] <= 214)   // use an "O"
                        p->framebuf[i] = 79;
                else if (p->framebuf[i] >= 217 && p->framebuf[i] <= 220)   // use an "U"
                        p->framebuf[i] = 85;
                else p->framebuf[i] = 32;  // other characters replaced by a space
        }
        snprintf(out, sizeof(out), "%c%c%c%c%c", 0xa0, 0x00, 0x80, 0x8a, 0x8a);
        write(p->fd, out, 5);
        write(p->fd, p->framebuf, p->width * p->height);
}

/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (9, 1).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 *
 * \todo  Boundary checks necessary
 */
MODULE_EXPORT void
EA65_chr (Driver *drvthis, int x, int y, char c)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        y--;
        x--;

        //if (c < 32 && c >= 0)
        //      c += 128;

        // For V2 of the firmware to get the block to display right
        //if (newfirmware && c==-1)
        //      c = 214;

        p->framebuf[(y * p->width) + x] = c;
}

/**
 * Sets the backlight on or off.
 * Uses the \c brightness / \c offbrightness values from \c LCDd.conf
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
EA65_backlight (Driver *drvthis, int on)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        char out[6];
        if (on) {
                snprintf (out, sizeof(out), "%c%c%c%c%c", 0xa0, 0x00, 0x50, 0x81, p->brightness);
        } else {
                snprintf (out, sizeof(out), "%c%c%c%c%c", 0xa0, 0x00, 0x50, 0x81, p->offbrightness);
        }
        write (p->fd, out, 5);
}

/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
EA65_clear (Driver *drvthis)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        memset (p->framebuf, ' ', p->width * p->height);

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
EA65_string (Driver *drvthis, int x, int y, const char string[])
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        int i;

        // Convert 1-based coords to 0-based...
        x -= 1;
        y -= 1;

        for (i = 0; string[i]; i++) {

                // Check for buffer overflows...
                if ((y * p->width) + x + i > (p->width * p->height))
                        break;
                p->framebuf[(y * p->width) + x + i] = string[i];
        }
}

/**
 * Turns the recording LED on or off as desired.
 * \param drvthis  Pointer to driver structure.
 * \param on       0 = LED off, 1 = LED on.
 */
MODULE_EXPORT void
EA65_output (Driver *drvthis, int on)
{
        PrivateData *p = (PrivateData *) drvthis->private_data;

        char out[6];
        if (on) {
                snprintf (out, sizeof(out), "%c%c%c%c%c", 0xa0,0x00,0x32,0x81,0x01);
        } else {
                snprintf (out, sizeof(out), "%c%c%c%c%c", 0xa0,0x00,0x32,0x81,0x00);
        }
        write (p->fd, out, 5);
}
