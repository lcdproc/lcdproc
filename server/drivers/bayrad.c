/* Code file for BayRAD driver
 * for LCDproc LCD software
 * by Nathan Yawn, yawn@emacinc.com
 * 3/24/01
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
#include "bayrad.h"
//#include "drv_base.h"
#include "shared/str.h"
#include "report.h"
#include "lcd_lib.h"

#define NUM_CCs 8 /* number of characters */
#define CCMODE_STANDARD 0 /* only char 0 is used for heartbeat */
#define CCMODE_VBAR 1
#define CCMODE_HBAR 2
#define CCMODE_BIGNUM 3

#define BAYRAD_DEFAULT_DEVICE	"/dev/lcd"


//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////

typedef struct driver_private_data {
  char device[256];
  int speed;
  int fd;
  int width;
  int height;
  int cellwidth;
  int cellheight;
  char *framebuf;
  char ccmode;
} PrivateData;	


// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "bayrad_";


////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
bayrad_init(Driver *drvthis)
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
  p->width = 20;
  p->height = 2;
  p->cellwidth = 5;
  p->cellheight = 8;
  p->framebuf = NULL;
  p->ccmode = CCMODE_STANDARD;


  /* Read config file */

  /* What device should be used */
  strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   BAYRAD_DEFAULT_DEVICE), sizeof(p->device));
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
  p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (p->fd == -1) {
    report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
    return -1;
  }

  //else debug(RPT_DEBUG, "bayrad_init: opened device %s", device);

  tcflush(p->fd, TCIOFLUSH);

  // We use RAW mode
#ifdef HAVE_CFMAKERAW
  // The easy way
  cfmakeraw(&portset);
#else
  // The hard way
  portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                        | INLCR | IGNCR | ICRNL | IXON );
  portset.c_oflag &= ~OPOST;
  portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
  portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
  portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

  portset.c_cc[VTIME] = 0;  // Don't use the timer, no workee
  portset.c_cc[VMIN] = 1;  // Need at least 1 char

  // Set port speed
  cfsetospeed(&portset, B9600);
  cfsetispeed(&portset, B0);

  // Do it...
  tcsetattr(p->fd, TCSANOW, &portset);
  tcflush(p->fd, TCIOFLUSH);

  /*------------------------------------*/

  p->framebuf = malloc(p->width * p->height);
  if (p->framebuf == NULL) {
    bayrad_close(drvthis);
    report(RPT_ERR, "%s: Error: unable to create framebuffer", drvthis->name);
    return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);

  /*** Open the port write-only, then fork off a process that reads chars ?!!? ***/

  /* Reset and clear the BayRAD */
  write(p->fd, "\x80\x86\x00\x1a\x1e", 5);  // sync,reset to type 0, clear screen, home

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 1;
}


// Below here, you may use either lcd.framebuf or drvthis->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

MODULE_EXPORT void
bayrad_close(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "Closing BayRAD");
  if (p != NULL) {
    if (p->fd >= 0) {
      write(p->fd, "\x8e\x00", 2);  // Backlight OFF
      close(p->fd);
    }  

    if (p->framebuf != NULL)
      free(p->framebuf);

    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);
}


/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
bayrad_width(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->width;
}


/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
bayrad_height(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellheight;
}


/////////////////////////////////////////////////////////////////
// Returns the display's cell width
//
MODULE_EXPORT int
bayrad_cellwidth(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->width;
}


/////////////////////////////////////////////////////////////////
// Returns the display's cell height
//
MODULE_EXPORT int
bayrad_cellheight(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellheight;
}


/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
bayrad_clear(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
  p->ccmode = CCMODE_STANDARD;
}


//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
bayrad_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "BayRAD flush");
  write(p->fd, "\x80\x1e", 2);  //sync, home
  write(p->fd, p->framebuf, 20);
  write(p->fd, "\x1e\x0a", 2);  //home, LF
  write(p->fd, p->framebuf+20, 20);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
bayrad_string(Driver *drvthis, int x, int y, const char string[])
{
  PrivateData *p = drvthis->private_data;
  int i;

  //debug(RPT_DEBUG, "Putting string %s at %i, %i", string, x, y);

  x--;  // Convert 1-based coords to 0-based...
  y--;

  for (i = 0; string[i] != '\0'; i++) {
    unsigned char c = (unsigned char) string[i];

    // Check for buffer overflows...
    if ((y * p->width) + x + i > (p->width * p->height))
      break;

    if ((c > 0x7F) && (c < 0x98)) {
      //c &= 0x7F;
      report(RPT_WARNING, "%s: illegal char 0x%02X requested in bayrad_string()",
			 drvthis->name, c);
      c = ' ';
    }

    if (c < 8)      /* The custom characters are mapped at 0x98 - 0x9F, */
      c += 0x98;   /* as 0x07 makes a beep instead of printing a character */

     p->framebuf[(y * p->width) + x + i] = c;
  }
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,2).
//
MODULE_EXPORT void
bayrad_chr(Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = drvthis->private_data;
  unsigned char ch = (unsigned char) c;

  //debug(RPT_DEBUG, "Putting char %c (%#x) at %i, %i", c, c, x, y);

  y--;
  x--;

  if ((ch > 0x7F) && (ch < 0x98)) {
    report(RPT_WARNING, "%s: illegal char 0x%02X requested in bayrad_chr()",
			drvthis->name, c);
    ch = ' ';
  }

  /* No shifting the custom chars here, so bayrad_chr() can beep */
  p->framebuf[(y * p->width) + x] = ch;
}

//////////////////////////////////////////////////////////////////////
// Turns the lcd backlight on or off...
//
MODULE_EXPORT void
bayrad_backlight(Driver *drvthis, int on)
{
  //PrivateData *p = drvthis->private_data;

  /* This violates the LCDd driver model, but it does leave the
   * backlight control entirely in the hands of the user via
   * BayRAd buttons, which is nice, since the backlights have
   * a finite lifespan... */

  if (on) {
    ;
    //write(p->fd, "\x8e\x0f", 2);
    //debug(RPT_DEBUG, "Backlight ON");
  }
  else {
    ;
    //write(p->fd, "\x8e\x00", 2);
    //debug(RPT_DEBUG, "Backlight OFF");
  }
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for vertical bargraphs.
//
static void
bayrad_init_vbar(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  static char bar_up[7][5*8] = {
      {
	 0,0,0,0,0, //  char u1[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
      }, {
	 0,0,0,0,0, //  char u2[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u3[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u4[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u5[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u6[] =
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u7[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      }
  };

  //debug(RPT_DEBUG,"Init Vertical bars");

  if (p->ccmode == CCMODE_VBAR) {
    /* Work already done */
    return;
  }

  if (p->ccmode != CCMODE_STANDARD) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: cannot combine two modes using user-defined characters",
		    drvthis->name);
    return;
  }
  p->ccmode = CCMODE_VBAR;

  bayrad_set_char(drvthis, 1, bar_up[0]);
  bayrad_set_char(drvthis, 2, bar_up[1]);
  bayrad_set_char(drvthis, 3, bar_up[2]);
  bayrad_set_char(drvthis, 4, bar_up[3]);
  bayrad_set_char(drvthis, 5, bar_up[4]);
  bayrad_set_char(drvthis, 6, bar_up[5]);
  bayrad_set_char(drvthis, 7, bar_up[6]);
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for horizontal bargraphs.
//
static void
bayrad_init_hbar(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  static char bar_right[5][5*8] = {
      {
	 1,0,0,0,0, //  char r1[] =
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
      },{
	 1,1,0,0,0, //  char r2[] =
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
      },{
	 1,1,1,0,0, //  char r3[] =
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
      },{
	 1,1,1,1,0, //  char r4[] =
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
      },{
	 1,1,1,1,1, //  char r5[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      }
  };

  //debug(RPT_DEBUG,"Init Horizontal bars");

  if (p->ccmode == CCMODE_HBAR) {
    /* Work already done */
    return;
  }

  if (p->ccmode != CCMODE_STANDARD) {
    /* Not supported (yet) */
    report(RPT_WARNING, "%s: cannot combine two modes using user-defined characters",
		    drvthis->name);
    return;
  }
  p->ccmode = CCMODE_HBAR;

  bayrad_set_char(drvthis, 1, bar_right[0]);
  bayrad_set_char(drvthis, 2, bar_right[1]);
  bayrad_set_char(drvthis, 3, bar_right[2]);
  bayrad_set_char(drvthis, 4, bar_right[3]);
  bayrad_set_char(drvthis, 5, bar_right[4]);
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for big numbers, if possible.
//
static void
bayrad_init_num(Driver *drvthis)
{
  //PrivateData *p = drvthis->private_data;

//  debug(RPT_DEBUG,"Big Numbers");
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
MODULE_EXPORT void
bayrad_num(Driver *drvthis, int x, int num)
{
  //PrivateData *p = drvthis->private_data;

//  debug(RPT_DEBUG,"BigNum(%i, %i)", x, num);
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
MODULE_EXPORT void
bayrad_set_char(Driver *drvthis, int n, char *dat)
{
  PrivateData *p = drvthis->private_data;
  char out[4];
  int row, col;

  //debug(RPT_DEBUG, "Set char %i", n);

  if ((n < 0) || (n >= NUM_CCs)) /* Do we want to the aliased indexes as well (0x98 - 0x9F?) */
    return;

  if (!dat)
    return;

  /* Set the LCD to accept data for rewrite-able char n */
  snprintf(out, sizeof(out), "\x88%c", 0x40 + (n * 8));
  write(p->fd, out, 2);

  for (row = 0; row < p->cellheight; row++) {
    char letter = 0;

    for (col = 0; col < p->cellwidth; col++) {
      letter <<= 1;
      letter |= (dat[(row * p->cellwidth) + col] > 0);
    }
    write(p->fd, &letter, 1);
  }

  /* return the LCD to normal operation */
  write(p->fd, "\x80", 1);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
bayrad_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "Vbar at %i, length %i", x, len);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'up' direction
   * (other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles (0..1000) that the bar should be filled.
   */

  bayrad_init_vbar(drvthis);

  lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0x98);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
bayrad_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;

  //debug(RPT_DEBUG, "Hbar at %i,%i; length %i", x, y, len);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'right' direction
   * (other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles (0..1000) that the bar should be filled.
   */

  bayrad_init_hbar(drvthis);

  lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0x98);
}


/////////////////////////////////////////////////////////////////
// Places an icon on screen
//
MODULE_EXPORT int
bayrad_icon(Driver *drvthis, int x, int y, int icon)
{
  /*PrivateData *p = drvthis->private_data;
  static char icons[3][5*8] = {
   {
     1,1,1,1,1,  // Empty Heart
     1,0,1,0,1,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,0,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },{
     1,1,1,1,1,  // Filled Heart
     1,0,1,0,1,
     0,1,0,1,0,
     0,1,1,1,0,
     0,1,1,1,0,
     1,0,1,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },{
     0,0,0,0,0,  // Ellipsis
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,1,0,1,
   }
  };*/

  switch (icon) {
    case ICON_BLOCK_FILLED:
      bayrad_chr( drvthis, x, y, 0xFF );
      break;
    default:
      return -1; /* Let the core do other icons */
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
MODULE_EXPORT const char *
bayrad_get_key(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  fd_set brfdset;
  struct timeval twait;
  char readchar;
  int retval;
  static char ret_val[2] = { 0, 0 };

  //debug(RPT_DEBUG, "BayRAD get_key...");

  /* Check for incoming data.  Turn backlight ON/OFF as needed */

  /* TODO: NEEDS TO BE ADAPTED TO RETURN REAL KEY DESCRIPTION STRINGS ! */

  FD_ZERO(&brfdset);
  FD_SET(p->fd, &brfdset);

  twait.tv_sec = 0;
  twait.tv_usec = 0;

  if (select(p->fd + 1, &brfdset, NULL, NULL, &twait)) {
    retval = read(p->fd, &readchar, 1);
    if (retval > 0) {
      debug(RPT_INFO, "bayrad_get_key: Got key: %c", readchar);

      if (readchar == 'Y') {
        write(p->fd, "\x8e\x0f", 2);
      }
      else if (readchar == 'N') {
        write(p->fd, "\x8e\x00", 2);
      }
    }  /* if read returned data */
    else {
      /* Read error */
      report(RPT_ERR, "%s: Read error in BayRAD getchar", drvthis->name);
    }
  }  /* if select */
  else {
      ;//debug(RPT_DEBUG, "No BayRAD data present");
  }

  ret_val[0] = readchar;
  return ret_val;
}

