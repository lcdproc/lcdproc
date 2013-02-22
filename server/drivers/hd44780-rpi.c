/** \file server/drivers/hd44780-rpi.c
 * \c rpi connection type of \c hd44780 driver for Hitachi HD44780 based LCD
 * displays connected to the GPIO pins on the Raspberry Pi.
 *
 * The LCD is operated in its 4 bit-mode. R/W (5) on the LCD MUST be hard wired
 * low to prevent 5V logic appearing on the GPIO pins.
 *
 * The default connections are:
 * header(GPIO)	  LCD
 * P1-12 (18)	  D7 (14)
 * P1-16 (23)	  D6 (13)
 * P1-18 (24)	  D5 (12)
 * P1-22 (25)	  D4 (11)
 * P1-24 (8)	  EN (6)
 * GND		  R/W (5)
 * P1-26 (7)	  RS (4)
 * P1-15 (22)	  EN2 (second controller, optional)
 * P1-11 (17)	  BL (backlight optional)
 *
 * Mappings can be set in the config file using the key-words:
 * pin_EN, pin_EN2, pin_RS, pin_D7, pin_D6, pin_D5, pin_D4, pin_BL
 * in the [HD44780] section.
 */

/*-
 * Copyright (c) 2012-2013 Paul Corner <paul_c@users.sourceforge.net>
 *                    2013 warhog <warhog@gmx.de> (Conditional ARM test)
 *                    2013 Serac <Raspberry Pi forum>
 *                               (Backlight & Rev2 board support)
 *
 * The code to access the GPIO on a Raspberry Pi draws on an example program
 * by Dom and Gert van Loo from:
 * http://elinux.org/Rpi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/* Default GPIO pin assignment */
#define RPI_DEF_D7  18
#define RPI_DEF_D6  23
#define RPI_DEF_D5  24
#define RPI_DEF_D4  25
#define RPI_DEF_RS   7
#define RPI_DEF_EN   8
#define RPI_DEF_EN2 22
#define RPI_DEF_BL  17

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "hd44780-rpi.h"
#include "hd44780-low.h"
#include "report.h"

void lcdrpi_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void lcdrpi_HD44780_backlight(PrivateData *p, unsigned char state);
void lcdrpi_HD44780_close(PrivateData *p);

/**
 * Pointer to the memory mapped GPIO registers. Note the pointer type is
 * (unsigned int) not (char). This is important when calculating offset
 * adresses!
 *
 * This pointer is outside PrivataData as it needs to be volatile!
 */
static volatile unsigned int *gpio_map = NULL;

/*
 * Two different board revisions are currently in widespread use. The
 * GPIO mappings differ slightly - Not enough to kill a system yet.
 * Future models *may* have extra pins added or reassigned to other
 * functions. Having a revision dependent mapping gives the user fair
 * warning if they mess up.
 */
static const int gpio_pins_R1[GPIO_PINS] = {
	0, 1, -1, -1, 4, -1, -1, 7,
	8, 9, 10, 11, -1, -1, 14, 15,
	-1, 17, 18, -1, -1, 21, 22, 23,
	24, 25, -1, -1, -1, -1, -1, -1
};

static const int gpio_pins_R2[GPIO_PINS] = {
	-1, -1, -1, 3, 4, -1, -1, 7,
	8, 9, 10, 11, -1, -1, 14, 15,
	-1, 17, 18, -1, -1, -1, 22, 23,
	/* 28-31 accessible via P5 */
	24, 25, -1, 27, 28, 29, 30, 31
};


/**
 * Maps a memory region to the address space of the BM2835 GPIO controller.
 * \param drvthis  Pointer to driver structure.
 * \return         0 on success, -1 on error.
 */
static int
setup_io(Driver *drvthis)
{
	int mem_fd = 0;

	if (gpio_map != NULL) {
		report(RPT_ERR, "setup_io: IO already in use");
		return -1;
	}

	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		report(RPT_ERR, "setup_io: can not open /dev/mem");
		return -1;
	}

	gpio_map = (unsigned int *) mmap(NULL,
					 GPIO_BLOCK_SIZE,
					 PROT_READ | PROT_WRITE,
					 MAP_SHARED,
					 mem_fd,
					 GPIO_BASE);

	if (gpio_map == MAP_FAILED) {
		report(RPT_ERR, "setup_io: mmap failed: %s", strerror(errno));
		close(mem_fd);
		return -1;
	}
	close(mem_fd);

	debug(RPT_INFO, "setup_io: finished");
	return 0;
}


/**
 * Check if LCDd runs on a Raspberry Pi board and which revision (Rev 1 or
 * Rev 2) it is.
 * \param drvthis  Pointer to driver structure.
 * \return         Pointer to a GPIO mapping table or NULL on error.
 */
static int const *
check_board_rev(Driver *drvthis)
{
	FILE *fp;
	char buf[80];
	char hw[8];
	unsigned int rev = 0;

	if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
		report(RPT_ERR, "check_board_rev: Can not open /proc/cpuinfo");
		return NULL;
	}

	while (!feof(fp)) {
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "Hardware	: %7s", hw);
		sscanf(buf, "Revision	: %x", &rev);
	}
	fclose(fp);

	/* On boards that have been overvolted, the MSB will be set */
	rev &= 0x00ff;

	if (strcmp(hw, "BCM2708") != 0 || rev == 0) {
		report(RPT_ERR, "check_board_rev: This board is not recognized as a Raspberry Pi!");
		return NULL;
	}

	/* Rev 1 boards will be 0x002 or 0x003 (not sure if 0x001 ever existed
	 * in the wild). */
	if (rev < 4) {
		report(RPT_INFO, "check_board_rev: Revision 1 board detected");
		return gpio_pins_R1;
	}
	/* Currently, Rev 2 boards are 0x004, 0x005, or 0x006. This will need
	 * updating as new revisions are released, but it is unlikely that P1
	 * will change. */
	if (rev > 3) {
		report(RPT_INFO, "check_board_rev: Revision 2 board detected");
		return gpio_pins_R2;
	}

	return NULL;
}


/**
 * Check user provided GPIO pin configuration.
 * \param drvthis       Pointer to driver structure.
 * \param pin           Number of the GPIO pin to use.
 * \param allowed_pins  Pointer to array of allowed GPIO pins (depends on
 *                      board revision).
 * \param used_pins     Pointer to array of pin usage status.
 * \return  0 on success, -1 on error.
 */
static int
check_pin(Driver *drvthis, int pin, const int *allowed_pins, int *used_pins)
{
	/* Check for errors */
	if (pin >= GPIO_PINS || pin < 0) {
		report(RPT_ERR, "check_pin: GPIO pin %i out of range", pin);
		return -1;
	}
	if (allowed_pins[pin] < 0) {
		report(RPT_ERR, "check_pin: Use of GPIO pin %i not allowed", pin, allowed_pins[pin]);
		return -1;
	}
	if (used_pins[pin] == 1) {
		report(RPT_ERR, "check_pin: GPIO pin %i already used", pin);
		return -1;
	}

	/* Mark used */
	used_pins[pin] = 1;

	return 0;
}


/**
 * Configures a GPIO pin: Disable pull-up/down and set it up as output.
 * \param drvthis  Pointer to driver structure.
 * \param gpio     Number of the GPIO pin to use.
 */
static void
setup_gpio(Driver *drvthis, int gpio)
{
	volatile int i;

#define GPPUP    gpio_map + 37
#define GPPUDCLK gpio_map + 38

	/* Disable pull-up/down */
	*(GPPUP) &= ~3;

	/*
	 * After writing to the GPPUD register, need to wait 150 cycles as per
	 * p101 BCM2835.pdf. The following while loop uses approx five
	 * instructions plus another two to load the counter. Note: the int
	 * must be volatile or gcc will optimize loop out.
	 */
	i = 30;
	while (--i);

	/* Enable Pull-up/down clock register for the pin */
	*(GPPUDCLK + (gpio / 32)) = 1 << (gpio % 32);

	/* Another 150 cycle wait required after a write to GPPUDCLK */
	i = 30;
	while (--i);

	/* Write again to GPPUD and disable clock */
	*(GPPUP) &= ~3;
	*(GPPUDCLK + (gpio / 32)) = 0;

	/*
	 * Now configure the pin as output: This reads the complete GPSELx
	 * register, clears all bits for the selected pin, enables it as
	 * output, and writes back the register.
	 */
	*(gpio_map + (gpio / 10)) =
		(*(gpio_map + (gpio / 10)) & ~(7 << ((gpio % 10) * 3))) | (1 << ((gpio % 10) * 3));
}


/**
 * Free resources used by this connection type.
 * \param p  Pointer to driver's PrivateData structure.
 */
void
lcdrpi_HD44780_close(PrivateData *p)
{
	/* Configure all pins as input */
	INP_GPIO(p->rpi_gpio->en);
	INP_GPIO(p->rpi_gpio->rs);
	INP_GPIO(p->rpi_gpio->d7);
	INP_GPIO(p->rpi_gpio->d6);
	INP_GPIO(p->rpi_gpio->d5);
	INP_GPIO(p->rpi_gpio->d4);
	if (p->have_backlight)
		INP_GPIO(p->backlight_bit);
	if (p->numDisplays > 1)
		INP_GPIO(p->rpi_gpio->en2);

	/* Unmap and free memory */
	if (gpio_map != NULL)
		munmap((caddr_t) gpio_map, GPIO_BLOCK_SIZE);
	if (p->rpi_gpio != NULL)
		free(p->rpi_gpio);
	p->rpi_gpio = NULL;
}

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_rpi(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	const int *allowed_gpio_pins = NULL;
	int used_pins[GPIO_PINS] = {};

	if ((allowed_gpio_pins = check_board_rev(drvthis)) == NULL)
		return -1;

	/* Get GPIO configuration */
	p->rpi_gpio = malloc(sizeof(struct rpi_gpio_map));
	if (p->rpi_gpio == NULL) {
		report(RPT_ERR, "hd_init_rpi: unable to allocate memory");
		return -1;
	}

	p->rpi_gpio->en = drvthis->config_get_int(drvthis->name, "pin_EN", 0, RPI_DEF_EN);
	p->rpi_gpio->rs = drvthis->config_get_int(drvthis->name, "pin_RS", 0, RPI_DEF_RS);
	p->rpi_gpio->d7 = drvthis->config_get_int(drvthis->name, "pin_D7", 0, RPI_DEF_D7);
	p->rpi_gpio->d6 = drvthis->config_get_int(drvthis->name, "pin_D6", 0, RPI_DEF_D6);
	p->rpi_gpio->d5 = drvthis->config_get_int(drvthis->name, "pin_D5", 0, RPI_DEF_D5);
	p->rpi_gpio->d4 = drvthis->config_get_int(drvthis->name, "pin_D4", 0, RPI_DEF_D4);

	debug(RPT_INFO, "hd_init_rpi: Pin EN mapped to GPIO%d", p->rpi_gpio->en);
	debug(RPT_INFO, "hd_init_rpi: Pin RS mapped to GPIO%d", p->rpi_gpio->rs);
	debug(RPT_INFO, "hd_init_rpi: Pin D4 mapped to GPIO%d", p->rpi_gpio->d4);
	debug(RPT_INFO, "hd_init_rpi: Pin D5 mapped to GPIO%d", p->rpi_gpio->d5);
	debug(RPT_INFO, "hd_init_rpi: Pin D6 mapped to GPIO%d", p->rpi_gpio->d6);
	debug(RPT_INFO, "hd_init_rpi: Pin D7 mapped to GPIO%d", p->rpi_gpio->d7);

	if (check_pin(drvthis, p->rpi_gpio->en, allowed_gpio_pins, used_pins) ||
	    check_pin(drvthis, p->rpi_gpio->rs, allowed_gpio_pins, used_pins) ||
	    check_pin(drvthis, p->rpi_gpio->d7, allowed_gpio_pins, used_pins) ||
	    check_pin(drvthis, p->rpi_gpio->d6, allowed_gpio_pins, used_pins) ||
	    check_pin(drvthis, p->rpi_gpio->d5, allowed_gpio_pins, used_pins) ||
	    check_pin(drvthis, p->rpi_gpio->d4, allowed_gpio_pins, used_pins)) {
		free(p->rpi_gpio);
		return -1;
	}

	if (p->numDisplays > 1) {	/* For displays with two controllers */
		p->rpi_gpio->en2 = drvthis->config_get_int(drvthis->name, "pin_EN2", 0, RPI_DEF_EN2);
		debug(RPT_INFO, "hd_init_rpi: Pin EN2 mapped to GPIO%d", p->rpi_gpio->en2);
		if (check_pin(drvthis, p->rpi_gpio->en2, allowed_gpio_pins, used_pins)) {
			free(p->rpi_gpio);
			return -1;
		}
	}

	if (p->have_backlight) {	/* Backlight setup is optional */
		p->backlight_bit = drvthis->config_get_int(drvthis->name, "pin_BL", 0, RPI_DEF_BL);
		debug(RPT_INFO, "hd_init_rpi: Backlight mapped to GPIO%d", p->backlight_bit);

		if (check_pin(drvthis, p->backlight_bit, allowed_gpio_pins, used_pins) != 0) {
			report(RPT_WARNING, "hd_init_rpi: Invalid backlight configuration - disabling backlight");
			p->have_backlight = 0;
		}
	}

	/* Now that configuration should be correct, set up the GPIO pins */
	if (setup_io(drvthis) < 0) {
		report(RPT_ERR, "hd_init_rpi: Failed to set up GPIO");
		free(p->rpi_gpio);
		return -1;
	}

	setup_gpio(drvthis, p->rpi_gpio->en);
	setup_gpio(drvthis, p->rpi_gpio->rs);
	setup_gpio(drvthis, p->rpi_gpio->d7);
	setup_gpio(drvthis, p->rpi_gpio->d6);
	setup_gpio(drvthis, p->rpi_gpio->d5);
	setup_gpio(drvthis, p->rpi_gpio->d4);

	p->hd44780_functions->senddata = lcdrpi_HD44780_senddata;
	p->hd44780_functions->close = lcdrpi_HD44780_close;

	if (p->have_backlight) {
		setup_gpio(drvthis, p->backlight_bit);
		p->hd44780_functions->backlight = lcdrpi_HD44780_backlight;
	}

	if (p->numDisplays > 1) {
		setup_gpio(drvthis, p->rpi_gpio->en2);
	}

	/* Setup the lcd in 4 bit mode: Send (FUNCSET | IF_8BIT) three times
	 * followed by (FUNCSET | IF_4BIT) using four nibbles. Timing is not
	 * exactly what is required by HD44780. */
	p->hd44780_functions->senddata(p, 0, RS_INSTR, 0x33);
	p->hd44780_functions->uPause(p, 4100);
	p->hd44780_functions->senddata(p, 0, RS_INSTR, 0x32 );
	p->hd44780_functions->uPause(p, 150);

	common_init(p, IF_4BIT);

	return 0;
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
lcdrpi_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	/* Safeguard: This should never happen */
	if (gpio_map == NULL) {
		return;
	}

	if (flags == RS_INSTR) {
		SET_GPIO(p->rpi_gpio->rs, 0);
	}
	else {			/* flags == RS_DATA */
		SET_GPIO(p->rpi_gpio->rs, 1);
	}
	/* Clear data lines ready for nibbles */
	SET_GPIO(p->rpi_gpio->d7, 0);
	SET_GPIO(p->rpi_gpio->d6, 0);
	SET_GPIO(p->rpi_gpio->d5, 0);
	SET_GPIO(p->rpi_gpio->d4, 0);
	p->hd44780_functions->uPause(p, 50);

	/* Output upper nibble first */
	SET_GPIO(p->rpi_gpio->d7, (ch & 0x80));
	SET_GPIO(p->rpi_gpio->d6, (ch & 0x40));
	SET_GPIO(p->rpi_gpio->d5, (ch & 0x20));
	SET_GPIO(p->rpi_gpio->d4, (ch & 0x10));
	p->hd44780_functions->uPause(p, 50);

	/* Data is clocked on the falling edge of EN */
	if (displayID == 1 || displayID == 0)
		SET_GPIO(p->rpi_gpio->en, 1);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		SET_GPIO(p->rpi_gpio->en2, 1);
	p->hd44780_functions->uPause(p, 50);

	if (displayID == 1 || displayID == 0)
		SET_GPIO(p->rpi_gpio->en, 0);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		SET_GPIO(p->rpi_gpio->en2, 0);
	p->hd44780_functions->uPause(p, 50);

	/* Do same for lower nibble */
	SET_GPIO(p->rpi_gpio->d7, 0);
	SET_GPIO(p->rpi_gpio->d6, 0);
	SET_GPIO(p->rpi_gpio->d5, 0);
	SET_GPIO(p->rpi_gpio->d4, 0);
	p->hd44780_functions->uPause(p, 50);

	SET_GPIO(p->rpi_gpio->d7, (ch & 0x08));
	SET_GPIO(p->rpi_gpio->d6, (ch & 0x04));
	SET_GPIO(p->rpi_gpio->d5, (ch & 0x02));
	SET_GPIO(p->rpi_gpio->d4, (ch & 0x01));
	p->hd44780_functions->uPause(p, 50);

	if (displayID == 1 || displayID == 0)
		SET_GPIO(p->rpi_gpio->en, 1);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		SET_GPIO(p->rpi_gpio->en2, 1);
	p->hd44780_functions->uPause(p, 50);

	if (displayID == 1 || displayID == 0)
		SET_GPIO(p->rpi_gpio->en, 0);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		SET_GPIO(p->rpi_gpio->en2, 0);
	p->hd44780_functions->uPause(p, 50);
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
lcdrpi_HD44780_backlight(PrivateData *p, unsigned char state)
{
	if (p->backlight_bit > -1 && p->backlight_bit < 32)
		SET_GPIO(p->backlight_bit, (state == BACKLIGHT_ON) ? 1 : 0);
}
