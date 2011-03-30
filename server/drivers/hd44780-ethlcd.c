/** \file server/drivers/hd44780-ethlcd.c
 * \c ethlcd connection type of \c hd44780 driver for Hitachi HD44780 based
 * LCD displays.
 *
 * ethlcd is an open source project based on Atmel's ATmega microcontroller
 * and ENC28J60 ethernet controller. The device is connected via ethernet, has
 * its own IP address and is available via TCP protocol. More info at project
 * homepage: http://manio.skyboo.net/ethlcd/
 */

/*-
 * Copyright (c) 2008  Mariusz Bialonczyk <manio@skyboo.net>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "lcd.h"
#include "hd44780-ethlcd.h"
#include "hd44780-low.h"
#include "shared/sockets.h"
#include "report.h"


void ethlcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
unsigned char ethlcd_HD44780_scankeypad(PrivateData *p);
void ethlcd_HD44780_backlight(PrivateData *p, unsigned char state);
void ethlcd_HD44780_close(PrivateData *p);

/* helper function */
static void ethlcd_send_low(PrivateData *p, unsigned char *data, int length);

/* fake pause function (pausing is handled by ethlcd device itself) */
void
ethlcd_HD44780_uPause(PrivateData *p, int usecs)
{
}

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_ethlcd(Driver *drvthis)
{
	char hostname[256];
	unsigned long flags = 0;
	struct timeval tv;

	PrivateData *p = (PrivateData *) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;

	hd44780_functions->senddata = ethlcd_HD44780_senddata;
	hd44780_functions->backlight = ethlcd_HD44780_backlight;
	hd44780_functions->scankeypad = ethlcd_HD44780_scankeypad;
	hd44780_functions->uPause = ethlcd_HD44780_uPause;
	hd44780_functions->close = ethlcd_HD44780_close;

	/* reading configuration file */
	strncpy(hostname, drvthis->config_get_string(drvthis->name, "Device", 0, "ethlcd"), sizeof(hostname));
	hostname[sizeof(hostname) - 1] = '\0';

	p->sock = sock_connect(hostname, DEFAULT_ETHLCD_PORT);
	if (p->sock < 0) {
		report(RPT_ERR, "%s[%s]: Connecting to %s:%d failed",
			drvthis->name, ETHLCD_DRV_NAME, hostname, DEFAULT_ETHLCD_PORT);
		return -1;
	}

	/* we need to have a blocking read back again: */
	if (fcntl(p->sock, F_GETFL, &flags) < 0) {
		report(RPT_ERR, "%s[%s]: Cannot obtain current flags: %s",
			drvthis->name, ETHLCD_DRV_NAME, strerror(errno));
		return -1;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(p->sock, F_SETFL, flags) < 0) {
		report(RPT_ERR, "%s[%s]: Unable to change socket to O_NONBLOCK: %s",
			drvthis->name, ETHLCD_DRV_NAME, strerror(errno));
		return -1;
	}

	/* setting timeouts */
	tv.tv_sec = ETHLCD_TIMEOUT;
	tv.tv_usec = 0;
	if (setsockopt(p->sock, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(struct timeval)) < 0) {
		report(RPT_ERR, "%s[%s]: Cannot set receive timeout: %s",
			drvthis->name, ETHLCD_DRV_NAME, strerror(errno));
		return -1;
	}
	if (setsockopt(p->sock, SOL_SOCKET, SO_SNDTIMEO, (void *)&tv, sizeof(struct timeval)) < 0) {
		report(RPT_ERR, "%s[%s]: Cannot set send timeout: %s",
			drvthis->name, ETHLCD_DRV_NAME, strerror(errno));
		return -1;
	}

	/* Set up two-line, small character (5x8) mode */
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);

	common_init(p, IF_4BIT);

	if (p->have_keypad) {
		/* Remember which input lines are stuck */
		p->stuckinputs = 0;
	}

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
ethlcd_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	static unsigned char buff[2];

	if (flags == RS_INSTR)
		buff[0] = ETHLCD_SEND_INSTR;
	else			/* RS_DATA */
		buff[0] = ETHLCD_SEND_DATA;
	buff[1] = ch;

	ethlcd_send_low(p, buff, 2);
}


/**
 * Read key press.
 * \param p  Pointer to driver's private data structure.
 * \return   Bitmap of the pressed keys.
 */
unsigned char
ethlcd_HD44780_scankeypad(PrivateData *p)
{
	unsigned char readval;
	static unsigned char buff[2];

	buff[0] = ETHLCD_GET_BUTTONS;

	ethlcd_send_low(p, buff, 1);

	/* answer should be in second byte on bits 0-6 in negative logic: */
	readval = buff[1];

	readval = ~readval & 0x3F;	/* reverse logic */
	switch (readval) {
	    case 1:
		return (0x34);	/* C */
	    case 2:
		return (0x24);	/* B */
	    case 4:
		return (0x14);	/* A */
	    case 8:
		return (0x25);	/* F */
	    case 16:
		return (0x15);	/* E */
	    case 32:
		return (0x44);	/* D */
	    default:
		return ('\0');
	}
}


/**
 * Turn the LCD backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void
ethlcd_HD44780_backlight(PrivateData *p, unsigned char state)
{
	static unsigned char buff[2];

	buff[0] = ETHLCD_SET_BACKLIGHT;

	if (state == BACKLIGHT_ON) {
		if (p->brightness >= 500)
			buff[1] = ETHLCD_BACKLIGHT_ON;
		else
			buff[1] = ETHLCD_BACKLIGHT_HALF;
	}
	else
		buff[1] = ETHLCD_BACKLIGHT_OFF;

	ethlcd_send_low(p, buff, 2);
}


/**
 * Close the driver (do necessary clean-up).
 * \param p  Pointer to driver's private data structure.
 */
void
ethlcd_HD44780_close(PrivateData *p)
{
	sock_close(p->sock);
}


/**
 * Send the data to ethlcd device.
 * \param p       Pointer to driver's private data structure.
 * \param data    Pointer to buffer with data to send.
 * \param length  Number of bytes to send.
 *
 * \todo  This functions makes LCDd exit on fatal error without any clean-up.
 *        There is currently no way to make LCDd exit cleanly in those cases.
 */
static void
ethlcd_send_low(PrivateData *p, unsigned char *data, int length)
{
	int response_len, len;
	unsigned char cmd;

	/* Send data to device */
	cmd = data[0];		/* store command byte for verification */
	len = sock_send(p->sock, data, length);
	if (len <= 0) {
		p->hd44780_functions->drv_report(RPT_CRIT, "%s: Write to socket failed: %s. Exiting",
					  ETHLCD_DRV_NAME, strerror(errno));
		exit(-1);
	}

	/* Check if this is a command with reply */
	if (cmd == ETHLCD_GET_BUTTONS)
		response_len = 2;
	else
		response_len = 1;

	/* Wait for reply */
	len = sock_recv(p->sock, data, response_len);
	if (len <= 0) {
		p->hd44780_functions->drv_report(RPT_CRIT, "%s: Read from socket failed: %s. Exiting",
					  ETHLCD_DRV_NAME, strerror(errno));
		exit(-1);
	}

	/* Check reply */
	if (data[0] != cmd) {
		p->hd44780_functions->drv_report(RPT_CRIT, "%s: Invalid device response (want 0x%02X, got 0x%02X). Exiting",
					     ETHLCD_DRV_NAME, cmd, data[0]);
		exit(-1);
	}
}
