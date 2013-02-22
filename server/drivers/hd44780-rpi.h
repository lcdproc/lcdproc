#ifndef HD_LCDRPI_H
#define HD_LCDRPI_H

#include "lcd.h"		/* for Driver */

/* initialize this particular driver */
int hd_init_rpi(Driver *drvthis);

/**
 * rpi_gpio_map is addressed through the hd44780_private_data struct. Data
 * stored here is used for mapping physical GPIO pins to BCM2835 gpio. */
struct rpi_gpio_map {
	int en;
	int en2;
	int rs;
	int d7;
	int d6;
	int d5;
	int d4;
};

/** Peripheral base address of the BCM2835 */
#define BCM2835_PERI_BASE       0x20000000
/** GPIO register start address */
#define GPIO_BASE               BCM2835_PERI_BASE + 0x200000
/** Length of register space */
#define GPIO_BLOCK_SIZE         180
/** The Pi has 32 GPIO pins */
#define GPIO_PINS               32


/** Setup a GPIO pin as input */
#define INP_GPIO(g) *(gpio_map+((g)/10)) &= ~(7<<(((g)%10)*3))
/** Sets or clears a GPIO pin */
#define SET_GPIO(g,a) *(gpio_map+((a)?7:10))=1<<((g)%32);

#endif				// HD_LCDRPI_H
