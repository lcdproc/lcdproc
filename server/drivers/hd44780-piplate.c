/** \file server/drivers/hd44780-piplate.c
 *
 * \c I2C (via MCP23017) connection type of \c hd44780 driver for Hitachi
 * HD44780 based LCD displays connected to the MCP23017 I2C I/O expander
 * (Adafruit Pi Plate, see http://www.adafruit.com/products/1109).
 *
 * The LCD is operated in its 4 bit-mode to be connected to the 8 bit-port
 * of a single MCP23017 that is accessed by the server via the I2C bus.
 */

/*-
 * Copyright (c)  2013 Jonathan Brogdon <jlbrogdon@gmail.com>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Based mostly on the hd44780-i2c module, see there for a complete history.
 */

/*-
 * The connections are:
 * MCP23017     LCD
 * GPA7         Green LCD backlight
 * GPA6         Red LCD backlight
 * GPA4         Left key
 * GPA3         Up key
 * GPA2         Down key
 * GPA1         Right key
 * GPA0         Select key
 * GPB7         RS (4)
 * GPB6         RW (5)
 * GPB5         EN (6)
 * GPB4         D7 (14)
 * GPB3         D6 (13)
 * GPB2         D5 (12)
 * GPB1         D4 (11)
 * GPB0         Blue LCD backlight
 *
 * Configuration:
 * device=/dev/i2c-1   # the device file of the i2c bus
 * port=0x20           # the i2c address of the i2c port expander
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "hd44780-piplate.h"
#include "hd44780-low.h"
#include "report.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef HAVE_DEV_IICBUS_IIC_H
# include <strings.h>
# include <dev/iicbus/iic.h>
#else				/* HAVE_LINUX_I2C_DEV_H */
# include <linux/i2c-dev.h>
#endif

#define DEFAULT_DEVICE		"/dev/i2c-1"

void i2c_piplate_HD44780_senddata(PrivateData *p,
				  unsigned char displayID, unsigned char flags, unsigned char ch);
void i2c_piplate_HD44780_backlight(PrivateData *p, unsigned char state);
unsigned char i2c_piplate_HD44780_scankeypad(PrivateData *p);
void i2c_piplate_HD44780_close(PrivateData *p);

/* MCP23017 registers */
#define MCP23017_IODIRA 0x00
#define MCP23017_IODIRB 0x01
#define MCP23017_GPPUA  0x0C
#define MCP23017_GPPUB  0x0D
#define MCP23017_GPIOA  0x12
#define MCP23017_GPIOB  0x13

/* GPIOA BITS */
#define G_BIT     (1 << 7)
#define R_BIT     (1 << 6)
#define L_KEY_BIT (1 << 4)
#define U_KEY_BIT (1 << 3)
#define D_KEY_BIT (1 << 2)
#define R_KEY_BIT (1 << 1)
#define S_KEY_BIT (1 << 0)

/* GPIOB BITS */
#define RS_BIT (1 << 7)
#define RW_BIT (1 << 6)
#define EN_BIT (1 << 5)
#define D0_BIT (1 << 4)
#define D1_BIT (1 << 3)
#define D2_BIT (1 << 2)
#define D3_BIT (1 << 1)
#define B_BIT  (1 << 0)

/* Reverses the bits in a 4 bit word */
#define SWIZZLE_4BITS(x) ((((x) & 0x01) << 3) | (((x) & 0x02) << 1) | (((x) & 0x04) >> 1) | (((x) & 0x08) >> 3))

/* Swizzle and offset bits correctly for the PiPlate wiring */
#define DATA_BIT_START_OFFSET 1
#define MAP_TO_DATA_PINS(x) (SWIZZLE_4BITS(x) << DATA_BIT_START_OFFSET)

#define I2C_ADDR_MASK 0x7f

/**
 * Read one of the MCP23017 registers
 * \param p     Pointer to driver private data.
 * \param reg   Address of the register to read.
 * \param val   Pointer to the read buffer.
 * \retval 0    Success.
 * \retval -1   Error.
 */
static int
i2c_read_reg(PrivateData *p, unsigned char reg, unsigned char *val)
{
	/* Set the address to be read */
	if (write(p->fd, &reg, 1) != 1) {
		return -1;
	}

	/* Read the value */
	if (read(p->fd, val, 1) != 1) {
		return -1;
	}

	return 0;
}

/**
 * Write one of the MCP23017 registers
 * \param p     Pointer to driver private data.
 * \param reg   Address of the register to write.
 * \param val   Value to be written.
 * \retval 0    Success.
 * \retval -1   Error.
 */
static int
i2c_write_reg(PrivateData *p, unsigned char reg, unsigned char val)
{
	unsigned char buf[2];

	buf[0] = reg;
	buf[1] = val;
	if (write(p->fd, buf, sizeof(buf)) != sizeof(buf)) {
		return -1;
	}
	return 0;
}

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_i2c_piplate(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;
	char device[256] = DEFAULT_DEVICE;
#ifdef HAVE_DEV_IICBUS_IIC_H
	struct iiccmd cmd;
	bzero(&cmd, sizeof(cmd));
#endif

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "HD44780: piplate: Using device '%s' and address 0x%02X for a MCP23017",
	       device, p->port & I2C_ADDR_MASK);

	/* Open the I2C device */
	p->fd = open(device, O_RDWR);
	if (p->fd < 0) {
		report(RPT_ERR, "HD44780: piplate: open i2c device '%s' failed: %s",
		       device, strerror(errno));
		return -1;
	}

	/* Set I2C address */
#ifdef HAVE_DEV_IICBUS_IIC_H
	cmd.slave = (p->port & I2C_ADDR_MASK) << 1;
	cmd.last = 0;
	cmd.count = 0;
	if (ioctl(p->fd, I2CRSTCARD, &cmd) < 0) {
		report(RPT_ERR, "HD44780: piplate: reset bus failed: %s", strerror(errno));
		return -1;
	}
	if (ioctl(p->fd, I2CSTART, &cmd) < 0) {
		report(RPT_ERR, "HD44780: piplate: set address to 0x%02X: %s",
		       p->port & I2C_ADDR_MASK, strerror(errno));
		return -1;
	}
#else				/* HAVE_LINUX_I2C_DEV_H */
	if (ioctl(p->fd, I2C_SLAVE, p->port & I2C_ADDR_MASK) < 0) {
		report(RPT_ERR, "HD44780: piplate: set address to 0x%02X: %s",
		       p->port & I2C_ADDR_MASK, strerror(errno));
		return -1;
	}
#endif

	/* IODIRA - keys as input, all other as output */
	i2c_write_reg(p, MCP23017_IODIRA,
		      (unsigned char) (L_KEY_BIT | U_KEY_BIT | D_KEY_BIT | R_KEY_BIT | S_KEY_BIT) & ~(G_BIT | R_BIT));

	/* IODIRB - LCD control/data bus signals as outputs */
	i2c_write_reg(p, MCP23017_IODIRB,
		      (unsigned char) ~(RS_BIT | RW_BIT | EN_BIT | D0_BIT | D1_BIT | D2_BIT | D3_BIT | B_BIT));

	/* GPPUA - Pullups for key switch inputs */
	i2c_write_reg(p, MCP23017_GPPUA,
		      (unsigned char) (L_KEY_BIT | U_KEY_BIT | D_KEY_BIT | R_KEY_BIT | S_KEY_BIT) & ~(G_BIT | R_BIT));

	/* GPPUB - Pullups disabled */
	i2c_write_reg(p, MCP23017_GPPUB, (unsigned char) 0x00);

	/* Set driver functions */
	hd44780_functions->senddata = i2c_piplate_HD44780_senddata;
	hd44780_functions->backlight = i2c_piplate_HD44780_backlight;
	hd44780_functions->scankeypad = i2c_piplate_HD44780_scankeypad;
	hd44780_functions->close = i2c_piplate_HD44780_close;

	/* Initialize the LCD */
	hd44780_functions->senddata(p, 0, RS_INSTR, 0x33);
	hd44780_functions->uPause(p, 1);
	hd44780_functions->senddata(p, 0, RS_INSTR, 0x32);
	hd44780_functions->uPause(p, 1);
	common_init(p, IF_4BIT);

	report(RPT_INFO, "HD44780: piplate: initialized!");

	return 0;
}

/**
 * Close the device.
 * \param p          Pointer to driver's private data structure.
 */
void
i2c_piplate_HD44780_close(PrivateData *p)
{
	if (p->fd >= 0) {
		close(p->fd);
	}
}

/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
i2c_piplate_HD44780_senddata(PrivateData *p,
			     unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char nibble[2];
	unsigned char write_val = 0;
	int nibble_idx = 0;

	/* Split the val byte into nibbles */
	nibble[0] = (ch >> 4);
	nibble[1] = (ch & 0x0f);

	for (nibble_idx = 0; nibble_idx < 2; nibble_idx++) {
		/* Adjust for 4-bit data pins */
		write_val = MAP_TO_DATA_PINS(nibble[nibble_idx]);

		/* Set the RS bit */
		if (flags == RS_INSTR)
			write_val &= ~RS_BIT;
		else
			write_val |= RS_BIT;

		/* Set the blue backlight bit if backlight is on */
		if (p->backlightstate == BACKLIGHT_OFF)
			write_val |= B_BIT;

		/* Set enable bit */
		i2c_write_reg(p, MCP23017_GPIOB, write_val | EN_BIT);

		p->hd44780_functions->uPause(p, 1);

		/* Clear enable bit */
		i2c_write_reg(p, MCP23017_GPIOB, write_val & ~EN_BIT);

	}			/* for each nibble */

	p->hd44780_functions->uPause(p, 1);
}

/**
 * Turn display backlight on or off. For the Pi Plate this means a read /
 * modify / write cycle is required, as the bits are shared with other signals
 * on the same byte.
 *
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
i2c_piplate_HD44780_backlight(PrivateData *p, unsigned char state)
{
	unsigned char gpioa = 0;
	unsigned char gpiob = 0;

	/* Read the RG bits */
	if (i2c_read_reg(p, MCP23017_GPIOA, &gpioa) != 0) {
		return;
	}

	/* Read the B bit */
	if (i2c_read_reg(p, MCP23017_GPIOB, &gpiob) != 0) {
		return;
	}

	/* Set or clear the RGB bits. Backlight is ON if the bits are CLEAR */
	if (state == BACKLIGHT_ON) {
		gpioa &= ~(R_BIT | G_BIT);
		gpiob &= ~(B_BIT);
	}
	else {
		gpioa |= (R_BIT | G_BIT);
		gpiob |= (B_BIT);
	}

	/* Write the RGB bits */
	i2c_write_reg(p, MCP23017_GPIOA, gpioa);
	i2c_write_reg(p, MCP23017_GPIOB, gpiob);
}

/**
 * Scan the LCD keys
 * \param p      Pointer to driver's private data structure.
 * \retval key   Key number
 */
unsigned char
i2c_piplate_HD44780_scankeypad(PrivateData *p)
{
	unsigned char gpioa = 0;

	/* Read the key bits */
	if (i2c_read_reg(p, MCP23017_GPIOA, &gpioa) != 0)
		return 0;

	/* Select key */
	if ((gpioa & S_KEY_BIT) == 0)
		return 1;

	/* Up key */
	if ((gpioa & U_KEY_BIT) == 0)
		return 2;

	/* Down key */
	if ((gpioa & D_KEY_BIT) == 0)
		return 3;

	/* Left key */
	if ((gpioa & L_KEY_BIT) == 0)
		return 4;

	/* Right key */
	if ((gpioa & R_KEY_BIT) == 0)
		return 5;

	return 0;
}
