#ifndef HD_LCDSTAT_H
#define HD_LCDSTAT_H

#include "lcd.h"					  /* for lcd_logical_driver */
#include "hd44780-low.h"		  /* for HD44780_functions */

// initialise this particular driver
int hd_init_4bit (HD44780_functions * hd44780_functions, lcd_logical_driver * driver, char *args, unsigned int port);

#endif
