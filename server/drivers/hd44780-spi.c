/** \file server/drivers/hd44780-spi.c
 * \c SPI connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its serial mode to be connected via the SPI bus
 * using the Linux kernel spidev interface. The LCD SPI device should be
 * declared in your board setup code.
 */

/*-
 * Copyright (C) 2013 Simon Dawson <spdawson@gmail.com>
 *
 * Based mostly on the hd44780-i2c module, see there for a complete history.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Configuration:
 * Device=/dev/spidev0.0   # the device file of the spi bus (spi 0, chipselect 0)
 * BacklightDevice=(undef)  # the device file for the backlight (1 = on, 0 = off)
 */

#include "hd44780-spi.h"
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

void spi_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void spi_HD44780_backlight(PrivateData *p, unsigned char state);

#define DEFAULT_DEVICE		"/dev/spidev0.0"

/** KS0073 5-bit sync lead-in on SPI interface */
#define SYNC	0xF8u
/** KS0073 Read Write bit: 1 = read selected data/register, 0 = write to selected data/register */
#define RW	0x04u
/** KS0073 Register Select bit: 0 = instruction register follows, 1 = data register follows */
#define RS	0x02u


/**
 * Reverses the bits of \a u8.
 * \param u8  byte of which to reverse the bits.
 * \return Value resulting from \a u8 with reversed bits.
 */
static inline uint8_t
bit_reverse8(uint8_t u8)
{
	/*
	 * See
	 * http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
	 * for how this works and. This code snippet is in the public domain.
	 */
	return (uint8_t) (((u8 * 0x0802LU & 0x22110LU) | (u8 * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16);
}


/**
 * Do a SPI transfer by sending \c length bytes of \c outbuf and read the same
 * number of bytes into \c inbuf.
 * \param p       Pointer to driver's private data structure.
 * \param outbuf  Buffer holding data to send.
 * \param inbuf   Buffer to store incoming data (or NULL).
 * \param length  Length of buffers (both).
 * \return  Status from ioctl.
 */
static int
spi_transfer(PrivateData *p, const unsigned char *outbuf, unsigned char *inbuf, unsigned length)
{
	struct spi_ioc_transfer xfer;
	int status;

	static unsigned char no_more_errormsgs = 0;

	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = (unsigned long) outbuf;
	xfer.rx_buf = (unsigned long) inbuf;
	xfer.len = length;

	p->hd44780_functions->drv_debug(RPT_DEBUG, "SPI sending %02x %02x %02x",
					outbuf[0], outbuf[1], length > 2 ? outbuf[2] : 0xFFu);
	status = ioctl(p->fd, SPI_IOC_MESSAGE(1), &xfer);
	if (status < 0) {
		p->hd44780_functions->drv_report(no_more_errormsgs ? RPT_DEBUG : RPT_ERR,
						 "HD44780: SPI: spidev write data %u failed: %s",
						 status, strerror(errno));
		no_more_errormsgs = 1;
	}
#ifdef DEBUG
	if (inbuf)
		p->hd44780_functions->drv_debug(RPT_DEBUG, "SPI returned data %02x %02x %02x",
						inbuf[0], inbuf[1], length > 2 ? inbuf[2] : 0xFFu);
#endif

	return status;
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_spi(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	char device[256] = DEFAULT_DEVICE;
	char backlight_device[256] = "";

	/* READ CONFIG FILE */

	/* Get and open SPI device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, DEFAULT_DEVICE),
		sizeof(device));
	device[sizeof(device) - 1] = '\0';
	report(RPT_INFO, "HD44780: SPI: Using device '%s'", device);

	p->fd = open(device, O_RDWR);
	if (p->fd < 0) {
		report(RPT_ERR, "HD44780: SPI: open spidev device '%s' failed: %s", device,
		       strerror(errno));
		return -1;
	}

	/* Get and open the backlight device */
	p->backlight_bit = -1;
	strncpy(backlight_device,
		drvthis->config_get_string(drvthis->name, "BacklightDevice", 0, ""),
		sizeof(backlight_device));
	backlight_device[sizeof(backlight_device) - 1] = '\0';

	if (strlen(backlight_device) > 0) {
		report(RPT_INFO, "HD44780: SPI: Using backlight_device '%s'", backlight_device);

		p->backlight_bit = open(backlight_device, O_RDWR);
		if (p->backlight_bit < 0) {
			report(RPT_ERR, "HD44780: SPI: open backlight_device '%s' failed: %s",
			       backlight_device, strerror(errno));
		}
		else {
			hd44780_functions->backlight = spi_HD44780_backlight;
		}
	}

	hd44780_functions->senddata = spi_HD44780_senddata;
	common_init(p, IF_8BIT);

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
spi_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char buf[3];
	unsigned char reverse;

	p->hd44780_functions->drv_report(RPT_DEBUG, "HD44780: SPI: sending %s %02x",
					 RS_INSTR == flags ? "CMD" : "DATA", ch);

	if (flags == RS_INSTR)
		buf[0] = SYNC;
	else
		buf[0] = SYNC | RS;

	/* KS0073 wants Least Significant Bit first, with the added twist of
	 * peculiar splitting of a byte across 4 nibbles. If we ever need to
	 * read from the device, remember to bit_reverse8() each byte (note
	 * that replies aren't split into nibbles). */
	reverse = bit_reverse8(ch);
	buf[1] = reverse & 0xF0;
	buf[2] = (reverse & 0x0F) << 4;

	spi_transfer(p, buf, NULL, sizeof(buf));
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 *
 * KS0073 in SPI mode does not have a backlight control function.  We
 * assume instead that there is some device mapped to a file which we can
 * write to.
 */
void
spi_HD44780_backlight(PrivateData *p, unsigned char state)
{
	if (p->backlight_bit != -1) {
		unsigned char byte = (state == BACKLIGHT_ON) ? '1' : '0';
		if (write(p->backlight_bit, &byte, sizeof(byte)) < 0)
			p->hd44780_functions->drv_report(RPT_ERR,
							 "HD44780: SPI: Cannot write to backlight device: %d (%s)",
							 errno, strerror(errno));
	}
}
