/** \file server/drivers/hd44780-pifacecad.c
 * \c PiFace Control and Display connection type (SPI) of \c hd44780 driver for
 * Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 4 bit-mode to be connected to the 8 bit Port B
 * of a single MCP23S17 that is accessed by the server via the SPI bus.
 */

/*-
 * Copyright (c) 2013 Thomas Preston <thomas.preston@openlx.org.uk>
 *
 * Based mostly on the hd44780-i2c module, see there for a complete history.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/*-
 * The connections are (MCP23S17):
 * Port A Pin 0     Switch 0 (Left)
 * Port A Pin 1     Switch 1 (Down)
 * Port A Pin 2     Switch 2 (Up)
 * Port A Pin 3     Switch 3 (Right)
 * Port A Pin 4     Switch 4 (Enter)
 * Port A Pin 5     Switch 5 (Nav push)
 * Port A Pin 6     Switch 6 (Nav left)
 * Port A Pin 6     Switch 7 (Nav right)
 * Port B Pin 0     D4
 * Port B Pin 1     D5
 * Port B Pin 2     D6
 * Port B Pin 3     D7
 * Port B Pin 4     EN
 * Port B Pin 5     RW
 * Port B Pin 6     RS
 *
 * Backlight
 * Port B Pin 7   Backlight (optional, active-high)
 *
 * Configuration:
 * device=/dev/spidev0.1   # the device file of the SPI bus
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "hd44780-pifacecad.h"
#include "hd44780-low.h"
#include "report.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

void pifacecad_HD44780_senddata(PrivateData *p,
				unsigned char displayID, unsigned char flags, unsigned char ch);
void pifacecad_HD44780_backlight(PrivateData *p, unsigned char state);
void pifacecad_HD44780_close(PrivateData *p);
unsigned char pifacecad_HD44780_scankeypad(PrivateData *p);

/** \name SPI bus configuration
 *@{*/
#define DEFAULT_DEVICE "/dev/spidev0.1"
#define SPI_HW_ADDR 0		/**< Hardware address set by pins A0:2 */

const unsigned char spi_mode = 0;	/**< Set SPI mode to CPOL = 0 (leading edge is rising)
					 * and CPHA = 0 (sample on leading edge) */
const unsigned char spi_bpw = 8;	/**< Set SPI to 8 bits per word */
const unsigned long spi_speed = 10000000L;	/**< Set SPI speed to 10MHz */
const unsigned char spi_delay = 0;
/**@}*/

/** \name MCP23S17 commands
 *@{*/
#define WRITE_CMD 0
#define READ_CMD 1
/**@}*/

/** \name MCP23S17 Register addresses
 *@{*/
#define IODIRA		0x00	/**< I/O direction A */
#define IODIRB		0x01	/**< I/O direction B */
#define IPOLA		0x02	/**< Input polarity A */
#define IPOLB		0x03	/**< Input polarity B */
#define IOCON		0x0A	/**< I/O config (also 0x0B) */
#define GPPUA		0x0C	/**< Port A pull-ups when input */
#define GPPUB		0x0D	/**< Port B pull-ups when input */
#define GPIOA		0x12	/**< Port A */
#define GPIOB		0x13	/**< Port B */
/**@}*/

/* Ports */
#define SWITCH_PORT	GPIOA	/**< Switches are connected to port A */
#define LCD_PORT	GPIOB	/**< LCD and backlight are connected to port B */

/* LCD timing delays */
#define DELAY_PULSE_US 1	/**< 1us hold time for EN */
#define DELAY_SETTLE_US 40	/**< 40us delay for executing LCD command */

/** \name LCD control pins
 * LCD control pins are connected to upper nibble of Port B on the MCP23S17
 *@{*/
#define EN 0x10
#define RW 0x20
#define RS 0x40
#define BL 0x80
/**@}*/

/**
 * Writes to a register on the MCP23S17.
 *
 * \param p     Pointer to PrivateData structure.
 * \param reg   Register address to write to.
 * \param data  Data to write
 */
static void
mcp23s17_write_reg(PrivateData *p, unsigned char reg, unsigned char data)
{
	unsigned char tx_buf[3] = { 0x40 | SPI_HW_ADDR | WRITE_CMD, reg, data };
	unsigned char rx_buf[3];

	struct spi_ioc_transfer spi;
	spi.tx_buf = (unsigned long) tx_buf;
	spi.rx_buf = (unsigned long) rx_buf;
	spi.len = sizeof(tx_buf);
	spi.delay_usecs = spi_delay;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bpw;

	/* do the SPI transaction */
	if (ioctl(p->fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
		p->hd44780_functions->drv_report(RPT_ERR,
						 "HD44780: PiFaceCAD: mcp23s17_write_reg: There was "
						 "a error during the SPI transaction: %s",
						 strerror(errno));
	}
}


/**
 * Reads from a register on the MCP23S17.
 *
 * \param p     Pointer to PrivateData structure.
 * \param reg   Register address to read from.
 * \retval      Value of the register. Returns zero (0) on error, too.
 */
static unsigned char
mcp23s17_read_reg(PrivateData *p, unsigned char reg)
{
	unsigned char tx_buf[3] = { 0x40 | SPI_HW_ADDR | READ_CMD, reg, 0 };
	unsigned char rx_buf[3] = { 0, 0, 0 };

	struct spi_ioc_transfer spi;
	spi.tx_buf = (unsigned long) tx_buf;
	spi.rx_buf = (unsigned long) rx_buf;
	spi.len = sizeof(tx_buf);
	spi.delay_usecs = spi_delay;
	spi.speed_hz = spi_speed;
	spi.bits_per_word = spi_bpw;

	/* do the SPI transaction */
	if (ioctl(p->fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
		p->hd44780_functions->drv_report(RPT_ERR,
						 "HD44780: PiFaceCAD: mcp23s17_read_reg: There was"
						 "a error during the SPI transaction: %s",
						 strerror(errno));
		return 0;
	}
	return rx_buf[2];
}

/**
 * Writes control and data to HD44780 through Port B on MCP23S17.
 *
 * \param p     Pointer to the PrivateData structure.
 * \param data  The data to write.
 */
static void
write_and_pulse(PrivateData *p, unsigned char data)
{
	/* Enable: Off/On/Off - Doesn't seem to work any other way. */
	mcp23s17_write_reg(p, LCD_PORT, data);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, DELAY_PULSE_US);
	mcp23s17_write_reg(p, LCD_PORT, data | EN);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, DELAY_PULSE_US);
	mcp23s17_write_reg(p, LCD_PORT, data);

	p->hd44780_functions->uPause(p, DELAY_SETTLE_US);
}


/**
 * Initialize the driver.
 *
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_pifacecad(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;
	char device[256] = DEFAULT_DEVICE;
	p->backlight_bit = BL;		/* Backlight on during init */

	/* READ CONFIG FILE */
	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "HD44780: PiFaceCAD: Using device '%s'", device);

	/* Open the SPI device */
	if ((p->fd = open(device, O_RDWR)) < 0) {
		report(RPT_ERR,
		       "HD44780: PiFaceCAD: open SPI device '%s' failed: %s",
		       device, strerror(errno));
		return -1;
	}

	/* configure SPI device */
	if (ioctl(p->fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
		report(RPT_ERR, "HD44780: PiFaceCAD: Could not set SPI mode.");
		return -1;
	}
	if (ioctl(p->fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0) {
		report(RPT_ERR, "HD44780: PiFaceCAD Could not set SPI bits per word.");
		return -1;
	}
	if (ioctl(p->fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
		report(RPT_ERR, "HD44780: PiFaceCAD: Could not set SPI speed.");
		return -1;
	}

	/* Set IO config */
	mcp23s17_write_reg(p, IOCON, 0x08);	/* Hardware Address Enable */
	mcp23s17_write_reg(p, IODIRB, 0x00);	/* Set GPIOB (LCD port) to output */
	mcp23s17_write_reg(p, IODIRA, 0xff);	/* Set GPIOA (switches) to input */
	mcp23s17_write_reg(p, GPPUA, 0xff);	/* enable pull-ups on input */
	mcp23s17_write_reg(p, IPOLA, 0xff);	/* invert inputs */

	hd44780_functions->senddata = pifacecad_HD44780_senddata;
	hd44780_functions->backlight = pifacecad_HD44780_backlight;
	hd44780_functions->close = pifacecad_HD44780_close;
	hd44780_functions->scankeypad = pifacecad_HD44780_scankeypad;

	/* Send 0x03 a couple of times, which is ((FUNCSET | IF_8BIT) >> 4) */
	write_and_pulse(p, 0x03);
	hd44780_functions->uPause(p, 15000);

	write_and_pulse(p, 0x03);
	hd44780_functions->uPause(p, 5000);

	write_and_pulse(p, 0x03);
	hd44780_functions->uPause(p, 1000);

	/* Now switch to IF_4BIT */
	write_and_pulse(p, 0x02);
	hd44780_functions->uPause(p, DELAY_SETTLE_US);

	common_init(p, IF_4BIT);

	report(RPT_INFO, "HD44780: PiFaceCAD: initialized");

	return 0;
}


/**
 * Closes the file descriptor of the HD44780 (SPI bus to MCP23S17).
 *
 * \param p  Pointer to driver's private data structure.
 */
void
pifacecad_HD44780_close(PrivateData *p)
{
	if (p->fd >= 0) {
		close(p->fd);
	}
}


/**
 * Send data or commands to the display.
 *
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
pifacecad_HD44780_senddata(PrivateData *p,
			   unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char portControl = 0;
	unsigned char h = (ch >> 4) & 0x0f;	/* high nibble */
	unsigned char l = ch & 0x0f;	/* low nibble */

	if (flags == RS_INSTR)
		portControl = 0;
	else
		portControl = RS;

	portControl |= p->backlight_bit;

	write_and_pulse(p, portControl | h);
	write_and_pulse(p, portControl | l);
}


/**
 * Turn display backlight on or off.
 *
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
pifacecad_HD44780_backlight(PrivateData *p, unsigned char state)
{
	unsigned char port_state = mcp23s17_read_reg(p, LCD_PORT);
	p->backlight_bit = (state == BACKLIGHT_ON) ? BL : 0;
	if (p->backlight_bit)
		port_state |= BL;	/* set */
	else
		port_state &= 0xff ^ BL;	/* clear */
	mcp23s17_write_reg(p, LCD_PORT, port_state);
}


/**
 * Scan the LCD keys. Each button is connected to a pin on Port A. As the
 * hd44780 driver only supports 5 direct keys return the pressed key as a
 * matrix key code instead.
 *
 * \param p      Pointer to driver's private data structure.
 * \retval key   Key code as matrix code (1,1) to (1,8).
 */
unsigned char
pifacecad_HD44780_scankeypad(PrivateData *p)
{
	unsigned char keycode = 0;
	unsigned char switch_state;
	int i;

	if ((switch_state = mcp23s17_read_reg(p, SWITCH_PORT)) == 0)
		return 0;

	/* detect which key is pressed (which bit is set) */
	for (i = 0; i < 8; i++) {
		if (switch_state & (1 << i)) {
			/* convert to matrix code */
			keycode = ((i + 1) << 4 | 1);
			break;
		}
	}

	return keycode;
}
