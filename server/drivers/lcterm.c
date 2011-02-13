/** \file server/drivers/lcterm.c
 * LCDd \c lcterm driver for the LCTerm serial LCD terminal from Helmut Neumark Elektronik,
 * www.neumark.de.
 *
 * \todo Support keyboard input
 */

/*
  Copyright (C) 2002  Michael Schwingen <michael@schwingen.org>

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

  This driver is mostly based on the HD44780 and the LCDM001 driver.
  (Hopefully I have NOT forgotten any file I have stolen code from.
  If so send me an e-mail or add your copyright here!)
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
#include "lcd_lib.h"
#include "lcterm.h"
#include "report.h"
#include "adv_bignum.h"


/** private data for the \c lcterm driver */
typedef struct lcterm_private_data {
  CGmode ccmode;	/**< custom character mode for current display */
  CGmode last_ccmode;	/**< custom character set that is loaded in the display */
  unsigned char *framebuf;	/**< frame buffer */
  unsigned char *last_framebuf;	/**< old frame buffer contents */
  int width;		/**< display width in characters */
  int height;		/**< display height in characters */
  int fd;		/**< handle to the device */
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "lcterm_";


/**
 * Initialize the driver.
 * Open com port and set baud correctly...
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
lcterm_init (Driver *drvthis)
{
  char device[200];
  int speed=B9600;
  struct termios portset;
  PrivateData *p;

  debug(RPT_INFO, "LCTERM: init(%p)", drvthis);

  // Alocate and store private data
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
    return -1;
  if (drvthis->store_private_ptr(drvthis, p))
    return -1;

  // initialize private data
  p->fd = -1;
  p->ccmode = p->last_ccmode = standard;

  // READ CONFIG FILE:
  // which serial device should be used
  strncpy(device, drvthis->config_get_string(drvthis->name , "Device" , 0 , DEFAULT_DEVICE),
	  sizeof(device));
  device[sizeof(device)-1] = '\0';
  report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

  /* Get and parse size */
  {
    int w, h;
    const char *s = drvthis->config_get_string(drvthis->name, "Size", 0, "16x2");

    debug(RPT_DEBUG, "%s: reading size: %s", __FUNCTION__, s);

    if ((sscanf(s, "%dx%d", &w, &h) != 2)
	|| (w <= 0) || (w > LCD_MAX_WIDTH)
	|| (h <= 0) || (h > LCD_MAX_HEIGHT))
    {
      report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
		      drvthis->name, s, "16x2");
      sscanf("16x2", "%dx%d", &w, &h);
    }
    p->width  = w;
    p->height = h;
  }
  report(RPT_INFO, "%s: using Size: %dx%d", drvthis->name, p->width, p->height);

  p->framebuf = malloc(p->width * p->height);
  p->last_framebuf = malloc(p->width * p->height);
  if ((p->framebuf == NULL) || (p->last_framebuf == NULL)) {
    report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
    return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);
  memset(p->last_framebuf, ' ', p->width * p->height);

  // Set up io port correctly, and open it...
  debug(RPT_DEBUG, "%s: Opening serial device: %s", drvthis->name, device);
  p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (p->fd == -1) {
    report(RPT_ERR, "%s: open(%) failed (%s)", drvthis->name, device, strerror(errno));
    if (errno == EACCES)
	report(RPT_ERR, "%s: make sure you have rw access to %s!", drvthis->name, device);
    return -1;
  }
  report(RPT_INFO, "%s: opened display on %s", drvthis->name, device);

  tcgetattr(p->fd, &portset);
#ifdef HAVE_CFMAKERAW
  /* The easy way: */
  cfmakeraw(&portset);
#else
  /* The hard way: */
  portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
			| INLCR | IGNCR | ICRNL | IXON );
  portset.c_oflag &= ~OPOST;
  portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
  portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
  portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif
  cfsetospeed(&portset, speed);
  cfsetispeed(&portset, speed);
  tcsetattr(p->fd, TCSANOW, &portset);
  tcflush(p->fd, TCIOFLUSH);

  // clear the display, disable cursor, disable key scanning
  write(p->fd, "\x1a\x16\x1bK", 4);

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
lcterm_close (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  if (p != NULL) {
    if (p->framebuf != NULL)
      free(p->framebuf);
    if (p->last_framebuf != NULL)
      free(p->last_framebuf);

    // clear the display, disable key scanning
    if (p->fd >= 0) {
      write(p->fd, "\x1a\x1bK", 3);
      close(p->fd);
    }

    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);
  report(RPT_INFO, "%s: closed", drvthis->name);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
lcterm_width (Driver *drvthis)
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
lcterm_height (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
lcterm_clear (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
  p->ccmode = standard;
}


/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
lcterm_flush (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int i, line;
  unsigned char *buf, *sp, *dp, c;

  if (memcmp(p->framebuf, p->last_framebuf, p->width * p->height) == 0)
    return;

  buf = alloca(p->width * p->height * 2 + 5); /* worst case: we need to
						 escape *every* character */
  dp = buf;
  sp = p->framebuf;

  *dp++ = 0x1E;  // cursor home

  for (line = p->height; line > 0; line--)
  {
    for (i = p->width; i > 0; i--)
    {
      if ((c = *sp++) < 0x08) // need to escape used-defined characters
	*dp++ = 0x1B;
      *dp++ = c;
    }
    *dp++ = 0x0a;
    *dp++ = 0x0d;
  }
  write(p->fd, buf, dp-buf);
  memcpy(p->last_framebuf, p->framebuf, p->width * p->height);
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
lcterm_chr (Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  y--;
  x--;
  //debug(RPT_DEBUG, "lcterm_chr: x=%d, y=%d, c=%x", x,y,c);
  if ((x >= 0) && (x < p->width) && (y >= 0) && (y < p->height))
    p->framebuf[y * p->width + x] = c;
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
lcterm_string (Driver *drvthis, int x, int y, const char string[])
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  x --;  // Convert 1-based coords to 0-based
  y --;

  for ( ; (*string != '\0') && (x < p->width); x++)
    p->framebuf[y * p->width + x] = *string++;
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always NUM_CCs).
 */
MODULE_EXPORT int
lcterm_get_free_chars(Driver *drvthis)
{
	return NUM_CCs;
}

/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8 (= cellheight) bytes, each representing a row in
 *                 CGRAM starting from the top.
 */
MODULE_EXPORT void
lcterm_set_char (Driver *drvthis, int n, unsigned char *dat)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int row;
  int data;
  unsigned char buf[11];
  unsigned char mask = (1 << LCD_DEFAULT_CELLWIDTH) - 1;

  if ((n < 0) || (n > 7) || (!dat))
    return;

  buf[0] = 0x1F;
  buf[1] = 8 * n;   // CG RAM address */
  for (row = 0; row < LCD_DEFAULT_CELLHEIGHT; row++) {
    data = dat[row] & mask;
    buf[2+row] = data | 0x80;
  }
  buf[10] = 0x1E; // Cursor Home - exit CG-RAM mode
  write(p->fd, buf, 11);
}


/**
 * Set up vertical bars.
 * \param drvthis  Pointer to driver structure.
 */
static void
lcterm_init_vbar (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  static unsigned char vbar_char[8][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
  };
  int i;

  if (p->last_ccmode == vbar)    /* Work already done */
    return;

  if (p->ccmode != standard) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: init_vbar: cannot combine two modes using user-defined characters",
	   drvthis->name);
    return;
  }

  p->ccmode = p->last_ccmode = vbar;

  for (i = 0; i < 8; i++)
    lcterm_set_char(drvthis, i + 1, vbar_char[i]);
}


/**
 * Set up horizontal bars.
 * \param drvthis  Pointer to driver structure.
 */
static void
lcterm_init_hbar (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int i;
  static unsigned char hbar_char[5][8] = {
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
    {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
  };

  if (p->last_ccmode == hbar) /* Work already done */
    return;

  if (p->ccmode != standard) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: init_hbar: cannot combine two modes using user-defined characters",
           drvthis->name);
    return;
  }

  p->ccmode = p->last_ccmode = hbar;

  for (i = 0; i < 5; i++)
    lcterm_set_char(drvthis, i + 1, hbar_char[i]);
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
lcterm_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
  lcterm_init_vbar(drvthis);
  lib_vbar_static(drvthis, x, y, len, promille, options, LCD_DEFAULT_CELLHEIGHT, 0);
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
lcterm_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  lcterm_init_hbar(drvthis);
  lib_hbar_static(drvthis, x, y, len, promille, options, LCD_DEFAULT_CELLWIDTH, 0);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
lcterm_num (Driver *drvthis, int x, int num)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int do_init = 0;

  if ((num < 0) || (num > 10))
    return;

  if (p->height >= 4) {		/* Use standard bignum library */
    if (p->last_ccmode != bignum) {
      if (p->ccmode != standard) {
	report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
	       drvthis->name);
	return;
      }

      p->ccmode = p->last_ccmode = bignum;
      do_init = 1;
    }

    lib_adv_bignum(drvthis, x, num, 0, do_init);
  }
  else {			/* Roll our own 'mini big numbers' */
    lcterm_chr(drvthis, x, 1 + (p->height - 1) / 2, (num == 10) ? ':' : (num + '0'));
  }
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
lcterm_icon (Driver *drvthis, int x, int y, int icon)
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

  switch (icon)
  {
    case ICON_BLOCK_FILLED:
      lcterm_chr(drvthis, x, y, 255);
      break;
    case ICON_HEART_FILLED:
      lcterm_set_char(drvthis, 0, heart_filled);
      lcterm_chr(drvthis, x, y, 0);
      break;
    case ICON_HEART_OPEN:
      lcterm_set_char(drvthis, 0, heart_open);
      lcterm_chr(drvthis, x, y, 0);
      break;
    default:
      return -1;
  }
  return 0;
}
