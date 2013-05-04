/** \file server/drivers/pylcd.c
 * LCDd \c pyramid driver for the programmable LC displays
 * from Pyramid Computer GmbH.
 *
 * For more information see
 * http://www.pyramid.de/en/products/specialities.php
 */

/*-
 * Copyright (C) 2005 Silvan Marco Fin <silvan@kernelconcepts.de>
 *               2006 coresystems GmbH <info@coresystems.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "shared/defines.h"
#include "lcd.h"
#include "pylcd.h"
#include "lcd_lib.h"
#include "report.h"
#include "adv_bignum.h"

#define True 1
#define False 0

#define MICROTIMEOUT 50000	/* Timeout for select() */
#define NOKEY "00000"


/* variables for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "pyramid_";

/* Prototypes: */
static int data_ready(PrivateData *p);
static int read_tele(PrivateData *p, char *buffer);
static int real_send_tele(PrivateData *p, char *buffer, int len);
static int send_tele(PrivateData *p, char *buffer);
static int send_ACK(PrivateData *p);
static unsigned long long timestamp(void);
static int initTTY(Driver *drvthis, int FD);

/* local functions for pylcd.c */

/**
 * Performs an select on the device used. Returns if no data is available at
 * the moment. Used to keep reading-operations from blocking the driver.
 */
static int
data_ready(PrivateData *p)
{
    FD_ZERO(&p->rdfs);
    FD_SET(p->FD, &p->rdfs);
    return select(p->FD + 1, &p->rdfs, NULL, NULL, &p->timeout) > 0;
}


/**
 * Reads one telegramm, stores the telegramm without ETX/STX in buffer.
 * \param p       Pointer to driver's private data
 * \param buffer  Pointer to memory to store the telegram in. Must be at least
 *                MAXOUNT size!
 * \return  True (1) on successful detection of a telegram, False (0) if
 *          nothing was read or the telegramm didn't match its CC or if
 *          MAXCOUNT was exeeded without reading a complete telegramm.
 */
static int
read_tele(PrivateData *p, char *buffer)
{
    char zeichen = 0;
    int len = 0;
    char cc = 0x00;

    /* Try to find STX within first 10 chars */
    while (data_ready(p)
	   && (read(p->FD, &zeichen, 1) > 0)
	   && (zeichen != 0x02)
	   && (len < MAXCOUNT))
	len++;

    /* If no STX available, set buffer all zero and return */
    if (zeichen != 0x02) {
	memset(buffer, 0, MAXCOUNT);
	return False;
    }

    /* Now start reading until ETX */
    cc ^= zeichen;
    len = 0;

    while (data_ready(p)
	   && (read(p->FD, &zeichen, 1) > 0)
	   && (len < MAXCOUNT)) {
	buffer[len] = zeichen;
	cc ^= zeichen;
	if (zeichen == 0x03)
	    break;		/* break before len++! */
	len++;
    }

    /*
     * Read the next character. If the previous character was ETX and the
     * read charcters is a valid checksum, replace the ETX with NUL and
     * return the resulting string. Otherwise clear buffer (throw away all
     * read data) and return.
     */
    if (data_ready(p)
	&& (read(p->FD, &zeichen, 1) > 0)
	&& (buffer[len] == 0x03)
	&& (zeichen == cc)) {
	buffer[len] = 0x00;
	return True;
    }
    else {
	memset(buffer, 0, MAXCOUNT);
	return False;
    }
}

/**
 * Send the input as telegramm.
 *
 * The telegram buffer just contains the raw telegram data:
 *  - It shall not contain STX and ETX marks.
 *  - It may contain bytes below 0x20, they are automatically escaped.
 *
 * \param p       Pointer to driver's private data
 * \param buffer  Buffer containing the data to send
 * \param len     Number of bytes to send from buffer
 * \return  Always 0
 *
 * \note This function does not wait for any ACKs.
 */
static int
real_send_tele(PrivateData *p, char *buffer, int len)
{
    char cc = 0x00;
    int i, j;
    char buffer2[255];

    i = 0;
    j = 0;
    buffer2[j++] = 0x02;	/* emit <STX> */

    /*
     * copy the whole telegram package and escape characters below 0x20. ie.
     * 0x8 --> <ESC> 0x28.
     */

    while (len-- && j < 253) {
	if (buffer[i] >= 0x00 && buffer[i] < 0x20) {
	    buffer2[j++] = 0x1b;
	    buffer2[j++] = buffer[i++] + 0x20;
	}
	else {
	    buffer2[j++] = buffer[i++];
	}
    }
    buffer2[j++] = 0x03;	/* emit <ETX> */
    len = j;			/* new package length */

    /* calculate <BCC> over all bytes */
    for (i = 0; i < len; i++)
	cc ^= buffer2[i];

    buffer2[len++] = cc;

    write(p->FD, buffer2, len);

    /* Take a little nap. This works as a pacemaker */
    usleep(50);

    return 0;
}

/**
 * Send the string given in buffer. The string must be NUL-terminated.
 *
 * \param p       Pointer to driver's private data
 * \param buffer  Pointer to the string to send
 * \return  Always 0
 */
static int
send_tele(PrivateData *p, char *buffer)
{
    return real_send_tele(p, buffer, strlen(buffer));
}

/**
 * Sends an acknowledge to the display.
 */
static int
send_ACK(PrivateData *p)
{
    return send_tele(p, "Q");
}

/**
 * Returns the current time in microseconds since the Epoch.
 */
static unsigned long long
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec * 1000000 + (unsigned long long)tv.tv_usec;
}


/**
 * Sets the serial device, used for communication with the LCD, into raw mode.
 * \param drvthis  Pointer to driver
 * \param FD       File descriptor of serial device to configure.
 * \return  -1 on error, 0 otherwise
 */
static int
initTTY(Driver *drvthis, int FD)
{
    struct termios tty_mode;

    if (tcgetattr(FD, &tty_mode) == 0) {
	cfmakeraw(&tty_mode);
#ifdef CBAUDEX			/* CBAUDEX not defined in FreeBSD */
	tty_mode.c_cflag |= CBAUDEX;
#endif
	cfsetospeed(&tty_mode, B115200);
	cfsetispeed(&tty_mode, 0);
	tty_mode.c_cc[VMIN] = 1;
	tty_mode.c_cc[VTIME] = 1;

	if (tcsetattr(FD, TCSANOW, &tty_mode) != 0) {
	    report(RPT_ERR, "%s: setting TTY failed: %s", drvthis->name, strerror(errno));
	    return -1;
	}
    }
    else {
	report(RPT_ERR, "%s: reading TTY failed: %s", drvthis->name, strerror(errno));
	return -1;
    }

    return 0;
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
pyramid_init(Driver *drvthis)
{
    char buffer[MAXCOUNT];
    int i;
    PrivateData *p;

    /* get memory for private data */
    p = (PrivateData *) malloc(sizeof(PrivateData));
    if ((p == NULL) || (drvthis->store_private_ptr(drvthis, p) < 0)) {
	report(RPT_ERR, "%s: error allocating memory for modules private data", drvthis->name);
	return -1;
    }

    /* initialize elements of private data */
    p->width = WIDTH;
    p->height = HEIGHT;
    p->customchars = CUSTOMCHARS;
    p->cellwidth = CELLWIDTH;
    p->cellheight = CELLHEIGHT;
    p->ccmode = standard;
    memset(p->framebuffer, ' ', SCREEN_SIZE);
    memset(p->backingstore, ' ', SCREEN_SIZE);

    strcpy(p->last_key_pressed, NOKEY);
    p->last_key_time = timestamp();

    p->timeout.tv_sec = 0;
    p->timeout.tv_usec = MICROTIMEOUT;

    /*
     * read config file, fill configuration dependent elements of private
     * data
     */

    /* Which serial device should be used? */
    strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0, "/dev/lcd"), sizeof(p->device));
    p->device[sizeof(p->device) - 1] = '\0';
    report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

    /* Initialize connection to the LCD  */

    /* open and initialize serial device */
    p->FD = open(p->device, O_RDWR);

    if (p->FD == -1) {
	report(RPT_ERR, "%s: open(%s) failed: %s", drvthis->name, p->device, strerror(errno));
	return -1;
    }

    if (initTTY(drvthis, p->FD) != 0)
	return -1;

    /*
     * Acknowledge all telegramms, the device may yet be sending.
     * (Reset doesn't clear telegramms, darn protocol ... )
     */
    tcflush(p->FD, TCIFLUSH);	/* clear port buffer */
    while (1) {
	i = read_tele(p, buffer);
	if (i == True)
	    send_ACK(p);
	else
	    break;
	usleep(600000);
    }

    /* Initialize the display hardware: reset, clear and set cursor shape */
    send_tele(p, "R");
    send_tele(p, "C0101");
    send_tele(p, "D                                ");
    send_tele(p, "C0101");
    send_tele(p, "M3");

    /* invalidate LED status to make output(0) work */
    for (i = 0; i < 7; i++)
	p->led[i] = -1;
    /* hardware selftest + clear all LEDs */
    pyramid_output(drvthis, 0);	/* needed, otherwise L11 won't work */
    for (i = 0; i < 7; i++) {
	pyramid_output(drvthis, 1 << i);
	usleep(10000);
    }
    for (i = 6; i >= 0; i--) {
	pyramid_output(drvthis, 1 << i);
	usleep(10000);
    }
    pyramid_output(drvthis, 0);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
pyramid_close(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    if (p->FD) {
	tcflush(p->FD, TCIFLUSH);
	close(p->FD);
    }

}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
pyramid_width(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
pyramid_height(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
pyramid_clear(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    memset(p->framebuffer, ' ', SCREEN_SIZE);
    p->ccmode = standard;
}


/**
 * Flush data on screen to the display. Compares the current frame buffer
 * contents with the stored backing store. Send complete screen content if
 * they are different.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
pyramid_flush(Driver *drvthis)
{
    static char mesg[33];
    PrivateData *p = (PrivateData *) drvthis->private_data;
    int i;

    if (memcmp(p->framebuffer, p->backingstore, SCREEN_SIZE)) {
	memcpy(p->backingstore, p->framebuffer, SCREEN_SIZE);

	/* Prepare send buffer */
	mesg[0] = 'D';
	memcpy(mesg + 1, p->framebuffer, SCREEN_SIZE);

	/*
	 * we got the japanese HD44780U, so convert the german umlauts and
	 * other stuff, assuming input is iso_8859-1.
	 */
	for (i = 1; i < 33; i++) {
	    switch ((unsigned char)mesg[i]) {
	    case 0xe4:		/* ä */
		mesg[i] = 0xe1;
		break;
	    case 0xf6:		/* ö */
		mesg[i] = 0xef;
		break;
	    case 0xfc:		/* ü */
		mesg[i] = 0xf5;
		break;
	    case 0xdf:		/* ß */
		mesg[i] = 0xe2;
		break;
	    case 0xb7:		/* · */
		mesg[i] = 0xa5;
		break;
	    case 0xb0:		/* ° */
		mesg[i] = 0xdf;
		break;
	    }
	}

	/* We do not wait for the ACK here, these are read by get_key() */
	send_tele(p, "C0101");
	real_send_tele(p, mesg, 33);

	/* Minimum execution time 40ms */
	usleep(40000);
    }

    /* Set cursor */
    sprintf(mesg, "C%02d%02d", p->C_x, p->C_y);
    real_send_tele(p, mesg, 5);
    sprintf(mesg, "M%d", p->C_state);
    real_send_tele(p, mesg, 2);
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
pyramid_string(Driver *drvthis, int x, int y, const char string[])
{
    int offset;
    int len;
    PrivateData *p = (PrivateData *) drvthis->private_data;

    x = min(p->width, x);
    y = min(p->height, y);
    offset = (x - 1) + p->width * (y - 1);
    len = min(strlen(string), p->width * p->height - offset + 1);
    memcpy(&p->framebuffer[offset], string, len);
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
pyramid_chr(Driver *drvthis, int x, int y, char c)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    x = min(p->width, x);
    y = min(p->height, y);
    p->framebuffer[(x - 1) + p->width * (y - 1)] = c;
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8 (= cellheight) bytes, each representing a row in
 *                 CGRAM starting from the top. The five rightmost bits in
 *                 each byte represent the pixels in that row.
 */
MODULE_EXPORT void
pyramid_set_char(Driver *drvthis, int n, unsigned char *dat)
{
    char tele[10] = "G@ABCDEFGH";
    int row, pixels;
    PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned char mask = (1 << p->cellwidth) - 1;

    if (n < 0 || n > 7) {
	debug(RPT_WARNING, "only characters 0-7 can be changed");
	return;
    }

    if (!dat) {
	debug(RPT_WARNING, "no character data");
	return;
    }

    if (memcmp(dat, p->cc_cache[n], CELLHEIGHT)) {
	memcpy(p->cc_cache[n], dat, CELLHEIGHT);

	/* which character? */
	tele[1] = n + 0x40;

	for (row = 0; row < p->cellheight; row++) {
	    pixels = dat[row] & mask;

	    /* pixel information is transferred with an offset of 40h */
	    pixels |= 0x40;
	    tele[row + 2] = pixels;
	}
	real_send_tele(p, tele, 10);
	usleep(200);		/* extra delay required for processing this */
    }
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of custom characters.
 */
MODULE_EXPORT int
pyramid_get_free_chars(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    return (p->customchars);
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
pyramid_cellwidth(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    return (p->cellwidth);
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
pyramid_cellheight(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    return (p->cellheight);
}


/**
 * Defines some custom characters.
 * These characters are enabled if the value 0x100 is sent to the \c output command.
 * \param drvthis  Pointer to driver structure.
 */
static void
pyramid_init_custom1(Driver *drvthis)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    static unsigned char a[] = {
	b_______,
	b_______,
	b_______,
	b___XXXX,
	b___XXXX,
	b___XXXX,
	b___XXXX,
	b___XXXX
    };

    static unsigned char b[] = {
	b_______,
	b_______,
	b_______,
	b__XXXXX,
	b__XXXX_,
	b__XXX__,
	b__XX___,
	b__X____,
    };

    static unsigned char c[] = {
	b___XXXX,
	b___XXXX,
	b___XXXX,
	b___XXXX,
	b___XXXX,
	b_______,
	b_______,
	b_______
    };

    static unsigned char d[] = {
	b__X____,
	b__XX___,
	b__XXX__,
	b__XXXX_,
	b__XXXXX,
	b_______,
	b_______,
	b_______
    };

    if (p->ccmode != custom) {
	pyramid_set_char(drvthis, 1, a);
	pyramid_set_char(drvthis, 2, b);
	pyramid_set_char(drvthis, 3, c);
	pyramid_set_char(drvthis, 4, d);
	p->ccmode = custom;
    }
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
pyramid_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    static unsigned char vBars[7][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
	{0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F},
	{0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
	{0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
	{0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    };

    if (p->ccmode != vbar) {
	int i;

	if (p->ccmode != standard) {
	    report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
		   drvthis->name);
	    return;
	}

	for (i = 0; i < 7; i++)
	    pyramid_set_char(drvthis, i + 1, vBars[i]);
	p->ccmode = vbar;
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
pyramid_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    static unsigned char hBars[4][8] = {
	{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
	{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
	{0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
	{0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
    };

    if (p->ccmode != hbar) {
	int i;

	if (p->ccmode != standard) {
	    report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
		   drvthis->name);
	    return;
	}

	for (i = 0; i < 4; i++)
	    pyramid_set_char(drvthis, i + 1, hBars[i]);
	p->ccmode = hbar;
    }

    lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
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
pyramid_icon(Driver *drvthis, int x, int y, int icon)
{
    static unsigned char heart_open[] = {
	b__XXXXX,
	b__X_X_X,
	b_______,
	b_______,
	b_______,
	b__X___X,
	b__XX_XX,
	b__XXXXX
    };
    static unsigned char heart_filled[] = {
	b__XXXXX,
	b__X_X_X,
	b___X_X_,
	b___XXX_,
	b___XXX_,
	b__X_X_X,
	b__XX_XX,
	b__XXXXX
    };
    static unsigned char arrow_up[] = {
	b____X__,
	b___XXX_,
	b__X_X_X,
	b____X__,
	b____X__,
	b____X__,
	b____X__,
	b_______
    };
    static unsigned char arrow_down[] = {
	b____X__,
	b____X__,
	b____X__,
	b____X__,
	b__X_X_X,
	b___XXX_,
	b____X__,
	b_______
    };
    static unsigned char checkbox_off[] = {
	b_______,
	b_______,
	b__XXXXX,
	b__X___X,
	b__X___X,
	b__X___X,
	b__XXXXX,
	b_______
    };
    static unsigned char checkbox_on[] = {
	b____X__,
	b____X__,
	b__XXX_X,
	b__X_XX_,
	b__X_X_X,
	b__X___X,
	b__XXXXX,
	b_______
    };
    static unsigned char checkbox_gray[] = {
	b_______,
	b_______,
	b__XXXXX,
	b__X_X_X,
	b__XX_XX,
	b__X_X_X,
	b__XXXXX,
	b_______
    };
    static unsigned char ellipsis[] = {
	b_______,
	b_______,
	b_______,
	b_______,
	b_______,
	b_______,
	b__X_X_X,
	b_______
    };

    switch (icon) {
    case ICON_BLOCK_FILLED:
	pyramid_chr(drvthis, x, y, 255);
	break;

    case ICON_HEART_OPEN:
	pyramid_set_char(drvthis, 0, heart_open);
	pyramid_chr(drvthis, x, y, 0);
	break;

    case ICON_HEART_FILLED:
	pyramid_set_char(drvthis, 0, heart_filled);
	pyramid_chr(drvthis, x, y, 0);
	break;

    case ICON_ARROW_UP:
	pyramid_set_char(drvthis, 2, arrow_up);
	pyramid_chr(drvthis, x, y, 2);
	break;

    case ICON_ARROW_DOWN:
	pyramid_set_char(drvthis, 3, arrow_down);
	pyramid_chr(drvthis, x, y, 3);
	break;

    case ICON_ARROW_LEFT:
	pyramid_chr(drvthis, x, y, 127);
	break;

    case ICON_ARROW_RIGHT:
	pyramid_chr(drvthis, x, y, 126);
	break;

    case ICON_CHECKBOX_OFF:
	pyramid_set_char(drvthis, 4, checkbox_off);
	pyramid_chr(drvthis, x, y, 4);
	break;

    case ICON_CHECKBOX_ON:
	pyramid_set_char(drvthis, 5, checkbox_on);
	pyramid_chr(drvthis, x, y, 5);
	break;

    case ICON_CHECKBOX_GRAY:
	pyramid_set_char(drvthis, 6, checkbox_gray);
	pyramid_chr(drvthis, x, y, 6);
	break;

    case ICON_ELLIPSIS:
	pyramid_set_char(drvthis, 7, ellipsis);
	pyramid_chr(drvthis, x, y, 7);
	break;

    default:
	debug(RPT_DEBUG, "%s: x=%d, y=%d, icon=%x", __FUNCTION__, x, y, icon);
	return -1;
    }
    return 0;
}


/**
 * API: Draw a big number (0-9 and colon) on the screen.
 */
MODULE_EXPORT void
pyramid_num(Driver *drvthis, int x, int num)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    int do_init = 0;

    if ((num < 0) || (num > 10))
	return;

    if (p->ccmode != bignum) {
	if (p->ccmode != standard) {
	    report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
		   drvthis->name);
	    return;
	}
	p->ccmode = bignum;
	do_init = 1;
    }

    lib_adv_bignum(drvthis, x, num, 0, do_init);
};


/**
 * Set cursor position and state. Cursor position and state is not immediately
 * set, but stored in PrivateData and set on flush.
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal cursor position (column).
 * \param y        Vertical cursor position (row).
 * \param state    New cursor state.
 */
MODULE_EXPORT void
pyramid_cursor(Driver *drvthis, int x, int y, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;

    p->C_x = x;
    p->C_y = y;

    switch (state) {
    case CURSOR_OFF:
	state = 0;
	break;
    case CURSOR_DEFAULT_ON:
	state = 3;
	break;
    case CURSOR_UNDER:
	state = 1;
	break;
    case CURSOR_BLOCK:
	state = 2;
	break;
    default:
	report(RPT_WARNING, "%s: Unknown cursor state %d", drvthis->name, state);
	state = 0;
    }
    p->C_state = state;
}


/**
 * Set output port.
 * Setting an output port bit lights the associated LED;
 * re-setting the bit turns the LED off.
 * \param drvthis  Pointer to driver structure.
 * \param state    Integer with bits representing port states.
 */
MODULE_EXPORT void
pyramid_output(Driver *drvthis, int state)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    int i;
    char tele[] = {"L00"};

    for (i = 0; i < 7; i++) {
	if (p->led[i] != (state & (1 << i))) {
	    p->led[i] = state & (1 << i);
	    tele[1] = i + '1';
	    tele[2] = p->led[i] ? '1' : '0';
	    send_tele(p, tele);
	}
    }

    if (state & (1 << 8)) {
	pyramid_init_custom1(drvthis);
    }

}


/**
 * Get key from the device
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key.
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
pyramid_get_key(Driver *drvthis)
{
    /* supports only one key at a time */

    static char buffer[MAXCOUNT];	/* has to be static to be visible
					 * outside this function */
    unsigned long long current_time;
    int retval;
    PrivateData *p = (PrivateData *) drvthis->private_data;

    /*
     * Now we read everything from the display, and as long as we got ACKs,
     * we ignore them. (eat up all pending ACKs)
     */

    while (1) {
	retval = read_tele(p, buffer);
	if ((retval == False) || (buffer[0] != 'Q'))
	    break;
    }
    if (retval == False)
	strcpy(buffer, p->last_key_pressed);
    else
	send_ACK(p);

    /* If a key wasn't released yet it may be released now. */
    if (buffer[0] == 'K') {
	/* test if its a release event */
	if ((strcmp(buffer, "K0003") == 0)
	    || (strcmp(buffer, "K0030") == 0)
	    || (strcmp(buffer, "K0300") == 0)
	    || (strcmp(buffer, "K3000") == 0)) {
	    debug(RPT_DEBUG, "%s: Key released: %s", __FUNCTION__, p->last_key_pressed);
	    strcpy(p->last_key_pressed, NOKEY);
	    return NULL;
	}
	else {			/* It must be a new key event */
	    strcpy(p->last_key_pressed, buffer);
	    debug(RPT_DEBUG, "%s: Key pressed: %s", __FUNCTION__, p->last_key_pressed);
	}
    }
    /* If no keys are pressed at this time, we are done. */
    if (p->last_key_pressed[0] == NOKEY[0])
	return NULL;

    current_time = timestamp();
    if (current_time > p->last_key_time + 500000)	/* New keys only every
							 * 0.5 seconds */
	p->last_key_time = current_time;
    else
	return NULL;

    if (strcmp(p->last_key_pressed, "K0001") == 0)	/* first from left */
	return "Up";
    if (strcmp(p->last_key_pressed, "K0010") == 0)	/* second from left */
	return "Down";
    if (strcmp(p->last_key_pressed, "K0100") == 0)	/* third from left */
	return "Enter";
    if (strcmp(p->last_key_pressed, "K1000") == 0)	/* last from left */
	return "Escape";

#ifdef PYRAMID_DECODE_COMBINED_KEYPRESSES
    /* Do we really want to type that much */
    if (strcmp(p->last_key_pressed, "K0012") == 0)	/* A+B */
	return "Up+Down";
    if (strcmp(p->last_key_pressed, "K0021") == 0)	/* B+A */
	return "Down+Up";

    if (strcmp(p->last_key_pressed, "K0102") == 0)	/* A+C */
	return "Up+Enter";
    if (strcmp(p->last_key_pressed, "K0201") == 0)	/* C+A */
	return "Enter+Up";
    if (strcmp(p->last_key_pressed, "K1002") == 0)	/* A+D */
	return "Up+Escape";
    if (strcmp(p->last_key_pressed, "K2001") == 0)	/* D+A */
	return "Escape+Up";

    if (strcmp(p->last_key_pressed, "K0120") == 0)	/* B+C */
	return "Down+Enter";
    if (strcmp(p->last_key_pressed, "K0210") == 0)	/* C+B */
	return "Enter+Down";
    if (strcmp(p->last_key_pressed, "K1020") == 0)	/* B+D */
	return "Down+Escape";
    if (strcmp(p->last_key_pressed, "K2010") == 0)	/* D+B */
	return "Escape+Down";

    if (strcmp(p->last_key_pressed, "K0012") == 0)	/* C+D */
	return "Enter+Escape";
    if (strcmp(p->last_key_pressed, "K0021") == 0)	/* D+C */
	return "Escape+Enter";
#endif

    return NULL;		/* Ignore combined key events */
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
pyramid_get_info(Driver *drvthis)
{
    static char *pyramid_info_string = "Pyramid LCD driver";

    return pyramid_info_string;
}
