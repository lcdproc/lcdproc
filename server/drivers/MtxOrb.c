/*  This is the LCDproc driver for Matrix Orbital devices
    (http://www.matrixorbital.com)

    For the Matrix Orbital LCD* LKD* VFD* and VKD* displays

    NOTE: GLK displays have a different driver.

    Copyright (C) 1999, William Ferrell and Scott Scriven
		  2001, André Breiler
		  2001, Philip Pokorny
		  2001, David Douthitt
		  2001, David Glaude
		  2001, Joris Robijn
		  2001, Rene Wagner

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "MtxOrb.h"

#include "report.h"
/* #include "shared/str.h"
   #include "input.h"
   Above 3 lines modified by Joris */
#define INPUT_PAUSE_KEY         'A'
#define INPUT_BACK_KEY          'B'
#define INPUT_FORWARD_KEY       'C'
#define INPUT_MAIN_MENU_KEY     'D'
#define MTXORB_DEFAULT_PAUSE_KEY         'A'
#define MTXORB_DEFAULT_BACK_KEY          'B'
#define MTXORB_DEFAULT_FORWARD_KEY       'C'
#define MTXORB_DEFAULT_MAIN_MENU_KEY     'D'
#define DEFAULT_SIZE "20x4"
#define DEFAULT_BACKLIGHT 1
#define DEFAULT_TYPE "lcd"
/* #define CONFIG_FILE Non config file code removed by David GLAUDE */
/* Above 5 lines added by Joris :( */

#define IS_LCD_DISPLAY	(p->MtxOrb_type == MTXORB_LCD)
#define IS_LKD_DISPLAY	(p->MtxOrb_type == MTXORB_LKD)
#define IS_VFD_DISPLAY	(p->MtxOrb_type == MTXORB_VFD)
#define IS_VKD_DISPLAY	(p->MtxOrb_type == MTXORB_VKD)

/* TODO: Remove this if not in use anymore...
 * #define NotEnoughArgs (i + 1 > argc)
 */

/*
 NOTE: This does not appear to make use of the
       hbar and vbar functions present in the LKD202-25.
       Why I do not know.
 RESP: Because software emulated hbar/vbar permit simultaneous use.
*/

/* TODO: Find a better way to deal with the bitmap/name of custom char
 *       Here the value link to the enum and the definition at the end.
 */
#define START_FONT 30
#define WHITE 254
#define BLACK 255

/* Starting from now bigfont.h does not define S=White and F=Black anymore */
/* Due to the way _num is implemented we need to delta by START_FONT */
#define S WHITE - START_FONT
#define F BLACK - START_FONT
#include "bigfont.h"
#undef F
#undef S

typedef enum {
/* This is for standard icon: IT MUST START AT POSITION ZERO */
	empty_heart   = 0,
	filled_heart  = 1,
	ellipsis      = 2,
/* This was for standard icon */
/* This is for non standard icon */
	play          = 3,
	fforward      = 4,
	frewind       = 5,
	uparrow       = 6,
	downarrow     = 7,
/* This was for non standard icon */
/* This is for bar up */
	baru1         = 8,
	baru2         = 9,
	baru3         = 10,
	baru4         = 11,
	baru5         = 12,
	baru6         = 13,
	baru7         = 14,
/* This was for bar up */
/* This is for bar down */
	bard1         = 15,
	bard2         = 16,
	bard3         = 17,
	bard4         = 18,
	bard5         = 19,
	bard6         = 20,
	bard7         = 21,
/* This was for bar down */
/* This is for bar right */
	barr1         = 22,
	barr2         = 23,
	barr3         = 24,
	barr4         = 25,
/* This was for bar right */
/* This is for bar left */
	barl1         = 26,
	barl2         = 27,
	barl3         = 28,
	barl4         = 29,
/* This was for bar left */
/* This is for sofware bigfont */
	bigfonta      = 30,
	bigfontb      = 31,
	bigfontc      = 32,
	bigfontd      = 33,
	bigfonte      = 34,
	bigfontf      = 35,
	bigfontg      = 36,
	bigfonth      = 37,
/* This was for software bigfont */
	barw          = WHITE,
	barb          = BLACK
} bar_type;

typedef enum {
	MTXORB_LCD,
	MTXORB_LKD,
	MTXORB_VFD,
	MTXORB_VKD
} MtxOrb_type_type;

static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;


/* This is an embrionic Private Date, it need to grow ! */
typedef struct p {
	int def[9];
	int use[9];
	int circular;		/* static data from MtxOrb_ask_bar */
	int output_state;	/* static data from MtxOrb_output */
	int backlight_state;	/* static data from MtxOrb_backlight */
	int width;
	int height;
	char *framebuf;		/* Frame buffer */
	char *old;		/* Current on screen frame buffer */
	int widthBYheight;	/* Avoid computing width * height frequently */
	int clear;		/* Control when the LCD is cleared */
	int fd;			/* The LCD file descriptor */
	int contrast;		/* static data from set/get_contrast */
	int backlightenabled;
	MtxOrb_type_type MtxOrb_type; 
	int timer;		/* static data from MtxOrb_heartbeat */
	char pause_key;
	char back_key;
	char forward_key;
	char main_menu_key;
	int keypad_test_mode;
        } PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "MtxOrb_";


static int  MtxOrb_ask_bar (Driver *drvthis, int type);
static void MtxOrb_set_known_char (Driver * drvthis, int car, int type);
static void MtxOrb_linewrap (Driver *drvthis, int on);
static void MtxOrb_autoscroll (Driver *drvthis, int on);
static void MtxOrb_cursorblink (Driver *drvthis, int on);


/* Parse one key from the configfile */
static char MtxOrb_parse_keypad_setting (Driver *drvthis, char * keyname, char default_value)
{
	char return_val = 0;
	char * s;
	char buf [255];

	s = drvthis->config_get_string ( drvthis->name, keyname, 0, NULL);
	if (s != NULL){
		strncpy (buf, s, sizeof(buf));
		buf[sizeof(buf)-1]=0;
		return_val = buf[0];
	} else {
		return_val=default_value;
	}
	return return_val;
}

/* TODO:  Get the frame buffers working right */

/* Opens com port and sets baud correctly...
 *
 * Called to initialize driver settings
 */
MODULE_EXPORT int
MtxOrb_init (Driver *drvthis, char *args)
{
	/* Start of command line parsing*/

	struct termios portset;

	int contrast = DEFAULT_CONTRAST;
	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	char buf[256] = "";
	int tmp, w, h;
        PrivateData *p;

/*	int def[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };	*/
/*	int use[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };		*/
	/* Alocate and store private data */

        p = (PrivateData *) malloc( sizeof( PrivateData) );
	if( ! p )
	        return -1;
	if( drvthis->store_private_ptr( drvthis, p ) )
	        return -1;

	memset( p->def, -1, sizeof(p->def) );
	memset( p->use,  0, sizeof(p->use) );
	p->circular = -1;	/* static data from MtxOrb_ask_bar */
	p->output_state = -1;	/* static data from MtxOrb_output */
	p->backlight_state = 1; /* static data from MtxOrb_backlight */
	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->widthBYheight = LCD_DEFAULT_WIDTH * LCD_DEFAULT_HEIGHT;
	p->clear = 1;		/* assume LCD is cleared at startup */
	p->contrast = DEFAULT_CONTRAST;
	p->framebuf = NULL;
	p->backlightenabled = DEFAULT_BACKLIGHT;
	p->old = NULL;
	p->MtxOrb_type = MTXORB_LKD;  /* Assume it's an LCD w/keypad */
	p->timer = 0;		/* static data from MtxOrb_heartbeat */
	p->pause_key = MTXORB_DEFAULT_PAUSE_KEY;
	p->back_key = MTXORB_DEFAULT_BACK_KEY;
	p->forward_key = MTXORB_DEFAULT_FORWARD_KEY;
	p->main_menu_key = MTXORB_DEFAULT_MAIN_MENU_KEY;
	p->keypad_test_mode = 0;

	debug( RPT_INFO, "MtxOrb: init(%p,%s)", drvthis, args );


	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string ( drvthis->name , "device" , 0 , DEFAULT_DEVICE),sizeof(device));
	device[sizeof(device)-1]=0;
	report (RPT_INFO,"MtxOrb: Using device: %s", device);

	/* Get display size */
	strncpy(size, drvthis->config_get_string ( drvthis->name , "size" , 0 , DEFAULT_SIZE),sizeof(size));
	size[sizeof(size)-1]=0;
	if( sscanf(size , "%dx%d", &w, &h ) != 2
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
		report (RPT_WARNING, "MtxOrb: Cannot read size: %s. Using default value %s.", size, DEFAULT_SIZE);
		sscanf( DEFAULT_SIZE , "%dx%d", &w, &h );
	}
	p->width = w;
	p->height = h;
	p->widthBYheight = w * h;

	/* Get contrast */
	if (0<=drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) && drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) <= 255) {
		contrast = drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST);
	} else {
		report (RPT_WARNING, "MtxOrb: Contrast must be between 0 and 255. Using default value.");
	}

	/* Get speed */
	tmp = drvthis->config_get_int ( drvthis->name , "Speed" , 0 , DEFAULT_SPEED);

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
			speed = DEFAULT_SPEED;
			switch (speed) {
				case B1200:
					strncpy(buf,"1200", sizeof(buf));
					break;
				case B2400:
					strncpy(buf,"2400", sizeof(buf));
					break;
				case B9600:
					strncpy(buf,"9600", sizeof(buf));
					break;
				case B19200:
					strncpy(buf,"19200", sizeof(buf));
					break;
			}
			report (RPT_WARNING , "MtxOrb: Speed must be 1200, 2400, 9600 or 19200. Using default value of %s baud!", buf);
			strncpy(buf,"", sizeof(buf));
	}


	/* Get backlight setting*/
	if(drvthis->config_get_bool( drvthis->name , "enablebacklight" , 0 , DEFAULT_BACKLIGHT)) {
		p->backlightenabled = 1;
	}

	/* Get display type */
	strncpy(buf, drvthis->config_get_string ( drvthis->name , "type" , 0 , DEFAULT_TYPE),sizeof(buf));
	buf[sizeof(buf)-1]=0;

	if (strncasecmp(buf, "lcd", 3) == 0) {
		p->MtxOrb_type = MTXORB_LCD;
	} else if (strncasecmp(buf, "lkd", 3) == 0) {
		p->MtxOrb_type = MTXORB_LKD;
	} else if (strncasecmp (buf, "vfd", 3) == 0) {
		p->MtxOrb_type = MTXORB_VFD;
	} else if (strncasecmp (buf, "vkd", 3) == 0) {
		p->MtxOrb_type = MTXORB_VKD;
	} else {
		report (RPT_ERR, "MtxOrb: unknwon display type %s; must be one of lcd, lkd, vfd, or vkd", buf);
		return (-1);
		}

	/* Get keypad settings*/

	/* keypad test mode? */
	if (drvthis->config_get_bool( drvthis->name , "keypad_test_mode" , 0 , 0)) {
		report (RPT_INFO, "MtxOrb: Entering keypad test mode...\n");
		p->keypad_test_mode = 1;
	}

	if (!p->keypad_test_mode) {
		/* We don't send any chars to the server in keypad test mode.
		 * So there's no need to get them from the configfile in keypad test mode.
		 */
		/* pause_key */
		p->pause_key = MtxOrb_parse_keypad_setting (drvthis, "PauseKey", MTXORB_DEFAULT_PAUSE_KEY);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as pause_key.", p->pause_key);

		/* back_key */
		p->back_key = MtxOrb_parse_keypad_setting (drvthis, "BackKey", MTXORB_DEFAULT_BACK_KEY);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as back_key", p->back_key);

		/* forward_key */
		p->forward_key = MtxOrb_parse_keypad_setting (drvthis, "ForwardKey", MTXORB_DEFAULT_FORWARD_KEY);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as forward_key", p->forward_key);

		/* main_menu_key */
		p->main_menu_key = MtxOrb_parse_keypad_setting (drvthis, "MainMenuKey", MTXORB_DEFAULT_MAIN_MENU_KEY);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as main_menu_key", p->main_menu_key);
	}
	/* End of config file parsing*/

	/* Set up io port correctly, and open it... */
	p->fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (p->fd == -1) {
		switch (errno) {
			case ENOENT: report (RPT_ERR, "MtxOrb_init: %s device file missing!\n", device);
				break;
			case EACCES: report (RPT_ERR, "MtxOrb_init: %s device could not be opened...\n", device);
				report (RPT_ERR, "MtxOrb_init: perhaps you should run LCDd as root?\n");
				break;
			default: report (RPT_ERR, "MtxOrb_init: failed (%s)\n", strerror (errno));
				break;
		}
  		return -1;
	} else
		report (RPT_INFO, "MtxOrb: opened display on %s\n", device);

	tcgetattr (p->fd, &portset);

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
	tcsetattr (p->fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	if (!p->framebuf)
		p->framebuf = (unsigned char *)
			malloc (p->widthBYheight);
	memset (p->framebuf, ' ', p->widthBYheight);

	/*
	 * Configure display
	 */

	MtxOrb_linewrap (drvthis, DEFAULT_LINEWRAP);
	MtxOrb_autoscroll (drvthis, DEFAULT_AUTOSCROLL);
	MtxOrb_cursorblink (drvthis, DEFAULT_CURSORBLINK);
	MtxOrb_set_contrast (drvthis, contrast);

	return 0;
}

#define ValidX(x) if ((x) > p->width) { (x) = p->width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > p->height) { (y) = p->height; } else (y) = (y) < 1 ? 1 : (y);

/*
 * TODO: Check this quick hack to detect clear of the screen.
 *
 * Clear: catch up when the screen get clear to be able to
 * forget bar caracter not in use anymore and reuse the
 * slot for another bar caracter.
 */

MODULE_EXPORT void
MtxOrb_clear (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	if (p->framebuf != NULL)
		memset (p->framebuf, ' ', (p->widthBYheight));

	/* We don't use hardware clear anymore.
	 * We use incremental update with the frame_buffer. */
	/* write(fd, "\x0FE" "X", 2); */ /* instant clear... */
	p->clear = 1;

	debug(RPT_DEBUG, "MtxOrb: cleared screen");
}

/******************************
 * Clean-up
 */
MODULE_EXPORT void
MtxOrb_close (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	close (p->fd);

	if (p->framebuf)
		free (p->framebuf);
	p->framebuf = NULL;

        free( p );

	debug(RPT_DEBUG, "MtxOrb: closed");
}

/******************************
 * Returns the display width
 */
MODULE_EXPORT int
MtxOrb_width (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

        return p->width;
}

/******************************
 * Returns the display height
 */
MODULE_EXPORT int
MtxOrb_height (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	return p->height;
}

MODULE_EXPORT void
MtxOrb_string (Driver *drvthis, int x, int y, char *string)
{
	int offset, siz;

        PrivateData * p = drvthis->private_data;

	ValidX(x);
	ValidY(y);

	x--; y--; /* Convert 1-based coords to 0-based... */
	offset = (y * p->width) + x;
	siz = (p->widthBYheight) - offset;
	siz = siz > strlen(string) ? strlen(string) : siz;

	memcpy(p->framebuf + offset, string, siz);

	debug(RPT_DEBUG, "MtxOrb: printed string at (%d,%d)", x, y);
}

MODULE_EXPORT void
MtxOrb_flush (Driver *drvthis)
{
	char out[12];
	int i,j,mv = 1;
	char *xp, *xq;

        PrivateData * p = drvthis->private_data;

	if (p->old == NULL) {
		p->old = malloc(p->widthBYheight);

		write(p->fd, "\x0FEG\x01\x01", 4);
		write(p->fd, p->framebuf, p->widthBYheight);

		strncpy(p->old, p->framebuf, p->widthBYheight);

		return;

	} else {
		/* CODE TEMPORARY DISABLED (joris)
		   UNSURE IF IT STILL WORKS NOW
	David GLAUDE:	It seems to work...
			But I did not try to understand...
		if (! new_framebuf(drvthis, old))
			return;
		*/
	}

	xp = p->framebuf;
	xq = p->old;

	for (i = 1; i <= p->height; i++) {
		for (j = 1; j <= p->width; j++) {

			if ((*xp == *xq) && (*xp > 8))
				mv = 1;
			else {
			/* Draw characters that have changed, as well
			 * as custom characters.  We know not if a custom
			 * character has changed.
			 */

				if (mv == 1) {
					snprintf(out, sizeof(out), "\x0FEG%c%c", j, i);
					write (p->fd, out, 4);
					mv = 0;
				}
				write (p->fd, xp, 1);
			}
			xp++;
			xq++;
		}
	}


	/* for (i = 0; i < height; i++) {
	 *	snprintf (out, sizeof(out), "\x0FEG\x001%c", i + 1);
	 *	write (fd, out, 4);
	 *	write (fd, framebuf + (width * i), width);
	 * }
	 */

	strncpy(p->old, p->framebuf, p->widthBYheight);

	debug(RPT_DEBUG, "MtxOrb: frame buffer flushed");
}

/******************************
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
MtxOrb_chr (Driver *drvthis, int x, int y, char c)
{
	int offset;

	/* Characters may or may NOT be alphabetic; it appears
	 * that characters 0..4 (or similar) are graphic fonts
	 */

        PrivateData * p = drvthis->private_data;

	ValidX(x);
	ValidY(y);

	/* write immediately to screen... this code was taken
	 * from the LK202-25; should work for others, yes?
	 * snprintf(out, sizeof(out), "\x0FEG%c%c%c", x, y, c);
	 * write (fd, out, 4);
	 */

	/* write to frame buffer */
	y--; x--; /* translate to 0-index */
	offset = (y * p->width) + x;
	p->framebuf[offset] = c;

	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
}

MODULE_EXPORT int
MtxOrb_get_contrast (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	return p->contrast;
}

/******************************
 * Changes screen contrast in promille
 * The hardware support value from 0 to 255.
 * Hardware 140 seems good
 * note: works only for LCD displays
 * Is it better to use the brightness for VFD/VKD displays ?
 */
MODULE_EXPORT void
MtxOrb_set_contrast (Driver *drvthis, int promille)
{
	char out[4];
	int real_contrast;

        PrivateData * p = drvthis->private_data;

	/* Check it */
	if( promille < 0 || promille > 1000 )
		return;

	/* Store it */
	p->contrast = promille;

	real_contrast = (int) ((long)promille * 255 / 1000 );

	/* And do it */

	if (IS_LCD_DISPLAY || IS_LKD_DISPLAY) {
		snprintf (out, sizeof(out), "\x0FEP%c", real_contrast);
		write (p->fd, out, 3);

	report(RPT_DEBUG, "MtxOrb: contrast set to %d", real_contrast);
	} else {
	report(RPT_DEBUG, "MtxOrb: contrast not set to %d - not LCD or LKD display", real_contrast);
	}
}

/******************************
 * Sets the backlight on or off -- can be done quickly for
 * an intermediate brightness...
 *
 * WARNING: off switches vfd/vkd displays off entirely
 *	    so maybe it is best to start LCDd with -b on
 *
 * WARNING: there seems to be a movement afoot to add more
 *          functions than just on/off to this..
 */

#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON 1

MODULE_EXPORT void
MtxOrb_backlight (Driver *drvthis, int on)
{
        PrivateData * p = drvthis->private_data;

	if (p->backlight_state == on)
		return;

	p->backlight_state = on;

	switch (on) {
		case BACKLIGHT_ON:
			write (p->fd, "\x0FE" "F", 2);
	debug(RPT_DEBUG, "MtxOrb: backlight turned on");
			break;
		case BACKLIGHT_OFF:
			if (IS_VKD_DISPLAY || IS_VFD_DISPLAY) {
	debug(RPT_DEBUG, "MtxOrb: backlight ignored - not LCD or LKD display");
				; /* turns display off entirely (whoops!) */
			} else {
	debug(RPT_DEBUG, "MtxOrb: backlight turned off");
				write (p->fd, "\x0FE" "B" "\x000", 3);
			}
			break;
		default: /* ignored... */
	debug(RPT_DEBUG, "MtxOrb: backlight - invalid setting");
			break;
		}
}

/******************************
 * Sets output port on or off
 * displays with keypad have 6 outputs but the one without kepad
 * have only one output
 * NOTE: length of command are different
 */
MODULE_EXPORT void
MtxOrb_output (Driver *drvthis, int on)
{
	char out[5];

        PrivateData * p = drvthis->private_data;

	on = on & 077;	/* strip to six bits */

	if (p->output_state == on)
		return;

	p->output_state = on;

	debug(RPT_DEBUG, "MtxOrb: output pins set: %04X", on);

	if (IS_LCD_DISPLAY || IS_VFD_DISPLAY) {
		/* LCD and VFD displays only have one output port */
		(on) ?
			write (p->fd, "\x0FEW", 2) :
			write (p->fd, "\x0FEV", 2);
	} else {
		int i;

		/* Other displays have six output ports;
		 * the value "on" is a binary value determining which
		 * ports are turned on (1) and off (0).
		 */

		for(i = 0; i < 6; i++) {
			(on & (1 << i)) ?
				snprintf (out, sizeof(out), "\x0FEW%c", i + 1) :
				snprintf (out, sizeof(out), "\x0FEV%c", i + 1);
			write (p->fd, out, 3);
		}
	}
}

/******************************
 * Toggle the built-in linewrapping feature
 */
static void
MtxOrb_linewrap (Driver *drvthis, int on)
{
        PrivateData * p = drvthis->private_data;

	if (on) {
		write (p->fd, "\x0FE" "C", 2);

	debug(RPT_DEBUG, "MtxOrb: linewrap turned on");
	} else {
		write (p->fd, "\x0FE" "D", 2);

	debug(RPT_DEBUG, "MtxOrb: linewrap turned off");
	}
}

/******************************
 * Toggle the built-in automatic scrolling feature
 */
static void
MtxOrb_autoscroll (Driver *drvthis, int on)
{
        PrivateData * p = drvthis->private_data;

	if (on) {
		write (p->fd, "\x0FEQ", 2);

	debug(RPT_DEBUG, "MtxOrb: autoscroll turned on");
	} else {
		write (p->fd, "\x0FER", 2);

	debug(RPT_DEBUG, "MtxOrb: autoscroll turned off");
	}
}

/* TODO: make sure this doesn't mess up non-VFD displays
 ******************************
 * Toggle cursor blink on/off
 */
static void
MtxOrb_cursorblink (Driver *drvthis, int on)
{
        PrivateData * p = drvthis->private_data;

	if (on) {
		write (p->fd, "\x0FES", 2);

	debug(RPT_DEBUG, "MtxOrb: cursorblink turned on");
	} else {
		write (p->fd, "\x0FET", 2);

	debug(RPT_DEBUG, "MtxOrb: cursorblink turned off");
	}
}

/* TODO: REMOVE ME */
MODULE_EXPORT void MtxOrb_init_old_vbar (Driver *drvthis) { }

/* TODO: REMOVE ME */
MODULE_EXPORT void MtxOrb_init_old_hbar (Driver *drvthis) { }

/******************************
 * Returns string with general information about the display
 */
MODULE_EXPORT char *
MtxOrb_get_info (Driver *drvthis)
{
	char in = 0;
	static char info[255];
	char tmp[255], buf[64];
	/* int i = 0; */
        PrivateData * p = drvthis->private_data;

	fd_set rfds;

	struct timeval tv;
	int retval;

	debug(RPT_DEBUG, "MtxOrb: get_info");

	memset(info, '\0', sizeof(info));
	strcpy(info, "Matrix Orbital Driver ");

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

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (p->fd, &in, 1) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			switch (in) {
				case '\x01': strcat(info, "LCD0821 "); break;
				case '\x03': strcat(info, "LCD2021 "); break;
				case '\x04': strcat(info, "LCD1641 "); break;
				case '\x05': strcat(info, "LCD2041 "); break;
				case '\x06': strcat(info, "LCD4021 "); break;
				case '\x07': strcat(info, "LCD4041 "); break;
				case '\x08': strcat(info, "LK202-25 "); break;
				case '\x09': strcat(info, "LK204-25 "); break;
				case '\x0A': strcat(info, "LK404-55 "); break;
				case '\x0B': strcat(info, "VFD2021 "); break;
				case '\x0C': strcat(info, "VFD2041 "); break;
				case '\x0D': strcat(info, "VFD4021 "); break;
				case '\x0E': strcat(info, "VK202-25 "); break;
				case '\x0F': strcat(info, "VK204-25 "); break;
				case '\x10': strcat(info, "GLC12232 "); break;
				case '\x11': strcat(info, "GLC12864 "); break;
				case '\x12': strcat(info, "GLC128128 "); break;
				case '\x13': strcat(info, "GLC24064 "); break;
				case '\x14': strcat(info, "GLK12864-25 "); break;
				case '\x15': strcat(info, "GLK24064-25 "); break;
				case '\x21': strcat(info, "GLK128128-25 "); break;
				case '\x22': strcat(info, "GLK12232-25 "); break;
				case '\x31': strcat(info, "LK404-AT "); break;
				case '\x32': strcat(info, "VFD1621 "); break;
				case '\x33': strcat(info, "LK402-12 "); break;
				case '\x34': strcat(info, "LK162-12 "); break;
				case '\x35': strcat(info, "LK204-25PC "); break;
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

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (p->fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Serial No: %ld ", (long int) tmp);
			strcat(info, buf);
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

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (p->fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Firmware Rev. %ld ", (long int) tmp);
			strcat(info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device firmware revision");

	return info;
}

/* TODO: Finish the support for bar growing reverse way.
 * TODO: Need a "y" as input also !!!
 * TODO: Migrate to the new vbar.
 ******************************
 * Draws a vertical bar...
 * This is the new version using dynamic icon alocation
 */
MODULE_EXPORT void
MtxOrb_old_vbar (Driver *drvthis, int x, int len)
{
	/* baru5 = bigfonte ... This is a way the cache can benefit from it. */
	unsigned char mapu[9] = { barw, baru1, baru2, baru3, baru4, bigfonte, baru6, baru7, barb };
	/* bard5 = bigfontf ... This is a way the cache can benefit from it. */
	unsigned char mapd[9] = { barw, bard1, bard2, bard3, bard4, bigfontf, bard6, bard7, barb };

	int y;

        PrivateData * p = drvthis->private_data;

	debug(RPT_DEBUG, "MtxOrb: vertical bar at %d set to %d", x, len);

/* REMOVE THE NEXT LINE FOR TESTING ONLY... */
/*  len=-len; */
/* REMOVE THE PREVIOUS LINE FOR TESTING ONLY... */

	if (len > 0) {
		for (y = p->height; y > 0 && len > 0; y--) {
			if (len >= cellheight)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapu[len]);

			len -= cellheight;
		}
	} else {
		len = -len;
		for (y = 2; y <= p->height && len > 0; y++) {
			if (len >= cellheight)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapd[len]);

			len -= cellheight;
		}
	}

}

/* TODO: Finish the support for bar growing reverse way.
 * TODO: Migrate to the new hbar.
 ******************************
 * Draws a horizontal bar to the right.
 * This is the new version ussing dynamic icon alocation
 */
MODULE_EXPORT void
MtxOrb_old_hbar (Driver *drvthis, int x, int y, int len)
{
	unsigned char mapr[6] = { barw, barr1, barr2, barr3, barr4, barb };
	unsigned char mapl[6] = { barw, barl1, barl2, barl3, barl4, barb };

        PrivateData * p = drvthis->private_data;

	ValidX(x);
	ValidY(y);

	debug(RPT_DEBUG, "MtxOrb: horizontal bar at %d set to %d", x, len);

	if (len > 0) {
		for (; x <= p->width && len > 0; x++) {
			if (len >= cellwidth)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapr[len]);

			len -= cellwidth;

		}
	} else {
		len = -len;
		for (; x > 0 && len > 0; x--) {
			if (len >= cellwidth)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapl[len]);

			len -= cellwidth;

		}
	}

}

/* TODO: REMOVE ME */
MODULE_EXPORT void MtxOrb_init_old_num (Driver *drvthis) { }


/* TODO: MtxOrb_set_char is doing the job "real-time" as oppose
 * to at flush time. Call to this function should be done in flush
 * this mean in  raw_frame. GLU
 ******************************
 * Writes a big number.
 */
MODULE_EXPORT void
MtxOrb_num (Driver *drvthis, int pos, int val)
{
	int x, y;
	int c;

	debug(RPT_DEBUG, "MtxOrb: write big number %d at %d", val, pos);

/*	We don't use hardware bignum anymore, so we remove those. 	*/
/*	snprintf (out, sizeof(out), "\x0FE#%c%c", x, num);        	*/
/*	write (fd, out, 4);						*/

/* Currently we are bignum but if bigalpha is there remove this line */
  c=val+'0';	/* We transform from 0-9 to 'O' to '9' */

  if ((pos < -2) || (pos > 20)) return;  /* are we outisde the visible spectrum */
  if (('c' < 32) || ('c' > 127)) return; /* are we characteristic or not? */

  c -= 32;

/*  printf("pos: %d char: %d val: %d\n", pos, c, val); */
  for (y=0;y<4;y++) {
    for (x=0;x<3;x++) {
      MtxOrb_icon (drvthis, x+pos, y+1, START_FONT + normal[c][x + (y * 3)]);
    }
  }
}

/* TODO: Every time we define a custom char within the LCD,
 * we have to compute the binary value we are going to use.
 * It is easy to keep the bitmap in this source file,
 * but we compute that once rather than every time. GLU
 *
 * TODO: MtxOrb_set_char is doing the job "real-time" as oppose
 * to at flush time. Call to this function should be done in flush. GLU
 *
 * TODO: We make one 3 bytes write folowed by cellheight one byte
 * write. This should be done in one single write. GLU
 */

#define MAX_CUSTOM_CHARS 7

/******************************
 * Sets a custom character from 0-7...
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
MtxOrb_set_char (Driver *drvthis, int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

        PrivateData * p = drvthis->private_data;

	if (n < 0 || n > MAX_CUSTOM_CHARS)
		return;
	if (!dat)
		return;

	snprintf (out, sizeof(out), "\x0FEN%c", n);
	write (p->fd, out, 3);

	for (row = 0; row < cellheight; row++) {
		letter = 0;
		for (col = 0; col < cellwidth; col++) {
			/* shift to make room for new scan line data */
			letter <<= 1;
			/* Now read a single bit of data
			 * -- one entry in dat[] --
			 * and add it to the binary data in "letter"
			 */
			letter |= (dat[(row * cellwidth) + col] > 0);
		}
		write (p->fd, &letter, 1); /* write one character for each row */
	}
}

/* TODO: REMOVE ME */
MODULE_EXPORT void MtxOrb_old_icon (Driver *drvthis, int which, char dest) { }

/* TODO: This is not yet my idea of icon frame buffer but it work well.
 */
MODULE_EXPORT void
MtxOrb_icon (Driver *drvthis, int x, int y, int icon)
{
	MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, icon));
}

/* TODO: Recover the code for I2C connectivity to MtxOrb
 * and don't query the LCD if it does not support keypad.
 * Otherwise crash of the LCD and/or I2C bus.
 *
 ******************************
 * returns one character from the keypad...
 * (A-Z) on success, 0 on failure...
 */

MODULE_EXPORT char
MtxOrb_getkey (Driver *drvthis)
{
	char in = 0;

        PrivateData * p = drvthis->private_data;

	read (p->fd, &in, 1);
	switch (in) {
		case KEY_LEFT:
			in = INPUT_BACK_KEY;
			break;
		case KEY_RIGHT:
			in = INPUT_FORWARD_KEY;
			break;
		case KEY_DOWN:
			in = INPUT_MAIN_MENU_KEY;
			break;
		case KEY_F1:
			in = INPUT_PAUSE_KEY;
			break;
		/*TODO: add more translations here (if neccessary)*/
		default:
			in = 0;
			break;
	}

	return in;
}

/*************************************************************************
 * Ask for allocation of a custom caracter to be a well known graphic.
 * The function is suppose to return a value between 0 and 7.
 * 0 is NOT reserver anymore for heartbeat or icon.
 * This function manadge a cache of graphical caracter in use.
 */
static int
MtxOrb_ask_bar (Driver *drvthis, int type)
{
	int i;
	int pos;
	int last_not_in_use;

        PrivateData * p = drvthis->private_data;

/*	fprintf(stderr, "GLU: MtxOrb_ask_bar(%d).\n", type); */

/* This bypass the search for WHITE and BLACK */
	if (type==barw) return 32;
	if (type==barb) return 255;

	/* If the screen was clear then no graphic caracter are in use yet. */
	if (p->clear) {
  	  for (pos = 0; pos < 8; pos++) p->use[pos] = 0;
	  p->clear = 0;
	}

	/* Search for a match with caracter already defined. */
	pos = 8;		             /* Not found. */
	for (i = 0; i < 8; i++) {            /* For all including heartbeat. */
	  if (p->def[i] == type) pos = i;	     /* Founded (should break now). */
	}

	if (pos == 8) {
		last_not_in_use = 8;	     /* No empty slot to reuse. */
/*		fprintf(stderr, "GLU: MtxOrb_ask_bar| not found.\n");  */
		p->circular = (p->circular + 1) % 8;

		for (i = 0; i < 8; i++) {
			if (!p->use[(i + p->circular) % 8])
				last_not_in_use = (i + p->circular) % 8;
		}
		pos = last_not_in_use;
	}

	if (pos != 8) {
	/* A caracter is found (Best match could solve our problem).
 	 * REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| found at %d.\n", pos);
	 */
		if (p->def[pos] != type) {
			MtxOrb_set_known_char (drvthis, pos, type);
/* fprintf(stderr, "GLU: MtxOrb_ask_bar [Set a char] pos: %d.\n", pos); */
			/* Define a new graphic caracter. */
			p->def[pos] = type;
			/* Remember that now the caracter is available. */
		}
		if (!p->use[pos]) {
			/* If the caracter is no yet in use (but defined). */
			p->use[pos] = 1;
			/* Remember it is in use (so protect it from re-use). */
		}
	}
	if (pos==8) {
		/* pos=65;  ("A")? */
		switch (type) {
		case baru1: pos = '_';  break;
		case baru2: pos = '.';  break;
		case baru3: pos = ',';  break;
		case baru4: pos = 'o';  break;
		case baru5: pos = 'o';  break;
		case baru6: pos = 'O';  break;
		case baru7: pos = '8';  break;
		case bard1: pos = '\''; break;
		case bard2: pos = '"';  break;
		case bard3: pos = '^';  break;
		case bard4: pos = '^';  break;
		case bard5: pos = '*';  break;
		case bard6: pos = 'O';  break;
		case bard7: pos = '8';  break;
		case barr1: pos = '-';  break;
		case barr2: pos = '-';  break;
		case barr3: pos = '=';  break;
		case barr4: pos = '=';  break;
		case barl1: pos = '-';  break;
		case barl2: pos = '-';  break;
		case barl3: pos = '=';  break;
		case barl4: pos = '=';  break;
		case barw:  pos = ' ';  break;
		case barb:  pos = 255;  break;

		case bigfonta:  pos = '\\';  break;
		case bigfontb:  pos = '/';  break;
		case bigfontc:  pos = '\\';  break;
		case bigfontd:  pos = '\\';  break;
		case bigfonte:  pos = '-';  break;
		case bigfontf:  pos = '-';  break;
		case bigfontg:  pos = '\\';  break;
		case bigfonth:  pos = '/';  break;

		case play:      pos = 'P'; break;
		case fforward:  pos = '>'; break;
		case frewind:   pos = '<'; break;
		case uparrow:   pos = '^'; break;
		case downarrow: pos = 'v'; break;
		default:        pos = '?';  break;
		}
	}

	return (pos);
}

/******************************
 * Does the heartbeat...
 */
MODULE_EXPORT void
MtxOrb_heartbeat (Driver *drvthis, int type)
{
	int whichIcon;

        PrivateData * p = drvthis->private_data;

	if (type == HEARTBEAT_ON) {
		/* Set this to pulsate like a real heart beat... */
		whichIcon = (! ((p->timer + 4) & 5));

		/* Ask for the right heartbeat icon... */
		MtxOrb_icon (drvthis, p->width, 1, whichIcon);

		/* change display... */
		MtxOrb_flush (drvthis);
	}

	p->timer++;
	p->timer &= 0x0f;
}

/******************************
 * Sets up a well known character for use.
 */
static void
MtxOrb_set_known_char (Driver *drvthis, int car, int type)
{
	char all_bar[39][5 * 8] = {
/* Here start 3 standard icon used by heartbear and other. */
		{
		1, 1, 1, 1, 1,	/* Empty Heart */
		1, 0, 1, 0, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 0, 0, 0, 1,
		1, 1, 0, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		1, 1, 1, 1, 1,	/* Filled Heart */
		1, 0, 1, 0, 1,
		0, 1, 0, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		1, 0, 1, 0, 1,
		1, 1, 0, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/* Ellipsis */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 0, 1, 0, 1,
		},
/* This is not suppose to be standard icon, just I don't want to lose them.
 * They come directly from the old patch for bignum any MP3 player love them.
 */
		{
		0, 1, 0, 0, 0,  /* Play */
     		0, 1, 1, 0, 0,
     		0, 1, 1, 1, 0,
     		0, 1, 1, 1, 1,
     		0, 1, 1, 1, 0,
     		0, 1, 1, 0, 0,
     		0, 1, 0, 0, 0,
     		0, 0, 0, 0, 0,
   		}, {
		0, 0, 0, 0, 0,  /* FForward */
     		1, 0, 1, 0, 0,
     		1, 1, 0, 1, 0,
     		1, 1, 1, 0, 1,
     		1, 1, 0, 1, 0,
     		1, 0, 1, 0, 0,
     		0, 0, 0, 0, 0,
     		0, 0, 0, 0, 0,
   		}, {
     		0, 0, 0, 0, 0,  /* FRewind */
     		0, 0, 1, 0, 1,
     		0, 1, 0, 1, 1,
     		1, 0, 1, 1, 1,
     		0, 1, 0, 1, 1,
     		0, 0, 1, 0, 1,
     		0, 0, 0, 0, 0,
     		0, 0, 0, 0, 0,
   		}, {
     		0, 0, 1, 0, 0,  /* Up arrow */
     		0, 1, 1, 1, 0,
     		1, 1, 1, 1, 1,
     		0, 0, 1, 0, 0,
     		0, 0, 1, 0, 0,
     		0, 0, 1, 0, 0,
     		0, 0, 1, 0, 0,
     		0, 0, 0, 0, 0,
   		}, {
     		0, 0, 1, 0, 0,  /* Down arrow */
     		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		1, 1, 1, 1, 1,
		0, 1, 1, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
		},
/* Here start the bar stuff */
		{
		0, 0, 0, 0, 0,	/*  char u1[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u2[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u3[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u4[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u5[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u6[] = */
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	/*  char u7[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		1, 1, 1, 1, 1,	/*  char d1[] = */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d2[] = */
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d3[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d4[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d5[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d6[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, {
		1, 1, 1, 1, 1,	/*  char d7[] = */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		}, {
		1, 0, 0, 0, 0,	/*  char r1[] = */
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		}, {
		1, 1, 0, 0, 0,	/*  char r2[] = */
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		}, {
		1, 1, 1, 0, 0,	/*  char r3[] = */
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		}, {
		1, 1, 1, 1, 0,	/*  char r4[] = */
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		}, {
		0, 0, 0, 0, 1,	/*  char l1[] = */
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
		}, {
		0, 0, 0, 1, 1,	/*  char l2[] = */
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 1, 1,
		}, {
		0, 0, 1, 1, 1,	/*  char l3[] = */
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		}, {
		0, 1, 1, 1, 1,	/*  char l4[] = */
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		},
/* Here start the 8 basic block used by bigfont.h */
		{ /* char a[] */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		}, { /* char b[] */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 1,
		0, 0, 0, 1, 1,
		0, 0, 1, 1, 1,
		0, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, { /* char c[] */
		1, 1, 1, 1, 1,
		0, 1, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 0, 1, 1,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, { /* char d[] */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 0,
		1, 1, 1, 0, 0,
		1, 1, 0, 0, 0,
		1, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, { /* char e[] = u5 */
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, { /* char f[] = d5 */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		}, { /* char g[] */
		1, 0, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, { /* char h[] */
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 0,
		1, 1, 1, 0, 0,
		1, 1, 0, 0, 0,
		1, 0, 0, 0, 0,
		}
	};

	MtxOrb_set_char (drvthis, car, &all_bar[type][0]);
}


