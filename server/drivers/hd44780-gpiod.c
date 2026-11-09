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
#include <gpiod.h>

#include "hd44780-gpiod.h"
#include "hd44780-low.h"
#include "shared/report.h"

void gpiod_HD44780_senddata(PrivateData *p, unsigned char displayID,
			   unsigned char flags, unsigned char ch);
void gpiod_HD44780_backlight(PrivateData *p, unsigned char state);
void gpiod_HD44780_reset(PrivateData *p);
void gpiod_HD44780_close(PrivateData *p);

typedef struct {
	uint offset;
	struct gpiod_line_request *req;
	struct gpiod_line_settings *settings;
	struct gpiod_line_config *line_cfg;
	struct gpiod_request_config *req_cfg;
} gpio_pin;

typedef struct {
	struct gpiod_chip *chip;
	gpio_pin *en;
	gpio_pin *rs;
	gpio_pin *d7;
	gpio_pin *d6;
	gpio_pin *d5;
	gpio_pin *d4;
	gpio_pin *en2;
	gpio_pin *bl;
	gpio_pin *rw;
} gpio_pins;

/**
 * Set GPIO pin state.
 *
 * \param pin     Pointer to gpio_pin structure.
 * \param value   The new PIN state to set.
 */
static int
gpio_set_value(gpio_pin *pin, int value)
{
	enum gpiod_line_value new_val = value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
	return gpiod_line_request_set_value(pin->req, pin->offset, new_val);
}


/**
 * Initialize a struct gpiod_line context by reading the related configuration
 * option.
 *
 * \param drvthis   Pointer to driver structure.
 * \param pin       Pointer to struct gpiod_line *structure to be initialized.
 * \param name      Name of the GPIO pin.
 * \return          0 on success; -1 on error.
 */
static int
init_gpio_pin(Driver *drvthis, gpio_pin **gpin, const char *name)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	gpio_pins *pins = (gpio_pins *) p->connection_data;
	char config_key[10];
	int number;

	snprintf(config_key, sizeof(config_key), "pin_%s", name);
	number = drvthis->config_get_int(drvthis->name, config_key, 0, -1);
	if (number == -1)
		return -1;

	*gpin = calloc(1, sizeof(gpio_pin));
	(*gpin)->offset = number;

	(*gpin)->settings = gpiod_line_settings_new();
	gpiod_line_settings_set_direction((*gpin)->settings, GPIOD_LINE_DIRECTION_OUTPUT);
	gpiod_line_settings_set_output_value((*gpin)->settings, GPIOD_LINE_VALUE_INACTIVE);

	(*gpin)->line_cfg = gpiod_line_config_new();
	gpiod_line_config_add_line_settings((*gpin)->line_cfg, &(*gpin)->offset, 1, (*gpin)->settings);

	// generate and register a friendly consumer name with real PIN function
	(*gpin)->req_cfg = gpiod_request_config_new();
	snprintf(config_key, sizeof(config_key), "LCDd_%s", name);
	gpiod_request_config_set_consumer((*gpin)->req_cfg, config_key);

	(*gpin)->req = gpiod_chip_request_lines(pins->chip, (*gpin)->req_cfg, (*gpin)->line_cfg);

	if (!(*gpin)->req) {
		report(RPT_ERR, "init_gpio_pin: unable to open file descriptor for GPIO%d: %s",
		       number, strerror(errno));
		return -1;
	}

	report(RPT_INFO, "init_gpio_pin: Pin %s mapped to GPIO%d", name, number);

	return 0;
}


/**
 * Free related pin resources which should be freed.
 *
 * \param pin     Pointer to gpio_pin structure.
 */
static void
release_gpio_pin(gpio_pin *pin)
{
	if (!pin)
		return;

	if (pin->req_cfg)
		gpiod_request_config_free(pin->req_cfg);
	if (pin->line_cfg)
		gpiod_line_config_free(pin->line_cfg);
	if (pin->settings)
		gpiod_line_settings_free(pin->settings);
	if (pin->req)
		gpiod_line_request_release(pin->req);
	free(pin);
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

	gpio_set_value(pins->d7, ch & 0x08);
	gpio_set_value(pins->d6, ch & 0x04);
	gpio_set_value(pins->d5, ch & 0x02);
	gpio_set_value(pins->d4, ch & 0x01);

	/* Data is clocked on the falling edge of EN */
	if (displayID == 1 || displayID == 0)
		gpio_set_value(pins->en, 1);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		gpio_set_value(pins->en2, 1);
        p->hd44780_functions->uPause(p, 50);

	if (displayID == 1 || displayID == 0)
		gpio_set_value(pins->en, 0);
	if (displayID == 2 || (p->numDisplays > 1 && displayID == 0))
		gpio_set_value(pins->en2, 0);
	p->hd44780_functions->uPause(p, 50);
}


/**
 * Initialize the driver.
 *
 * \param drvthis   Pointer to driver structure.
 * \return          0 on success; -1 on error.
 */
int
hd_init_gpiod(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	gpio_pins *pins;
	const char *chip;

	/* Be strict here instead of just using the first chip:
	 * Better to force everybody to update instead of silently breaking
	 * some configurations and have others stay with unsafe configurations.
	 */
	chip = drvthis->config_get_string(drvthis->name, "gpiochip", 0, NULL);
	if (!chip) {
		report(RPT_ERR, "%s: gpio chip not configured, but legacy GPIO numbering not supported", drvthis->name);
		report(RPT_ERR, "Please add 'gpiochip' setting and update pin numbers accordingly.");
		return -1;
	}

	pins = calloc(1, sizeof(gpio_pins));

	pins->chip = gpiod_chip_open(chip);
	if (!pins->chip) {
		report(RPT_ERR, "%s: can't open gpio chip '%s'", drvthis->name, chip);
		free(pins);
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
		gpiod_HD44780_close(p);
		return -1;
	}

	if (p->numDisplays > 1) {       /* For displays with two controllers */
		if (init_gpio_pin(drvthis, &pins->en2, "EN2") != 0) {
			report(RPT_ERR, "hd_init_gpio: unable to initialize GPIO pins");
			gpiod_HD44780_close(p);
			return -1;
		}
	}

	p->hd44780_functions->senddata = gpiod_HD44780_senddata;
	p->hd44780_functions->close = gpiod_HD44780_close;
	p->hd44780_functions->reset = gpiod_HD44780_reset;

	if (have_backlight_pin(p)) {
		if (init_gpio_pin(drvthis, &pins->bl, "BL") != 0) {
			report(RPT_WARNING,
			       "hd_init_gpio: unable to initialize pin_BL - disabling backlight");
			set_have_backlight_pin(p, 0);
		}
		else {
			p->hd44780_functions->backlight = gpiod_HD44780_backlight;
		}
	}

	/* This is enough to set the RW pin to low, if it is available. */
	init_gpio_pin(drvthis, &pins->rw, "RW");

	gpiod_HD44780_reset(p);

	return 0;
}

/**
 * Reset the display to a known state.
 *
 * \param p             Pointer to driver's private data structure.
 */
void
gpiod_HD44780_reset(PrivateData *p)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	gpio_set_value(pins->rs, 0);

	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	p->hd44780_functions->uPause(p, 4100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	p->hd44780_functions->uPause(p, 100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4, 0);
	send_nibble(p, (FUNCSET | IF_4BIT) >> 4, 0);

	common_init(p, IF_4BIT);
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
gpiod_HD44780_senddata(PrivateData *p, unsigned char displayID,
		      unsigned char flags, unsigned char ch)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	gpio_set_value(pins->rs, (flags == RS_INSTR) ? 0 : 1);

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
gpiod_HD44780_backlight(PrivateData *p, unsigned char state)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	gpio_set_value(pins->bl, (state == BACKLIGHT_ON) ? 1 : 0);
}


/**
 * Free resources used by this connection type.
 *
 * \param p     Pointer to driver's PrivateData structure.
 */
void
gpiod_HD44780_close(PrivateData *p)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	release_gpio_pin(pins->en);
	release_gpio_pin(pins->rs);
	release_gpio_pin(pins->d7);
	release_gpio_pin(pins->d6);
	release_gpio_pin(pins->d5);
	release_gpio_pin(pins->d4);
	release_gpio_pin(pins->en2);
	release_gpio_pin(pins->bl);
	release_gpio_pin(pins->rw);

	gpiod_chip_close(pins->chip);

	free(pins);
}
