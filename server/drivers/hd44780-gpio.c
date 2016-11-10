/** \file server/drivers/hd44780-gpio.c
 * \c GPIO connection type of \c hd44780 driver for Hitachi HD44780 based LCD
 * displays.
 *
 * The LCD is operated in its 4 bit-mode. R/W (5) on the LCD MUST be hard wired
 * low to prevent 5V logic appearing on the GPIO pins.
 *
 * Mappings can be set in the config file using the keys:
 * pin_EN, pin_RS, pin_D7, pin_D6, pin_D5, pin_D4, pin_BL
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
#include <unistd.h>

#include "hd44780-gpio.h"
#include "hd44780-low.h"
#include "report.h"

void gpio_HD44780_senddata(PrivateData *p, unsigned char displayID,
			   unsigned char flags, unsigned char ch);
void gpio_HD44780_backlight(PrivateData *p, unsigned char state);
void gpio_HD44780_close(PrivateData *p);

typedef struct {
	const char *name;
	int number;
	int fd;
	int last_value;
} gpio_pin;

typedef struct {
	gpio_pin en;
	gpio_pin rs;
	gpio_pin d7;
	gpio_pin d6;
	gpio_pin d5;
	gpio_pin d4;
	gpio_pin bl;
} gpio_pins;


/**
 * Ask Linux kernel to export node for a GPIO.
 *
 * \param num   Number of the GPIO pin.
 * \return      0 on success; -1 on error.
 */
static int
export_gpio(int num)
{
	FILE *f = fopen("/sys/class/gpio/export", "w");
	int ret;

	if (f == NULL)
		return -1;

	fprintf(f, "%d", num);
	ret = fclose(f);

	/* If EBUSY is returned, this GPIO might be exported already.
	 * Return failure for other errors. */
	if (ret == EOF && errno != EBUSY)
		return -1;

	return 0;
}


/**
 * Ask Linux kernel to remove a GPIO node exported previously.
 *
 * \param num   Number of the GPIO pin.
 * \return      0 on success; -1 on error.
 */
static int
unexport_gpio(int num)
{
	FILE *f = fopen("/sys/class/gpio/unexport", "w");

	if (f == NULL)
		return -1;

	fprintf(f, "%d", num);
	fclose(f);

	return 0;
}


/**
 * Initialize a gpio_pin structure by reading the related configuration
 * option.
 *
 * \param drvthis   Pointer to driver structure.
 * \param pin       Pointer to gpio_pin structure to be initialized.
 * \param name      Name of the GPIO pin.
 */
static void
init_gpio_pin(Driver *drvthis, gpio_pin *pin, const char *name)
{
	char config_key[8];

	snprintf(config_key, sizeof(config_key), "pin_%s", name);
	pin->name = name;
	pin->number = drvthis->config_get_int(drvthis->name, config_key, 0, -1);
	pin->fd = -1;
}


/**
 * Configure a GPIO pin: Export it, set it up as output and get a file
 * descriptor for future write calls.
 *
 * \param drvthis   Pointer to driver structure.
 * \param pin       Pointer to gpio_pin structure.
 * \return          0 on success; -1 on error.
 */
static int
setup_gpio_pin(Driver *drvthis, gpio_pin *pin)
{
	char buf[40];
	int fd;

	if (pin->number < 0) {
		report(RPT_ERR, "setup_gpio_pin: pin_%s must be set to a valid value", pin->name);
		return -1;
	}

	if (export_gpio(pin->number) != 0) {
		report(RPT_ERR, "setup_gpio_pin: unable to export GPIO %d: %s",
		       pin->number, strerror(errno));
		return -1;
	}

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", pin->number);
	fd = open(buf, O_WRONLY);

	if (fd < 0)
		goto open_failed;

	write(fd, "low", 3);
	pin->last_value = 0;
	close(fd);

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin->number);
	pin->fd = open(buf, O_WRONLY | O_SYNC);

	if (pin->fd < 0)
		goto open_failed;

	debug(RPT_INFO, "setup_gpio_pin: Pin %s mapped to GPIO%d", pin->name, pin->number);

	return 0;

open_failed:
	report(RPT_ERR, "setup_gpio_pin: unable to open %s for writing: %s", buf, strerror(errno));
	unexport_gpio(pin->number);
	return -1;
}


/**
 * Close the file descriptor of the pin and unexport the GPIO node in sysfs.
 *
 * \param pin   Pointer to gpio_pin structure.
 */
static void
release_gpio_pin(const gpio_pin *pin)
{
	if (pin->fd >= 0) {
		close(pin->fd);
		unexport_gpio(pin->number);
	}
}


/**
 * Set the GPIO output value.
 *
 * \param pin   Pointer to gpio_pin structure.
 * \param value New output value: 0 (low), 1 (high)
 */
static void
set_gpio_pin(gpio_pin *pin, int value)
{
	if (value != pin->last_value) {
		write(pin->fd, value ? "1" : "0", 1);
		pin->last_value = value;
	}
}


/**
 * Send 4-bit data.
 *
 * \param p     Pointer to driver's private data structure.
 * \param ch    The value to send (lower nibble must contain the data).
 */
static void
send_nibble(PrivateData *p, unsigned char ch)
{
	gpio_pins *pins = (gpio_pins *) p->connection_data;

	set_gpio_pin(&pins->d7, ch & 0x08);
	set_gpio_pin(&pins->d6, ch & 0x04);
	set_gpio_pin(&pins->d5, ch & 0x02);
	set_gpio_pin(&pins->d4, ch & 0x01);

	/* Data is clocked on the falling edge of EN */
	set_gpio_pin(&pins->en, 1);
	p->hd44780_functions->uPause(p, 50);
	set_gpio_pin(&pins->en, 0);
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

	init_gpio_pin(drvthis, &pins->en, "EN");
	init_gpio_pin(drvthis, &pins->rs, "RS");
	init_gpio_pin(drvthis, &pins->d7, "D7");
	init_gpio_pin(drvthis, &pins->d6, "D6");
	init_gpio_pin(drvthis, &pins->d5, "D5");
	init_gpio_pin(drvthis, &pins->d4, "D4");
	init_gpio_pin(drvthis, &pins->bl, "BL");

	if (setup_gpio_pin(drvthis, &pins->en) != 0 ||
	    setup_gpio_pin(drvthis, &pins->rs) != 0 ||
	    setup_gpio_pin(drvthis, &pins->d7) != 0 ||
	    setup_gpio_pin(drvthis, &pins->d6) != 0 ||
	    setup_gpio_pin(drvthis, &pins->d5) != 0 ||
	    setup_gpio_pin(drvthis, &pins->d4) != 0) {
		report(RPT_ERR, "hd_init_gpio: unable to initialize GPIO pins");
		gpio_HD44780_close(p);
		return -1;
	}

	p->hd44780_functions->senddata = gpio_HD44780_senddata;
	p->hd44780_functions->close = gpio_HD44780_close;

	if (p->have_backlight) {
		if (setup_gpio_pin(drvthis, &pins->bl) != 0) {
			report(RPT_WARNING,
			       "hd_init_gpio: unable to initialize pin_BL - disabling backlight");
			p->have_backlight = 0;
		}
		else {
			p->hd44780_functions->backlight = gpio_HD44780_backlight;
		}
	}

	set_gpio_pin(&pins->rs, 0);

	send_nibble(p, (FUNCSET | IF_8BIT) >> 4);
	p->hd44780_functions->uPause(p, 4100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4);
	p->hd44780_functions->uPause(p, 100);
	send_nibble(p, (FUNCSET | IF_8BIT) >> 4);
	send_nibble(p, (FUNCSET | IF_4BIT) >> 4);

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

	set_gpio_pin(&pins->rs, (flags == RS_INSTR) ? 0 : 1);

	send_nibble(p, ch >> 4);
	send_nibble(p, ch);
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

	set_gpio_pin(&pins->bl, (state == BACKLIGHT_ON) ? 1 : 0);
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

	if (p->have_backlight)
		release_gpio_pin(&pins->bl);

	free(pins);
}
