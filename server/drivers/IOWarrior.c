/*  This is the LCDproc driver for IO-Warrior devices(http://www.codemercs.de)

      Copyright(C) 2004, Peter Marschall <peter@adpm.de>

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

#define ValidX(x) if ((x) > p->width) { (x) = p->width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > p->height) { (y) = p->height; } else (y) = (y) < 1 ? 1 :(y);



/* ===================== IOWarrior low level routines ====================== */

/* ------------------- IOWarrior LCD routines ------------------------------ */

/* write a set report to interface 1 of warrior */
static int iow_lcd_wcmd(usb_dev_handle *udh, unsigned char data[8])
{
  return(usb_control_msg(udh, USB_DT_HID, USB_REQ_SET_REPORT, 0, 1,
                    data, 8, iowTimeout) == 8) ? IOW_OK : IOW_ERROR;
}


/* ------------------- IOWarrior LED routines ------------------------------ */

/* write a set report to interface 0 of warrior */
static int iow_led_wcmd(usb_dev_handle *udh,int len,unsigned char *data)
{
  return (usb_control_msg(udh, USB_DT_HID, USB_REQ_SET_REPORT, 2, 0,
                          data, len, iowTimeout) == len) ? IOW_OK : IOW_ERROR;
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
int IOWarrior_init(Driver *drvthis, char *args)
{
char serial[LCD_MAX_WIDTH+1] = DEFAULT_SERIALNO;
char size[LCD_MAX_WIDTH+1] = DEFAULT_SIZE;

struct usb_bus *busses;
struct usb_bus *bus;
  
int w;
int h;

PrivateData *p;

  /* Alocate and store private data */
  p =(PrivateData *) calloc(1, sizeof(PrivateData));
  if (p == NULL)
      return -1;
  if (drvthis->store_private_ptr(drvthis, p))
      return -1;

  /* Initialise the PrivateData structure */

  p->cellwidth = LCD_DEFAULT_CELLWIDTH;
  p->cellheight = LCD_DEFAULT_CELLHEIGHT;

  p->backlight = DEFAULT_BACKLIGHT;

  debug(RPT_INFO, "IOWarrior: init(%p,%s)", drvthis, args);

  /* Read config file */

  /* What IO-Warrior device should be used */
  strncpy(serial, drvthis->config_get_string(drvthis->name, "SerialNumber",
                                             0, DEFAULT_SERIALNO), sizeof(serial));
  serial[sizeof(serial)-1] = '\0';
  if (*serial != '\0') {
    report(RPT_INFO, "IOWarrior: Using serial number: %s", serial);
  }

  /* Which size */
  strncpy(size, drvthis->config_get_string(drvthis->name, "Size",
                                           0, DEFAULT_SIZE), sizeof(size));
  size[sizeof(size) - 1] = 0;
  if ((sscanf(size, "%dx%d", &w, &h) != 2) ||
      (w <= 0) || (w > LCD_MAX_WIDTH) ||
      (h <= 0) || (h > LCD_MAX_HEIGHT)) {
    report(RPT_WARNING, "IOWarrior: Cannot read size: %s. Using default value.\n", size);
    sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
  }
  p->width = w;
  p->height = h;

  /* Contrast of the LCD can be changed by adjusting a trimpot */

  /* End of config file parsing */

  /* Allocate framebuffer memory */
  p->framebuf =(unsigned char *) calloc(p->width * p->height, 1);
  if (p->framebuf == NULL) {
    report(RPT_ERR, "IOWarrior: unable to create framebuffer.\n");
    return -1;
  }

  /* Allocate and clear the buffer for incremental updates */
  p->backingstore =(unsigned char *) calloc(p->width * p->height, 1);
  if (p->backingstore == NULL) {
    report(RPT_ERR, "IOWarrior: unable to create lcd_buffer.\n");
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
          report(RPT_WARNING, "IOWarrior: unable to open device");
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
            report(RPT_ERR, "IOWarrior: unable to get device's serial number");
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
    debug(RPT_DEBUG, "IOWarrior: opening device succeeded\n");

    if (usb_set_configuration(p->udh, 1) < 0) {
      usb_close(p->udh);
      report(RPT_ERR, "IOWarrior: unable to set configuration");
      return -1;
    }

    if (usb_claim_interface(p->udh, 1) < 0) {
#if defined(LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP)
      if ((usb_detach_kernel_driver_np(p->udh, 1) < 0) ||
          (usb_claim_interface(p->udh, 1) < 0)) {
        usb_close(p->udh);
        report(RPT_ERR, "IOWarrior: unable to re-claim interface");
        return -1;
      }
#else
      usb_close(p->udh);
      report(RPT_ERR, "IOWarrior: unable to claim interface");
      return -1;
#endif
    }
  }
  else {
    report(RPT_ERR, "IOWarrior: no (matching) IO-Warrior device found");
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

  report(RPT_DEBUG, "IOWarrior_init: done\n");

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

  if (p != NULL)
    free(p);
  drvthis->store_private_ptr(drvthis, NULL);

  debug(RPT_DEBUG, "IOWarrior: closed");
}

/******************************************************
 * API: Returns the display's width
 */
MODULE_EXPORT int 
IOWarrior_width(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "IOWarrior: returning width");

  return p->width;
}

/******************************************************
 * API: Returns the display's height
 */
MODULE_EXPORT int 
IOWarrior_height(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

  debug(RPT_DEBUG, "IOWarrior: returning height");

  return p->height;
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
        char buffer[LCD_MAX_WIDTH];

        for (count = 0; count < p->width; count++) {
          buffer[count] = HD44780_charmap[(unsigned char) p->framebuf[offset+count]];
          p->backingstore[offset+count] = p->framebuf[offset+count];
        }
        iowlcd_set_text(p->udh, 0, y, count, buffer);
        debug(RPT_DEBUG, "IOWarrior: flushed %d chars at (%d,%d)", count, 0, y);

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
//		debug(RPT_DEBUG, "IOWarrior: flushed %d chars at (%d,%d)", count, x, y);

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
  debug(RPT_DEBUG, "IOWarrior: flushed %d custom chars", count);
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
  
  debug(RPT_DEBUG, "IOWarrior: cleared framebuffer");
}


/*******************************************************************
 * API: Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
IOWarrior_chr(Driver *drvthis, int x, int y, char c)
{
PrivateData *p = drvthis->private_data;

  ValidX(x);
  ValidY(y);

  y--;
  x--;

  p->framebuf[(y * p->width) + x] = c;

  debug(RPT_DEBUG, "IOWarrior: writing char 0x%02hhX at (%d,%d)", (unsigned) c, x, y);
}

/*****************************************************************
 * API: Prints a string on the lcd display, at position (x,y).
 * The upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
IOWarrior_string(Driver *drvthis, int x, int y, char *string)
{
PrivateData *p = drvthis->private_data;

int offset, siz;

  ValidX(x);
  ValidY(y);

  x--;
  y--;

  offset =(y * p->width) + x;

  siz = (p->width * p->height) - offset;
  siz = siz > strlen(string) ? strlen(string) : siz;

  memcpy(p->framebuf + offset, string, siz);

  debug(RPT_DEBUG, "IOWarrior: writing string \"%s\" at (%d,%d)", string, x, y);
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


/*********************************************************
 * NOTAPI: Inits vertical bars...
 * This was part of API in 0.4 and removed in 0.5
 */
void IOWarrior_init_vbar(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

char a[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1 };
char b[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };
char c[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };
char d[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };
char e[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };
char f[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };
char g[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1 };

  if (p->ccmode != vbar) {
    if (p->ccmode != standard) {
      /* Not supported(yet) */
      report(RPT_WARNING, "IOWarrior_init_vbar: Cannot combine two modes using user defined characters");
      return;
    }
    p->ccmode = vbar;

    IOWarrior_set_char(drvthis, 1, a);
    IOWarrior_set_char(drvthis, 2, b);
    IOWarrior_set_char(drvthis, 3, c);
    IOWarrior_set_char(drvthis, 4, d);
    IOWarrior_set_char(drvthis, 5, e);
    IOWarrior_set_char(drvthis, 6, f);
    IOWarrior_set_char(drvthis, 7, g);
  }
}

/*********************************************************
 * NOTAPI: Inits horizontal bars...
 * This was part of API in 0.4 and removed in 0.5
 */
void IOWarrior_init_hbar(Driver *drvthis)
{
PrivateData *p = drvthis->private_data;

char a[CELLWIDTH*CELLHEIGHT] = {
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0,
  1, 0, 0, 0, 0 };
char b[CELLWIDTH*CELLHEIGHT] = {
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0,
  1, 1, 0, 0, 0 };
char c[CELLWIDTH*CELLHEIGHT] = {
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0,
  1, 1, 1, 0, 0 };
char d[CELLWIDTH*CELLHEIGHT] = {
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0,
  1, 1, 1, 1, 0 };

  if (p->ccmode != hbar) {
    if (p->ccmode != standard) {
      /* Not supported(yet) */
      report(RPT_WARNING, "IOWarrior_init_hbar: Cannot combine two modes using user defined characters");
      return;
    }

    p->ccmode = hbar;

    IOWarrior_set_char(drvthis, 1, a);
    IOWarrior_set_char(drvthis, 2, b);
    IOWarrior_set_char(drvthis, 3, c);
    IOWarrior_set_char(drvthis, 4, d);
  }
}


/*************************************************************
 * API: Draws a vertical bar...
 */
MODULE_EXPORT void
IOWarrior_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
PrivateData *p = drvthis->private_data;

  IOWarrior_init_vbar(drvthis);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'up' direction
   *(other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles(0..1000) that the bar should be filled.
   */

  lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 0);
}


/*****************************************************************
 * API: Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
IOWarrior_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
PrivateData *p = drvthis->private_data;

  IOWarrior_init_hbar(drvthis);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'cwrightup' direction
   *(other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles(0..1000) that the bar should be filled.
   */

  lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 0);
}


/*******************************************************************
 * API: Sets up for big numbers
 */
MODULE_EXPORT void
IOWarrior_init_num(Driver *drvthis)
{
PrivateData *p =(PrivateData *) drvthis->private_data;

char bignum_ccs[8][CELLWIDTH*CELLHEIGHT] = {
  { 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 },

  { 0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0 },

  { 1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 },

  { 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0 },

  { 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1 },

  { 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1,
    1, 1, 0, 1, 1 },

  { 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 0, 0, 0 },

  { 0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 }
};

  if (p->ccmode != bignum) {
    int i;

    if (p->ccmode != standard) {
      /* Not supported (yet) */
      report(RPT_WARNING, "IOWarrior_init_num: Cannot combine two modes using user defined characters");
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
PrivateData *p =(PrivateData *) drvthis->private_data;

/* each bignum is constructed in a 3 x 4 matrix and consists
 * of only the 8 characters defiend above as well as ' '
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
  { /* colon: */
    { 32, 32, 32 },
    {  7, 32, 32 },
    {  7, 32, 32 },
    { 32, 32, 32 } }
};

  if (num < 0 || num > 11)
    return;

  IOWarrior_init_num(drvthis);

  if (p->width >= 20 && p->height >= 4) {
    int y = (p->height - 2) / 2;	/* center vertically */
    int x2, y2;

    for (x2 = 0; x2 <= 2; x2++) {
      for (y2 = 0; y2 <= 3; y2++) {
        IOWarrior_chr(drvthis, x+x2, y+y2, bignum_map[num][y2][x2]);
      }
      if (num == 10)
        x2 = 2; /* = break, for colon only */
    }
  }
  else {
    IOWarrior_chr(drvthis, x, p->height / 2, (num >= 11) ? ':' : (num + '0'));
  }
}


/*********************************************************************
 * API: Sets a custom character from 0 - 7
 *
 * The API only permit setting to off=0 and on<>0
 * For input is just an array of characters:
 *  values > 0 mean "on" and values <= 0 are "off".
 */
MODULE_EXPORT void
IOWarrior_set_char(Driver *drvthis, int n, char *dat)
{
PrivateData *p = drvthis->private_data;

int row;

  if ((n < 0) || (n >= NUM_CCs))
    return;
  if (dat == NULL)
    return;

  for (row = 0; row < p->cellheight; row++) {
    int letter = 0;
    int col;

    for (col = 0; col < p->cellwidth; col++) {
      letter <<= 1;
      letter |=(dat[(row * p->cellwidth) + col] > 0);
    }
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
char heart_open[CELLWIDTH*CELLHEIGHT] = {
  1, 1, 1, 1, 1,
  1, 0, 1, 0, 1,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 0, 0, 0, 1,
  1, 1, 0, 1, 1,
  1, 1, 1, 1, 1 };
char heart_filled[CELLWIDTH*CELLHEIGHT] = {
  1, 1, 1, 1, 1,
  1, 0, 1, 0, 1,
  0, 1, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 1, 1, 1, 0,
  1, 0, 1, 0, 1,
  1, 1, 0, 1, 1,
  1, 1, 1, 1, 1 };
char arrow_up[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 0, 1, 0, 1,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 0, 0 };
char arrow_down[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  1, 0, 1, 0, 1,
  0, 1, 1, 1, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 0, 0 };
char checkbox_off[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 0, 0, 0, 1,
  1, 0, 0, 0, 1,
  1, 0, 0, 0, 1,
  1, 1, 1, 1, 1,
  0, 0, 0, 0, 0 };
char checkbox_on[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  1, 1, 1, 0, 1,
  1, 0, 1, 1, 0,
  1, 0, 1, 0, 1,
  1, 0, 0, 0, 1,
  1, 1, 1, 1, 1,
  0, 0, 0, 0, 0 };
char checkbox_gray[CELLWIDTH*CELLHEIGHT] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  1, 0, 1, 0, 1,
  1, 1, 0, 1, 1,
  1, 0, 1, 0, 1,
  1, 1, 1, 1, 1,
  0, 0, 0, 0, 0 };

  /* Yes we know, this is a VERY BAD implementation */
  switch(icon) {
    case ICON_BLOCK_FILLED:
      IOWarrior_chr(drvthis, x, y, 255);
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
 * API: Palces an icon on screen
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


/* EOF */
