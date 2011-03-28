/** \file server/drivers/EyeboxOne.c
 * LCDd \c EyeboxOne driver for the display used in the Eyebox ONE server
 * by RightVision.
 */

/*  This is the LCDproc driver for EyeboxOne devices

    This code is based on MtxOrb driver (William Ferrell and Selene Scriven)

    Copyright (C) 2006 Cedric TESSIER (aka NeZetiC) http://www.nezetic.info

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
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <poll.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "EyeboxOne.h"

#include "report.h"

#define INPUT_PAUSE_KEY         'A'
#define INPUT_BACK_KEY          'B'
#define INPUT_FORWARD_KEY       'D'
#define INPUT_MAIN_MENU_KEY     'C'
#define EYEBOXONE_DEFAULT_Left     'D'
#define EYEBOXONE_DEFAULT_Right    'C'
#define EYEBOXONE_DEFAULT_Up       'A'
#define EYEBOXONE_DEFAULT_Down     'B'
#define EYEBOXONE_DEFAULT_Enter    '\r'
#define EYEBOXONE_DEFAULT_Escape   'P'

#define DEFAULT_SIZE "20x4"
#define DEFAULT_BACKLIGHT 1

#define LED_OFF 0
#define LED_RED 1
#define LED_ORANGE 2
#define LED_GREEN 3

/** private data for the \c EyeBoxOne driver */
typedef struct EyeboxOne_private_data {
	int def[9];
	int use[9];
	int backlight_state;	/**< static data from EyeboxOne_backlight */
	int width;		/**< display width in characters */
	int height;		/**< display height in characters */
	char *framebuf;		/**< frame buffer */
	char *old;		/**< current on screen frame buffer */
	int widthBYheight;	/**< avoid computing width * height frequently */
	int clear;		/**< control when the LCD is cleared */
	int fd;			/**< display file descriptor */
	int backlightenabled;
	int cursorenabled;
	char left_key;
	char right_key;
	char up_key;
	char down_key;
	char enter_key;
	char escape_key;
	int keypad_test_mode;
	int cellwidth;		/**< character cell width */
	int cellheight;		/**< character cell height */
	char info[255]; 	/**< info string contents */
	/*	LibData *libdata;		*/
	/* Private Data of the new library: Work in progress */
} PrivateData;

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "EyeboxOne_";

static void EyeboxOne_showcursor (Driver *drvthis, int on);
static void EyeboxOne_use_bar(int fd, int bar, int level);
static void EyeboxOne_use_led(int fd, int led, int color);

/* Parse one key from the configfile */
static char
EyeboxOne_parse_keypad_setting (Driver *drvthis, char * keyname, char default_value)
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

/* Opens com port and sets baud correctly...
 *
 * Called to initialize driver settings
 */
MODULE_EXPORT int
EyeboxOne_init (Driver *drvthis)
{
	struct termios portset;

	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char size[256] = DEFAULT_SIZE;
	int tmp, w, h;

	PrivateData *p;

	/* Alocate and store private data */
	p = (PrivateData *) malloc(sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialise the PrivateData structure */
	memset(p->def, -1, sizeof(p->def));
	memset(p->use,  0, sizeof(p->use));
	p->fd = -1;
	p->backlight_state = 2; /* static data from EyeboxOne_backlight */
	p->width = LCD_DEFAULT_WIDTH;
	p->height = LCD_DEFAULT_HEIGHT;
	p->widthBYheight = LCD_DEFAULT_WIDTH * LCD_DEFAULT_HEIGHT;
	p->clear = 1;		/* assume LCD is cleared at startup */
	p->framebuf = NULL;
	p->backlightenabled = DEFAULT_BACKLIGHT;
	p->cursorenabled = DEFAULT_CURSOR;
	p->old = NULL;
	p->left_key = EYEBOXONE_DEFAULT_Left;
	p->right_key = EYEBOXONE_DEFAULT_Right;
	p->up_key = EYEBOXONE_DEFAULT_Up;
	p->down_key = EYEBOXONE_DEFAULT_Down;
	p->enter_key = EYEBOXONE_DEFAULT_Enter;
	p->escape_key = EYEBOXONE_DEFAULT_Escape;
	p->keypad_test_mode = 0;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;

	debug(RPT_INFO, "EyeBO: init(%p)", drvthis);

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
	p->widthBYheight = w * h;

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

	/* Get backlight setting*/
	p->backlightenabled = drvthis->config_get_bool(drvthis->name, "Backlight", 0, DEFAULT_BACKLIGHT);

	/* Get backlight setting*/
	p->cursorenabled = drvthis->config_get_bool(drvthis->name, "Cursor", 0, DEFAULT_CURSOR);


	/* Get keypad settings*/

	/* keypad test mode? */
	if (drvthis->config_get_bool(drvthis->name, "keypad_test_mode", 0, 0)) {
		fprintf(stdout, "EyeBO: Entering keypad test mode...\n");
		p->keypad_test_mode = 1;
		stay_in_foreground = 1;
	}

	if (!p->keypad_test_mode) {
		/* We don't send any chars to the server in keypad test mode.
		 * So there's no need to get them from the configfile in keypad
		 * test mode.
		 */

		/* left_key */
		p->left_key = EyeboxOne_parse_keypad_setting(drvthis, "LeftKey", EYEBOXONE_DEFAULT_Left);
		report(RPT_DEBUG, "%s: Using \"%c\" as Leftkey", drvthis->name, p->left_key);

		/* right_key */
		p->right_key = EyeboxOne_parse_keypad_setting(drvthis, "RightKey", EYEBOXONE_DEFAULT_Right);
		report(RPT_DEBUG, "%s: Using \"%c\" as RightKey", drvthis->name, p->right_key);

		/* up_key */
		p->up_key = EyeboxOne_parse_keypad_setting(drvthis, "UpKey", EYEBOXONE_DEFAULT_Up);
		report(RPT_DEBUG, "%s: Using \"%c\" as UpKey", drvthis->name, p->up_key);

		/* down_key */
		p->down_key = EyeboxOne_parse_keypad_setting(drvthis, "DownKey", EYEBOXONE_DEFAULT_Down);
		report(RPT_DEBUG, "%s: Using \"%c\" as DownKey", drvthis->name, p->down_key);

		/* right_key */
		p->enter_key = EyeboxOne_parse_keypad_setting(drvthis, "EnterKey", EYEBOXONE_DEFAULT_Enter);
		report(RPT_DEBUG, "%s: Using \"%c\" as EnterKey", drvthis->name, p->enter_key);

		/* escape_key */
		p->escape_key = EyeboxOne_parse_keypad_setting(drvthis, "EscapeKey", EYEBOXONE_DEFAULT_Escape);
		report(RPT_DEBUG, "%s: Using \"%c\" as EscapeKey", drvthis->name, p->escape_key);

	}
	/* End of config file parsing*/

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
	/* Set timeouts */
	portset.c_cc[VMIN] = 1;
	portset.c_cc[VTIME] = 3;

	/* Set port speed */
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, B0);

	/* Do it... */
	tcsetattr(p->fd, TCSANOW, &portset);

	/* Make sure the frame buffer is there... */
	p->framebuf = (char *) calloc(p->widthBYheight, 1);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
		return -1;
	}
	memset(p->framebuf, ' ', p->widthBYheight);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}

#define ValidX(x) if ((x) > p->width) { (x) = p->width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > p->height) { (y) = p->height; } else (y) = (y) < 1 ? 1 : (y);


/********************
 * Use bars
 */
static void
EyeboxOne_use_bar(int fd, int bar, int level)
{
	char buffer[16];

	if(bar > 2 || bar < 1)
		return;

	if(level > 10 || level < 0)
		return;

	sprintf(buffer, "\E[%d;%dB",bar,level);
	write(fd,buffer,strlen(buffer));

}

/********************
 * Use leds
 */
static void
EyeboxOne_use_led(int fd, int led, int color)
{
	char buffer[16];
	int a,b;

	if(led > 3 || led < 1)
		return;

	switch(color){
		case LED_OFF:
			a=b=0;
			break;
		case LED_RED:
			a=1;
			b=0;
			break;
		case LED_ORANGE:
			a=1;
			b=1;
			break;
		case LED_GREEN:
			a=0;
			b=1;
			break;
		default:
			a=b=0;
	}

	sprintf(buffer, "\E[%d;%dL",led+(led-1),a);
	write(fd,buffer,strlen(buffer));
	sprintf(buffer, "\E[%d;%dL",led+(led-1)+1,b);
	write(fd,buffer,strlen(buffer));

}

/******************************
 * Clear the screen (the frame buffer)
 */
MODULE_EXPORT void
EyeboxOne_clear (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;

	if (p->framebuf != NULL)
		memset(p->framebuf, ' ', (p->widthBYheight));
	p->clear = 1; /* Remember that custom char are no more visible. */

	debug(RPT_DEBUG, "EyeBO: cleared screen");
}

/******************************
 * Clean-up
 */
MODULE_EXPORT void
EyeboxOne_close (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;
	/* Clear bars && leds before exit */
	EyeboxOne_use_bar(p->fd,1,0);
	EyeboxOne_use_bar(p->fd,2,0);
	EyeboxOne_use_led(p->fd,1,LED_OFF);
	EyeboxOne_use_led(p->fd,2,LED_OFF);
	EyeboxOne_use_led(p->fd,3,LED_OFF);

	debug(RPT_DEBUG, "EyeBO: cleared bars and leds");

	if (p != NULL) {
		if (p->fd >= 0)
			close(p->fd);

		if (p->framebuf)
			free(p->framebuf);
		p->framebuf = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
	debug(RPT_DEBUG, "EyeBO: closed");
}

/******************************
 * Returns the display width
 */
MODULE_EXPORT int
EyeboxOne_width (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;

	return p->width;
}

/******************************
 * Returns the display height
 */
MODULE_EXPORT int
EyeboxOne_height (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;

	return p->height;
}

/******************************
 * Display a string at x,y
 */
MODULE_EXPORT void
EyeboxOne_string (Driver *drvthis, int x, int y, const char string[])
{
	int offset, siz;
	int bar,level;

	PrivateData * p = drvthis->private_data;
	/*
	 * /xBab = Use Bar
	 * a = Bar ID
	 * b = Level
	 */
	if(strncmp(string,"/xB",3) == 0){
		bar = (int) (string[3] - 48);
		level = (int) (string[4] - 48);
		if(level == 1)
			if(strlen(string) > 5)
				if(((int) (string[5] - 48)) == 0)
					level = 10;
		EyeboxOne_use_bar(p->fd, bar, level);
		report(RPT_DEBUG, "EyeBO: Changed bar %d to level %d", bar, level);
		return;
	}

	ValidX(x);
	ValidY(y);

	x--; y--;
	offset = (y * p->width) + x;
	siz = (p->widthBYheight) - offset;
	siz = (siz > strlen(string)) ? strlen(string) : siz;
	memcpy(p->framebuf + offset, string, siz);

	debug(RPT_DEBUG, "EyeBO: printed string at (%d,%d)", x, y);
}

/******************************
 * Send what we have to the hardware
 */
MODULE_EXPORT void
EyeboxOne_flush (Driver *drvthis)
{
	char out[12];
	int i,j,mv = 1;
	char *xp, *xq;

	PrivateData * p = drvthis->private_data;

	if (p->old == NULL) {
		p->old = malloc(p->widthBYheight);

		write(p->fd, "\E[H\E[2J", 7); // Clear Screen
		EyeboxOne_showcursor(drvthis, p->cursorenabled);
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
				if (mv == 1) {
					snprintf(out, sizeof(out), "\E[%d;%dH", j-1, i);
					write(p->fd, out, strlen(out));
					mv = 0;
				}
				write(p->fd, xp, 1);
			}
			xp++;
			xq++;
		}
	}

	strncpy(p->old, p->framebuf, p->widthBYheight);
	debug(RPT_DEBUG, "EyeBO: frame buffer flushed");
}

/******************************
 * Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
EyeboxOne_chr (Driver *drvthis, int x, int y, char c)
{
	int offset;

	PrivateData * p = drvthis->private_data;

	ValidX(x);
	ValidY(y);

	y--; x--;
	offset = (y * p->width) + x;
	p->framebuf[offset] = c;

	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
}

/******************************
 * Sets the backlight on or off
 */
MODULE_EXPORT void
EyeboxOne_backlight (Driver *drvthis, int on)
{
	PrivateData * p = drvthis->private_data;
	/* Disable in LCDd.conf
	 * Force Off mode */
	if(p->backlight_state == -1)
		return;

	if (p->backlight_state == on)
		return;

	p->backlight_state = on;

	if(!p->backlightenabled){ /* Disable in LCDd.conf */
		p->backlight_state = -1;
		on = 0;
	}

	switch (on) {
		case BACKLIGHT_ON:
			write(p->fd, "\E[E", 3);
			debug(RPT_DEBUG, "EyeBO: backlight turned on");
			break;
		case BACKLIGHT_OFF:
			debug(RPT_DEBUG, "EyeBO: backlight turned off");
			write(p->fd, "\E[e", 3);
			break;
		default: /* ignored... */
			debug(RPT_DEBUG, "EyeBO: backlight - invalid setting");
			break;
	}
}

/**********************
 * Toggle cursor on/off
 */
static void
EyeboxOne_showcursor (Driver *drvthis, int on)
{
	PrivateData * p = drvthis->private_data;

	if (on) {
		write(p->fd, "\E[S", 3);
		debug(RPT_DEBUG, "EyeBO: cursor turned on");
	} else {
		write(p->fd, "\E[s", 3);
		debug(RPT_DEBUG, "EyeBO: cursor turned off");
	}
}

/*****************************************************************
 * returns a string for the function characters from the keypad...
 * (A-Z) on success, 0 on failure...
 */
MODULE_EXPORT const char *
EyeboxOne_get_key (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;

	char in = 0;

	// POLL For data or return
	struct pollfd fds[1];
	fds[0].fd = p->fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	poll(fds,1,0);
	if (fds[0].revents == 0)
		return NULL;

	(void) read(p->fd, &in, 1);
	report(RPT_DEBUG, "%s: get_key: key 0x%02X", drvthis->name, in);
	/* There is a lot of undesirable chars which appear
	 * when a key is pressed
	 * */
	if (in == '\0' || (int) in == 19 || (int) in == 91 || (int) in == 79 || (int) in == 27)
		return NULL;

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
			//report(RPT_INFO, "%s: untreated key 0x%02X", drvthis->name, in);
			return NULL;
		}
	}
	else {
		fprintf(stdout, "EyeBO: Received character %c (%d)\n", in, (int)in);
		fprintf(stdout, "EyeBO: Press another key of your device.\n");
	}
	return NULL;
}

/******************************
 * Returns string with general information about the display
 * I think lcd is able to tell us more, but I can't find how...
 */
MODULE_EXPORT const char *
EyeboxOne_get_info (Driver *drvthis)
{
	PrivateData * p = drvthis->private_data;

	debug(RPT_DEBUG, "EyeBO: get_info");

	memset(p->info, '\0', sizeof(p->info));
	strcpy(p->info, "Eyebox Driver ");
	strcat(p->info, "(c) NeZetiC (nezetic.info)");

	return p->info;
}

