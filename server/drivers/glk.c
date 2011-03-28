/** \file server/drivers/glk.c
 * LCDd \c glk driver for graphical displays by MatrixOrbital (GLK series).
 */

/*
 * MatrixOrbital GLK Graphic Display Driver
 *
 * http://www.matrixorbital.com
 */

/*-
 * Copyright ???
 *
 * License ???
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

#include "lcd.h"
#include "glk.h"
#include "glkproto.h"
#include "report.h"
#include "adv_bignum.h"

#define GLK_DEFAULT_DEVICE	"/dev/lcd"
#define GLK_DEFAULT_SPEED	19200
#define GLK_DEFAULT_CONTRAST	560
#define GLK_DEFAULT_CELLWIDTH	6
#define GLK_DEFAULT_CELLHEIGHT	8


/** private data for the \c glk driver */
typedef struct glk_private_data {
  char device[256];
  GLKDisplay *fd;
  speed_t speed;

  const char *model;

  int fontselected;
  int gpo_count;

  unsigned char *framebuf;
  unsigned char *backingstore;

  int width;
  int height;

  int cellwidth;
  int cellheight;

  int contrast;

  int clearcount;
  unsigned char CGRAM[8];
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "glk_";



/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0   Success.
 * \retval <0  Error.
 */
MODULE_EXPORT int
glk_init(Driver *drvthis)
{
  PrivateData *p;
  int i;

  /* Allocate and store private data */
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
    return -1;
  if (drvthis->store_private_ptr(drvthis, p))
    return -1;

  /* initialize private data */
  p->fd = NULL;
  p->speed = GLK_DEFAULT_SPEED;
  p->backingstore = NULL;
  p->fontselected = -1;		/* No font selected */
  p->gpo_count = 0;
  p->framebuf = NULL;
  p->cellwidth = GLK_DEFAULT_CELLWIDTH;
  p->cellheight = GLK_DEFAULT_CELLHEIGHT;
  p->contrast = GLK_DEFAULT_SPEED;
  p->clearcount = 0;

  /* Read config file */

  /* What device should be used */
  strncpy(p->device, drvthis->config_get_string(drvthis->name, "Device", 0,
					     GLK_DEFAULT_DEVICE), sizeof(p->device));
  p->device[sizeof(p->device)-1] = '\0';
  report(RPT_INFO, "%s: using Device %s", drvthis->name, p->device);

  /* What speed to use */
  p->speed = drvthis->config_get_int(drvthis->name, "Speed", 0, 19200);

  if (p->speed == 9600)       p->speed = B9600;
  else if (p->speed == 19200) p->speed = B19200;
  /* not in the specs: */
  //else if (p->speed == 38400) p->speed = B38400;
  else if (p->speed == 57600) p->speed = B57600;
  else if (p->speed == 115200) p->speed = B115200;
  else {
    report(RPT_WARNING, "%s: illegal Speed: %d; must be one of 9600, 19200, 57600 or 115200; using default %d",
		    drvthis->name, p->speed, GLK_DEFAULT_SPEED);
    p->speed = B19200;
  }

  /* Which contrast */
  p->contrast = drvthis->config_get_int(drvthis->name, "Contrast" , 0 , GLK_DEFAULT_CONTRAST);
  if ((p->contrast < 0) || (p->contrast > 1000)) {
    report(RPT_WARNING, "%s: Contrast must be between 0 and 1000. Using default %d",
		    drvthis->name, GLK_DEFAULT_CONTRAST);
    p->contrast = GLK_DEFAULT_CONTRAST;
  }

  /* End of config file parsing */

  /* open device */
  p->fd = glkopen(p->device, p->speed);
  if (p->fd == NULL) {
    report(RPT_ERR, "%s: unable to open device %s", drvthis->name, p->device);
    return -1;
  }

  /* Query the module for a device type */
  glkputl(p->fd, GLKCommand, 0x37, EOF);
  i = glkget(p->fd);
  if (i < 0) {
    report(RPT_ERR, "%s: GLK did not respond to READ MODULE TYPE", drvthis->name);
    return -1;
  }
  else {
    switch (i) {
      case 0x10 :
        p->model = "GLC12232";
	p->width = 20; p->height = 4;
	break;
      case 0x11 :
        p->model = "GLC12864";
	p->width = 20; p->height = 8;
	break;
      case 0x12 :
        p->model = "GLC128128";
	p->width = 20; p->height = 16;
	break;
      case 0x13 :
        p->model = "GLC24064";
	p->width = 40; p->height = 8;
	p->gpo_count = 1;
	break;
      case 0x14 :
        p->model = "GLK12864-25";
	p->width = 20; p->height = 8;
	break;
      case 0x15 :
        p->model = "GLK24064-25";
	p->width = 40; p->height = 8;
	p->gpo_count = 1;
	break;
      case 0x21 :
        p->model = "GLK128128-25";
	p->width = 20; p->height = 16;
	break;
      case 0x22 :
        p->model = "GLK12232-25";
	p->width = 20; p->height = 4;
	p->gpo_count = 2;
	break;
      case 0x23 :
      case 0x24 :
        p->model = "GLK12232-25-SM";
	p->width = 20; p->height = 4;
	p->gpo_count = 2;
	break;
      default :
	report(RPT_ERR, "%s: unrecognized module type: 0x%02X", drvthis->name, i);
	return -1;
    }
    report(RPT_INFO, "%s: Model identified by byte: 0x%02X; size: %ix%i",
    		drvthis->name, i, p->width, p->height);
  }

  p->framebuf = malloc(p->width * p->height);
  p->backingstore = malloc(p->width * p->height);

  if (p->framebuf == NULL || p->backingstore == NULL) {
    report(RPT_ERR, "%s: Unable to allocate memory for screen buffers", drvthis->name);
    glk_close(drvthis);
    return -1;
  }

  memset(p->framebuf, ' ', p->width * p->height);

  glkputl(p->fd, GLKCommand, 0x58, EOF);

  /* Enable flow control */
  glkflow(p->fd, 40, 2);

  /* Set read character timeout to 0 */
  glktimeout(p->fd, 0);

  /*
   * Enable auto-transmit of up/down key events. This allows us to generate
   * REPEAT keys distinct from normal keys using timeouts. (see glk_get_key)
   */
  glkputl(p->fd, GLKCommand, 0x7e, 1, GLKCommand, 0x41, EOF);

  /* Set p->contrast */
  glk_set_contrast(drvthis, p->contrast);

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glk_close(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	if (p != NULL) {
		if (p->fd != NULL)
			glkclose(p->fd);

		if (p->framebuf != NULL)
			free(p->framebuf);
		p->framebuf = NULL;

		if (p->backingstore != NULL)
			free(p->backingstore);
		p->backingstore = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is wide.
 */
MODULE_EXPORT int
glk_width(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of characters the display is high.
 */
MODULE_EXPORT int
glk_height(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return the width of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel columns a character cell is wide.
 */
MODULE_EXPORT int
glk_cellwidth(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return the height of a character in pixels.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of pixel lines a character cell is high.
 */
MODULE_EXPORT int
glk_cellheight(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


#define CLEARCOUNT  (1000000)

/**
 * Clears the LCD screen using a hardware command
 * \param drvthis  Pointer to driver structure.
 */
void glk_clear_forced(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	p->clearcount = CLEARCOUNT;
	glkputl(p->fd, GLKCommand, 0x58, EOF);
	memset(p->backingstore, ' ', p->width * p->height);
}

/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glk_clear(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	memset(p->framebuf, ' ', p->width * p->height);

	/* do a hardware clear very CLEARCOUNT invocation */
	if (--p->clearcount < 0)
		glk_clear_forced(drvthis);
}


/**
 * Flush data on screen to the LCD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
glk_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  unsigned char *pf = p->framebuf;
  unsigned char *qf = p->backingstore;
  int x, y;
  unsigned char *ps = NULL;

  debug(RPT_DEBUG, "flush()");

  for (y = 0; y < p->height; ++y) {
    int xs = -1;  /* XStart not set */

    for (x = 0; x < p->width; ++x) {
      if ((*qf == *pf) && (xs >= 0)) {
        /* Write accumulated string */
        glkputl(p->fd, GLKCommand, 0x79, xs * p->cellwidth + 1, y * p->cellheight, EOF);
        glkputa(p->fd, x - xs, ps);
        debug(RPT_DEBUG, "flush: Writing at (%d,%d) for %d", xs, y, x - xs);
        xs = -1;
      }
      else if ((*qf != *pf) && (xs < 0)) {
        /* Start new string of changes */
        ps = pf;
        xs = x;
      }
      *qf++ = *pf++;  /* Update p->backingstore from p->framebuf */
    }
    if (xs >= 0) {
      /* Write accumulated line */
      glkputl(p->fd, GLKCommand, 0x79, xs * p->cellwidth + 1, y * p->cellheight, EOF);
      glkputa(p->fd, p->width - xs, ps);
      debug(RPT_DEBUG, "flush: Writing at (%d,%d) for %d", xs, y, p->width - xs);
    }
  }
}


/**
 * Prints a string on the lcd display at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
glk_string(Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	const char *s;

	debug(RPT_DEBUG, "glk_string(%d, %d, \"%s\")", x, y, string);

	if ((y <= 0) || (y > p->height))
		return;

	for (s = string; (*s != '\0') && (x <= p->width); s++, x++) {
		glk_chr(drvthis, x, y, *s);
	}
}


/**
 * Print a character on the lcd at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
glk_chr(Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;
	int  myc = (unsigned char) c;

	x--;			/* Convert 1-based coords to 0-based... */
	y--;

	if (p->fontselected != 1) {
		debug(RPT_DEBUG, "Switching to font 1");
		/* Select font 2 */
		glkputl(p->fd, GLKCommand, 0x31, 1, EOF);
		p->fontselected = 1;
		/* Set font metrics */
		glkputl(p->fd, GLKCommand, 0x32, 1, 0, 0, 0, 32, EOF);
		/* Clear the screen */
		glk_clear_forced(drvthis);
	}

	if ((myc >= 0) && (myc <= 15)) {
		/* CGRAM */
		debug(RPT_DEBUG, "CGRAM changing %d => %d", myc, p->CGRAM[myc & 7]);
		myc = p->CGRAM[myc & 7];
	} else if ((myc == 255) || (myc == -1)) {
		/* Solid block */
		myc = 133;
	} else if (((myc > 15) && (myc < 32)) || (myc > 143)) {
		debug(RPT_DEBUG, "Attempt to write %d to (%d,%d)", myc, x, y);
		myc = 133;
	}

	if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
		p->framebuf[(y * p->width) + x] = myc;
}


/**
 * Get current LCD contrast.
 * This is only the locally stored contrast, the contrast value
 * cannot be retrieved from the LCD.
 * \param drvthis  Pointer to driver structure.
 * \return  Stored contrast in promille [0-1000].
 */
MODULE_EXPORT int
glk_get_contrast(Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->contrast;
}


/**
 * Sets the contrast of the display.
 * \param drvthis   Pointer to driver structure.
 * \param promille  New contrast value in promille.
 */
MODULE_EXPORT void
glk_set_contrast(Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;

	if ((promille < 0) || (promille > 1000))
		return;

	p->contrast = promille;

	/* Do it: map logical [0, 1000] -> physical [0, 255] for the hardware */
	debug(RPT_DEBUG, "Contrast: %d", p->contrast);
	glkputl(p->fd, GLKCommand, 0x50, (int) ((long) promille * 255 / 1000), EOF);
}


/**
 * Turn the LCD backlight on or off.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
glk_backlight(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (on) {
		debug(RPT_DEBUG, "Backlight ON");
		glkputl(p->fd, GLKCommand, 0x42, 0, EOF);
	}
	else {
		debug(RPT_DEBUG, "Backlight OFF");
		glkputl(p->fd, GLKCommand, 0x46, EOF);
	}
}


/**
 * Sets general purpose outputs on or off
 * \param drvthis  Pointer to driver structure.
 * \param on       Integer with bits representing GPIO states.
 */
MODULE_EXPORT void
glk_output(Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (p->gpo_count < 2) {
    		glkputl(p->fd, GLKCommand, ((on) ? 'W' : 'V'), EOF);
	}
	else {
		int  i;

		for (i = 1; i <= p->gpo_count; ++i, on >>= 1) {
			glkputl(p->fd, GLKCommand, ((on & 1) ? 'W' : 'V'), i, EOF);
		}
	}
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
glk_num(Driver *drvthis, int x, int num)
{
	int do_init = 1;

	debug(RPT_DEBUG, "glk_num(%d, %d)", x, num);

	if ((num < 0) || (num > 10))
		return;

	/* Lib_adv_bignum does everything needed to show the bignumbers. */
	lib_adv_bignum(drvthis, x, num, 0, do_init);
}


/**
 * Get total number of custom characters available. This is required in order
 * to be able to use bignum library.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters (always 0).
 */
MODULE_EXPORT int
glk_get_free_chars(Driver *drvthis)
{
	debug(RPT_DEBUG, "glk_get_free_chars()");

	return 0;
}


/**
 * Changes the font data of character n.
 * This function is a stub and currently does nothing.
 */
MODULE_EXPORT void
glk_set_char(Driver *drvthis, int n, char *dat)
{
	debug(RPT_DEBUG, "glk_set_char(%d)", n);
}


/**
 * Draws a vertical bar, from line y the screen up.
 */
MODULE_EXPORT void
glk_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;

	debug(RPT_DEBUG, "glk_old_vbar(%d, %d)", x, pixels);

	while (pixels > p->cellheight) {
		glk_chr(drvthis, x, y, 255);
		--y;
		pixels -= p->cellheight;
	}

	if (y >= 0) {
		int lastc;

		switch (pixels) {
			case 0 :  return;	/* Don't output a char */
			case 1 :  lastc = 138; break;  /* One bar */
			case 2 :  lastc = 139; break;
			case 3 :  lastc = 140; break;
			case 4 :  lastc = 141; break;
			case 5 :  lastc = 142; break;
			case 6 :  lastc = 143; break;
			default:  lastc = 133; break;
		}
		glk_chr(drvthis, x, y, lastc);
	}
}


/**
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
glk_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;

	debug(RPT_DEBUG, "glk_old_hbar(%d, %d, %d)", x, y, pixels);

	while (pixels > p->cellwidth) {
		glk_chr(drvthis, x, y, 255);
		++x;
		pixels -= p->cellwidth;
	}

	if (x <= p->width) {
		int lastc;

		switch (pixels) {
			case 0 :  lastc = ' '; break;
			case 1 :  lastc = 134; break;  /* One bar */
			case 2 :  lastc = 135; break;
			case 3 :  lastc = 136; break;
			case 4 :  lastc = 137; break;
			default:  lastc = 133; break;
		}
		glk_chr(drvthis, x, y, lastc);
	}
}


/**
 * API: Draw an icon character on the screen.
 */
MODULE_EXPORT int
glk_icon (Driver *drvthis, int x, int y, int icon)
{
    debug(RPT_DEBUG, "%s: x=%d, y=%d, icon=%x", __FUNCTION__, x, y, icon);

    switch (icon) {
      case ICON_BLOCK_FILLED:
	glk_chr(drvthis, x, y, 255);
	break;
      case ICON_HEART_FILLED:
	glk_chr(drvthis, x, y, 132);
	break;
      case ICON_HEART_OPEN:
	glk_chr(drvthis, x, y, 131);
	break;
      case ICON_ELLIPSIS:
	glk_chr(drvthis, x, y, 128);
	break;
      default:
	return -1;		/* Let the core do the others */
    }

    return 0;
}


/**
 * Get key from the key panel connected to the display.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
glk_get_key(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  int c;
  static int keycode = -1;
  static struct timeval lastkey;
  struct timeval now;
  const char *key = NULL;

  debug(RPT_DEBUG, "glk_get_key()");

  c = glkgetc(p->fd);

  if ((c >= 'A') && (c <= 'Z')) {
    /* Key down event */
    keycode = c;
    gettimeofday(&lastkey, NULL);
    debug(RPT_DEBUG, "KEY %c at %ld.%06ld", c, lastkey.tv_sec, lastkey.tv_usec);
  }
  else if ((c >= 'a') && (c <= 'z')) {
    /* Key up event */
    debug(RPT_DEBUG, "KEY %c UP", c);
    keycode = -1;
    c = 0;
  }
  else {
    /* Assume timeout */
    c = 0;
    if (keycode > 0) {
      int msec_diff;

      /* A key is down */
      gettimeofday(&now, NULL);
      msec_diff  = (now.tv_sec - lastkey.tv_sec) * 1000;
      msec_diff += (now.tv_usec - lastkey.tv_usec) / 1000;
      debug(RPT_DEBUG, "KEY %c down for %d msec", keycode, msec_diff);
      if (msec_diff > 1000) {
        /* Generate repeat event */
        c = keycode | 0x20;  /* Upper case to lower case */
        ++lastkey.tv_sec;  /* HACK HACK. repeat at 1 sec intervals */
        debug(RPT_DEBUG, "KEY %c REPEAT", c);
      }
    }
  }

  /* Remap keys according to what LCDproc expects */
  switch (c) {
    case 'V' : key = "Enter";
	       break;
    case 'P' : key = "Left";
	       break;
    case 'Q' : key = "Right";
	       break;
    case 'L' : key = "Escape";
	       break;
    case 'U' : key = "Up";
	       break;
    case 'K' : key = "Down";
	       break;
    default :  break;

    /* What to do with repeated keys? We currently ignore them. */
    //case 'v' : c = 'N'; break;
    //case 'p' : c = 'O'; break;
    //case 'q' : c = 'P'; break;
    //case 'l' : c = 'Q'; break;
    //case 'u' : c = 'R'; break;
    //case 'k' : c = 'S'; break;
  }

  debug(RPT_DEBUG, "%s_ get_key() returns %s", drvthis->name, (key != NULL) ? key : "<null>");

  return key;
}

/* EOF */
