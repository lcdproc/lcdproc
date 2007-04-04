#ifndef HD_LCD2USB_H
#define HD_LCD2USB_H

#include "lcd.h"		/* for Driver */
#include "hd44780-low.h"

/* vendor and product id */
#define LCD2USB_VENDORID	0x0403
#define LCD2USB_PRODUCTID	0xc630

/* target is a bit map for CMD/DATA */
#define LCD2USB_CTRL_0		(1<<3)
#define LCD2USB_CTRL_1		(1<<4)
#define LCD2USB_CTRL_BOTH	(LCD2USB_CTRL_0 | LCD2USB_CTRL_1)

#define LCD2USB_ECHO		(0<<5)
#define LCD2USB_CMD		(1<<5)
#define LCD2USB_DATA		(2<<5)
#define LCD2USB_SET		(3<<5)
#define LCD2USB_GET		(4<<5)

/* target is value to set */
#define LCD2USB_SET_CONTRAST	(LCD2USB_SET | (0<<3))
#define LCD2USB_SET_BRIGHTNESS	(LCD2USB_SET | (1<<3))
#define LCD2USB_SET_RESERVED0	(LCD2USB_SET | (2<<3))
#define LCD2USB_SET_RESERVED1	(LCD2USB_SET | (3<<3))

/* target is value to get */
#define LCD2USB_GET_FWVER	(LCD2USB_GET | (0<<3))
#define LCD2USB_GET_KEYS	(LCD2USB_GET | (1<<3))
#define LCD2USB_GET_CTRL	(LCD2USB_GET | (2<<3))
#define LCD2USB_GET_RESERVED1	(LCD2USB_GET | (3<<3))

#define DEFAULT_CONTRAST	300
#define DEFAULT_BRIGHTNESS	600
#define DEFAULT_OFFBRIGHTNESS	300

// initialise this particular driver
int hd_init_lcd2usb(Driver *drvthis);

void lcd2usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcd2usb_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char lcd2usb_HD44780_scankeypad(PrivateData *p);
void lcd2usb_HD44780_close(PrivateData *p);

#endif
