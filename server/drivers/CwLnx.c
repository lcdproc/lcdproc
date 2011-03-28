/** \file server/drivers/CwLnx.c
 *  LCDd \c CwLnx driver for CwLinux devices (http://www.cwlinux.com).
 */

/* Applicable Data Sheets:
    - http://www.cwlinux.com/downloads/cw1602/cw1602-manual.pdf
    - http://www.cwlinux.com/downloads/lcd/cw12232-manual.pdf

        Copyright (C) 2002, Andrew Ip
                      2003, David Glaude
                      2006,7,8 Peter Marschall

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
*/

/*
    Feedback from Tomislav Secen, who tested it with a 1602:

    Great, this is much better:
    - icons are displayed nicely
    - bars are OK (at least the few I've seen)
    - heartbeat icon flashes nicely in the top right corner.

    Only issue I encountered was (similar came up before) - when setting some menu
    options, or just entering a certain menu (like Options->CwLnx->OnBrightness,
    lcdvc client menu), the LCD becomes garbled  (i.e. displays two blinking hearts,
    boxes, '%' symbols), sometimes starts displaying just '%' symbols over the whole
    LCD while I'm pressing Left/Right keys (is this the screen-saver?), scrolling
    from right to left. Even the "Thank you for using ..." message is garbled (each
    time in a different way) if I kill the daemon after that. Pressing 'X' when the
    garbled characters occur exits the menu and the other (client) screens are
    displayed correctly after that. So only the menus are affected by this issue -
    it seems that this screen-saver mode kicks in in the wrong time, and tries to
    write to LCD faster than it can process chars/commands.
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
#include "CwLnx.h"
#include "report.h"
#include "lcd_lib.h"

/* for the icon definitions & the big numbers */
#include "adv_bignum.h"

#define ValidX(x) if ((x) > p->width) { (x) = p->width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > p->height) { (y) = p->height; } else (y) = (y) < 1 ? 1 : (y);

#define MaxKeyMap 6

static char *defaultKeyMap[MaxKeyMap] = { "Up", "Down", "Left", "Right", "Enter", "Escape" };


/** private data for the \c CwLnx driver */
typedef struct CwLnx_private_data {
	int fd;

	int have_keypad;
	int keypad_test_mode;
	char *KeyMap[MaxKeyMap];

	int model;

	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* framebuffer and buffer for old LCD contents */
	unsigned char *framebuf;
	unsigned char *backingstore;

	/* definable characters */
	CGmode ccmode;

	char saved_backlight;	/* current state of the backlight */
	char backlight;		/* state of the backlight at next flush */

	int saved_brightness;	/* brightness as displayed on the LCD currently */
	int brightness;		/* brightness as it will be displayed at next flush */
} PrivateData;


/* API: Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0; /* For testing only */
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "CwLnx_";


static void CwLnx_linewrap(int fd, int on);
static void CwLnx_autoscroll(int fd, int on);
static void CwLnx_hidecursor(int fd);
static void CwLnx_set_char_unrestricted(Driver *drvthis, int n, unsigned char *dat);


#define LCD_CMD			254
#define LCD_CMD_END		253
#define LCD_INIT_CHINESE_T	56
#define LCD_INIT_CHINESE_S	55
#define LCD_LIGHT_ON		66
#define LCD_LIGHT_OFF		70
#define LCD_LIGHT_BRIGHTNESS	65
#define LCD_CLEAR		88
#define LCD_SET_INSERT		71	/* go to X,Y */
#define LCD_INIT_INSERT		72	/* go to home */
#define LCD_SET_BAUD		57
#define LCD_ENABLE_WRAP		67
#define LCD_DISABLE_WRAP	68
#define LCD_SETCHAR		78
#define LCD_ENABLE_SCROLL	81
#define LCD_DISABLE_SCROLL	82
#define LCD_SOFT_RESET		86
#define LCD_OFF_CURSOR		72	/* is this correct? */
#define LCD_UNDERLINE_CURSOR_ON	74	/* set cursor on at X,Y */
#define	LCD_UNDERLINE_CURSOR_OFF	75
#define LCD_MOVE_CURSOR_LEFT	76
#define LCD_MOVE_CURSOR_RIGHT	77
#define LCD_INVERSE_TEXT_ON	102
#define LCD_INVERSE_TEXT_OFF	103

#define LCD_PUT_PIXEL		112
#define LCD_CLEAR_PIXEL		113

#define DELAY			2000	/* 2 milli sec */
#define UPDATE_DELAY		20000	/* 20 milliseconds */
#define SETUP_DELAY		20000	/* 20 milliseconds */

#define MOVE_COST		5	/* # bytes for most move-to ops */

static int Write_LCD(int fd, char *c, int size)
{
    int rc, wrote = 0;
    int retries = 30;

    do {
	rc = write(fd, c, size);
	if (rc > 0) {
	    c += rc;
	    size -= rc;
	    wrote += rc;
	} else if (rc == 0 || (rc < 0 && errno == EAGAIN)) { /* would have blocked */
	    usleep(DELAY);
	} else {
	    break;
	}
    } while (size > 0 && --retries > 0);

    return wrote;
}


/*********************************************
 * Real hardware function.
 * Will be called by API function.
 */


/* Hardware function */
static void Enable_Backlight(int fd)
{
    char cmd[] = { LCD_CMD, LCD_LIGHT_ON, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Disable_Backlight(int fd)
{
    char cmd[] = { LCD_CMD, LCD_LIGHT_OFF, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Enable_Pixel(int fd, int x, int y)
{
    char cmd[] = { LCD_CMD, LCD_PUT_PIXEL, 0, 0, LCD_CMD_END };

    cmd[2] = (char) x;
    cmd[3] = (char) y;

    Write_LCD(fd, cmd, 5);
}


/* Hardware function */
static void Disable_Pixel(int fd, int x, int y)
{
    char cmd[] = { LCD_CMD, LCD_CLEAR_PIXEL, 0, 0, LCD_CMD_END };

    cmd[2] = (char) x;
    cmd[3] = (char) y;

    Write_LCD(fd, cmd, 5);
}


/* Hardware function */
static void Backlight_Brightness(int fd, int brightness)
{
    if (brightness == 1) {
	Disable_Backlight(fd);
    } else if (brightness == 7) {
	Enable_Backlight(fd);
    } else {
	char cmd[] = { LCD_CMD, LCD_LIGHT_BRIGHTNESS, 0, LCD_CMD_END };

	cmd[2] = (char) brightness;

	Write_LCD(fd, cmd, 4);
    }
}


/* Hardware function */
static void Enable_Scroll(int fd)
{
    char cmd[] = { LCD_CMD, LCD_ENABLE_SCROLL, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Disable_Scroll(int fd)
{
    char cmd[] = { LCD_CMD, LCD_DISABLE_SCROLL, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Clear_Screen(int fd)
{
    char cmd[] = { LCD_CMD, LCD_CLEAR, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
    usleep(UPDATE_DELAY);
}


/* Hardware function */
static void Enable_Wrap(int fd)
{
    char cmd[] = { LCD_CMD, LCD_ENABLE_WRAP, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Disable_Wrap(int fd)
{
    char cmd[] = { LCD_CMD, LCD_DISABLE_WRAP, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Disable_Cursor(int fd)
{
    char cmd[] = { LCD_CMD, LCD_OFF_CURSOR, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
}


/* Hardware function */
static void Init_Port(int fd)
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
static void Setup_Port(int fd, speed_t speed)
{
    struct termios portset;

    tcgetattr(fd, &portset);

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
    portset.c_cc[VTIME] = 5;

    /* Set speed */
    cfsetospeed(&portset, speed);
    cfsetispeed(&portset, speed);

    tcsetattr(fd, TCSANOW, &portset);
}


/* Hardware function */
static void Set_9600(int fd)
{
    char cmd[] = { LCD_CMD, LCD_SET_BAUD, 0x20, LCD_CMD_END };

    Write_LCD(fd, cmd, 4);
}


/* Hardware function */
static void Set_19200(int fd)
{
    char cmd[] = { LCD_CMD, LCD_SET_BAUD, 0x0F, LCD_CMD_END };

    Write_LCD(fd, cmd, 4);
}


/* Hardware function */
static void Set_Insert(int fd, int row, int col)
{
    if (row == 0 && col == 0) {
        char cmd[] = { LCD_CMD, LCD_INIT_INSERT, LCD_CMD_END };

    	Write_LCD(fd, cmd, 3);
    }
    else {
	char cmd[] = { LCD_CMD, LCD_SET_INSERT, 0, 0, LCD_CMD_END };

	cmd[2] = (char) col;
	cmd[3] = (char) row;

	Write_LCD(fd, cmd, 5);
    }
}


/**
 * Toggle the built-in linewrapping feature
 */
static void
CwLnx_linewrap(int fd, int on)
{
    if (on)
	    Enable_Wrap(fd);
    else
	    Disable_Wrap(fd);
}


/**
 * Toggle the built-in automatic scrolling feature
 */
static void
CwLnx_autoscroll(int fd, int on)
{
    if (on)
	    Enable_Scroll(fd);
    else
	    Disable_Scroll(fd);
}


/**
 * Get rid of the blinking curson
 */
static void
CwLnx_hidecursor(int fd)
{
    Disable_Cursor(fd);
}


/********************************************************************
 * Reset the display bios
 */
static void CwLnx_reboot(int fd)
{
    char cmd[] = { LCD_CMD, LCD_SOFT_RESET, LCD_CMD_END };

    Write_LCD(fd, cmd, 3);
    usleep(SETUP_DELAY);
    return;
}


/*****************************************************
 * Here start the API function
 */


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
CwLnx_init(Driver *drvthis)
{
    char device[200] = DEFAULT_DEVICE;
    int speed = DEFAULT_SPEED;
    char size[200] = DEFAULT_SIZE;
    int default_speed = DEFAULT_SPEED;
    char *default_size = DEFAULT_SIZE;

    int tmp;
    int w;
    int h;
    const char *s;

    PrivateData *p;

    /* Allocate and store private data */
    p = (PrivateData *) malloc(sizeof(PrivateData));
    if (p == NULL)
        return -1;
    if (drvthis->store_private_ptr(drvthis, p))
        return -1;

    /* Initialise the PrivateData structure */
    p->fd = -1;
    p->cellwidth = DEFAULT_CELL_WIDTH;
    p->cellheight = DEFAULT_CELL_HEIGHT;

    p->ccmode = standard;

    p->saved_backlight = -1;
    p->backlight = DEFAULT_BACKLIGHT;

    p->saved_brightness = -1;
    p->brightness = DEFAULT_BRIGHTNESS;

    debug(RPT_INFO, "%s: init(%p)", drvthis->name, drvthis);

    /* Read config file */

    /* Which model is it (1602, 12232 or 12832)? */
    tmp = drvthis->config_get_int(drvthis->name, "Model", 0, 12232);
    debug(RPT_INFO, "%s: Model (in config) is '%d'", __FUNCTION__, tmp);
    if ((tmp != 1602) && (tmp != 12232) && (tmp != 12832)) {
	tmp = 12232;
	report(RPT_WARNING, "%s: Model must be 12232, 12832 or 1602; using default %d",
		drvthis->name, tmp);
    }
    p->model = tmp;

    /* Which size & cell dimensions */
    if (p->model == 1602) {
	default_size = DEFAULT_SIZE_1602;
	default_speed = DEFAULT_SPEED_1602;
	p->cellwidth = DEFAULT_CELL_WIDTH_1602;
	p->cellheight = DEFAULT_CELL_HEIGHT_1602;
    } else if (p->model == 12232) {
	default_size = DEFAULT_SIZE_12232;
	default_speed = DEFAULT_SPEED_12232;
	p->cellwidth = DEFAULT_CELL_WIDTH_12232;
	p->cellheight = DEFAULT_CELL_HEIGHT_12232;
    } else if (p->model == 12832) {
	default_size = DEFAULT_SIZE_12832;
	default_speed = DEFAULT_SPEED_12832;
	p->cellwidth = DEFAULT_CELL_WIDTH_12832;
	p->cellheight = DEFAULT_CELL_HEIGHT_12832;
    }

    /* Which device should be used */
    strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
    device[sizeof(device) - 1] = '\0';
    report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

    /* Which size */
    strncpy(size, drvthis->config_get_string(drvthis->name, "Size", 0, default_size), sizeof(size));
    size[sizeof(size) - 1] = '\0';
    if ((sscanf(size, "%dx%d", &w, &h) != 2)
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT)) {
	report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
			drvthis->name, size, default_size);
	sscanf(default_size, "%dx%d", &w, &h);
    }
    p->width = w;
    p->height = h;

    /* Contrast of the LCD can be changed by adjusting the trimpot R7  */

    /* Which speed */
    tmp = drvthis->config_get_int(drvthis->name, "Speed", 0, default_speed);

    switch (tmp) {
	case 9600:
	    speed = B9600;
	    break;
	case 19200:
	    speed = B19200;
	    break;
	default:
	    speed = B19200;
	    report(RPT_WARNING, "%s: Speed must be 9600 or 19200. Using default %d",
			    drvthis->name, default_speed);
    }

    /* do we have a keypad? */
    if (drvthis->config_get_bool(drvthis->name , "Keypad", 0, 0)) {
	report(RPT_INFO, "%s: Config tells us we have a keypad", drvthis->name);
	p->have_keypad = 1;
    }

    /* keypad test mode? */
    if (drvthis->config_get_bool(drvthis->name , "keypad_test_mode", 0, 0)) {
	report(RPT_INFO, "%s: Config tells us to test the keypad mapping", drvthis->name);
	p->keypad_test_mode = 1;
	stay_in_foreground = 1;
    }

    /* read the keypad mapping only if we have a keypad. */
    if (p->have_keypad) {
	int x;

	/* Read keymap */
	for (x = 0; x < MaxKeyMap; x++) {
	    char buf[40];

	    /* First fill with default value */

	    p->KeyMap[x] = defaultKeyMap[x];
/* The line above make a warning... the code is comming from hd44780.c */

/* printf("%s-%s\n", defaultKeyMap[x], p->KeyMap[x]);     */

	    /* Read config value */
	    sprintf(buf, "KeyMap_%c", x+'A');
	    s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

	    /* Was a key specified in the config file ? */
	    if (s != NULL) {
		p->KeyMap[x] = strdup(s);
		report(RPT_INFO, "%s: Key '%c' to \"%s\"", drvthis->name, x+'A', s);
	    }
	}
    }

    /* End of config file parsing */

    /* Allocate framebuffer memory */
    p->framebuf = (unsigned char *) malloc(p->width * p->height);
    if (p->framebuf == NULL) {
	report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
	return -1;
    }
    memset(p->framebuf, ' ', p->width * p->height);

    /* make sure the framebuffer backing store is there... */
    p->backingstore = (unsigned char *) malloc(p->width * p->height);
    if (p->backingstore == NULL) {
	report(RPT_ERR, "%s: unable to create backingstore", drvthis->name);
	return -1;
    }
    memset(p->backingstore, ' ', p->width * p->height);


    /* Set up io port correctly, and open it... */
    debug(RPT_DEBUG, "%s: Opening device: %s", drvthis->name, device);
    p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (p->fd == -1) {
	report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, device, strerror(errno));
	return -1;
    }
    report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

    /*
       Since we don't know what speed the display is using when
       we first connect to it, configure the port for the speed
       we don't want to use, send a command to switch the display
       to the speed we want to use, and flush the command.
    */

    Init_Port(p->fd);
    if (speed == B9600) {
	Setup_Port(p->fd, B19200);
    Set_9600(p->fd);
    } else {
	Setup_Port(p->fd, B9600);
	Set_19200(p->fd);
    }
    tcdrain(p->fd);
    usleep(SETUP_DELAY);
    Init_Port(p->fd);
    Setup_Port(p->fd, speed);

    CwLnx_hidecursor(p->fd);
    CwLnx_linewrap(p->fd, 1);
    CwLnx_autoscroll(p->fd, 0);
    CwLnx_backlight(drvthis, 1); /* WHY force the backlight to on ? */
    /* What is the default brightness ? */

    Clear_Screen(p->fd);
    CwLnx_clear(drvthis);
    usleep(SETUP_DELAY);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CwLnx_close(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (p != NULL) {
	if (p->fd >= 0)
	    close(p->fd);

	if (p->framebuf != NULL)
	    free(p->framebuf);
	p->framebuf = NULL;

	if (p->backingstore != NULL)
	    free(p->backingstore);
	p->backingstore = NULL;

	free(p);
    }
    drvthis->store_private_ptr(drvthis, NULL);

    debug(RPT_DEBUG, "CwLnx: closed");
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
CwLnx_width(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning width");

    return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
CwLnx_height(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning height");

    return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
CwLnx_cellwidth(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning cellwidth");

    return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
CwLnx_cellheight(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    debug(RPT_DEBUG, "CwLnx: returning cellheight");

    return p->cellheight;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CwLnx_flush(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    int i, j;
    int iUpdate = 0, jUpdate = 0;
    unsigned char *firstUpdate = NULL, *lastUpdate = NULL;

    unsigned char *q = p->framebuf;
    unsigned char *r = p->backingstore;

    for (i = 0; i < p->height; i++) {
	for (j = 0; j < p->width; j++) {
	    if ((*q == *r) && !((0 < *q) && (*q < 16))) {
		if (firstUpdate && q - lastUpdate > MOVE_COST) {
		    Set_Insert(p->fd, iUpdate, jUpdate);
		    Write_LCD(p->fd, (char *) firstUpdate,
			  lastUpdate - firstUpdate + 1);
		    firstUpdate = lastUpdate = NULL;
		}
	    } else {
		lastUpdate = q;
		if (!firstUpdate) {
		    firstUpdate = q;
		    iUpdate = i;
		    jUpdate = j;
		}
	    }
	    q++;
	    r++;
	}
    }
    if (firstUpdate) {
	Set_Insert(p->fd, iUpdate, jUpdate);
	Write_LCD(p->fd, (char *) firstUpdate,
		lastUpdate - firstUpdate + 1);
    }

    memcpy(p->backingstore, p->framebuf, p->width * p->height);

    if (p->backlight != p->saved_backlight ||
	p->brightness != p->saved_brightness) {
	if (!p->backlight) {
	    Backlight_Brightness(p->fd, 1);
	} else {
	    Backlight_Brightness(p->fd, 1 + p->brightness * 6 / 900); /* 90% and up is full brightness */
	}
	p->saved_backlight = p->backlight;
	p->saved_brightness = p->brightness;
    }
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
CwLnx_chr(Driver *drvthis, int x, int y, char c)
{
    PrivateData *p = drvthis->private_data;

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

/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
CwLnx_backlight(Driver *drvthis, int on)
{
    PrivateData *p = drvthis->private_data;

    p->backlight = on;
}


/**
 * Retrieve brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want the value.
 * \return Stored brightness in promille.
 */
MODULE_EXPORT int
CwLnx_get_brightness(Driver *drvthis, int state)
{
        PrivateData *p = drvthis->private_data;

        return p->saved_brightness;
}


/**
 * Set on/off brightness.
 * \param drvthis  Pointer to driver structure.
 * \param state    Brightness state (on/off) for which we want to store the value.
 * \param promille New brightness in promille.
 */
MODULE_EXPORT void
CwLnx_set_brightness(Driver *drvthis, int state, int promille)
{
        PrivateData *p = drvthis->private_data;

        p->brightness = promille;
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
CwLnx_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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
	    // add pixel line per pixel line ...
	    vBar[p->cellheight - i] = 0xFF;
	    CwLnx_set_char(drvthis, i+1, vBar);
	}
    }

    lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 1);
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
CwLnx_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
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

	for (i = 1; i <= p->cellwidth; i++) {
	    // fill pixel columns from left to right.
	    memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
#if defined(SEAMLESS_HBARS)
	    CwLnx_set_char_unrestricted(drvthis, i+1, hBar);
#else
	    CwLnx_set_char(drvthis, i+1, hBar);
#endif
	}
    }

    lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 1);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
CwLnx_num(Driver *drvthis, int x, int num)
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
    lib_adv_bignum(drvthis, x, num, 1, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
CwLnx_get_free_chars(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return (p->model == 1602) ? 8 : 16;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [1 - free_chars].
 * \param dat      Array of 8(=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
CwLnx_set_char(Driver *drvthis, int n, unsigned char *dat)
{
    PrivateData *p = drvthis->private_data;

    char c;
    int rc;

    if ((n <= 0) || (n > CwLnx_get_free_chars(drvthis)))
	return;
    if (!dat)
	return;

    c = LCD_CMD;
    rc = Write_LCD(p->fd, &c, 1);
    c = LCD_SETCHAR;
    rc = Write_LCD(p->fd, &c, 1);
    c = (char) n;
    rc = Write_LCD(p->fd, &c, 1);

    if (p->model == 1602) {	// the character model
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	for (row = 0; row < p->cellheight; row++) {
	    c = dat[row] & mask;
	    Write_LCD(p->fd, &c, 1);
	}
    } else if ((p->model == 12232) || (p->model == 12832)) {	// graphical models
	int col;

	for (col = p->cellwidth - 1; col >= 0; col--) {
	    int letter = 0;
	    int row;

	    for (row = p->cellheight - 1; row >= 0; row--) {
		letter <<= 1;
		letter |= ((dat[row] >> col) & 1);
	    }

	    /* restrict width to 5 pixels */
	    c = (col < p->cellwidth - 1) ? letter : '\0';

	    Write_LCD(p->fd, &c, 1);
	}
    }

    c = LCD_CMD_END;
    rc = Write_LCD(p->fd, &c, 1);
}


/*
 * Identical to CwLnx_set_char, but it doesn't restrict the 12232 to
 * using only 5 of its 6 columns.  Full 6-column mode is required
 * for seamless H-bars.
 */

#if defined(SEAMLESS_HBARS)
static void
CwLnx_set_char_unrestricted(Driver *drvthis, int n, unsigned char *dat)
{
    PrivateData *p = drvthis->private_data;

    char c;
    int rc;

    if ((n <= 0) || (n > CwLnx_get_free_chars(drvthis)))
	return;
    if (!dat)
	return;

    c = LCD_CMD;
    rc = Write_LCD(p->fd, &c, 1);
    c = LCD_SETCHAR;
    rc = Write_LCD(p->fd, &c, 1);
    c = (char) n;
    rc = Write_LCD(p->fd, &c, 1);

    if (p->model == 1602) {	// the character model
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;

	for (row = 0; row < p->cellheight; row++) {
	    c = dat[row] & mask;
	    Write_LCD(p->fd, &c, 1);
	}
    } else if ((p->model == 12232) || (p->model == 12832)) {	// graphical models
	int col;

	for (col = p->cellwidth - 1; col >= 0; col--) {
	    int letter = 0;
	    int row;

	    for (row = p->cellheight - 1; row >= 0; row--) {
		letter <<= 1;
		letter |= ((dat[row] >> col) & 1);
	    }

	    c = letter;

	    Write_LCD(p->fd, &c, 1);
	}
    }

    c = LCD_CMD_END;
    rc = Write_LCD(p->fd, &c, 1);
}
#endif


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
CwLnx_icon(Driver *drvthis, int x, int y, int icon)
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
/*
	static unsigned char arrow_left[] =
		{ b_______,
		  b____X__,
		  b___X___,
		  b__XXXXX,
		  b___X___,
		  b____X__,
		  b_______,
		  b_______ };
	static unsigned char arrow_right[] =
		{ b_______,
		  b____X__,
		  b_____X_,
		  b__XXXXX,
		  b_____X_,
		  b____X__,
		  b_______,
		  b_______ };
*/
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
/*
	static unsigned char selector_left[] =
		{ b___X___,
		  b___XX__,
		  b___XXX_,
		  b___XXXX,
		  b___XXX_,
		  b___XX__,
		  b___X___,
		  b_______ };
	static unsigned char selector_right[] =
		{ b_____X_,
		  b____XX_,
		  b___XXX_,
		  b__XXXX_,
		  b___XXX_,
		  b____XX_,
		  b_____X_,
		  b_______ };
	static unsigned char ellipsis[] =
		{ b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b_______,
		  b__X_X_X,
		  b_______ };
*/
	static unsigned char block_filled[] =
		{ b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };

	/* Yes we know, this is a VERY BAD implementation */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			CwLnx_set_char(drvthis, 7, block_filled);
			CwLnx_chr(drvthis, x, y, 7);
			break;
		case ICON_HEART_FILLED:
			CwLnx_set_char(drvthis, 1, heart_filled);
			CwLnx_chr(drvthis, x, y, 1);
			break;
		case ICON_HEART_OPEN:
			CwLnx_set_char(drvthis, 1, heart_open);
			CwLnx_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_UP:
			CwLnx_set_char(drvthis, 2, arrow_up);
			CwLnx_chr(drvthis, x, y, 2);
			break;
		case ICON_ARROW_DOWN:
			CwLnx_set_char(drvthis, 3, arrow_down);
			CwLnx_chr(drvthis, x, y, 3);
			break;
		case ICON_ARROW_LEFT:
			if (p->model == 1602)
				CwLnx_chr(drvthis, x, y, 0x7F);
			else
				return -1;
			break;
		case ICON_ARROW_RIGHT:
			if (p->model == 1602)
				CwLnx_chr(drvthis, x, y, 0x7E);
			else
				return -1;
			break;
		case ICON_CHECKBOX_OFF:
			CwLnx_set_char(drvthis, 4, checkbox_off);
			CwLnx_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_ON:
			CwLnx_set_char(drvthis, 5, checkbox_on);
			CwLnx_chr(drvthis, x, y, 5);
			break;
		case ICON_CHECKBOX_GRAY:
			CwLnx_set_char(drvthis, 6, checkbox_gray);
			CwLnx_chr(drvthis, x, y, 6);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
CwLnx_clear(Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->framebuf, ' ', p->width * p->height);
    p->ccmode = standard;

    debug(RPT_DEBUG, "CwLnx: cleared framebuffer");
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
CwLnx_string(Driver *drvthis, int x, int y, const char string[])
{
    PrivateData *p = drvthis->private_data;

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


/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
CwLnx_get_key(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	char key = '\0';

	read(p->fd, &key, 1);

	if (key != '\0') {
		if ((key >= 'A') && (key <= 'F')) {
			return p->KeyMap[key-'A'];
		}
		else {
			report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
		}
	}

	return NULL;
}


