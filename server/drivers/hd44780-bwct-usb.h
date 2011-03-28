#ifndef HD_BWCT_USB_H
#define HD_BWCT_USB_H

#include "lcd.h"		/* for Driver */

/* vendor and product id */
#define BWCT_USB_VENDORID	0x03DA
#define BWCT_USB_PRODUCTID	0x0002

#define BWCT_LCD_RESET		1
#define BWCT_LCD_CMD		2
#define BWCT_LCD_DATA		3
#define BWCT_LCD_SET_CONTRAST	4

#define DEFAULT_SERIALNO	""

// initialise this particular driver
int hd_init_bwct_usb(Driver *drvthis);

#endif
