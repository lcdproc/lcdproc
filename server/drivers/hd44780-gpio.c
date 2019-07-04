/** \file server/drivers/hd44780-gpio.c
 * \c GPIO connection type of \c hd44780 driver for Hitachi HD44780 based LCD
 * displays.
 *
 * The LCD is operated in its 4 bit-mode. R/W (5) on the LCD MUST be hard wired
 * low to prevent 5V logic appearing on the GPIO pins.
 *
 * Mappings can be set in the config file using the keys:
 * pin_EN, pin_EN2, pin_RS, pin_D7, pin_D6, pin_D5, pin_D4, pin_BL, pin_RW
 * in the [hd44780] section.
 */

/*-
 * Copyright (c) 2015 Fatih Aşıcı <fatih.asici@gmail.com>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ugpio/ugpio.h>

#include "hd44780-gpio.h"
#include "hd44780-low.h"
#include "shared/report.h"

void gpio_HD44780_senddata(PrivateData *p, unsigned char displayID,
			   unsigned char flags, unsigned char ch);
void gpio_HD44780_backlight(PrivateData *p, unsigned char state);
void gpio_HD44780_close(PrivateData *p);

typedef struct {
	ugpio_t *en;
	ugpio_t *rs;
	ugpio_t *d7;
	ugpio_t *d6;
	ugpio_t *d5;
	ugpio_t *d4;
	ugpio_t *en2;
	ugpio_t *bl;
	ugpio_t *rw;
} gpio_pins;

/**
 * Initialize a ugpio_t context by reading the related configuration
 * option.
 *
 * \param drvthis   Pointer to driver structure.
 * \param pin       Pointer to ugpio_t *structure to be initialized.
 * \param name      Name of the GPIO pin.
 * \return          0 on success; -1 on error.
 */
static int
init_gpio_pin(Driver *drvthis, ugpio_t **pin, const char *name)
{
	char config_key[8];
	int number;

	snprintf(config_key, sizeof(config_key), "pin_%s", name);
	number = drvthis->config_get_int(drvthis->name, config_key, 0, -1);
	if (number == -1)
		return -1;

	*pin = ugpio_request_one(number, GPIOF_OUT_INIT_LOW, name);
	if (*pin == NULL) {
		report(RPT_ERR, "init_gpio_pin: unable to request GPIO%d: %s",
		       number, strerror(errno));
		return -1;
	}

	if (ugpio_open(*pin) < 0) {
		report(RPT_ERR, "init_gpio_pin: unable to open file descriptor for GPIO%d: %s",
		       number, strerror(errno));
		ugpio_free(*pin);
		*pin = NULL;
		return -1;
	}

	report(RPT_INFO, "init_gpio_pin: Pin %s mapped to GPIO%d", name, number);

	return 0;
}


/**
 * Send 4-bit data.
 *
 * \param p     Pointer to driver's private data structure.
 * \param ch    The value to send (lower nibble must contain the data).
 */
static void
send_nibble(PrivateData *p, unsigned char ch, unsigned char displayID)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	ugpio_set_value(pins->d7, ch & 0x08);
	ugpio_set_value(pins->d6, ch & 0x04);
	ugpio_set_value(pins->d5, ch & 0x02);
	ugpio_set_value(pins->d4, ch & 0x01);

	/* Data is clocked on the falling edge of EN */
	if (displayID == 1 || displayID == 0)
		ugpio_set_value(pins->en, 1);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		ugpio_set_value(pins->en2, 1);
        p->hd44780_functions->uPause(p, 50);

	if (displayID == 1 || displayID == 0)
		ugpio_set_value(pins->en, 0);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		ugpio_set_value(pins->en2, 0);
	p->hd44780_functions->uPause(p, 50);
}


/**
 * Initialize the driver.
 *
 * \param drvthis   Pointer to driver structure.
 * \return          0 on success; -1 on error.
 */
int
hd_init_gpio(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	gpio_pins *pins = malloc(sizeof(gpio_pins));

	if (pins == NULL) {
		report(RPT_ERR, "hd_init_gpio: unable to allocate memory");
		return -1;
	}

	p->connection_data = pins;

	if (init_gpio_pin(drvthis, &pins->en, "EN") != 0 ||
	    init_gpio_pin(drvthis, &pins->rs, "RS") != 0 ||
	    init_gpio_pin(drvthis, &pins->d7, "D7") != 0 ||
	    init_gpio_pin(drvthis, &pins->d6, "D6") != 0 ||
	    init_gpio_pin(drvthis, &pins->d5, "D5") != 0 ||
	    init_gpio_pin(drvthis, &pins->d4, "D4") != 0) {
		report(RPT_ERR, "hd_init_gpio: unable to initialize GPIO pins");
		gpio_HD44780_close(p);
		return -1;
	}

	if (p->numDisplays > 1) {       /* For displays with two controllers */
		if (init_gpio_pin(drvthis, &pins->en2, "EN2") != 0) {
			report(RPT_ERR, "hd_init_gpio: unable to initialize GPIO pins");
			gpio_HD44780_close(p);
			return -1;
		}
	}

	p->hd44780_functions->senddata = gpio_HD44780_senddata;
	p->hd44780_functions->close = gpio_HD44780_close;

	if (have_backlight_pin(p)) {
		if (init_gpio_pin(drvthis, &pins->bl, "BL") != 0) {
			report(RPT_WARNING,
			       "hd_init_gpio: unable to initialize pin_BL - disabling backlight");
			set_have_backlight_pin(p, 0);
		}
		else {
			p->hd44780_functions->backlight = gpio_HD44780_backlight;
		}
	}

	/* This is enough to set the RW pin to low, if it is available. */
	init_gpio_pin(drvthis, &pins->rw, "RW");

	ugpio_set_value(pins->rs, 0);

	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	p->hd44780_functions->uPause(p, 4100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	p->hd44780_functions->uPause(p, 100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	send_nibble(p, (FUNCSET | IF_4BIT) >> 4, 0);

	common_init(p, IF_4BIT);

	return 0;
}


/**
 * Send data or commands to the display.
 *
 * \param p             Pointer to driver's private data structure.
 * \param displayID     ID of the display (or 0 for all) to send data to.
 * \param flags         Defines whether to end a command or data.
 * \param ch            The value to send.
 */
void
gpio_HD44780_senddata(PrivateData *p, unsigned char displayID,
		      unsigned char flags, unsigned char ch)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	ugpio_set_value(pins->rs, (flags == RS_INSTR) ? 0 : 1);

	send_nibble(p, ch >> 4, displayID);
	send_nibble(p, ch, displayID);
}


/**
 * Turn display backlight on or off.
 *
 * \param p         Pointer to driver's private data structure.
 * \param state     New backlight status.
 */
void
gpio_HD44780_backlight(PrivateData *p, unsigned char state)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	ugpio_set_value(pins->bl, (state == BACKLIGHT_ON) ? 1 : 0);
}

/**
 * Free resources used by pin
 *
 * \param pin     Pointer to gpio context.
 */
void
release_gpio_pin(ugpio_t **pin)
{
	ugpio_close(*pin);
	ugpio_free(*pin);
	*pin = NULL;
}

/**
 * Free resources used by this connection type.
 *
 * \param p     Pointer to driver's PrivateData structure.
 */
void
gpio_HD44780_close(PrivateData *p)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	release_gpio_pin(&pins->en);
	release_gpio_pin(&pins->rs);
	release_gpio_pin(&pins->d7);
	release_gpio_pin(&pins->d6);
	release_gpio_pin(&pins->d5);
	release_gpio_pin(&pins->d4);

	if (p->numDisplays > 1)
		release_gpio_pin(&pins->en2);

	if (have_backlight_pin(p))
		release_gpio_pin(&pins->bl);

	if (pins->rw)
		release_gpio_pin(&pins->rw);

	free(pins);
}
