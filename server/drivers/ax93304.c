/** \file server/drivers/ax93304.c
 * LCDd \c ax93304 driver for AX93304 LCD modules by Axiomtek
*/

/*
    This is the LCDproc driver for Axiomtek AX93304

    Applicable external links:
    - https://www.axiomtek.com/
	- https://www.axiomtek.com/Download/download/NA590/LCM%20for%20AX93304.zip

    Copyright (C) 2026 Sebastian Stolz

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
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
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
#ifdef SOLARIS
#  include <strings.h>
#endif
#include "lcd.h"
#include "ax93304.h"
#include "shared/report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"

#define NUM_CCs 8 /* number of characters */

#define AX93304_DEFAULT_DEVICE	"/dev/lcd"

#define AX93304_WIDTH               16
#define AX93304_HEIGHT              2
#define AX93304_CELLWIDTH           5
#define AX93304_CELLHEIGHT          8

#define AX93304_CMD                 0xFE
#define AX93304_BACKLIGHT_ON        0xFB
#define AX93304_BACKLIGHT_OFF       0xFC
#define AX93304_READ_KEY            0xFD
#define AX93304_ESCAPE              0xFF

#define AX93304_CHAR_ARROW_RIGHT    0x1A
#define AX93304_CHAR_ARROW_LEFT     0x1B
#define AX93304_CHAR_BLOCK_FILLED   0xFF /* Data byte; ax93304_write_data() prefixes AX93304_ESCAPE */

/* Key input is enabled once during init, then handled passively.
 * Some AX93304 firmware appears to require a single 0xFD key-listen command,
 * but continuously sending 0xFD from get_key() can slow display updates.
 */

static int
ax93304_write_all(int fd, const void *buf, size_t len)
{
  const unsigned char *p = (const unsigned char *) buf;

  while (len > 0) {
    ssize_t n = write(fd, p, len);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      return -1;
    }
    if (n == 0)
      return -1;
    p += n;
    len -= n;
  }
  return 0;
}

static void
ax93304_write_byte(int fd, unsigned char byte)
{
  ax93304_write_all(fd, &byte, 1);
}

static void
ax93304_write_cmd(int fd, unsigned char cmd)
{
  unsigned char buf[2] = { AX93304_CMD, cmd };
  ax93304_write_all(fd, buf, sizeof(buf));
}

static void
ax93304_write_data(int fd, const char *buf, size_t len)
{
  size_t i;

  for (i = 0; i < len; i++) {
    unsigned char c = (unsigned char) buf[i];

    /* 0xFB..0xFF are control bytes on the AX93304 protocol. */
    if (c >= AX93304_BACKLIGHT_ON)
      ax93304_write_byte(fd, AX93304_ESCAPE);

    ax93304_write_byte(fd, c);
  }
}

static void
ax93304_goto(int fd, int row, int col)
{
  unsigned char addr = ((row == 0) ? 0x00 : 0x40) + (unsigned char) col;
  ax93304_write_cmd(fd, 0x80 | addr);
}


/** private data for the \c ax93304 driver */
typedef struct ax93304_private_data {
  char device[256];
  int speed;
  int fd;
  int width;
  int height;
  int cellwidth;
  int cellheight;
  char *framebuf;
  CGmode ccmode;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "ax93304_";


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
ax93304_init(Driver *drvthis)
{
  PrivateData *p;
  struct termios portset;

  /* Allocate and store private data */
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
    return -1;
  if (drvthis->store_private_ptr(drvthis, p))
    return -1;

  /* initialize private data */
  p->fd = -1;
  p->speed = B9600;
  p->width = AX93304_WIDTH;
  p->height = AX93304_HEIGHT;
  p->cellwidth = AX93304_CELLWIDTH;
  p->cellheight = AX93304_CELLHEIGHT;
  p->framebuf = NULL;
  p->ccmode = standard;


  /* Read config file */

  /* What device should be used */
  strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   AX93304_DEFAULT_DEVICE), sizeof(p->device));
  p->device[sizeof(p->device)-1] = '\0';
  report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

  /* What speed to use */
  p->speed = drvthis->config_get_int(drvthis->name, "Speed", 0, 9600);

  if (p->speed == 1200)       p->speed = B1200;
  else if (p->speed == 2400)  p->speed = B2400;
  else if (p->speed == 9600)  p->speed = B9600;
  else if (p->speed == 19200) p->speed = B19200;
  else {
    report(RPT_WARNING, "%s: illegal Speed %d; must be one of 1200, 2400, 9600 or 19200; using default %d",
		    drvthis->name, p->speed, 9600);
    p->speed = B9600;
  }

  // Set up io port correctly, and open it...
  p->fd = open(p->device, O_RDWR | O_NOCTTY);
  if (p->fd == -1) {
    report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
    return -1;
  }

  //debug(RPT_DEBUG, "ax93304_init: opened device %s", device);

  tcflush(p->fd, TCIOFLUSH);

  if (tcgetattr(p->fd, &portset) < 0) {
    report(RPT_ERR, "%s: tcgetattr(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
    ax93304_close(drvthis);
    return -1;
  }

  // We use RAW mode: 9600 8N1, no flow control
#ifdef HAVE_CFMAKERAW
  cfmakeraw(&portset);
#else
  portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                        | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY );
  portset.c_oflag &= ~OPOST;
  portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
  portset.c_cflag &= ~( CSIZE | PARENB | CSTOPB | CRTSCTS );
  portset.c_cflag |= CS8;
#endif

  portset.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
  portset.c_cflag |= CS8 | CREAD | CLOCAL;
#ifdef CRTSCTS
  portset.c_cflag &= ~CRTSCTS;
#endif
#ifdef CNEW_RTSCTS
  portset.c_cflag &= ~CNEW_RTSCTS;
#endif
  portset.c_iflag &= ~(IXON | IXOFF | IXANY);

  portset.c_cc[VTIME] = 0;
  portset.c_cc[VMIN] = 0;

  cfsetospeed(&portset, p->speed);
  cfsetispeed(&portset, p->speed);

  if (tcsetattr(p->fd, TCSANOW, &portset) < 0) {
    report(RPT_ERR, "%s: tcsetattr(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
    ax93304_close(drvthis);
    return -1;
  }
  tcflush(p->fd, TCIOFLUSH);

  /*------------------------------------*/

  p->framebuf = malloc(p->width * p->height);
  if (p->framebuf == NULL) {
    ax93304_close(drvthis);
    report(RPT_ERR, "%s: Error: unable to create framebuffer", drvthis->name);
    return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);

  /*** Open the port write-only, then fork off a process that reads chars ?!!? ***/

  /* Reset and clear the AX93304 */
  ax93304_write_byte(p->fd, AX93304_BACKLIGHT_ON);
  ax93304_write_cmd(p->fd, 0x01);  // clear screen
  usleep(2000);
  ax93304_write_cmd(p->fd, 0x0C);  // display on, cursor off
  ax93304_write_cmd(p->fd, 0x02);  // home

  /* Enable keypad listen mode once.  Do not repeat this in get_key(); LCDd
   * calls get_key() frequently and repeated 0xFD writes can starve display I/O.
   */
  ax93304_write_byte(p->fd, AX93304_READ_KEY);
  tcflush(p->fd, TCIFLUSH);

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ax93304_close(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "Closing AX93304");
  if (p != NULL) {
    if (p->fd >= 0) {
      ax93304_write_cmd(p->fd, 0x08);  // display off
      ax93304_write_byte(p->fd, AX93304_BACKLIGHT_OFF);
      close(p->fd);
    }

    if (p->framebuf != NULL)
      free(p->framebuf);

    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
ax93304_width(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
ax93304_height(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
ax93304_cellwidth(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
ax93304_cellheight(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellheight;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ax93304_clear(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
  p->ccmode = standard;
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
ax93304_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "AX93304 flush");
  ax93304_goto(p->fd, 0, 0);
  ax93304_write_data(p->fd, p->framebuf, p->width);
  ax93304_goto(p->fd, 1, 0);
  ax93304_write_data(p->fd, p->framebuf + p->width, p->width);

  /* Do not call tcdrain() here. LCDd may flush often; draining on every
   * refresh can make animations and the heartbeat appear very sluggish on
   * some UARTs/USB-serial stacks. The blocking write_all() above is enough
   * to enqueue complete frames reliably.
   */
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
ax93304_string(Driver *drvthis, int x, int y, const char string[])
{
  PrivateData *p = drvthis->private_data;
  int i;

  //debug(RPT_DEBUG, "Putting string %s at %i, %i", string, x, y);

  x--;  // Convert 1-based coords to 0-based...
  y--;

  for (i = 0; string[i] != '\0'; i++) {
    unsigned char c = (unsigned char) string[i];

    // Check for buffer overflows...
    if ((x + i) >= p->width || y < 0 || y >= p->height)
      break;

    if ((c > 0x7F) && (c < 0x98)) {
      report(RPT_WARNING, "%s: illegal char 0x%02X requested in ax93304_string()",
			 drvthis->name, c);
      c = ' ';
    }

    p->framebuf[(y * p->width) + x + i] = c;
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
ax93304_chr(Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = drvthis->private_data;
  unsigned char ch = (unsigned char) c;

  //debug(RPT_DEBUG, "Putting char %c (%#x) at %i, %i", c, c, x, y);

  y--;
  x--;

  if (x < 0 || x >= p->width || y < 0 || y >= p->height)
    return;

  if ((ch > 0x7F) && (ch < 0x98)) {
    report(RPT_WARNING, "%s: illegal char 0x%02X requested in ax93304_chr()",
			drvthis->name, c);
    ch = ' ';
  }

  /* No shifting the custom chars here, so ax93304_chr() can beep */
  p->framebuf[(y * p->width) + x] = ch;
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
ax93304_backlight(Driver *drvthis, int on)
{
  PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "Backlight %s", (on) ? "ON" : "OFF");

  if (on)
    ax93304_write_byte(p->fd, AX93304_BACKLIGHT_ON);
  else
    ax93304_write_byte(p->fd, AX93304_BACKLIGHT_OFF);
}


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8 (= cellheight) bytes, each representing a row in
 *                 CGRAM starting from the top.
 */
MODULE_EXPORT void
ax93304_set_char(Driver *drvthis, int n, unsigned char *dat)
{
  PrivateData *p = drvthis->private_data;
  int row;
  unsigned char mask = (1 << p->cellwidth) - 1;
 
  //debug(RPT_DEBUG, "Set char %i", n);
 
  if ((n < 0) || (n >= NUM_CCs)) /* Do we want to the aliased indexes as well (0x98 - 0x9F?) */
    return;
 
  if (!dat)
    return;
 
  /* Set the LCD to accept data for rewrite-able char n */
  ax93304_write_cmd(p->fd, 0x40 + (n * 8));
 
  for (row = 0; row < p->cellheight; row++) {
    unsigned char letter = dat[row] & mask;
    ax93304_write_byte(p->fd, letter);
  }
 
  /* Return to DDRAM. The next flush() will position the cursor explicitly. */
  ax93304_goto(p->fd, 0, 0);
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
ax93304_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;
  static unsigned char bar_up[7][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
  };

  //debug(RPT_DEBUG, "Vbar at %i, length %i", x, len);

  if (p->ccmode != vbar) {
    int i;

    if (p->ccmode != standard) {
      /* Not supported (yet) */
      report(RPT_WARNING, "%s: cannot combine two modes using user-defined characters",
		      drvthis->name);
      return;
    }
    p->ccmode = vbar;

    for (i = 0; i < 7; i++)
      ax93304_set_char(drvthis, i + 1, bar_up[i]);
  }

  lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0x01);
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
ax93304_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;
  static unsigned char bar_right[4][8] = {
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
    {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
  };

  //debug(RPT_DEBUG, "Hbar at %i,%i; length %i", x, y, len);

  if (p->ccmode != hbar) {
    int i;

    if (p->ccmode != standard) {
      report(RPT_WARNING, "%s: cannot combine two modes using user-defined characters",
	     drvthis->name);
      return;
    }
    p->ccmode = hbar;

    for (i = 0; i < 4; i++)
      ax93304_set_char(drvthis, i + 1, bar_right[i]);
  }

  lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0x01);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
ax93304_num(Driver *drvthis, int x, int num)
{
        PrivateData *p = drvthis->private_data;
        static int do_init = 1;

        if (p->ccmode != bignum) {
                if (p->ccmode != standard) {
                        report(RPT_WARNING,
                               "%s: num: cannot combine two modes using user-defined characters",
                               drvthis->name);
                        return;
                }

                p->ccmode = bignum;
                do_init = 1;
        }

        lib_adv_bignum(drvthis, x, num, 0, do_init);
        do_init = 0;
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
ax93304_get_free_chars(Driver *drvthis)
{
        return NUM_CCs;
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
ax93304_icon(Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	
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

	/* Icons from CGROM will always work */
	switch (icon) {
	    case ICON_ARROW_LEFT:
			ax93304_chr(drvthis, x, y, AX93304_CHAR_ARROW_LEFT);
			return 0;
	    case ICON_ARROW_RIGHT:
			ax93304_chr(drvthis, x, y, AX93304_CHAR_ARROW_RIGHT);
			return 0;
		case ICON_BLOCK_FILLED:
			ax93304_chr(drvthis, x, y, AX93304_CHAR_BLOCK_FILLED);
			return 0;
	}

	/* The heartbeat icons do not work in bignum and vbar mode */
	if ((icon == ICON_HEART_FILLED) || (icon == ICON_HEART_OPEN)) {
		if ((p->ccmode != bignum) && (p->ccmode != vbar)) {
			switch (icon) {
			    case ICON_HEART_FILLED:
					ax93304_set_char(drvthis, 7, heart_filled);
					ax93304_chr(drvthis, x, y, 7);
					return 0;
			    case ICON_HEART_OPEN:
					ax93304_set_char(drvthis, 7, heart_open);
					ax93304_chr(drvthis, x, y, 7);
					return 0;
			}
		}
		else {
			return -1;
		}
	}

	switch (icon) {
		case ICON_ARROW_UP:
			ax93304_set_char(drvthis, 1, arrow_up);
			ax93304_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_DOWN:
			ax93304_set_char(drvthis, 2, arrow_down);
			ax93304_chr(drvthis, x, y, 2);
			break;
		case ICON_CHECKBOX_OFF:
			ax93304_set_char(drvthis, 3, checkbox_off);
			ax93304_chr(drvthis, x, y, 3);
			break;
		case ICON_CHECKBOX_ON:
			ax93304_set_char(drvthis, 4, checkbox_on);
			ax93304_chr(drvthis, x, y, 4);
			break;
		case ICON_CHECKBOX_GRAY:
			ax93304_set_char(drvthis, 5, checkbox_gray);
			ax93304_chr(drvthis, x, y, 5);
			break;
		default:
			return -1;	/* Let the core do other icons */
	}
	return 0;
}


/**
 * Get key from the device.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
ax93304_get_key(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  fd_set brfdset;
  struct timeval twait;
  char ch;
  int retval;

  /* Passive key handling:
   * The Axiomtek BSP sample reads key bytes directly from the UART and does
   * not continuously transmit 0xFD in its main loop. Avoid active key polling
   * here because LCDd calls get_key() frequently; sending 0xFD on every poll
   * can congest the tiny controller and make display refresh extremely slow.
   */
  FD_ZERO(&brfdset);
  FD_SET(p->fd, &brfdset);

  twait.tv_sec = 0;
  twait.tv_usec = 0;

  if (select(p->fd + 1, &brfdset, NULL, NULL, &twait) <= 0)
    return NULL;

  retval = read(p->fd, &ch, 1);
  if (retval <= 0) {
    if (retval < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
      report(RPT_ERR, "%s: Read error in AX93304 getchar", drvthis->name);
    return NULL;
  }

  debug(RPT_INFO, "ax93304_get_key: Got key: %c", ch);

  switch (ch) {
    case 'M': return "Up";
    case 'G': return "Down";
    case 'K': return "Escape";
    case 'N': return "Enter";
    default:  return NULL;
  }
}
