/*  This is the LCDproc driver for CrystalFontz 633 devices
    (get yours from http://crystalfontz.com)

    Copyright (C) 2002 David GLAUDE

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


/*
 * Driver status
 * 04/04/2002: Working driver
 * BUT
 * + No support for keypad (inactive)
 * + No checking of LCD response (ACK of each message)
 * + No checking if right hardware is connected (firmware/hardware)
 * + No BigNum (but screen is too small ???)
 * + No support for multiple instance (require private structure)
 * + No cache of custom char usage (like in MtxOrb)
 *
 * THINGS DONE:
 * + Stopping the live reporting (of temperature)
 * + Stopping the reporting of temp and fan (is it necessary after reboot)
 * + Use of library for hbar and vbar (good but library could be better)
 *
 * THINGS TO DO:
 * + make the caching at least for heartbeat icon
 * +
 *
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
#include "CFontz633.h"
#include "CFontz633io.h"
#include "report.h"
#include "lcd_lib.h"

static int custom = 0;
typedef enum {
	hbar = 1,
	vbar = 2
} custom_type;

static int fd;
static char *framebuf = NULL;
static char *old = NULL;
static int width = 0;
static int height = 0;
static int cellwidth = DEFAULT_CELL_WIDTH;
static int cellheight = DEFAULT_CELL_HEIGHT;
static int contrast = DEFAULT_CONTRAST;
static int brightness = DEFAULT_BRIGHTNESS;
static int offbrightness = DEFAULT_OFFBRIGHTNESS;
static int newfirmware = 0;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "CFontz633_";


/* Internal functions */
/* static void CFontz633_linewrap (int on); */
/* static void CFontz633_autoscroll (int on);  */
static void CFontz633_hidecursor ();
static void CFontz633_reboot ();
static void CFontz633_init_vbar (Driver * drvthis);
static void CFontz633_init_hbar (Driver * drvthis);
static void CFontz633_no_live_report ();
static void CFontz633_hardware_clear (Driver * drvthis);


/*
 * Opens com port and sets baud correctly...
 */
MODULE_EXPORT int
CFontz633_init (Driver * drvthis, char *args)
{
	struct termios portset;
	int tmp, w, h;
	int reboot = 0;

	int contrast = DEFAULT_CONTRAST;
	char device[200] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[200] = DEFAULT_SIZE;

	debug(RPT_INFO, "CFontz633: init(%p,%s)", drvthis, args );

	/* Read config file */

	/* Which serial device should be used */
	strncpy(device, drvthis->config_get_string ( drvthis->name , "Device" , 0 , DEFAULT_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	debug (RPT_INFO,"CFontz633: Using device: %s", device);

	/* Which size */
	strncpy(size, drvthis->config_get_string ( drvthis->name , "Size" , 0 , DEFAULT_SIZE),sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size , "%dx%d", &w, &h ) != 2
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report (RPT_WARNING, "CFontz633_init: Cannot read size: %s. Using default value.\n", size);
		sscanf( DEFAULT_SIZE , "%dx%d", &w, &h );
	} else {
		width = w;
		height = h;
	}

	/* Which contrast */
	if (0<=drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) && drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) <= 1000) {
		contrast = drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST);
	} else {
		report (RPT_WARNING, "CFontz633_init: Contrast must between 0 and 1000. Using default value.\n");
	}

	/* Which backlight brightness */
	if (0<=drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS) && drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS) <= 100) {
		brightness = drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , DEFAULT_BRIGHTNESS);
	} else {
		report (RPT_WARNING, "CFontz633_init: Brightness must between 0 and 100. Using default value.\n");
	}

	/* Which backlight-off "brightness" */
	if (0<=drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS) && drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS) <= 100) {
		offbrightness = drvthis->config_get_int ( drvthis->name , "OffBrightness" , 0 , DEFAULT_OFFBRIGHTNESS);
	} else {
		report (RPT_WARNING, "CFontz633_init: OffBrightness must between 0 and 100. Using default value.\n");
	}


	/* Which speed */
	tmp = drvthis->config_get_int ( drvthis->name , "Speed" , 0 , DEFAULT_SPEED);
	if (tmp == 1200) speed = B1200;
	else if (tmp == 2400) speed = B2400;
	else if (tmp == 9600) speed = B9600;
	else if (tmp == 19200) speed = B19200;
	else { report (RPT_WARNING, "CFontz633_init: Speed must be 1200, 2400, 9600 or 19200. Using default value.\n", speed);
	}

	/* New firmware version? */
	if(drvthis->config_get_bool( drvthis->name , "NewFirmware" , 0 , 0)) {
		newfirmware = 1;
	}

	/* Reboot display? */
	if (drvthis->config_get_bool( drvthis->name , "Reboot" , 0 , 0)) {
		report (RPT_INFO, "LCDd: rebooting CrystalFontz LCD...\n");
		reboot = 1;
	}

	/* Set up io port correctly, and open it... */
	debug( RPT_DEBUG, "CFontz633: Opening serial device: %s", device);
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		report (RPT_ERR, "CFontz633_init: failed (%s)\n", strerror (errno));
		return -1;
	}

	tcgetattr (fd, &portset);

	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW
	/* The easy way */
	cfmakeraw( &portset );
#else
	/* The hard way */
	portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	portset.c_oflag &= ~OPOST;
	portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

	/* Set port speed */
	cfsetospeed (&portset, speed);
	cfsetispeed (&portset, B0);

	/* Do it... */
	tcsetattr (fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	framebuf = (unsigned char *) malloc (width * height);
	memset (framebuf, ' ', width * height);

	/* Set display-specific stuff.. */
	if (reboot) {
		CFontz633_reboot ();
		sleep (4);
		reboot = 0;
	}
	sleep (1);
	CFontz633_hidecursor ();

	CFontz633_set_contrast (drvthis, contrast);
	CFontz633_no_live_report ();


	report (RPT_DEBUG, "CFontz633_init: done\n");

	return 0;
}

/*
 * Clean-up
 */
MODULE_EXPORT void
CFontz633_close (Driver * drvthis)
{
	close (fd);

	if(framebuf) free (framebuf);
	framebuf = NULL;

	if(old) free (old);
	old = NULL;
}

/*
 * Returns the display width
 */
MODULE_EXPORT int
CFontz633_width (Driver *drvthis)
{
	return width;
}

/*
 * Returns the display height
 */
MODULE_EXPORT int
CFontz633_height (Driver *drvthis)
{
	return height;
}

/*
 * Flushes all output to the lcd...
 */
MODULE_EXPORT void
CFontz633_flush (Driver * drvthis)
{
	int i;
	char *xp, *xq;
/*
 * We don't use delta update yet.
 * It should be possible but since we can only update one.
 */
	if (old==NULL) {
		old = (unsigned char *) malloc (width * height);
		memset (old, ' ', width * height);
		CFontz633_hardware_clear (drvthis);
	}

xp = framebuf;
xq = old;

for (i=0; i<width; i++) {
	if (*xp != *xq) {
send_bytes_message(fd, 16, CF633_Set_LCD_Contents_Line_One, &framebuf[0]);
memcpy(old, framebuf, width);
		break;
		}
	xp++; xq++;
	}

xp = &framebuf[width];
xq = &old[width];

for (i=0; i<width; i++) {
	if (*xp != *xq) {
send_bytes_message(fd, 16, CF633_Set_LCD_Contents_Line_Two, &framebuf[width]);
memcpy(&old[width], &framebuf[width], width);
		break;
		}
	xp++; xq++;
	}


}

/*
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
CFontz633_chr (Driver * drvthis, int x, int y, char c)
{
	y--;
	x--;

	framebuf[(y * width) + x] = c;
}

/*
 * Returns current contrast
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * Value 0 to 1000.
 */
MODULE_EXPORT int
CFontz633_get_contrast (Driver * drvthis)
{
	return contrast;
}

/*
 *  Changes screen contrast (valid hardware value: 0-50)
 *  Value 0 to 1000.
 */
MODULE_EXPORT void
CFontz633_set_contrast (Driver * drvthis, int promille)
{
	int hardware_contrast;

	/* Check it */
	if( promille < 0 || promille > 1000 )
		return;

	/* Store the software value since there is not get. */
	contrast = promille;

	hardware_contrast = contrast/20;

//send_onebyte_message(fd, CF633_Set_LCD_Contrast, hardware_contrast);
}

/*
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 * Need to find out if we have support for intermediate value.
 */
MODULE_EXPORT void
CFontz633_backlight (Driver * drvthis, int on)
{
	if (on) {
//send_onebyte_message(fd, CF633_Set_LCD_And_Keypad_Backlight, brightness);
	} else {
//send_onebyte_message(fd, CF633_Set_LCD_And_Keypad_Backlight, offbrightness);
	}
}

/*
 * Get rid of the blinking curson
 */
static void
CFontz633_hidecursor ()
{
send_onebyte_message(fd, CF633_Set_LCD_Cursor_Style, 0);
}


/*
 * Stop live reporting of temperature.
 */
static void
CFontz633_no_live_report ()
{
	char out[2]= {0, 0};

	for (out[0]=0; out[0]<8; out[0]++)
	    send_bytes_message(fd, 2, CF633_Set_Up_Live_Fan_or_Temperature_Display , out);
}

/*
 * Stop the reporting of any fan.
 */
static void
CFontz633_no_fan_report ()
{
send_onebyte_message(fd, CF633_Set_Up_Fan_Reporting, 0);
}

/*
 * Stop the reporting of any temperature.
 */
static void
CFontz633_no_temp_report ()
{
char out[4]= {0, 0, 0, 0};
send_bytes_message(fd, 4, CF633_Set_Up_Temperature_Reporting, out);
}

/*
 * Reset the display bios
 */
static void
CFontz633_reboot ()
{
char out[3]= {8, 18, 99};
send_bytes_message(fd, 3, CF633_Reboot, out);
}

/*
 * Sets up for vertical bars.
 */
static void
CFontz633_init_vbar (Driver * drvthis)
{
	char a[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};

	if (custom != vbar) {
	//	printf("+++ vbar +++\n");
		CFontz633_set_char (drvthis, 1, a);
		CFontz633_set_char (drvthis, 2, b);
		CFontz633_set_char (drvthis, 3, c);
		CFontz633_set_char (drvthis, 4, d);
		CFontz633_set_char (drvthis, 5, e);
		CFontz633_set_char (drvthis, 6, f);
		CFontz633_set_char (drvthis, 7, g);
		custom = vbar;
	}
}

/*
 * Inits horizontal bars...
 */
static void
CFontz633_init_hbar (Driver * drvthis)
{

	char a[] = {
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
	};

	if (custom != hbar) {
		CFontz633_set_char (drvthis, 1, a);
		CFontz633_set_char (drvthis, 2, b);
		CFontz633_set_char (drvthis, 3, c);
		CFontz633_set_char (drvthis, 4, d);
		CFontz633_set_char (drvthis, 5, e);
		CFontz633_set_char (drvthis, 6, f);
		custom = hbar;
	}
}


/*
 * Draws a vertical bar...
 */
MODULE_EXPORT void
CFontz633_vbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'up' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */

	CFontz633_init_vbar(drvthis);

	lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 0);
}


/*
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
CFontz633_hbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
/* x and y are the start position of the bar.
 * The bar by default grows in the 'right' direction
 * (other direction not yet implemented).
 * len is the number of characters that the bar is long at 100%
 * promille is the number of promilles (0..1000) that the bar should be filled.
 */

	CFontz633_init_hbar(drvthis);

	lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 0);
}


/*
 * Writes a big number.
 * This is not supported on 633 because we only have 2 lines...
 */
MODULE_EXPORT void
CFontz633_num (Driver * drvthis, int x, int num)
{
/*
	char out[5];
	snprintf (out, sizeof(out), "%c%c%c", 28, x, num);
	write (fd, out, 3);
*/
}

/*
 * Sets a custom character from 0-7...
 *
 * For input, values > 0 mean "on" and values <= 0 are "off".
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
CFontz633_set_char (Driver * drvthis, int n, char *dat)
{
	char out[9];
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	out[0]=n;	/* Custom char to define. xxx */

	for (row = 0; row < cellheight; row++) {
		letter = 0;
		for (col = 0; col < cellwidth; col++) {
			letter <<= 1;
			letter |= (dat[(row * cellwidth) + col] > 0);
		//	if (dat[(row * cellheight) + col] == 0) printf(".");
		//	if (dat[(row * cellheight) + col] == 1) printf("+");
		//	if (dat[(row * cellheight) + col] == 2) printf("x");
		//	if (dat[(row * cellheight) + col] == 3) printf("*");
		//	printf("'%1d'", dat[(row * cellwidth) + col]);
		//	printf("%3d ", letter);
		}
		out[row+1]=letter;
	//	printf(": %d\n", letter);
	}
send_bytes_message(fd, 9, CF633_Set_LCD_Special_Character_Data , out);
}

/*
 * Places an icon on screen
 */
MODULE_EXPORT int
CFontz633_icon (Driver * drvthis, int x, int y, int icon)
{
	char icons[3][6 * 8] = {
		{
		 1, 1, 1, 1, 1, 1,		  /* Empty Heart */
		 1, 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 0, 0,
		 1, 1, 0, 0, 0, 1,
		 1, 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1, 1,		  /* Filled Heart */
		 1, 1, 0, 1, 0, 1,
		 1, 0, 1, 0, 1, 0,
		 1, 0, 1, 1, 1, 0,
		 1, 0, 1, 1, 1, 0,
		 1, 1, 0, 1, 0, 1,
		 1, 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0, 0,		  /* Ellipsis */
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1, 0,
		 },

	};

	switch( icon ) {
		case ICON_BLOCK_FILLED:
			CFontz633_chr( drvthis, x, y, 255 );
			break;
		case ICON_HEART_FILLED:
			CFontz633_set_char( drvthis, 0, icons[1] );
			CFontz633_chr( drvthis, x, y, 0 );
			break;
		case ICON_HEART_OPEN:
			CFontz633_set_char( drvthis, 0, icons[0] );
			CFontz633_chr( drvthis, x, y, 0 );
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;

}

/*
 * Clears the LCD screen
 */
MODULE_EXPORT void
CFontz633_clear (Driver * drvthis)
{
	memset (framebuf, ' ', width * height);
}

/*
 * Hardware clears the LCD screen
 */
static void
CFontz633_hardware_clear (Driver * drvthis)
{
send_zerobyte_message(fd, CF633_Clear_LCD_Screen);
}

/*
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (16,2).
 */
MODULE_EXPORT void
CFontz633_string (Driver * drvthis, int x, int y, char string[])
{
	int i;

	/* Convert 1-based coords to 0-based... */
	x -= 1;
	y -= 1;

	for (i = 0; string[i]; i++) {

		/* Check for buffer overflows... */
		if ((y * width) + x + i > (width * height))
			break;
		framebuf[(y * width) + x + i] = string[i];
	}
}
