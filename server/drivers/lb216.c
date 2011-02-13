/** \file server/drivers/lb216.c
 * LCDd \c lb216 driver for the LB216 by R.T.N. Australia.
 */

/*
 * Chris Debenham - Sun Systems Engineer <chris.debenham@aus.sun.com>
 *
 * Heres a bit more info on the display.
 * It is the LB216 and is made by R.T.N. Australia
 * The web page for it is http://www.nollet.com.au/
 * It is a serial 16x2 LCD with software controllable backlight.
 * They also make 40x4 displays (which I'll be getting one of soon :-) )
 * 3 wire connection (5V,0V and serial), 2400 or 9600 bps.
 * 8 custom characters
 * 40*83.5MM size
 * made in australia :-)
 *
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
#include "lb216.h"
#include "lcd_lib.h"
#include "report.h"

#define LB216_DEFAULT_DEVICE		"/dev/lcd"
#define LB216_DEFAULT_SPEED		9600
#define LB216_DEFAULT_BRIGHTNESS	255

/** private data for the \c lb216 driver */
typedef struct LB216_private_data {
	char device[256];
	int speed;
	int fd;
	char *framebuf;
	int width;
	int height;
	int cellwidth;
	int cellheight;
   	int backlight_brightness;
	CGmode custom;
} PrivateData;


static void LB216_hidecursor(Driver *drvthis);
static void LB216_reboot(Driver *drvthis);

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "LB216_";


// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
LB216_init(Driver *drvthis)
{
  PrivateData *p;
  struct termios portset;
  int reboot = 0;

  /* Allocate and store private data */
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
    return -1;
  if (drvthis->store_private_ptr(drvthis, p))
    return -1;

  /* initialize private data */
  p->speed = LB216_DEFAULT_SPEED;
  p->fd = -1;
  p->framebuf = NULL;
  p->width = LCD_DEFAULT_WIDTH;
  p->height = LCD_DEFAULT_HEIGHT;
  p->cellwidth = LCD_DEFAULT_CELLWIDTH;
  p->cellheight = LCD_DEFAULT_CELLHEIGHT;
  p->custom = standard;

  /* Read config file */

  /* What device should be used */
  strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
					     LB216_DEFAULT_DEVICE), sizeof(p->device));
  p->device[sizeof(p->device)-1] = '\0';
  report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

  /* What speed to use */
  p->speed = drvthis->config_get_int(drvthis->name, "Speed", 0, LB216_DEFAULT_SPEED);

  if (p->speed == 2400)       p->speed = B2400;
  else if (p->speed == 9600)  p->speed = B9600;
  else {
    report(RPT_WARNING, "%s: illegal Speed: %d; must be 2400 or 9600; using default %d",
		    drvthis->name, p->speed, LB216_DEFAULT_SPEED);
    p->speed = B9600;
  }

  /* Which backlight brightness */
  p->backlight_brightness = drvthis->config_get_int(drvthis->name, "Brightness", 0, LB216_DEFAULT_BRIGHTNESS);
  if ((p->backlight_brightness < 0) || (p->backlight_brightness > 255)) {
    report(RPT_WARNING, "%s: Brightness must be between 0 and 255; using default %d",
		    drvthis->name, LB216_DEFAULT_BRIGHTNESS);
    p->backlight_brightness = LB216_DEFAULT_BRIGHTNESS;
  }

  /* Reboot display? */
  reboot = drvthis->config_get_bool(drvthis->name , "Reboot", 0, 0);

  /* End of config file parsing */

  // Set up io port correctly, and open it...
  p->fd = open(p->device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (p->fd == -1) {
    report(RPT_ERR, "%s: open(%s) failed (%s)", drvthis->name, p->device, strerror(errno));
    return -1;
  }
  report(RPT_DEBUG, "%s: opened device %s", drvthis->name, p->device);

  tcgetattr(p->fd, &portset);

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

  // Set port speed
  cfsetospeed(&portset, p->speed);
  cfsetispeed(&portset, B0);

  // Do it...
  tcsetattr(p->fd, TCSANOW, &portset);

  // Make sure the frame buffer is there...
  p->framebuf = malloc(p->width * p->height);
  if (p->framebuf == NULL) {
     report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
     return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);

  // Set display-specific stuff..
  if (reboot) {
    report(RPT_INFO, "%s: rebooting LCD...", drvthis->name);
    LB216_reboot(drvthis);
    sleep(4);
    reboot = 0;
  }
  sleep(1);
  LB216_hidecursor(drvthis);
  LB216_backlight(drvthis, p->backlight_brightness);

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 0;
}



/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
LB216_close(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  if (p != NULL) {
    if (p->fd >= 0)
      close(p->fd);

    if (p->framebuf)
      free(p->framebuf);

    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
LB216_width (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
LB216_height (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->height;
}


/////////////////////////////////////////////////////////////////
// Returns the display's cell width
//
MODULE_EXPORT int
LB216_cellwidth (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellwidth;
}

/////////////////////////////////////////////////////////////////
// Returns the display's cell height
//
MODULE_EXPORT int
LB216_cellheight (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->cellheight;
}


/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
LB216_clear (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
}


/////////////////////////////////////////////////////////////////
// Flushes the framebuffer to the LCD
//
MODULE_EXPORT void
LB216_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
  int i, j;

  snprintf(out, sizeof(out), "%c%c", 254, 80);
  write(p->fd, out, 2);

  for (j = 0; j < p->height; j++) {
    if (j >= 2)
      snprintf(out, sizeof(out), "%c%c", 254, 148 + (64 * (j - 2)));
    else
      snprintf(out, sizeof(out), "%c%c", 254, 128 + (64 * j));
    write(p->fd, out, 2);

    for (i = 0; i < p->width; i++)
      write(p->fd, &p->framebuf[i + (j * p->width)], 1);
  }
}


/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
LB216_chr(Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = drvthis->private_data;

  y--;
  x--;

  if ((unsigned char) c == 254) 	/* protect command starting char */
    c= '#';

  if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
    p->framebuf[(y * p->width) + x] = c;
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
MODULE_EXPORT void
LB216_backlight(Driver *drvthis, int on)
{
  PrivateData *p = drvthis->private_data;

  char out[4];

  snprintf(out, sizeof(out), "%c%c", 254, (on) ? 253 : 252);
  write(p->fd, out, 2);
}


/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void LB216_hidecursor(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  char out[4];

  snprintf(out, sizeof(out), "%c%c", 254, 12);
  write(p->fd, out, 2);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void LB216_reboot(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  char out[4];

  snprintf(out, sizeof(out), "%c%c", 254, 1);
  write(p->fd, out, 2);
}


MODULE_EXPORT void
LB216_string (Driver *drvthis, int x, int y, const char string[])
{
  PrivateData *p = drvthis->private_data;
  int i;

  y--;
  x--;

  if ((y < 0) || (y >= p->height))
    return;

  for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
    unsigned char c = string[i];

    if (c == 254) 	/* protect command starting char */
      c= '#';

    if (x >= 0)
      p->framebuf[(y * p->width) + x] = c;
  }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
LB216_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;
  static unsigned char vbar_char[7][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
  };

  if (p->custom != vbar) {
    int i;

    for (i = 0; i < 7; i++)
      LB216_set_char(drvthis, i + 1, vbar_char[i]);
    p->custom = vbar;
  }

  lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
LB216_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
  PrivateData *p = drvthis->private_data;
  static unsigned char hbar_char[5][8] = {
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C},
    {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E},
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}
  };

  if (p->custom != hbar) {
    int i;

    for (i = 0; i < 5; i++)
        LB216_set_char(drvthis, i + 1, hbar_char[i]);
    p->custom = hbar;
  }
  lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
LB216_set_char(Driver *drvthis, int n, unsigned char *dat)
{
  PrivateData *p = drvthis->private_data;
  char out[4];
  int row;
  unsigned char mask = (1 << p->cellwidth) - 1;

  if ((n < 0) || (n > 7) || (!dat))
    return;

  snprintf(out, sizeof(out), "%c%c", 254, 64 + (8 * n));
  write(p->fd, out, 2);

  for (row = 0; row < p->cellheight; row++) {
    int letter = dat[row] & mask;

    /* For some reason the previous implementation had the 6th bit set. */
    letter |= 0x20;

    snprintf(out, sizeof(out), "%c", letter);
    write(p->fd, out, 1);
  }
}


MODULE_EXPORT int
LB216_icon(Driver *drvthis, int x, int y, int icon)
{
  static unsigned char heart_open[] =
    {0x1F, 0x15, 0x00, 0x00, 0x00, 0x11, 0x1B, 0x1F};

  static unsigned char heart_filled[] =
    {0x1F, 0x15, 0x0A, 0x0E, 0x0E, 0x15, 0x1B, 0x1F};


  switch (icon) {
    case ICON_BLOCK_FILLED:
      LB216_chr(drvthis, x, y, 255);
      break;
    case ICON_HEART_FILLED:
      LB216_set_char(drvthis, 0, heart_filled);
      LB216_chr(drvthis, x, y, 0);
      break;
    case ICON_HEART_OPEN:
      LB216_set_char(drvthis, 0, heart_open);
      LB216_chr(drvthis, x, y, 0);
      break;
    default:
      return -1;
  }
  return 0;
}

