/* #define COUNT -1 */

/*
List of driver entry point:
   
init		Work in progress...
close		Implemented.
width		If this is a variable, then implemented.
height		If this is a variable, then implemented.
clear		Implemented by space filling no custom char info.
flush		Calling draw_frame only.
string		Implemented.
chr		Implemented.
vbar		.
hbar		.
num		.
heartbeat	Implemented for testing only.
icon		.
cursor		NOT IMPLEMENTED: Is it really used?
set_char	.
get_free_chars	NOT IMPLEMENTED: Custom char should not be exported.
cellwidth	If this is a variable, then implemented.
cellheight	If this is a variable, then implemented.
get_contrast	Not implemented, no software control.
set_contrast	Not implemented, no software control.
get_brightness	Implemented, still need flush code.
set_brightness	Implemented, still need flush code.
backlight	Implemented, still need flush code.
output		.
get_key		Implemented like in CF633, need cleaning in init.
get_info	.

*/

/*  This is the LCDproc driver for Cwlinux devices (http://www.cwlinux.com)

        Copyright (C) 2002, Andrew Ip
                      2003, David Glaude

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
#include "CwLnx.h"
#include "report.h"
#include "lcd_lib.h"

#define ValidX(x) if ((x) > p->width) { (x) = p->width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > p->height) { (y) = p->height; } else (y) = (y) < 1 ? 1 : (y);

#define MaxKeyMap 6

static char *defaultKeyMap[MaxKeyMap] = { "Up", "Down", "Left", "Right", "Enter", "Escape" };

typedef enum {
    hbar = 1,
    vbar = 2,
    bign = 4,
} custom_type;

typedef struct p {
	int custom;
	int fd;

	int have_keypad;
	int keypad_test_mode;
        char *KeyMap[MaxKeyMap];

/*I*/ int width;
/*I*/ int height;

/*I*/ int cellwidth;
/*I*/ int cellheight;

/*I*/ char *backingstore;
/*I*/ char *framebuf;					/* Frame buffer */

/*I*/ char saved_backlight; /* current state of the backlight */
/*I*/ char backlight; /* state of the backlight at next flush */

/*I*/ int saved_brightness; /* brightness as displayed on the LCD currently */
/*I*/ int brightness; /* brightness as it will be displayed at next flush */

/*I*/ int saved_heartbeat; /* heartbeat as displayed on the LCD currently */
/*I*/ int heartbeat; /* heartbeat as it will be displayed at next flush */

/*I*/ int heartbeat_state; /* toggle to remember when we turn on or off */

} PrivateData;

/* API: Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0; /* For testing only */
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "CwLnx_";


static void CwLnx_linewrap(int fd, int on);
static void CwLnx_autoscroll(int fd, int on);
static void CwLnx_hidecursor(int fd);

void CwLnx_draw_frame(Driver *drvthis, char *dat);

#define LCD_CMD			254
#define LCD_CMD_END		253
#define LCD_INIT_CHINESE_T	56
#define LCD_INIT_CHINESE_S	55
#define LCD_LIGHT_ON		66
#define LCD_LIGHT_OFF		70
#define LCD_LIGHT_BRIGHTNESS	65
#define LCD_CLEAR		88
#define LCD_SET_INSERT		71
#define LCD_INIT_INSERT		72
#define LCD_SET_BAUD		57
#define LCD_ENABLE_WRAP		67
#define LCD_DISABLE_WRAP	68
#define LCD_SETCHAR		78
#define LCD_ENABLE_SCROLL	81
#define LCD_DISABLE_SCROLL	82
#define LCD_OFF_CURSOR		72

#define LCD_PUT_PIXEL		112
#define LCD_CLEAR_PIXEL		113

#define LCD_LENGTH		20

#define DELAY			20
#define UPDATE_DELAY		0	/* 1 sec */
#define SETUP_DELAY		1	/* 2 sec */

/* Parse one key from the configfile */
static char CwLnx_parse_keypad_setting (Driver *drvthis, char * keyname, char default_value)
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

int Read_LCD(int fd, char *c, int size)
{
    int rc;
    rc = read(fd, c, size);
/*    usleep(DELAY); */
    return rc;
}

int Write_LCD(int fd, char *c, int size)
{
    int rc;
    rc = write(fd, c, size);
/* Debuging code to be cleaned when very stable */
/* 
    if (size==1) {
	    if (*c>=0) 
		    printf("%3d ", *c);
          else
              {
		    if (*c+256==254)

		    printf("\n%3d ", *c+256);
		    else printf("%3d ", *c+256);
	      }
    }
*/
/*    usleep(DELAY); */
    return rc;
}




/*********************************************
 * Real hardware function.
 * Will be called by API function.
 */


/* Hardware function */
void Enable_Backlight(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_LIGHT_ON;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Disable_Backlight(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_LIGHT_OFF;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Enable_Pixel(int fd, int x, int y)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_PUT_PIXEL;
    rc = Write_LCD(fd, &c, 1);

    c = x;
    rc = Write_LCD(fd, &c, 1);
    c = y;
    rc = Write_LCD(fd, &c, 1);

    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Disable_Pixel(int fd, int x, int y)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CLEAR_PIXEL;
    rc = Write_LCD(fd, &c, 1);

    c = x;
    rc = Write_LCD(fd, &c, 1);
    c = y;
    rc = Write_LCD(fd, &c, 1);

    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}


/* Hardware function */
void Backlight_Brightness(int fd, int brightness)
{
    char c;
    int rc;

    if (brightness == 1) {
	Disable_Backlight(fd);
    } else if (brightness == 7) {
	Enable_Backlight(fd);
    } else {
	c = LCD_CMD;
	rc = Write_LCD(fd, &c, 1);
	c = LCD_LIGHT_BRIGHTNESS;
	rc = Write_LCD(fd, &c, 1);
	c = (char) brightness;
	rc = Write_LCD(fd, &c, 1);
	c = LCD_CMD_END;
	rc = Write_LCD(fd, &c, 1);
    }
}

/* Hardware function */
void Enable_Scroll(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_ENABLE_SCROLL;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Disable_Scroll(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_DISABLE_SCROLL;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}


/* Hardware function */
void Clear_Screen(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CLEAR;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
    usleep(UPDATE_DELAY);
}

/* Hardware function */
void Enable_Wrap(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_ENABLE_WRAP;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Disable_Wrap(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_DISABLE_WRAP;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Disable_Cursor(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_OFF_CURSOR;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}


/* Hardware function */
void Init_Port(fd)
{
    /* Posix - set baudrate to 0 and back */
    struct termios tty, old;

    tcgetattr(fd, &tty);
    tcgetattr(fd, &old);
    cfsetospeed(&tty, B0);
    cfsetispeed(&tty, B0);
    tcsetattr(fd, TCSANOW, &tty);
    usleep(SETUP_DELAY);
    tcsetattr(fd, TCSANOW, &old);
}

/* Hardware function */
void Setup_Port(int fd, speed_t speed)
{
    struct termios portset;

    tcgetattr(fd, &portset);
    cfsetospeed(&portset, speed);
    cfsetispeed(&portset, speed);
    portset.c_iflag = IGNBRK;
    portset.c_lflag = 0;
    portset.c_oflag = 0;
    portset.c_cflag |= CLOCAL | CREAD;
    portset.c_cflag &= ~CRTSCTS;
    portset.c_cc[VMIN] = 1;
    portset.c_cc[VTIME] = 5;
    tcsetattr(fd, TCSANOW, &portset);
}

/* Hardware function */
void Set_9600(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_SET_BAUD;
    rc = Write_LCD(fd, &c, 1);
    c = 0x20;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/* Hardware function */
void Set_19200(int fd)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_SET_BAUD;
    rc = Write_LCD(fd, &c, 1);
    c = 0xf;
    rc = Write_LCD(fd, &c, 1);
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}


/*
 * ONLY USED BY flush_box
int Write_Line_LCD(int fd, char *buf)
{
    int i;
    char c;
    int isEnd = 0;
    int rc;

    for (i = 0; i < LCD_LENGTH; i++) {
	if (buf[i] == '\0') {
	    isEnd = 1;
	}
	if (isEnd) {
	    c = ' ';
	} else {
	    c = buf[i];
	}
	rc = Write_LCD(fd, &c, 1);
    }
*/
/*    printf("%s\n", buf); */
/*
    return 0;
}
*/


/*****************************************************
 * Here start the API function
 */

/*****************************************************
 * API: Opens com port and sets baud correctly...
 */
int CwLnx_init(Driver * drvthis, char *args)
{
    struct termios portset_save;

    char device[200] = DEFAULT_DEVICE;
    int speed = DEFAULT_SPEED;
    char size[200] = DEFAULT_SIZE;

    char buf[256] = "";
    int tmp;
    int w;
    int h;
    char *s;

    PrivateData *p;

    /* Alocate and store private data */
    p = (PrivateData *) malloc( sizeof( PrivateData) );
    if( ! p )
        return -1;
    if( drvthis->store_private_ptr( drvthis, p ) )
        return -1;

    /* Initialise the PrivateData structure */

    p->framebuf = NULL;

/* height and width are computed from DEFAULT_SIZE */
/*    p->width = DEFAULT_WIDTH;	*/
/*    p->height = DEFAULT_HEIGHT; */
    p->cellwidth = DEFAULT_CELLWIDTH;
    p->cellheight = DEFAULT_CELLHEIGHT;

    p->saved_backlight = -1;
    p->backlight = DEFAULT_BACKLIGHT;

    p->saved_brightness = -1;
    p->brightness = DEFAULT_BRIGHTNESS;

    p->heartbeat_state = 0;
    p->saved_heartbeat = -1;
    p->heartbeat = 0;

    debug(RPT_INFO, "CwLnx: init(%p,%s)", drvthis, args);

    /* Read config file */

    
    /* Which serial device should be used */
    strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
    device[sizeof(device) - 1] = 0;
    report(RPT_INFO, "CwLnx: Using device: %s", device);

    /* Which size */
    strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, DEFAULT_SIZE), sizeof(size));
    size[sizeof(size) - 1] = 0;
    if ((sscanf(size, "%dx%d", &w, &h) != 2)
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
	report(RPT_WARNING, "CwLnx: Cannot read size: %s. Using default value.\n", size);
	sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
    }
    p->width = w;
    p->height = h;

    /* Contrast of the LCD can be change by adjusting the trimpot R7  */

    /* Which speed */
    tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, DEFAULT_SPEED);

    switch (tmp) {
    case 9600:
	speed = B9600;
	break;
    case 19200:
	speed = B19200;
	break;
    default:
	speed = DEFAULT_SPEED;

	switch (speed) {
	case B9600:
	    strncpy(buf, "9600", sizeof(buf));
	    break;
	case B19200:
	    strncpy(buf, "19200", sizeof(buf));
	    break;
	}

	report(RPT_WARNING,
	       "CwLnx: Speed must be 9600 or 19200. Using default value of %s baud!",
	       buf);
	strncpy(buf, "", sizeof(buf));
    }


    /* do we have a keypad? */
    if (drvthis->config_get_bool( drvthis->name , "Keypad", 0, 0)) {
	report (RPT_INFO, "CwLnx: Config file tell us we have a keypad...\n");
	p->have_keypad = 1;
    }

    /* keypad test mode? */
    if (drvthis->config_get_bool( drvthis->name , "keypad_test_mode", 0, 0)) {
	report (RPT_INFO, "CwLnx: Config tell us to test the keypad mapping...\n");
	p->keypad_test_mode = 1;
	stay_in_foreground = 1;
    }

    /* read the keypad mapping only if we have a keypad. */
    if (p->have_keypad) 
    	{
	int x;

	/* Read keymap */
	for(x=0; x<MaxKeyMap; x++ ) 
		{
		char buf[40];

		/* First fill with default value */

		p->KeyMap[x] = defaultKeyMap[x];
/* The line above make a warning... the code is comming from hd44780.c */

/* printf("%s-%s\n", defaultKeyMap[x], p->KeyMap[x]);     */

		/* Read config value */
		sprintf( buf, "KeyMap_%c", x+'A' );
		s = drvthis->config_get_string( drvthis->name, buf, 0, NULL );

		/* Was a key specified in the config file ? */
		if( s ) {
			p->KeyMap[x] = strdup( s );     
/* printf("CwLnx: Key '%c' to \"%s\"\n", x+'A', s );      */
			report( RPT_INFO, "CwLnx: Key '%c' to \"%s\"", x+'A', s );
			}
		}

	}

    /* End of config file parsing */

    /* Allocate framebuffer memory */
    if (!p->framebuf) {
	p->framebuf = malloc(p->width * p->height);
	p->backingstore = calloc(p->width * p->height, 1);
        memset(p->backingstore, ' ', p->width * p->height);
    }

    if (!p->framebuf) {
	report(RPT_ERR, "CwLnx: Error: unable to create framebuffer.\n");
	return -1;
    }

    /* Set up io port correctly, and open it... */
    debug(RPT_DEBUG, "CwLnx: Opening serial device: %s", device);
    p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (p->fd == -1) {
	report(RPT_ERR, "CwLnx: init() failed (%s)\n", strerror(errno));
	return -1;
    } else {
	report(RPT_INFO, "CwLnx: Opened display on %s", device);
    }

    Init_Port(p->fd);
    tcgetattr(p->fd, &portset_save);
    speed = B19200;
    Setup_Port(p->fd, speed);
    Set_9600(p->fd); 
    close(p->fd);

    p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (p->fd == -1) {
	report(RPT_ERR, "CwLnx: init() failed (%s)\n", strerror(errno));
	return -1;
    } else {
	report(RPT_INFO, "CwLnx: Opened display on %s", device);
    }
    Init_Port(p->fd);
    speed = B9600; 
    Setup_Port(p->fd, speed);
    CwLnx_hidecursor(p->fd);
    CwLnx_linewrap(p->fd, 1);
    CwLnx_autoscroll(p->fd, 0);
    CwLnx_backlight(drvthis, 1); /* WHY force the backlight to on ? */
    /* What is the default brightness ? */

    /* Set the functions the driver supports... */

    /* TODO: WHY this ??? $$$ */
/*    drvthis->daemonize = 1;	*/	/* make the server daemonize after initialization */
    /* no daemonize in Driver ??? */

    report(RPT_DEBUG, "CwLnx_init: done\n");

    Clear_Screen(p->fd);
    CwLnx_clear(drvthis);
    usleep(SETUP_DELAY);

    return p->fd;
}

/******************************************************
 * API: Clean-up
 */
MODULE_EXPORT void
CwLnx_close(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    close(p->fd);

    if (p->framebuf)
	free(p->framebuf);

    if (p->backingstore)
	free(p->backingstore);

    p->framebuf = NULL;
    p->backingstore = NULL;

    free(p);

    debug(RPT_DEBUG, "CwLnx: closed");
}

/******************************************************
 * API: Returns the displays width
 */
MODULE_EXPORT int 
CwLnx_width(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning width");

    return p->width;
}

/******************************************************
 * API: Returns the displays height
 */
MODULE_EXPORT int 
CwLnx_height(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning height");

    return p->height;
}



/******************************************************
 * API: Clean-up
 */
/*
MODULE_EXPORT void
CwLnx_flushtime_backlight(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    int bright;

    if (   ( (p->saved_backlight)&&(!p->backlight) )
	|| ( (p->backlight)&&(!p->saved_backlight) ) )
	{
	p->backlight = p->saved_backlight;
	if (p->backlight)
		{
		Enable_Backlight(p->fd);
		}
	else
		{
		Disable_Backlight(p->fd);
		}
	}

    if ( p->brightness != p->saved_brightness )
    {
	p->brightness = p->saved_brightness;
	Backlight_Brightness(p->fd, p->brightness)
    }

    debug(RPT_DEBUG, "CwLnx: updating the backlight and brightness at flush time");
}
*/

/*****************************************************
 * This is a test to see how a driver can overwrite build-in heartbeat.
 * It make a pixel blink at calling rate independently of flush call.
 */
MODULE_EXPORT void
CwLnx_flushtime_heartbeat( Driver * drvthis )
{
    PrivateData * p = drvthis->private_data;

    if ( p->heartbeat != p->saved_heartbeat ) {
        p->saved_heartbeat=p->heartbeat;
        if (p->heartbeat) {
            Enable_Pixel(p->fd, 121, 0);
            Enable_Pixel(p->fd, 60, 0);
            Enable_Pixel(p->fd, 121, 31);
	} else {
            Disable_Pixel(p->fd, 121, 0);
            Disable_Pixel(p->fd, 60, 0);
            Disable_Pixel(p->fd, 121, 31);
	}
    }
}

/******************************************************
 * API: Send what we have to the hardware
 */
MODULE_EXPORT void
CwLnx_flush(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    CwLnx_draw_frame(drvthis, p->framebuf);

    CwLnx_flushtime_heartbeat(drvthis);
/*    CwLnx_flushtime_backlight(drvthis); */ 
}

void Set_Insert(int fd, int row, int col)
{
    char c;
    int rc;

    c = LCD_CMD;
    rc = Write_LCD(fd, &c, 1);
    if (row==0 && col==0) 
    	{
    	c = LCD_INIT_INSERT;
    	rc = Write_LCD(fd, &c, 1);
    	}
    else
    	{
    c = LCD_SET_INSERT;
    rc = Write_LCD(fd, &c, 1);
    c = col;
    rc = Write_LCD(fd, &c, 1);
    c = row;
    rc = Write_LCD(fd, &c, 1);
    	}
    c = LCD_CMD_END;
    rc = Write_LCD(fd, &c, 1);
}

/***********************************************
 *
 * _flush_box is not an API entry anymore.
 * 
void CwLnx_flush_box(int lft, int top, int rgt, int bot)
{
    int y;

    debug(RPT_DEBUG, "CwLnx: flush_box (%i,%i)-(%i,%i)\n", lft, top, rgt,
	  bot);
    for (y = top; y <= bot; y++) {
	Set_Insert(fd, top, lft);
	Write_Line_LCD(fd, CwLnx->framebuf + (y * p->width) + lft);
    }
}
*/


/*******************************************************************
 * API: Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
CwLnx_chr(Driver *drvthis, int x, int y, char c)
{
    PrivateData * p = drvthis->private_data;

    int offset;

    ValidX(x);
    ValidY(y);

    y--;
    x--;

    offset = (y * p->width) + x;
    p->framebuf[offset] = c;

    debug(RPT_DEBUG, "CwLnx: writing character %02X to position (%d,%d)", c, x, y);
}

/*
 * The CwLinux hardware does not support contrast setting by software,
 * but only by changing the hardware configuration.
 * Since the get_contrast and set_contrast are not mandatory in the API,
 * it is better not to implement a dummy version.
 */




/*
 * CwLnx support 7 level of brightness
 * 1 is the minimum and correspond to backlight OFF
 * 7 is the maximum.
 *
 * When the backlight is turned off, we remember the brightness
 * so that when the backlight is turned back on, we have the previous
 * brightness.
 *
 * Backlight and Brightness in this driver will not be set real-time,
 * but we will wait for the flush command. The same way we use a framebuffer,
 * and we update the LCD only at flush time.
 */

/*********************************************************
 * API: Sets the backlight
 * the API only permit setting to off=0 and on<>0
 */
MODULE_EXPORT void
CwLnx_backlight(Driver * drvthis, int on)
{
    PrivateData * p = drvthis->private_data;

    p->backlight = on;
}


/*********************************************************
 * API: Get the backlight brightness
 */
MODULE_EXPORT int 
CwLnx_get_brightness(Driver * drvthis, int state)
{
        PrivateData * p = drvthis->private_data;

        return p->saved_brightness;
}


/*********************************************************
 * API: Set the backlight brightness
 */
MODULE_EXPORT void
CwLnx_set_brightness(Driver * drvthis, int state, int promille)
{
        PrivateData * p = drvthis->private_data;

        p->brightness = promille;
}


/*********************************************************
 * Toggle the built-in linewrapping feature
 */
static void CwLnx_linewrap (int fd, int on)
{
    if (on)
	    Enable_Wrap(fd);
    else
	    Disable_Wrap(fd);
}


/****************************************************************
 * Toggle the built-in automatic scrolling feature
 */
static void CwLnx_autoscroll(int fd, int on)
{
    if (on)
	    Enable_Scroll(fd);
    else
	    Disable_Scroll(fd);
}


/*******************************************************************
 * Get rid of the blinking curson
 */
static void CwLnx_hidecursor(int fd)
{
	Disable_Cursor(fd);
}


/*********************************************************
 * NOTAPI: Inits vertical bars...
 * This was part of API in 0.4 and removed in 0.5
 */
void CwLnx_init_vbar(Driver * drvthis)
{
    PrivateData * p = drvthis->private_data;

    char a[] = {
	1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0,
    };
    char b[] = {
	1, 1, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
    };
    char c[] = {
	1, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 0, 0, 0, 0, 0,
    };
    char d[] = {
	1, 1, 1, 1, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0,
    };
    char e[] = {
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 1, 0, 0, 0,
    };
    char f[] = {
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
	1, 1, 1, 1, 1, 1, 0, 0,
    };
    char g[] = {
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
    };

    if (p->custom != vbar) {
	CwLnx_set_char(drvthis, 1, a);
	CwLnx_set_char(drvthis, 2, b);
	CwLnx_set_char(drvthis, 3, c);
	CwLnx_set_char(drvthis, 4, d);
	CwLnx_set_char(drvthis, 5, e);
	CwLnx_set_char(drvthis, 6, f);
	CwLnx_set_char(drvthis, 7, g);
	p->custom = vbar;
    }
}

/*********************************************************
 * NOTAPI: Inits horizontal bars...
 * This was part of API in 0.4 and removed in 0.5
 */
void CwLnx_init_hbar(Driver * drvthis)
{
    PrivateData * p = drvthis->private_data;

    char a[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
    };
    char b[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
    };
    char c[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
    };
    char d[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
    };
    char e[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
    };
    char f[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
    };

    if (p->custom != hbar) {
	CwLnx_set_char(drvthis, 1, a);
	CwLnx_set_char(drvthis, 2, b);
	CwLnx_set_char(drvthis, 3, c);
	CwLnx_set_char(drvthis, 4, d);
	CwLnx_set_char(drvthis, 5, e);
	CwLnx_set_char(drvthis, 6, f);
	p->custom = hbar;
    }

}

/*************************************************************
 * API: Draws a vertical bar...
 */
MODULE_EXPORT void
CwLnx_vbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData * p = drvthis->private_data;

    CwLnx_init_vbar(drvthis);

    lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/*****************************************************************
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
CwLnx_hbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData * p = drvthis->private_data;

    CwLnx_init_hbar(drvthis);

    lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*******************************************************************
 * API: Writes a big number.
 */
/* Currently using the server ascii default */
/*
MODULE_EXPORT void
CwLnx_num(Driver * drvthis, int x, int num)
{
    return;
}
*/

/*********************************************************************
 * API: Sets a custom character...
 * This should be removed from API since it is hardware dependent.
 *
 * The API only permit setting to off=0 and on<>0
 * For input, values > 0 mean "on" and values <= 0 are "off".
 *
 * The input is just an array of characters...
 */
MODULE_EXPORT void
CwLnx_set_char(Driver * drvthis, int n, char *dat)
{
    PrivateData * p = drvthis->private_data;

    int row, col;
    int letter;
    char c;
    int rc;

    if (n < 1 || n > 16)
	return;
    if (!dat)
	return;

    c = LCD_CMD;
    rc = Write_LCD(p->fd, &c, 1);
    c = LCD_SETCHAR;
    rc = Write_LCD(p->fd, &c, 1);
    c = (char) n;
    rc = Write_LCD(p->fd, &c, 1);

    for (col = 0; col < p->cellwidth; col++) {
	letter = 0;
	for (row = 0; row < p->cellheight; row++) {
	    letter <<= 1;
	    letter |= (dat[(col * p->cellheight) + row] > 0);
	}
	c=letter;
	Write_LCD(p->fd, &c, 1);
    }
    c = LCD_CMD_END;
    rc = Write_LCD(p->fd, &c, 1);
}

MODULE_EXPORT int 
CwLnx_icon(Driver * drvthis, int x, int y, int icon)
{
    char heart_open[] = 
	{
	 1,1,1,0,0,0,1,1,
	 1,1,0,0,0,0,0,1,
	 1,0,0,0,0,0,1,1,
	 1,1,0,0,0,0,0,1,
	 1,1,1,0,0,0,1,1,
	 1,1,1,1,1,1,1,1 
	 };

    char heart_filled[] = 
	{
	 1,1,1,0,0,0,1,1,
	 1,1,0,1,1,1,0,1,
	 1,0,1,1,1,0,1,1,
	 1,1,0,1,1,1,0,1,
	 1,1,1,0,0,0,1,1,
	 1,1,1,1,1,1,1,1
	 };

    char arrow_up[] = 
	{
	 0,0,0,0,1,1,0,0,
	 0,0,0,0,0,1,1,0,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 0,0,0,0,0,1,1,0,
	 0,0,0,0,1,1,0,0
	 };

    char arrow_down[] = 
	{
	 0,0,1,1,0,0,0,0,
	 0,1,1,0,0,0,0,0,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 0,1,1,0,0,0,0,0,
	 0,0,1,1,0,0,0,0
	 };

    char checkbox_off[] = 
	{
	 0,1,1,1,1,1,0,0,
	 0,1,0,0,0,1,0,0,
	 0,1,0,0,0,1,0,0,
	 0,1,0,0,0,1,0,0,
	 0,1,1,1,1,1,0,0,
	 0,0,0,0,0,0,0,0
	 };

    char checkbox_on[] = 
	{
	 0,1,1,1,1,1,0,0,
	 0,1,0,0,0,1,0,0,
	 0,1,0,1,1,1,1,0,
	 0,1,0,0,1,0,0,0,
	 0,1,1,1,0,1,0,0,
	 0,0,0,0,0,0,1,0
	 };

    char checkbox_gray[] = 
	{
	 0,1,1,1,1,1,0,0,
	 0,1,0,1,0,1,0,0,
	 0,1,1,0,1,1,0,0,
	 0,1,0,1,0,1,0,0,
	 0,1,1,1,1,1,0,0,
	 0,0,0,0,0,0,0,0
	 };

    char block_filled[] = 
	{
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1
	 };
       
    char arrow_left[] = 
	{
	 0,0,0,1,1,0,0,0,
	 0,0,1,1,1,1,0,0,
	 0,1,1,1,1,1,1,0,
	 1,1,0,1,1,0,1,1,
	 0,0,0,1,1,0,0,0,
	 0,0,0,1,1,0,0,0
	 };

    char arrow_right[] = 
	{
	 0,0,0,1,1,0,0,0,
	 0,0,0,1,1,0,0,0,
	 1,1,0,1,1,0,1,1,
	 0,1,1,1,1,1,1,0,
	 0,0,1,1,1,1,0,0,
	 0,0,0,1,1,0,0,0
	 };
       

/* Yes we know, this is a VERY BAD implementation */
	switch( icon ) {
		case ICON_HEART_FILLED:
			CwLnx_set_char( drvthis, 8, heart_filled );
			CwLnx_chr( drvthis, x, y, 8 );
			break;
		case ICON_HEART_OPEN:
			CwLnx_set_char( drvthis, 8, heart_open );
			CwLnx_chr( drvthis, x, y, 8 );
			break;
		case ICON_CHECKBOX_GRAY:
			CwLnx_set_char( drvthis, 9, checkbox_gray );
			CwLnx_chr( drvthis, x, y, 9 );
			break;
		case ICON_BLOCK_FILLED:
			CwLnx_set_char( drvthis, 10, block_filled );
			CwLnx_chr( drvthis, x, y, 10 );
			break;
		case ICON_ARROW_UP:
			CwLnx_set_char( drvthis, 11, arrow_up );
			CwLnx_chr( drvthis, x, y, 11 );
			break;
		case ICON_ARROW_DOWN:
			CwLnx_set_char( drvthis, 12, arrow_down );
			CwLnx_chr( drvthis, x, y, 12 );
			break;
		case ICON_ARROW_LEFT:
			CwLnx_set_char( drvthis, 13, arrow_left );
			CwLnx_chr( drvthis, x, y, 13 );
			break;
		case ICON_ARROW_RIGHT:
			CwLnx_set_char( drvthis, 14, arrow_right );
			CwLnx_chr( drvthis, x, y, 14 );
			break;
		case ICON_CHECKBOX_OFF:
			CwLnx_set_char( drvthis, 15, checkbox_off );
			CwLnx_chr( drvthis, x, y, 15 );
			break;
		case ICON_CHECKBOX_ON:
			CwLnx_set_char( drvthis, 16, checkbox_on );
			CwLnx_chr( drvthis, x, y, 16 );
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}

/**********************************************************
 * ONLY CALLED FROM _flush ...
 *
 * Blasts a single frame onscreen, to the lcd...
 *
 * Input is a character array, sized CwLnx->wid*CwLnx->hgt
 */
void CwLnx_draw_frame(Driver *drvthis, char *dat)
{
    PrivateData * p = drvthis->private_data;

    int i, j, mv, rc;
    char *q, *r;
/*  char c; */
/*  static int count=0; */

    if (!dat)
	return;

    mv = 1;
    q = dat;
    r = p->backingstore;

/*    printf("\n_draw_frame: %d\n", count);   */

    for (i = 0; i < p->height; i++) 
            {
	    for (j = 0; j < p->width; j++) 
	            {
		    if ( (*q == *r) && !( (0<*q) && (*q<16) ) )
		    	{
				mv = 1;
/*         count++; if (count==COUNT) exit(0);       */
			}
		    else
		        {
			    /* Draw characters that have changed, as well
			     * as custom characters.  We know not if a custom
			     * character has changed.
			     */ 
		        if (mv == 1) 
			    {
			    Set_Insert(p->fd, i, j);
			    mv = 0;
		   	    }
                        rc = Write_LCD(p->fd, q, 1);
		        }
		    q++;
		    r++; 
	            }
            }
  strncpy(p->backingstore, dat, p->width * p->height);

}

/*********************************************************
 * API: Clears the LCD screen
 */
MODULE_EXPORT void
CwLnx_clear(Driver *drvthis)
{
    PrivateData * p = drvthis->private_data;

    /* WHY: are we the only function to check framebuf for null? */
    if (p->framebuf != NULL)
        memset(p->framebuf, ' ', p->width * p->height);

    /* We could remember the custom char are not in use anymore. $$$ */
    
    debug(RPT_DEBUG, "CwLnx: cleared framebuffer");
}

/*****************************************************************
 * API: Prints a string on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
CwLnx_string(Driver * drvthis, int x, int y, char *string)
{
    PrivateData * p = drvthis->private_data;

    int offset, siz;

    ValidX(x);
    ValidY(y);

    x--;
    y--;

    offset = (y * p->width) + x;

    siz = (p->width * p->height) - offset;
    siz = siz > strlen(string) ? strlen(string) : siz;

    memcpy(p->framebuf + offset, string, siz);

/*
    This is another way to check for buffer overflow
  
    for (int i = 0; string[i]; i++) {
	if ((y * p->width) + x + i > (p->width * p->height))
	    break;
	CwLnx->framebuf[(y * p->width) + x + i] = string[i];
    }
*/

    debug(RPT_DEBUG, "CwLnx: printed string at (%d,%d)", x, y);
}

/*********************************************************
 * API: Get a key translated into a string.
 */
MODULE_EXPORT char *
CwLnx_get_key(Driver * drvthis)
{
    PrivateData * p = drvthis->private_data;

	char akey = 0;
	char * keystr = NULL;

	read (p->fd, &akey, 1);

	if (akey != 0) 
	{
		if ((akey>='A')&&(akey<='F'))
		{
			keystr = p->KeyMap[akey-'A'];
		}
		else
		{
                        report( RPT_INFO, "CwLnx: Untreated key 0x%2x", akey);
		}
	}

	return keystr;
}

/*****************************************************
 * API: This is a test to see how a driver can overwrite build-in heartbeat.
 * It make a pixel blink at calling rate independently of flush call.
 */
/*
 * The Pixel is not very visible when located at (121,0).
 * Maybe the display of text caracter hide the pixel.
 * It might be the flush just after the heartbeat call.
 */

/*I*/ int saved_heartbeat;
/*I*/ int heartbeat;
/*I*/ int heartbeat_state;

MODULE_EXPORT void
CwLnx_heartbeat( Driver * drvthis, int type )
{
    PrivateData * p = drvthis->private_data;

    if (type) {
        if (p->heartbeat_state) {
	    p->heartbeat=1;
            p->heartbeat_state = 0;
	} else {
	    p->heartbeat=0;
            p->heartbeat_state = 1;
	}
    } else {
        if (p->heartbeat_state) {
	    p->heartbeat=0;
            p->heartbeat_state = 0;
	}
    }
}

/*
MODULE_EXPORT void
CwLnx_heartbeat( Driver * drvthis, int type )
{
    PrivateData * p = drvthis->private_data;

    if (type) {
        if (p->heartbeat_state) {
            Enable_Pixel(p->fd, 121, 0);
            p->heartbeat_state = 0;
	} else {
            Disable_Pixel(p->fd, 121, 0);
            p->heartbeat_state = 1;
	}
    } else {
        if (p->heartbeat_state) {
            Disable_Pixel(p->fd, 121, 0);
            p->heartbeat_state = 0;
	}
    }
}
*/
