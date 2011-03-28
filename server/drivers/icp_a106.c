/** \file server/drivers/icp_a106.c
 * LCDd \c icp_a106 for the ICP A106 alarm/LCD board used in 19 inch rack cases by ICP.
 */

/*
  This is the LCDproc driver for the ICP A106 alarm/LCD board,
  used in 19" rack cases by ICP

  Both LCD and alarm functions are accessed via one serial port, using
  separate commands. Unfortunately, the device runs at slow 1200bps and the
  LCD does not allow user-defined characters, so the bargraphs do not look
  very nice.

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
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "icp_a106.h"
#include "report.h"


/** private data for the \c icp_a106 driver */
typedef struct icp_a106_private_data {
  unsigned char *framebuf;
  unsigned char *last_framebuf;
  int width;
  int height;
  int fd;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "icp_a106_";


/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
icp_a106_init (Driver *drvthis)
{
  char device[200];
  int speed=B1200;
  struct termios portset;

  PrivateData *p;

  debug(RPT_INFO, "ICP_A106: init(%p)", drvthis );

  // Alocate and store private data
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
    return -1;
  if (drvthis->store_private_ptr(drvthis, p))
    return -1;

  // initialize PrivateData
  p->fd = -1;
  p->width = 20;
  p->height = 2;

  // READ CONFIG FILE:
  // which serial device should be used
  strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE),
	  sizeof(device));
  device[sizeof(device)-1] = '\0';
  report(RPT_INFO, "%s: using Device %s", drvthis->name, device);

  p->framebuf = malloc(p->width * p->height);
  p->last_framebuf = malloc(p->width * p->height);
  if ((p->framebuf == NULL) || (p->last_framebuf == NULL)) {
    report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
    return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);
  memset(p->last_framebuf, ' ', p->width * p->height);

  // Set up io port correctly, and open it...
  debug(RPT_DEBUG, "%s: opening serial device: %s", __FUNCTION__, device);
  p->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (p->fd == -1) {
    report(RPT_ERR, "%s: init() failed (%s)", drvthis->name, strerror(errno));
    if (errno == EACCES)
	report(RPT_ERR, "%s: make sure you have rw access to %s!", drvthis->name, device);
    return -1;
  }
  report(RPT_INFO, "%: opened display on %s", drvthis->name, device);

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

  // stop auto clock display, clear display
  write(p->fd, "\x4D\x28\x4D\x0D", 4);

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 0;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
icp_a106_close (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  if (p != NULL) {
    if (p->framebuf != NULL)
      free(p->framebuf);
    if (p->last_framebuf != NULL)
      free(p->last_framebuf);

    // clear display, start auto display
    if (p->fd >= 0) {
      write(p->fd, "\x4D\x0D\x4D\x29", 4);
      close(p->fd);
    }
    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);

  report(RPT_INFO, "%s: closed", drvthis->name);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
icp_a106_width (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
icp_a106_height (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  return p->height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
icp_a106_clear (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
icp_a106_flush (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  /*
    The ICP A106 is a bit difficult to handle - 1200bps, ho handshake, and
    the controller is easily overrun and displays garbage when it has too much
    of work to do. It seems we can handle two full updates per second, so if
    the last update was less than 0.5 seconds ago, we simply skip this one
    and update the display the next time.
  */
  struct timeval tv, tv2;
  static struct timeval tv_old; /* time of last update */
  int line;
  static char cmd[] = "\x4D\x0c\x00\x14";

  gettimeofday(&tv, 0);
  timersub(&tv, &tv_old, &tv2);

  if ((tv2.tv_sec ==0) && (tv2.tv_usec < 500000)) // less than 0.5s
    return;
  tv_old = tv;

  for (line = 0; line < p->height; line++) {
    if (memcmp(p->framebuf + line * p->width,
	       p->last_framebuf + line * p->width, p->width) != 0) {
      cmd[2] = line;
      write(p->fd, cmd, 4);
      write(p->fd, p->framebuf + line * p->width, 20);
    }
  }
  memcpy(p->last_framebuf, p->framebuf, p->width * p->height);
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
icp_a106_chr (Driver *drvthis, int x, int y, char ch)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  y--;
  x--;
  //debug(RPT_DEBUG, "icp_a106_chr: x=%d, y=%d, chr=%x", x,y,ch);
  if ((x >= 0) && (x < p->width) && (y >= 0) && (y < p->height))
    p->framebuf[ y * p->width + x] = ch;
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
icp_a106_string (Driver *drvthis, int x, int y, char *s)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  x--;  // Convert 1-based coords to 0-based
  y--;

  if ((y < 0) || (y >= p->height))
    return;

  for ( ; (*s != '\0') && (x < p->width); s++, x++)
    if (x >= 0)
      p->framebuf[y * p->width + x] = *s;
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
icp_a106_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
  int total_pixels = ((long) 2 * len * LCD_DEFAULT_CELLHEIGHT + 1 ) * promille / 2000;
  int pos;
  static char map[] = " __---=#";

  for (pos = 0; pos < len; pos ++) {
    int pixels = total_pixels - LCD_DEFAULT_CELLHEIGHT * pos;

    if (pixels >= LCD_DEFAULT_CELLHEIGHT) {
      /* write a "full" block to the screen... */
      icp_a106_icon(drvthis, x, y-pos, ICON_BLOCK_FILLED);
    }
    else {
      /* write a partial block... */
      icp_a106_chr(drvthis, x, y-pos, map[pixels]);
      break;
    }
  }
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
icp_a106_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  int total_pixels  = ((long) 2 * len * LCD_DEFAULT_CELLWIDTH + 1 ) * promille / 2000;
  int pos;

  for (pos = 0; pos < len; pos ++) {
    int pixels = total_pixels - LCD_DEFAULT_CELLWIDTH * pos;

    if (pixels >= LCD_DEFAULT_CELLWIDTH) {
      /* write a "full" block to the screen... */
      icp_a106_icon(drvthis, x+pos, y, ICON_BLOCK_FILLED);
    }
    else if (pixels > 0) {
      /* write a partial block... */
      icp_a106_chr(drvthis, x+pos, y, '|');
      break;
    }
    else {
      ; /* write nothing (not even a space) */
    }
  }
}


/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
icp_a106_num (Driver *drvthis, int x, int num)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  if ((num < 0) || (num > 10))
    return;

  icp_a106_chr(drvthis, x, 1 + (p->height - 1) / 2,
	       (num == 10) ? ':' : (num + '0'));
}


/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT int
icp_a106_icon (Driver *drvthis, int x, int y, int icon)
{
  switch (icon) {
    case ICON_BLOCK_FILLED:
      icp_a106_chr(drvthis, x, y, 255);
      break;
    case ICON_HEART_FILLED:
      break;
    case ICON_HEART_OPEN:
      break;
    default:
      return -1;
  }
  return 0;
}
