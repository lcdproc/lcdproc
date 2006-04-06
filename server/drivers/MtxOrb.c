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
#include <sys/poll.h>

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
#define MTXORB_DEFAULT_Left     'A'
#define MTXORB_DEFAULT_Right    'B'
#define MTXORB_DEFAULT_Up       'C'
#define MTXORB_DEFAULT_Down     'D'
#define MTXORB_DEFAULT_Enter    'E'
#define MTXORB_DEFAULT_Escape   'F'

#define DEFAULT_SIZE "20x4"
#define DEFAULT_BACKLIGHT 1
#define DEFAULT_TYPE "lcd"
/* #define CONFIG_FILE Non config file code removed by David GLAUDE */
/* Above 5 lines added by Joris :( */

#define IS_LCD_DISPLAY	(p->MtxOrb_type == MTXORB_LCD)
#define IS_LKD_DISPLAY	(p->MtxOrb_type == MTXORB_LKD)
#define IS_VFD_DISPLAY	(p->MtxOrb_type == MTXORB_VFD)
#define IS_VKD_DISPLAY	(p->MtxOrb_type == MTXORB_VKD)

/*
 * The MtxOrb driver do not use a lot of hardware feature.
 * We try to replace them by more flexible software version.
 * That's why vbar/hbar/bignum are using Matrix Orbital build-in function.
 * It permit simultanious use of those features and custom char.
 *
 * The same way we don't use the hardware clear but empty the frame buffer.
 * The frame buffer hold all the change that are requested,
 * until the server ask us to flush that to display.
 * This also permit to do incremental update and reduce the number of
 * character to be send to the display accross the serial link.
 *
 * In order to display graphic widget we use and define our own custom char.
 * To avoid multiple definition of the same custom character,
 * we use a caching mechanisme that remember what is currently define.
 * In order to avoid always redefining the same custom character
 * wich are at the begining of the table, we rotate the begining of the table.
 * This is suppose to reduce the number of caracter redefinition
 * and make the caching more effective. All in all we reduce the number
 * of caracter we need to send to the display.
 *
 * David GLAUDE
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
	char left_key;
	char right_key;
	char up_key;
	char down_key;
	char enter_key;
	char escape_key;
	int keypad_test_mode;
        int cellwidth;
	int cellheight;
	char info[255];		/* static data from MtxOrb_get_info */
/*	LibData *libdata;		*/
/* Private Data of the new library: Work in progress */
        } PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "MtxOrb_";

static int  MtxOrb_ask_bar (Driver *drvthis, int type);
static void MtxOrb_set_known_char (Driver * drvthis, int car, int type);
static void MtxOrb_linewrap (Driver *drvthis, int on);
static void MtxOrb_autoscroll (Driver *drvthis, int on);
static void MtxOrb_cursorblink (Driver *drvthis, int on);
static void MtxOrb_mold_vbar (Driver *drvthis, int x, int y, int len);
static void MtxOrb_old_hbar (Driver *drvthis, int x, int y, int len);


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
MtxOrb_init (Driver *drvthis)
{
	struct termios portset;

	int contrast = DEFAULT_CONTRAST;
	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	char buf[256] = "";
	int tmp, w, h;

        PrivateData *p;

	/* Alocate and store private data */
        p = (PrivateData *) malloc( sizeof( PrivateData) );
	if( ! p )
	        return -1;
	if( drvthis->store_private_ptr( drvthis, p ) )
	        return -1;

	/* Initialise the PrivateData structure */
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
	p->left_key = MTXORB_DEFAULT_Left;
	p->right_key = MTXORB_DEFAULT_Right;
	p->up_key = MTXORB_DEFAULT_Up;
	p->down_key = MTXORB_DEFAULT_Down;
	p->enter_key = MTXORB_DEFAULT_Enter;
	p->escape_key = MTXORB_DEFAULT_Escape;
	p->keypad_test_mode = 0;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	debug( RPT_INFO, "MtxOrb: init(%p)", drvthis );

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
	if (0<=drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) && drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST) <= 1000) {
		contrast = drvthis->config_get_int ( drvthis->name , "Contrast" , 0 , DEFAULT_CONTRAST);
	} else {
		report (RPT_WARNING, "MtxOrb: Contrast must be between 0 and 1000. Using default value.");
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
	if(drvthis->config_get_bool( drvthis->name , "Backlight" , 0 , DEFAULT_BACKLIGHT)) {
		p->backlightenabled = 1;
	}

	/* Get display type */
	strncpy(buf, drvthis->config_get_string ( drvthis->name , "Type" , 0 , DEFAULT_TYPE),sizeof(buf));
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
		fprintf( stdout, "MtxOrb: Entering keypad test mode...\n");
		p->keypad_test_mode = 1;
		stay_in_foreground = 1;
	}

	if (!p->keypad_test_mode) {
		/* We don't send any chars to the server in keypad test mode.
		 * So there's no need to get them from the configfile in keypad
		 * test mode.
		 */

		/* left_key */
		p->left_key = MtxOrb_parse_keypad_setting (drvthis, "LeftKey", MTXORB_DEFAULT_Left);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as Leftkey.", p->left_key);
		
		/* right_key */
		p->right_key = MtxOrb_parse_keypad_setting (drvthis, "RightKey", MTXORB_DEFAULT_Right);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as RightKey.", p->right_key);
		
		/* up_key */
		p->up_key = MtxOrb_parse_keypad_setting (drvthis, "UpKey", MTXORB_DEFAULT_Up);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as UpKey.", p->up_key);
		
		/* down_key */
		p->down_key = MtxOrb_parse_keypad_setting (drvthis, "DownKey", MTXORB_DEFAULT_Down);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as DownKey.", p->down_key);
		
		/* right_key */
		p->enter_key = MtxOrb_parse_keypad_setting (drvthis, "EnterKey", MTXORB_DEFAULT_Enter);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as EnterKey.", p->enter_key);
		
		/* escape_key */
		p->escape_key = MtxOrb_parse_keypad_setting (drvthis, "EscapeKey", MTXORB_DEFAULT_Escape);
		report (RPT_DEBUG, "MtxOrb: Using \"%c\" as EscapeKey.", p->escape_key);
		
	}
	/* End of config file parsing*/

	/* Set up io port correctly, and open it... */
	p->fd = open (device, O_RDWR | O_NOCTTY);
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

	// THIS ALL COMMENTED OUT BECAUSE WE NEED TO SET TIMEOUTS
	/* We use RAW mode */
#ifdef HAVE_CFMAKERAW_NOT
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
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;
#endif

	/* Set port speed */
	cfsetospeed (&portset, speed);
	cfsetispeed (&portset, B0);

	/* Do it... */
	tcsetattr (p->fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	if (!p->framebuf)
		p->framebuf = (char *)
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


/******************************
 * Clear the screen (the frame buffer)
 */
MODULE_EXPORT void
MtxOrb_clear (Driver *drvthis)
{
        PrivateData * p = drvthis->private_data;

	if (p->framebuf != NULL)
		memset (p->framebuf, ' ', (p->widthBYheight));
	p->clear = 1; /* Remember that custom char are no more visible. */

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

/******************************
 * Display a string at x,y
 */
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

/******************************
 * Send what we have to the hardware
 */
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


/******************************
 * Returns string with general information about the display
 */
MODULE_EXPORT char *
MtxOrb_get_info (Driver *drvthis)
{
	char in = 0;
	char tmp[255], buf[64];
	/* int i = 0; */
        PrivateData * p = drvthis->private_data;

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
				case '\x05': strcat(p->info, "LCD2041 "); break;
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
				case '\x12': strcat(p->info, "GLC128128 "); break;
				case '\x13': strcat(p->info, "GLC24064 "); break;
				case '\x14': strcat(p->info, "GLK12864-25 "); break;
				case '\x15': strcat(p->info, "GLK24064-25 "); break;
				case '\x21': strcat(p->info, "GLK128128-25 "); break;
				case '\x22': strcat(p->info, "GLK12232-25 "); break;
				case '\x31': strcat(p->info, "LK404-AT "); break;
				case '\x32': strcat(p->info, "VFD1621 "); break;
				case '\x33': strcat(p->info, "LK402-12 "); break;
				case '\x34': strcat(p->info, "LK162-12 "); break;
				case '\x35': strcat(p->info, "LK204-25PC "); break;
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
		if (read (p->fd, &tmp, 2) < 0) {
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
		if (read (p->fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Firmware Rev. %ld ", (long int) tmp);
			strcat(p->info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device firmware revision");

	return p->info;
}

/* TODO: Finish the support for bar growing reverse way.
 * TODO: Migrate to the new vbar.
 ******************************
 * Draws a vertical bar...
 * This is the new version using dynamic icon alocation
 */
static void MtxOrb_mold_vbar (Driver *drvthis, int x, int y, int len)
{
	/* baru5 = bigfonte ... The cache can benefit from it. */
	unsigned char mapu[9] =
	{ barw, baru1, baru2, baru3, baru4, bigfonte, baru6, baru7, barb };
	/* bard5 = bigfontf ... The cache can benefit from it. */
/*
	unsigned char mapd[9] =
	{ barw, bard1, bard2, bard3, bard4, bigfontf, bard6, bard7, barb };
*/


        PrivateData * p = drvthis->private_data;

	debug(RPT_DEBUG, "MtxOrb: vertical bar at %d set to %d", x, len);

/* REMOVE THE NEXT LINE FOR TESTING ONLY... */
/* len=-len; */ /* This is to test negative len. */
/* REMOVE THE PREVIOUS LINE FOR TESTING ONLY... */

	if (len > 0) {
		for (; y > 0 && len > 0; y--) {
			if (len >= p->cellheight)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapu[len]);

			len -= p->cellheight;
		}
/*
	} else {
		len = -len;
		for (y = 2; y <= p->height && len > 0; y++) {
			if (len >= p->cellheight)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapd[len]);

			len -= p->cellheight;
		}
*/
	}

}


/* TODO: Finish the support for bar growing reverse way.
 * TODO: Migrate to the new hbar.
 ******************************
 * Draws a horizontal bar to the right.
 * This is the new version ussing dynamic icon alocation
 */
static void MtxOrb_old_hbar (Driver *drvthis, int x, int y, int len)
{
	unsigned char mapr[6] = { barw, barr1, barr2, barr3, barr4, barb };
/*
	unsigned char mapl[6] = { barw, barl1, barl2, barl3, barl4, barb };
*/

        PrivateData * p = drvthis->private_data;

	ValidX(x);
	ValidY(y);

	debug(RPT_DEBUG, "MtxOrb: horizontal bar at %d set to %d", x, len);

	if (len > 0) {
		for (; x <= p->width && len > 0; x++) {
			if (len >= p->cellwidth)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapr[len]);

			len -= p->cellwidth;

		}
/*
	} else {
		len = -len;
		for (; x > 0 && len > 0; x--) {
			if (len >= p->cellwidth)
				MtxOrb_icon (drvthis, x, y, barb);
			else
				MtxOrb_icon (drvthis, x, y, mapl[len]);

			len -= p->cellwidth;

		}
*/
	}

}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
MtxOrb_vbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000)
	 * that the bar should be filled.
	 */

        PrivateData * p = drvthis->private_data;
	int total_pixels  = ((long) 2 * len * p->cellwidth + 1 ) * promille / 2000;

	/*
	 * This function does not fully implement API V0.5
	 * It is an adaptation layer using the API V0.4 function.
	 * Ideally it should use the library but then the library
	 * need to be enhanced with the idea privately exchange
	 * between David & Joris.
	 */

	MtxOrb_mold_vbar (drvthis, x, y, total_pixels);
	// this is what's in the library for it:
	// lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
MtxOrb_hbar (Driver * drvthis, int x, int y, int len, int promille, int options)
{
	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000)
	 * that the bar should be filled.
	 */

        PrivateData * p = drvthis->private_data;
	int total_pixels  = ((long) 2 * len * p->cellwidth + 1 ) * promille / 2000;

	/*
	 * This function does not fully implement API V0.5
	 * It is an adaptation layer using the API V0.4 function.
	 * Ideally it should use the library but then the library
	 * need to be enhanced with the idea privately exchange
	 * between David & Joris.
	 */

	MtxOrb_old_hbar (drvthis, x, y, total_pixels);
	// this is what's in the library for it:
	// lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


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

	for (row = 0; row < p->cellheight; row++) {
		letter = 0;
		for (col = 0; col < p->cellwidth; col++) {
			/* shift to make room for new scan line data */
			letter <<= 1;
			/* Now read a single bit of data
			 * -- one entry in dat[] --
			 * and add it to the binary data in "letter"
			 */
			letter |= (dat[(row * p->cellwidth) + col] > 0);
		}
		write (p->fd, &letter, 1); /* write one character for each row */
	}
}


/* TODO: This is not yet my idea of icon frame buffer but it work well.
 */
MODULE_EXPORT int
MtxOrb_icon (Driver *drvthis, int x, int y, int icon)
{
	return -1;
	/* Does not use current icon definitions */

	MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, icon));

}

/* TODO: Recover the code for I2C connectivity to MtxOrb
 * and don't query the LCD if it does not support keypad.
 * Otherwise crash of the LCD and/or I2C bus.
 *
 ******************************
 * returns a string for the function characters from the keypad...
 * (A-Z) on success, 0 on failure...
 */

/*
 * Return one char from the KeyRing
 */
MODULE_EXPORT const char *
MtxOrb_get_key (Driver *drvthis)
{
	char in = 0;

    PrivateData * p = drvthis->private_data;

	// POLL For data or return
	struct pollfd fds[1];
	fds[0].fd = p->fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	poll (fds,1,0);
	if (fds[0].revents == 0) { return NULL; }

	(void) read (p->fd, &in, 1);
	report(RPT_INFO, "MtxOrb: get_key: key X %i", in);

	if ( 0 == in ) {
		debug( RPT_INFO, "MtxOrb_get_key: in=>%d\n", in );
		return NULL;
	}

	if (!p->keypad_test_mode) {
	        if (in == p->left_key)
			return "Left";
		else if (in == p->right_key)
			return "Right";
		else if (in == p->up_key)
			return "Up";
		else if (in == p->down_key)
			return "Down";
		else if (in == p->enter_key)
			return "Enter"; 
		else if (in == p->escape_key)
			return "Escape";
		else {
        		report( RPT_INFO, "MtxOrb Untreated key 0x%2x", in);
			return NULL;
	        }
	} else {
		fprintf (stdout, "MtxOrb: Received character %c\n", in);
		fprintf (stdout, "MtxOrb: Press another key of your device.\n");
	}
	return NULL;
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


