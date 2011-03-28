#ifndef HD_USBTINY_H
#define HD_USBTINY_H

#include "lcd.h"		/* for Driver */

/* vendor and product id */
#define USBTINY_VENDORID	0x03EB
#define USBTINY_PRODUCTID	0x0002

/* Protocole IR/LCD */
#define USBTINY_INSTR      20
#define USBTINY_DATA       21


/* initialise this particular driver */
int hd_init_usbtiny(Driver *drvthis);

#endif
