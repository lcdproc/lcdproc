#ifndef HD_SPI_H
#define HD_SPI_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

/* initialise this particular driver */
int hd_init_spi(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
