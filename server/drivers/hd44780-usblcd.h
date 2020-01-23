#ifndef HD_USBLCD_H
#define HD_USBLCD_H

#include "lcd.h"			  /* for lcd_logical_driver */
#include "../elektragen.h"

// initialise this particular driver
int hd_init_usblcd(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
