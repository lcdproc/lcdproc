#ifndef HD_FTDI_H
#define HD_FTDI_H

#include "lcd.h"		/* for Driver */
#include "hd44780-low.h"

// initialise this particular driver
int hd_init_ftdi(Driver *drvthis);

void ftdi_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void ftdi_HD44780_backlight(PrivateData *p, unsigned char state);
void ftdi_HD44780_close(PrivateData *p);

#endif
