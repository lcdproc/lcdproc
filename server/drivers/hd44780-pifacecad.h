#ifndef HD_PIFACECAD_H
#define HD_PIFACECAD_H

#include "lcd.h"		/* for Driver */
#include "../elektragen.h"

/* initialise this particular driver */
int hd_init_pifacecad(Driver *drvthis, const Hd44780DriverConfig * config);

#endif
