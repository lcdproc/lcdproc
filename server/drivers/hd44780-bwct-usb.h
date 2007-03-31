#ifndef HD_BWCT_USB_H
#define HD_BWCT_USB_H

#include "lcd.h"		/* for Driver */
#include "hd44780-low.h"

/* vendor and product id */
#define BWCT_USB_VENDORID	0x03DA
#define BWCT_USB_PRODUCTID	0x0002

#define BWCT_LCD_RESET		1
#define BWCT_LCD_CMD		2
#define BWCT_LCD_DATA		3
#define BWCT_LCD_SET_CONTRAST	4

#define DEFAULT_SERIALNO	""
#define DEFAULT_CONTRAST	300

// initialise this particular driver
int hd_init_bwct_usb(Driver *drvthis);

void bwct_usb_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void bwct_usb_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char bwct_usb_HD44780_scankeypad(PrivateData *p);
void bwct_usb_HD44780_close(PrivateData *p);

#endif
