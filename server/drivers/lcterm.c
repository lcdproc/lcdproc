/*lcterm.c*/
/*
  This is the LCDproc driver for the "LCTerm" serial LCD terminal
  from Helmut Neumark Elektronik, www.neumark.de

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307

  This driver is mostly based on the HD44780 and the LCDM001 driver.
  (Hopefully I have NOT forgotten any file I have stolen code from.
  If so send me an e-mail or add your copyright here!)

  TODO: support keyboard input
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

typedef enum
{
  CCMODE_STANDARD,    /* only char 0 is used for heartbeat */
  CCMODE_VBAR,
  CCMODE_HBAR,
  CCMODE_BIGNUM
} CCMode;

typedef struct
{
  CCMode ccmode;      /* custom character mode for current display */
  CCMode last_ccmode; /* custom character set that is loaded in the display */
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
MODULE_EXPORT char *symbol_prefix = "lcterm_";


/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
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
  p->ccmode = p->last_ccmode = CCMODE_STANDARD;

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

  return 1;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
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

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
lcterm_width (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
lcterm_height (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  return p->height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
lcterm_clear (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
  p->ccmode = CCMODE_STANDARD;
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
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

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
lcterm_chr (Driver *drvthis, int x, int y, char ch)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  y--;
  x--;
  //debug(RPT_DEBUG, "lcterm_chr: x=%d, y=%d, chr=%x", x,y,ch);
  if ((x >= 0) && (x < p->width) && (y >= 0) && (y < p->height))
    p->framebuf[y * p->width + x] = ch;
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
lcterm_string (Driver *drvthis, int x, int y, char *s)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  x --;  // Convert 1-based coords to 0-based
  y --;

  for ( ; (*s != '\0') && (x < p->width); x++)
    p->framebuf[y * p->width + x] = *s++;
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
lcterm_set_char (Driver *drvthis, int n, char *dat)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int row, col;
  int data;
  unsigned char buf[11];

  if ((n < 0) || (n > 7) || (!dat))
    return;

  buf[0] = 0x1F;
  buf[1] = 8 * n;   // CG RAM address */
  for (row = 0; row < 8; row++) {
    data = 0;
    for (col = 0; col < 5; col++) {
      data <<= 1;
      data |= (*dat++ != 0);
    }
    buf[2+row] = data | 0x80;
  }
  buf[10] = 0x1E; // Cursor Home - exit CG-RAM mode
  write(p->fd, buf, 11);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcterm->vbar()
//
static void
lcterm_init_vbar (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  static char vbar_1[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
  };
  static char vbar_2[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };
  static char vbar_3[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };
  static char vbar_4[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };
  static char vbar_5[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };
  static char vbar_6[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };
  static char vbar_7[] = {
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  if (p->last_ccmode == CCMODE_VBAR)    /* Work already done */
    return;

  if (p->ccmode != CCMODE_STANDARD) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: init_vbar: cannot combine two modes using user defined characters",
		    drvthis->name);
    return;
  }

  p->ccmode = p->last_ccmode = CCMODE_VBAR;

  lcterm_set_char(drvthis, 1, vbar_1);
  lcterm_set_char(drvthis, 2, vbar_2);
  lcterm_set_char(drvthis, 3, vbar_3);
  lcterm_set_char(drvthis, 4, vbar_4);
  lcterm_set_char(drvthis, 5, vbar_5);
  lcterm_set_char(drvthis, 6, vbar_6);
  lcterm_set_char(drvthis, 7, vbar_7);
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
lcterm_init_hbar (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  static char hbar_1[] = {
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
  };
  static char hbar_2[] = {
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
  };
  static char hbar_3[] = {
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
    1, 1, 1, 0, 0,
  };
  static char hbar_4[] = {
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
    1, 1, 1, 1, 0,
  };
  static char hbar_5[] = {
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
  };

  if (p->last_ccmode == CCMODE_HBAR) /* Work already done */
    return;

  if (p->ccmode != CCMODE_STANDARD) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: init_hbar: cannot combine two modes using user defined characters",
		    drvthis->name);
    return;
  }

  p->ccmode = p->last_ccmode = CCMODE_HBAR;

  lcterm_set_char(drvthis, 1, hbar_1);
  lcterm_set_char(drvthis, 2, hbar_2);
  lcterm_set_char(drvthis, 3, hbar_3);
  lcterm_set_char(drvthis, 4, hbar_4);
  lcterm_set_char(drvthis, 5, hbar_5);
}


/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
lcterm_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
  lcterm_init_vbar(drvthis);
  lib_vbar_static(drvthis, x, y, len, promille, options, LCD_DEFAULT_CELLHEIGHT, 0);
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
lcterm_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  lcterm_init_hbar(drvthis);
  lib_hbar_static(drvthis, x, y, len, promille, options, LCD_DEFAULT_CELLWIDTH, 0);
}


/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
static void
lcterm_init_num (Driver *drvthis)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;
  int i;

  static char bignum_ccs[8][5*8] = {{
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
  }, {
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0
  }, {
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
  }, {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0
  }, {
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 0, 0
  }, {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    0, 0, 0, 0, 0
  }, {
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0
  }, {
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
  }};

  if (p->last_ccmode == CCMODE_BIGNUM) {
    /* Work already done */
    return;
  }

  if (p->ccmode != CCMODE_STANDARD) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: init_num: cannot combine two modes using user defined characters",
		    drvthis->name);
    return;
  }

  p->ccmode = p->last_ccmode = CCMODE_BIGNUM;

  for (i = 0; i < 8; i++)
    lcterm_set_char(drvthis, i, bignum_ccs[i]);
}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
lcterm_num (Driver *drvthis, int x, int num)
{
  PrivateData *p = (PrivateData *) drvthis->private_data;

  static char bignum_map[11][4][3] = {
    { /* 0: */
      {1,2,3},
      {6,32,6},
      {6,32,6},
      {7,2,32}
    },
    { /* 1: */
      {7,6,32},
      {32,6,32},
      {32,6,32},
      {7,2,32},
    },
    { /* 2: */
      {1,2,3},
      {32,5,0},
      {1,32,32},
      {2,2,0},
    },
    { /* 3: */
      {1,2,3},
      {32,5,0},
      {3,32,6},
      {7,2,32}
    },
    { /* 4: */
      {32,3,6},
      {1,32,6},
      {2,2,6},
      {32,32,0}
    },
    { /* 5: */
      {1,2,0},
      {2,2,3},
      {3,32,6},
      {7,2,32}
    },
    { /* 6: */
      {1,2,32},
      {6,5,32},
      {6,32,6},
      {7,2,32}
    },
    { /* 7: */
      {2,2,6},
      {32,1,32},
      {32,6,32},
      {32,0,32}
    },
    { /* 8: */
      {1,2,3},
      {4,5,0},
      {6,32,6},
      {7,2,32}
    },
    { /* 9: */
      {1,2,3},
      {4,3,6},
      {32,1,32},
      {7,32,32}
    },
    { /* colon: */
      {32},
      {7},
      {7},
      {32}
    }};

  if ((num < 0) || (num > 10))
    return;

  if (p->height >= 4) {
    int y = (p->height - 2) / 2;
    int x2, y2;

    lcterm_init_num(drvthis);

    for (x2 = 0; x2 <= 2; x2++) {
      for (y2 = 0; y2 <= 3; y2++) {
	lcterm_chr(drvthis, x+x2, y+y2, bignum_map[num][y2][x2]);
      }	
      if (num == 10)
	x2 = 2; /* =break, for colon only */
    }
  }
  else
    lcterm_chr(drvthis, x, 1 + (p->height - 1) / 2,
	       (num == 10) ? ':' : (num + '0'));
}


/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
MODULE_EXPORT int
lcterm_icon (Driver *drvthis, int x, int y, int icon)
{
  static char heart_open[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 0, 0, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1 };

  static char heart_filled[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 1, 1, 0,
    1, 0, 1, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1 };

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

