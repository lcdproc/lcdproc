/*  This is the LCDproc driver for IO-Warrior devices (http://www.codemercs.de)

      Copyright(C) 2004-2006 Peter Marschall <peter@adpm.de>

   based on GPL'ed code:

   * IOWarrior LCD routines 
       Copyright (c) 2004  Christian Vogelgsang <chris@lallafa.de>
	
   * misc. files from LCDproc source tree

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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Debug mode: un-comment to turn on debugging messages in the server */
/* #define DEBUG 1 */

#include "lcd.h"
#include "hd44780-charmap.h"
#include "IOWarrior.h"
#include "report.h"
#include "lcd_lib.h"

#define  b_______	0x00
#define  b_____X_	0x03
#define  b_____XX	0x03
#define  b____X__	0x04
#define  b____XX_	0x06
#define  b___X___	0x08
#define  b___XX__	0x09
#define  b___X_X_	0x0A
#define  b___XXX_	0x0E
#define  b___XXXX	0x0F
#define  b__X____	0x10
#define  b__X___X	0x11
#define  b__X_X_X	0x15
#define  b__X_XX_	0x16
#define  b__XX___	0x18
#define  b__XX_XX	0x1B
#define  b__XXX__	0x1C
#define  b__XXX_X	0x1D
#define  b__XXXX_	0x1E
#define  b__XXXXX	0x1F



/* ===================== IOWarrior low level routines ====================== */

/* ------------------- IOWarrior LCD routines ------------------------------ */

/* write a set report to interface 1 of warrior */
static int iow_lcd_wcmd(usb_dev_handle *udh, unsigned char data[8])
{
  return(usb_control_msg(udh, USB_DT_HID, USB_REQ_SET_REPORT, 0, 1,
                         (char *) data, 8, iowTimeout) == 8) ? IOW_OK : IOW_ERROR;
}


/* ------------------- IOWarrior LED routines ------------------------------ */

/* write a set report to interface 0 of warrior */
static int iow_led_wcmd(usb_dev_handle *udh,int len,unsigned char *data)
{
  return (usb_control_msg(udh, USB_DT_HID, USB_REQ_SET_REPORT, 2, 0,
                          (char *) data, len, iowTimeout) == len) ? IOW_OK : IOW_ERROR;
}


/* ================== IOWarrior intermediate level routines ================ */

/* ------------------- IOWarrior LCD routines ------------------------------ */

/* start IOWarrior's LCD mode */
int iowlcd_enable(usb_dev_handle *udh)
{
  unsigned char lcd_cmd[8] = { 0x04, 0x01, 0, 0, 0, 0, 0, 0 };
  int res = iow_lcd_wcmd(udh,lcd_cmd);
  usleep(30000); /* wait for 30ms */
  return res;
}

/* leave IOWarrior's LCD mode */
int iowlcd_disable(usb_dev_handle *udh)
{
  unsigned char lcd_cmd[8] = { 0x04, 0x00, 0, 0, 0, 0, 0, 0 };
  int res = iow_lcd_wcmd(udh,lcd_cmd);
  usleep(30000);
  return res;
}

/* clear IOWarrior's display */
int iowlcd_display_clear(usb_dev_handle *udh)
{
  unsigned char lcd_cmd[8] = { 0x05, 1, 0x01, 0, 0, 0, 0, 0 };
  int res = iow_lcd_wcmd(udh,lcd_cmd);
  usleep(3000); /* 3ms */
  return res;
}

int iowlcd_display_on_off(usb_dev_handle *udh,int display,int cursor,int blink)
{
  unsigned char lcd_cmd[8] = { 0x05, 1, 0x08, 0, 0, 0, 0, 0 };
  if (display) lcd_cmd[2] |= 0x04;
  if (cursor)  lcd_cmd[2] |= 0x02;
  if (blink)   lcd_cmd[2] |= 0x01;
  return iow_lcd_wcmd(udh,lcd_cmd);
}

int iowlcd_set_function(usb_dev_handle *udh,int eight_bit,int two_line,int ten_dots)
{
  unsigned char lcd_cmd[8] = { 0x05, 1, 0x20, 0, 0, 0, 0, 0 };
  if (eight_bit) lcd_cmd[2] |= 0x10;
  if (two_line)  lcd_cmd[2] |= 0x08;
  if (ten_dots)  lcd_cmd[2] |= 0x04;
  return iow_lcd_wcmd(udh,lcd_cmd);
}

int iowlcd_set_cgram_addr(usb_dev_handle *udh,int addr)
{
  unsigned char lcd_cmd[8] = { 0x05, 1, 0x40, 0, 0, 0, 0, 0 };
  lcd_cmd[2] |= (addr & 0x3f);
  return iow_lcd_wcmd(udh,lcd_cmd);
}

int iowlcd_set_ddram_addr(usb_dev_handle *udh,int addr)
{
  unsigned char lcd_cmd[8] = { 0x05, 1, 0x80, 0, 0, 0, 0, 0 };
  lcd_cmd[2] |= (addr & 0x7f);
  return iow_lcd_wcmd(udh,lcd_cmd);
}

int iowlcd_write_data(usb_dev_handle *udh,int len,unsigned char *data)
{
  unsigned char lcd_cmd[8] = { 0x05, 0x80, 0, 0, 0, 0, 0, 0 };
  unsigned char *ptr = data;
  int num_blk, last_blk, i;
  num_blk  = len / 6;
  last_blk = len % 6;

  /* write 6 data byte reports */
  for (i = 0; i < num_blk; i++) {
    lcd_cmd[1] = 0x86;
    memcpy(&lcd_cmd[2], ptr, 6);
    if (iow_lcd_wcmd(udh, lcd_cmd) == IOW_ERROR)
      return ptr - data;
    ptr += 6;
  }

  /* last block */
  if (last_blk > 0) {
    lcd_cmd[1] = 0x80 | last_blk;
    memcpy(&lcd_cmd[2], ptr, last_blk);
    if (iow_lcd_wcmd(udh, lcd_cmd) == IOW_ERROR)
      return ptr - data;
  }

  return len;
}

int iowlcd_set_pos(usb_dev_handle *udh,int x,int y)
{
  const unsigned char lineOff[4] = { 0x00, 0x40, 0x14, 0x54 };
  unsigned char addr = lineOff[y] + x;
  return iowlcd_set_ddram_addr(udh, addr);
}

int iowlcd_set_text(usb_dev_handle *udh,int x,int y,int len,unsigned char *data)
{
  if (iowlcd_set_pos(udh,x,y) == IOW_ERROR)
    return IOW_ERROR;
  return iowlcd_write_data(udh,len,data);
}

int iowlcd_load_chars(usb_dev_handle *udh,int offset,int num,unsigned char *bits)
{
  if (iowlcd_set_cgram_addr(udh, offset<<3) == IOW_ERROR)
    return IOW_ERROR;
  return iowlcd_write_data(udh, num*CELLHEIGHT, bits);
}


/* ------------------- IOWarrior LED routines ------------------------------ */

int iowled_on_off(usb_dev_handle *udh,int type, unsigned int pattern)
{
  unsigned char led_cmd[4] = { 0x00, 0x00, 0x00, 0x00 };
  int i;
  pattern ^= 0xFFFFFFFFU;	/* invert pattern */

  /* map pattern to bytes */
  for (i = 0; i < (type == iowProd40) ? 4 : 2; i++) {
    led_cmd[i] = (unsigned char) (0xFF & pattern);
    pattern >>= 8;
  }  
  
  return iow_led_wcmd(udh, (type == iowProd40) ? 4 : 2, led_cmd);
}



/*****************************************************
 * Here start the API function
 */

/*****************************************************
 * API: Open USB device and initialize it ...
 */
MODULE_EXPORT int
IOWarrior_init(Driver *drvthis)
{
char serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;
char size[LCD_MAX_WIDTH+1] = DEFAULT_SIZE;

struct usb_bus *busses;
struct usb_bus *bus;
  
int w;
int h;

PrivateData *p;

  /* Allocate and store private data */
  p = (PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
      return -1;
  if (drvthis->store_private_ptr(drvthis, p))
      return -1;

  /* Initialize the PrivateData structure */

  p->cellwidth = CELLWIDTH;
  p->cellheight = CELLHEIGHT;

  p->backlight = DEFAULT_BACKLIGHT;

  debug(RPT_INFO, "%s: init(%p)", drvthis->name, drvthis);

  /* Read config file */

  /* What IO-Warrior device should be used */
  strncpy(serial, drvthis->config_get_string(drvthis->name, "SerialNumber",
                                             0, DEFAULT_SERIALNO), sizeof(serial));
  serial[sizeof(serial)-1] = '\0';
  if (*serial != '\0') {
    report(RPT_INFO, "%s: using serial number: %s", drvthis->name, serial);
  }

  /* Which size */
  strncpy(size, drvthis->config_get_string(drvthis->name, "Size",
                                           0, DEFAULT_SIZE), sizeof(size));
  size[sizeof(size) - 1] = 0;
  if ((sscanf(size, "%dx%d", &w, &h) != 2) ||
      (w <= 0) || (w > LCD_MAX_WIDTH) ||
      (h <= 0) || (h > LCD_MAX_HEIGHT)) {
    report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
		    drvthis->name, size, DEFAULT_SIZE);
    sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
  }
  p->width = w;
  p->height = h;

  /* special option lastline (some displays need it against the underline effect) */
  p->lastline = drvthis->config_get_bool(drvthis->name, "lastline", 0, 1);
  
  /* Contrast of the LCD can be changed by adjusting a trimpot */

  /* End of config file parsing */

  /* Allocate framebuffer memory */
  p->framebuf = (unsigned char *) calloc(p->width * p->height, 1);
  if (p->framebuf == NULL) {
    report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
    return -1;
  }

  /* Allocate and clear the buffer for incremental updates */
  p->backingstore = (unsigned char *) calloc(p->width * p->height, 1);
  if (p->backingstore == NULL) {
    report(RPT_ERR, "%s: unable to create backingstore", drvthis->name);
    return -1;
  }

  /* set mode for custom chracter cache */
  p->ccmode = standard;

  /* initialize output stuff */
  p->output_mask = 0;	/* not yet supported */
  p->output_state = -1;

  /* find USB device */
  usb_init();
  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();

  /* on all busses look for IO-Warriors */
  p->udh = NULL;
  for (bus = busses; bus != NULL; bus = bus->next) {
    struct usb_device *dev;

    for (dev = bus->devices; dev; dev = dev->next) {
      /* Check if this device is a Code Mercenaries IO-Warrior */
      if ((dev->descriptor.idVendor == iowVendor) &&
         ((dev->descriptor.idProduct == iowProd24) ||
          (dev->descriptor.idProduct == iowProd40))) {

        /* IO-Warrior found; try to find it's description and serial number */
        p->udh = usb_open(dev);
        if (p->udh == NULL) {
          report(RPT_WARNING, "%s: unable to open device", drvthis->name);
          // return -1;		/* it's better to continue */
        }
        else {
          /* get device information & check for serial number */
          p->productID = dev->descriptor.idProduct;
          if (usb_get_string_simple(p->udh, dev->descriptor.iManufacturer,
            		            p->manufacturer, LCD_MAX_WIDTH) <= 0)
            *p->manufacturer = '\0';
          p->manufacturer[p->width] = '\0';
          if (usb_get_string_simple(p->udh, dev->descriptor.iProduct,
            		            p->product, LCD_MAX_WIDTH) <= 0)
            *p->product = '\0';
          p->product[p->width] = '\0';
          if (usb_get_string_simple(p->udh, dev->descriptor.iSerialNumber,
                                    p->serial, LCD_MAX_WIDTH) <= 0)
            *p->serial = '\0';
          p->serial[sizeof(p->serial)-1] = '\0';
          if ((*serial != '\0') && (*p->serial == '\0')) {
            report(RPT_ERR, "%s: unable to get device's serial number", drvthis->name);
            usb_close(p->udh);
            return -1;
          }

          /* succeed if no serial was given in the config or the 2 numbers match */
          if ((!*serial) || (strcmp(serial, p->serial) == 0))
            goto done;

          usb_close(p->udh);
          p->udh = NULL;
        }
      }
    }
  }
  done:

  if (p->udh != NULL) {
    debug(RPT_DEBUG, "%s: opening device succeeded", drvthis->name);

    if (usb_set_configuration(p->udh, 1) < 0) {
      usb_close(p->udh);
      report(RPT_ERR, "%s: unable to set configuration", drvthis->name);
      return -1;
    }

    if (usb_claim_interface(p->udh, 1) < 0) {
#if defined(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP)
      if ((usb_detach_kernel_driver_np(p->udh, 1) < 0) ||
          (usb_claim_interface(p->udh, 1) < 0)) {
        usb_close(p->udh);
        report(RPT_ERR, "%s: unable to re-claim interface", drvthis->name);
        return -1;
      }
#else
      usb_close(p->udh);
      report(RPT_ERR, "%s: unable to claim interface", drvthis->name);
      return -1;
#endif
    }
  }
  else {
    report(RPT_ERR, "%s: no (matching) IO-Warrior device found", drvthis->name);
    return -1;
  }

  /* enable LCD in IOW */
  if (iowlcd_enable(p->udh) == IOW_ERROR)
    return -1;
  /* enable 8bit transfer mode */
  if (iowlcd_set_function(p->udh, 1, 1, 0) == IOW_ERROR)
    return -1;
  /* enable display, disable cursor+blinking */
  if (iowlcd_display_on_off(p->udh, 1, 0, 0) == IOW_ERROR)
    return -1;

  report(RPT_DEBUG, "%s: init() done", drvthis->name);

  /* clear screen */
  IOWarrior_clear(drvthis);

  /* display information about the driver */
  {
    int y = 1;

    if (p->height > 2)
      IOWarrior_string(drvthis, 1, y++, p->manufacturer);
    IOWarrior_string(drvthis, 1, y++, p->product);
    if (p->height > 1) {
      IOWarrior_string(drvthis, 1, y, "# ");
      IOWarrior_string(drvthis, 3, y, p->serial);
    }

    IOWarrior_flush(drvthis);
    sleep(2);
  }

  return 0;
}


/******************************************************
 * API: Clean-up
 */
MODULE_EXPORT void
IOWarrior_close(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  if (p != NULL) {
    /* don't turn display off: keep the logoff message */
    // iowlcd_display_on_off(p->udh,0,0,0);

    /* IOW leave LCD mode */
    iowlcd_disable(p->udh);

    /* release interface 1 */
    usb_release_interface(p->udh, 1);

    /* close USB */
    usb_close(p->udh);

    if (p->framebuf != NULL)
      free(p->framebuf);
    p->framebuf = NULL;

    if (p->backingstore != NULL)
      free(p->backingstore);
    p->backingstore = NULL;

    free(p);
  }  
  drvthis->store_private_ptr(drvthis, NULL);

  debug(RPT_DEBUG, "%s: closed", drvthis->name);
}


/******************************************************
 * API: Returns the display's width
 */
MODULE_EXPORT int 
IOWarrior_width(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "%s: returning width", drvthis->name);

  return p->width;
}


/******************************************************
 * API: Returns the display's height
 */
MODULE_EXPORT int 
IOWarrior_height(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "%s: returning height", drvthis->name);

  return p->height;
}


/******************************************************
 * API: Returns the display's character cell width
 */
MODULE_EXPORT int 
IOWarrior_cellwidth(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "%s: returning cellwidth", drvthis->name);

  return p->cellwidth;
}


/******************************************************
 * API: Returns the display's character cell height
 */
MODULE_EXPORT int 
IOWarrior_cellheight(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "%s: returning cellheight", drvthis->name);

  return p->cellheight;
}


/******************************************************
 * API: Send what we have to the hardware
 */
MODULE_EXPORT void
IOWarrior_flush(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;
   
int x, y;
int i;
int count;

  /* Update LCD incrementally by comparing with last contents */
  for (y = 0; y < p->height; y++) {
    int offset = y * p->width;

    for (x = 0; x < p->width; x++) {
      if (p->backingstore[offset+x] != p->framebuf[offset+x]) {
        /* always flush a full line */
        unsigned char buffer[LCD_MAX_WIDTH];

        for (count = 0; count < p->width; count++) {
          buffer[count] = HD44780_charmap[(unsigned char) p->framebuf[offset+count]];
          p->backingstore[offset+count] = p->framebuf[offset+count];
        }
        iowlcd_set_text(p->udh, 0, y, count, buffer);
        debug(RPT_DEBUG, "%s: flushed %d chars at (%d,%d)",
			drvthis->name, count, 0, y);

//		/* Alternative: update the LCD in chunks of max 6 bytes
//		 * since IOWarrior needs only one command for it
//		 */
//		char buffer[7];
//
//		count = (y+1) * p->width - offset - x;
//		count = (count > 6) ? 6 : count;
//		for (i = 0; i < count; i++) {
//		    buffer[i] = HD44780_charmap[(unsigned char) p->framebuf[offset+x+i]];
//		    p->backingstore[offset+x+i] = p->framebuf[offset+x+i];
//		}
//		iowlcd_set_text(p->udh, x, y, count, buffer);
//		debug(RPT_DEBUG, "%s: flushed %d chars at (%d,%d)",
//			drvthis->name, count, x, y);

        x += count-1;
      }
    }
  }

  /* Check which defineable chars we need to update */
  count = 0;
  for (i = 0; i < NUM_CCs; i++) {
    if (!p->cc[i].clean) {
      iowlcd_load_chars(p->udh, i, 1, p->cc[i].cache);
      p->cc[i].clean = 1;	/* mark as clean */
      count++;
    }
  }
  debug(RPT_DEBUG, "%s: flushed %d custom chars", drvthis->name, count);
}


/*********************************************************
 * API: Clears the LCD screen
 */
MODULE_EXPORT void
IOWarrior_clear(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);

  p->ccmode = standard;
  
  debug(RPT_DEBUG, "%s: cleared framebuffer", drvthis->name);
}


/*******************************************************************
 * API: Prints a character on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right is (p->width,p->height).
 */
MODULE_EXPORT void
IOWarrior_chr(Driver *drvthis, int x, int y, char c)
{
PrivateData *p = drvthis->private_data;

  y--;
  x--;

  if ((x >= 0) && (y >= 0) && (x < p->width) && (y < p->height))
    p->framebuf[(y * p->width) + x] = c;

  debug(RPT_DEBUG, "%s: writing char 0x%02X at (%d,%d)",
		  drvthis->name, (unsigned) c, x, y);
}

/*****************************************************************
 * API: Prints a string on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right is (p->width,p->height).
 */
MODULE_EXPORT void
IOWarrior_string(Driver *drvthis, int x, int y, char *string)
{
PrivateData *p = drvthis->private_data;
int i;

  x--;
  y--;

  if ((y < 0) || (y >= p->height))
	  return;

  for (i = 0; (string[i] != '\0') && (x < p->width); i++, x++) {
    if (x >= 0)     // no write left of left border
      p->framebuf[(y * p->width) + x] = string[i];
  }

  debug(RPT_DEBUG, "%s: writing string \"%s\" at (%d,%d)",
		  drvthis->name, string, x, y);
}


/*
 * The IOWarriors' hardware does not support contrast or brightness settings
 * by software, but only by changing the hardware configuration.
 * Since the get_contrast and set_contrast are not mandatory in the API,
 * it is better not to implement a dummy version.
 */


/*********************************************************
 * API: Sets the backlight
 * the API only permit setting to off=0 and on<>0
 */
MODULE_EXPORT void
IOWarrior_backlight(Driver *drvthis, int on)
{
PrivateData *p = drvthis->private_data;

  p->backlight = on;
}


/*************************************************************
 * API: Draws a vertical bar...
 */
MODULE_EXPORT void
IOWarrior_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
PrivateData *p = drvthis->private_data;

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'up' direction
   *(other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles(0..1000) that the bar should be filled.
   */

  if (p->ccmode != vbar) {
    unsigned char vBar[p->cellheight];
    int i;

    if (p->ccmode != standard) {
      /* Not supported(yet) */
      report(RPT_WARNING, "%s: vbar: cannot combine two modes using user defined characters",
		      drvthis->name);
      return;
    }
    p->ccmode = vbar;

    memset(vBar, 0x00, sizeof(vBar));

    for (i = 1; i < p->cellheight; i++) {
      // add pixel line per pixel line ...
      vBar[p->cellheight - i] = 0xFF;
      IOWarrior_set_char(drvthis, i, vBar);
    }
  }

  lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/*****************************************************************
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
IOWarrior_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
PrivateData *p = drvthis->private_data;

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'cwrightup' direction
   *(other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles(0..1000) that the bar should be filled.
   */

  if (p->ccmode != hbar) {
    unsigned char hBar[p->cellheight];
    int i;

    if (p->ccmode != standard) {
      /* Not supported(yet) */
      report(RPT_WARNING, "%s: hbar: cannot combine two modes using user defined characters",
		      drvthis->name);
      return;
    }

    p->ccmode = hbar;

    for (i = 1; i <= p->cellwidth; i++) {
      // fill pixel columns from left to right.
      memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
      IOWarrior_set_char(drvthis, i, hBar);
    }
  }

  lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*******************************************************************
 * API: Sets up for big numbers
 */
static void
IOWarrior_init_num(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

unsigned char bignum_ccs[8][CELLHEIGHT] = {
	[0]
	{ b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b_______,
	  b_______,
	  b_______,
	  b_______ },
	[1]
	{ b_____XX,
	  b_____XX,
	  b_____XX,
	  b_____XX,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___ },
	[2]
	{ b__XX_XX,
	  b__XX_XX,
	  b__XX_XX,
	  b__XX_XX,
	  b_______,
	  b_______,
	  b_______,
	  b_______ },
	[3]
	{ b_______,
	  b_______,
	  b_______,
	  b_______,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___ },
	[4]
	{ b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b_____XX,
	  b_____XX,
	  b_____XX,
	  b_____XX },
	[5]
	{ b_______,
	  b_______,
	  b_______,
	  b_______,
	  b__XX_XX,
	  b__XX_XX,
	  b__XX_XX,
	  b__XX_XX },
	[6]
	{ b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___,
	  b__XX___ },
	[7]
	{ b_____XX,
	  b_____XX,
	  b_____XX,
	  b_____XX,
	  b_______,
	  b_______,
	  b_______,
	  b_______ }
};

  if (p->ccmode != bignum) {
    int i;

    if (p->ccmode != standard) {
      /* Not supported (yet) */
      report(RPT_WARNING, "%s: init_num: cannot combine two modes using user defined characters",
		      drvthis->name);
      return;
    }

    p->ccmode = bignum;

    for (i = 0; i < NUM_CCs; i++) {
      IOWarrior_set_char(drvthis, i, bignum_ccs[i]);
    }
  }
}


/*******************************************************************
 * API: Writes a big number.
 */
MODULE_EXPORT void
IOWarrior_num(Driver *drvthis, int x, int num)
{
PrivateData *p = drvthis->private_data;

/* each bignum is constructed in a 3 x 4 matrix and consists
 * of only the 8 characters defined above as well as ' '
 *
 * The following table defines the 11 big numbers '0'-'9', ':'
 * and the custom base characters they consist of
 */
char bignum_map[11][4][3] = {
  { /* 0: */
    {  1,  2,  3 },
    {  6, 32,  6 },
    {  6, 32,  6 },
    {  7,  2, 32 } },
  { /* 1: */
    {  7,  6, 32 },
    { 32,  6, 32 },
    { 32,  6, 32 },
    {  7,  2, 32 } },
  { /* 2: */
    {  1,  2,  3 },
    { 32,  5,  0 },
    {  1, 32, 32 },
    {  2,  2,  0 } },
  { /* 3: */
    {  1,  2,  3 },
    { 32,  5,  0 },
    {  3, 32,  6 },
    {  7,  2, 32 } },
  { /* 4: */
    { 32,  3,  6 },
    {  1, 32,  6 },
    {  2,  2,  6 },
    { 32, 32,  0 } },
  { /* 5: */
    {  1,  2,  0 },
    {  2,  2,  3 },
    {  3, 32,  6 },
    {  7,  2, 32 } },
  { /* 6: */
    {  1,  2, 32 },
    {  6,  5, 32 },
    {  6, 32,  6 },
    {  7,  2, 32 } },
  { /* 7: */
    {  2,  2,  6 },
    { 32,  1, 32 },
    { 32,  6, 32 },
    { 32,  0, 32 } },
  { /* 8: */
    {  1,  2,  3 },
    {  4,  5,  0 },
    {  6, 32,  6 },
    {  7,  2, 32 } },
  { /* 9: */
    {  1,  2,  3 },
    {  4,  3,  6 },
    { 32,  1, 32 },
    {  7, 32, 32 } },
  { /* colon: (only 1st column used) */
    { 32, 32, 32 },
    {  0, 32, 32 },
    {  0, 32, 32 },
    { 32, 32, 32 } }
};

  if ((num < 0) || (num > 10))
    return;

  IOWarrior_init_num(drvthis);

  if (p->height >= 4) {
    int y = (p->height - 2) / 2;	/* center vertically */
    int x2, y2;

    for (x2 = 0; x2 < 3; x2++) {
      for (y2 = 0; y2 < 4; y2++) {
        IOWarrior_chr(drvthis, x+x2, y+y2, bignum_map[num][y2][x2]);
      }
      if (num == 10)
        x2 = 2; /* = break, for colon only */
    }
  }
  else
    IOWarrior_chr(drvthis, x, 1 + (p->height - 1)/ 2,
		  (num == 10) ? ':' : (num + '0'));
}


/*********************************************************************
 * API: set cursor position and state
 */
/* not yet tested (needs input)
 * maybe better: set only variables here and update when flushing
MODULE_EXPORT void
IOWarrior_cursor (Driver *drvthis, int x, int y, int state)
{
PrivateData *p = drvthis->private_data;

  iowlcd_set_pos(p->udh, x, y);

  switch (state) {
    case CURSOR_OFF:
      iowlcd_display_on_off(p->udh, 1, 0, 0);
      break;
    case CURSOR_UNDER:
      iowlcd_display_on_off(p->udh, 1, 1, 0);
      break;
    case CURSOR_BLOCK:
    case CURSOR_DEFAULT_ON:
    default:
      iowlcd_display_on_off(p->udh, 1, 1, 1);
      break;
  }    
}
*/

/*********************************************************************
 * API: get number of custom chars (always NUM_CCs)
 */
MODULE_EXPORT int
IOWarrior_get_free_chars (Driver *drvthis)
{
//PrivateData *p = drvthis->private_data;

  return NUM_CCs;
}


/*********************************************************************
 * API: Sets a custom character from 0 - (NUM_CCs-1)
 *
 * The API only permit setting to off=0 and on<>0
 * For input is just an array of characters:
 *  values > 0 mean "on" and values <= 0 are "off".
 */
MODULE_EXPORT void
IOWarrior_set_char(Driver *drvthis, int n, unsigned char *dat)
{
PrivateData *p = drvthis->private_data;
unsigned char mask = (1 << p->cellwidth) - 1;
int row;

  if ((n < 0) || (n >= NUM_CCs))
    return;
  if (dat == NULL)
    return;

  for (row = 0; row < p->cellheight; row++) {
    int letter = 0;

    if (p->lastline || (row < p->cellheight - 1))
      letter = dat[row] & mask;	

    if (p->cc[n].cache[row] != letter)
      p->cc[n].clean = 0;	 /* only mark dirty if really different */
    p->cc[n].cache[row] = letter;
  }
}


/*********************************************************************
 * API: Places an icon on screen
 */
MODULE_EXPORT int 
IOWarrior_icon(Driver *drvthis, int x, int y, int icon)
{
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
      IOWarrior_set_char(drvthis, 6, block_filled);
      IOWarrior_chr(drvthis, x, y, 6);
      break;
    case ICON_HEART_FILLED:
      IOWarrior_set_char(drvthis, 0, heart_filled);
      IOWarrior_chr(drvthis, x, y, 0);
      break;
    case ICON_HEART_OPEN:
      IOWarrior_set_char(drvthis, 0, heart_open);
      IOWarrior_chr(drvthis, x, y, 0);
      break;
    case ICON_ARROW_UP:
      IOWarrior_set_char(drvthis, 1, arrow_up);
      IOWarrior_chr(drvthis, x, y, 1);
      break;
    case ICON_ARROW_DOWN:
      IOWarrior_set_char(drvthis, 2, arrow_down);
      IOWarrior_chr(drvthis, x, y, 2);
      break;
    case ICON_ARROW_LEFT:
      IOWarrior_chr(drvthis, x, y, 0x7F);
      break;
    case ICON_ARROW_RIGHT:
      IOWarrior_chr(drvthis, x, y, 0x7E);
      break;
    case ICON_CHECKBOX_OFF:
      IOWarrior_set_char(drvthis, 3, checkbox_off);
      IOWarrior_chr(drvthis, x, y, 3);
      break;
    case ICON_CHECKBOX_ON:
      IOWarrior_set_char(drvthis, 4, checkbox_on);
      IOWarrior_chr(drvthis, x, y, 4);
      break;
    case ICON_CHECKBOX_GRAY:
      IOWarrior_set_char(drvthis, 5, checkbox_gray);
      IOWarrior_chr(drvthis, x, y, 5);
      break;
    default:
      return -1; /* Let the core do other icons */
  }
  return 0;
}


/*********************************************************************
 * API: Places an icon on screen
 */
MODULE_EXPORT void
IOWarrior_output(Driver *drvthis, int on)
{
  PrivateData *p = drvthis->private_data;

  /* output disabled */
  if (!p->output_mask)
     return;

  p->output_state = on;
  
  iowled_on_off(p->udh, p->productID, (on) ? p->output_mask : 0);
}


/*********************************************************************
 * API: provides some info about this driver
 */
MODULE_EXPORT const char *
IOWarrior_get_info (Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  
  snprintf(p->info, sizeof(p->info)-1, "IOWarrior Driver: %s %s (0x%0x) S/N: %s",
           p->manufacturer, p->product, p->productID, p->serial);
  return p->info;
}

/* EOF */
