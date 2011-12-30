/** \file server/drivers/lis.c
 * LCDd \c lis driver for the L.I.S MCE VFD by vlsys.co.kr. This is a 20x2 VFD
 * using NEC UPD16314 driver and FTDI FT232RQ USB-to-serial converter.
 *
 * \note  This driver uses the libftdi library to interface to the FTDI chip.
 *        Make sure kernel module ftdi_sio doesn't claim the device.
 */

/*-
 * Copyright (c) 2007, Daryl Fonseca-Holt <wyatt@prairieturtle.ca>
 *
 * Based on:
 * ula200 driver, Copyright (C) 2006, Bernhard Walle
 * IOWarrior driver, Copyright(C) 2004-2006 Peter Marschall <peter@adpm.de>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

/*-
 * Notes on this driver
 *
 * vlsys is unwilling to provide information about the protocol for this
 * device. This driver was developed entirely by analyzing the USB data
 * flowing between the host and the device on a WinXP machine. As such it
 * probably isn't an optimal driver but it works.
 *
 * Using a dirty buffer/clean buffer approach was useless since I could
 * not determine how to write to specific columns. The only thing selectable
 * was which line would be written at column 1.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include <usb.h>
#include <ftdi.h>

#include "lcd.h"
#include "lis.h"
#include "report.h"
#include "lcd_lib.h"
#include "adv_bignum.h"
#include "timing.h"

//void lis_test(Driver *drvthis);

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "lis_";


/**
 * UPD16314 table. Charactar mapping for UPD16314 device by Daryl Fonseca-Holt
 * <wyatt@prairieturtle.ca> taken from Mark Haemmerling's HD44780 table.
 * Character mapping for HD44780 devices by Mark Haemmerling <mail@markh.de>.
 *
 * Translates ISO 8859-1 to any HD44780 charset.
 */
const unsigned char UPD16314_charmap[] = {
	/* #0 */
	 0,   1,   2,   3,   4,   5,   6,   7,
	 8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	/* #32 */
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	/* #64 */
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  47,  93,  94,  95,
	/* #96 */
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	/* #128 */
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	/* #160 */
	160, 33,  236, 237, 164,  92, 124, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	/* #192 */
	192, 193, 194, 195, 196, 197, 198, 199,
	200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 223, 224,
	/* #224 */
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 237, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 1
};

/**
 * Setup standard custom chars. By default, the full block character is placed
 * at 0x01 as the UPD16314 has none.
 */
static void
lis_standard_custom_chars (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	static unsigned char checkbox_gray[] =
		{ b_______,
		  b_______,
		  b__XXXXX,
		  b__X_X_X,
		  b__XX_XX,
		  b__X_X_X,
		  b__XXXXX,
		  b_______ };

	static unsigned char block_filled[] =
		{ b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX,
		  b__XXXXX };

	lis_set_char(drvthis, 1, block_filled);
	lis_set_char(drvthis, 2, checkbox_gray);
	p->ccmode = standard;
}

/**
 * Write a command to the display.
 *
 * \param drvthis  Pointer to Driver
 * \param data     Data bytes
 * \param length   The number of bytes in data which are valid
 * \return 0 on success, negative value on error
 */
static int
lis_ftdi_write_command(Driver *drvthis, unsigned char *data, int length)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int err;

	err = ftdi_write_data(&p->ftdic, data, length);
	if (err < 0) {
		report(RPT_WARNING, "%s: ftdi_write_data failed with %d", drvthis->name, err);
		return -1;
	}

	timing_uPause(16000);

	return 0;
}

/**
 * Displays a string at line n (n typically 1 or 2), starting in the first
 * column.
 *
 * \param drvthis  Pointer to Driver
 * \param line     The line number (counting from 1)
 * \param string   Pointer to data bytes
 * \param len      The number of bytes in string which are valid
 * \return 0 on success, negative value on error
 */
static int
lis_ftdi_line_to_display(Driver *drvthis, int line, unsigned char *string, int len)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char buffer[128];
	int err;
	int i;

	if (len > p->width || line < 1 || line > p->height) {
		return -EINVAL;
	}
	buffer[0] = 0xA0 + line;
	buffer[1] = 0;
	buffer[2] = 0xA7;

	for (i=0; i < len; i++) {
		buffer[i+3] = UPD16314_charmap[(unsigned char)string[i]];
	}
	buffer[i+3] = 0x00;

	err = lis_ftdi_write_command(drvthis, buffer, len+4);
	if (err < 0) {
		report(RPT_WARNING, "%s: lis_ftdi_string: "
			"lis_ftdi_write_command() failed", drvthis->name);
	}

	return err;
}


/**
 * API: Flush the framebuffer to the display.
 */
MODULE_EXPORT void
lis_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	int line, err, i, count;
	unsigned char buffer[65];

	// see if any custom characters changed
	for (i = 0, count = 0; i < NUM_CCs; i++) {
		if ( ! p->cc[i].clean ) {
			count++;
			p->cc[i].clean++;   // mark clean
		}
	}
	if (count) {
		// flush custom characters to device
		buffer[0] = 0xAD;		// command to write CGRAM at 0
		for (i = 0; i < NUM_CCs; i++) {
			memcpy(buffer + 1 + (i*CELLHEIGHT), p->cc[i].cache, CELLHEIGHT);
		}
		err = lis_ftdi_write_command(drvthis, buffer, sizeof(buffer));
		if (err < 0) {
			report(RPT_WARNING, "%s: lis_flush(): "
				"lis_ftdi_write_command() failed", drvthis->name);
		}
		report(RPT_DEBUG, "Flushed %d custom chars that changed", count);
		timing_uPause(16000);
	}

	// write any line that has a change in it
	for (line = 0; line < p->height; line++) {
		if (p->line_flags[line]) {
			report(RPT_DEBUG, "Flushing line %d", line+1);
			lis_ftdi_line_to_display(drvthis, line+1, p->framebuf + (line * p->width), p->width);
			p->line_flags[line] = 0;	// clean
			timing_uPause(16000);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Separate thread to keep a read up on the USB device at all times
int
lis_read_thread(void *arg)
{
	Driver *drvthis;
	PrivateData *p;
	char unsigned buffer[64];
	int size;

	drvthis = (Driver *)arg;
	p = (PrivateData *) drvthis->private_data;

	while(! p->child_flag) {
		for (size = ftdi_read_data(&p->ftdic, buffer, 64); size > 0; size = ftdi_read_data(&p->ftdic, buffer, 64))
			;
		if (size < 0) {
			p->parent_flag = 0;
			return size;
		}
	}
	p->parent_flag = 0;
	return 0;
}

/**
 * API: Control the display brightness
 */
MODULE_EXPORT int
lis_set_brightness(Driver *drvthis, int state, int promille)
{
	unsigned char buffer[2];
	int err;
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (promille < 0 || promille > 1000) {
		report(RPT_WARNING, "%s: invalid brightness %d less then 0 or greater than 1000",
		       drvthis->name, promille);
		return -EINVAL;
	}

	buffer[0] = 0xA5;
	if (promille < 251)
		buffer[1] = 0x3;/* 25% */
	else if (promille < 501)
		buffer[1] = 0x2;/* 50% */
	else if (promille < 751)
		buffer[1] = 0x1;/* 75% */
	else
		buffer[1] = 0x0;/* 100% */

	err = ftdi_write_data(&p->ftdic, buffer, 2);
	if (err < 0) {
		report(RPT_WARNING, "%s: lis_set_brightness(): ftdi_write_data failed with %d",
		       drvthis->name, err);
		return err;
	}
	else
		p->brightness = promille;

	report(RPT_DEBUG, "%s: brightness set to %d", drvthis->name, promille);

	return 0;
}

/***
 * API: Get the brightness setting
 */
MODULE_EXPORT int
lis_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	report(RPT_DEBUG, "%s: brightness query returns %d",
		drvthis->name, p->brightness);

	return p->brightness;
}

/**
 * API: Init the driver and display
 */
MODULE_EXPORT int
lis_init(Driver *drvthis)
{
	PrivateData *p;
	int err;
	const char *s;
	unsigned char buffer[64];
	int count;
	pthread_t read_thread;

	report(RPT_DEBUG, "%s: Initializing driver",
		drvthis->name);
	// Alocate and store private data
	p = (PrivateData *) malloc( sizeof( PrivateData) );
	if (p == NULL) {
		return -1;
	}
	if (drvthis->store_private_ptr(drvthis, p)) {
		return -1;
	}

	p->child_flag = 0;
	p->parent_flag = 0;
	p->cellwidth = CELLWIDTH;
	p->cellheight = CELLHEIGHT;

	// Get and parse size
	s = drvthis->config_get_string( drvthis->name, "size", 0, "20x2");
	if ((sscanf(s, "%dx%d", &(p->width), &(p->height)) != 2)
		|| (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
		|| (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s: cannot read Size %s", drvthis->name, s);
		return -1;
	}

	// set up the dirty flags
	if( ! (p->line_flags = (unsigned int *)malloc(sizeof(int) * p->height))) {
		report(RPT_ERR, "%s: cannot allocate line flags memory", drvthis->name);
		return -1;
	}
	for (count = 0; count < p->height; count++)
		p->line_flags[count] = 1;	/* dirty! */

	/* Which display brightness */
	count = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	if ((count < 0) || (count > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		count = DEFAULT_BRIGHTNESS;
	}
	p->brightness = count;

	p->VendorID = drvthis->config_get_int(drvthis->name, "VendorID", 0, DISPLAY_VENDOR_ID);
	p->ProductID = drvthis->config_get_int(drvthis->name, "ProductID", 0, DISPLAY_PRODUCT_ID);

	p->lastline = drvthis->config_get_bool(drvthis->name, "lastline", 0, 1);
	/* End of config file parsing */


	// initialise delay timing
	if (timing_init()) {
		report(RPT_ERR, "%s: cannot initialise delay timing", drvthis->name);
		return -1;
	}
	// Allocate framebuffer
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		goto err_begin;
	}

	// open the FTDI library
	ftdi_init(&p->ftdic);
//	(&p->ftdic)->usb_write_timeout = 20;
//	(&p->ftdic)->usb_read_timeout = 20;

	// open the device
	err = ftdi_usb_open(&p->ftdic, p->VendorID, p->ProductID);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot open USB device %x:%x",
			drvthis->name,
			p->VendorID,
			p->ProductID);
		goto err_framebuf;
	}

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, EVEN);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	for (count = 0; count < 6; count++) {
		// reset device
		err = ftdi_usb_reset(&p->ftdic);
		if (err < 0) {
			report(RPT_ERR, "%s: cannot reset device", drvthis->name);
			goto err_ftdi;
		}
	}
	// set latency timer
	err = ftdi_set_latency_timer(&p->ftdic, 16);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set latency timer", drvthis->name);
		goto err_ftdi;
	}

	// create a thread to keep a read up on the device
	err = pthread_create(	&read_thread,
				NULL,
				(void *) lis_read_thread,
				drvthis
				);
	if (err) {
		report(RPT_ERR, "%s: pthread_create() - %s", drvthis->name, strerror(err));
		goto err_framebuf;
	}
	report(RPT_INFO, "%s: read thread created");
	p->parent_flag = 1;		// show we're now a happy parent, birth successful.

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, EVEN);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(236000);

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(236000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(192000);

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(236000);

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(236000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(13*16000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(13*16000);

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(15*16000);

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(13*16000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(16*16000);

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(16000);

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	for (count = 0; count < 6; count++) {
		// reset device
		err = ftdi_usb_reset(&p->ftdic);
		if (err < 0) {
			report(RPT_ERR, "%s: cannot reset device", drvthis->name);
			goto err_ftdi;
		}
	}

	// set latency timer
	err = ftdi_set_latency_timer(&p->ftdic, 16);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set latency timer", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(16000);

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_7, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 1200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 38400);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters N,8,1
	err = ftdi_set_line_property(&p->ftdic, BITS_8, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties N,8,1", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// set DTR high
	err = ftdi_setrts(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

#if 0
	// uncomment this if using a patched version of ftdi
	// set the event character to 0x00
	err = ftdi_set_event_char(&p->ftdic, 0x100);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set event character ", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(16000);
#endif

	// don't know what this does exactly
	buffer[0] = 0xA4;
	buffer[1] = 0x7d;
	err = ftdi_write_data(&p->ftdic, buffer, 2);
	if (err < 0) {
		report(RPT_WARNING, "%s: ftdi_write_data failed with %d", drvthis->name, err);
		goto err_ftdi;
	}

	timing_uPause(3*16000);

	buffer[0] = 0xAA;
	buffer[1] = 0xAA;
	for(count = 0; count < 10; count++) {
		err = ftdi_write_data(&p->ftdic, buffer, 2);
		if (err < 0) {
			report(RPT_WARNING, "%s: ftdi_write_data failed with %d", drvthis->name, err);
			goto err_ftdi;
		}
		timing_uPause(3*16000);
	}

	// set the event character to 0x00 (only works with patched libftdi)
//	err = ftdi_set_event_char(&p->ftdic, 0x00);
//	if (err < 0) {
//		report(RPT_ERR, "%s: cannot set event character ", drvthis->name);
//		goto err_ftdi;
//	}
//
//	timing_uPause(16000);

	// set DTR high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR high", drvthis->name);
		goto err_ftdi;
	}

	// set RTS high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS high", drvthis->name);
		goto err_ftdi;
	}

	for (count = 0; count < 6; count++) {
		// reset device
		err = ftdi_usb_reset(&p->ftdic);
		if (err < 0) {
			report(RPT_ERR, "%s: cannot reset device", drvthis->name);
			goto err_ftdi;
		}
	}

	// set latency timer
	err = ftdi_set_latency_timer(&p->ftdic, 16);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set latency timer", drvthis->name);
		goto err_ftdi;
	}

	timing_uPause(16000);

	// set communication parameters N,8,1
	err = ftdi_set_line_property(&p->ftdic, BITS_8, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties N,8,1", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 38400);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate 38400", drvthis->name);
		goto err_ftdi;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 38400);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate 38400", drvthis->name);
		goto err_ftdi;
	}

	// set RTS mask and high
	err = ftdi_setrts(&p->ftdic, SIO_SET_RTS_MASK | SIO_SET_RTS_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set RTS mask", drvthis->name);
		goto err_ftdi;
	}

	// set DTR mask and high
	err = ftdi_setdtr(&p->ftdic, SIO_SET_DTR_MASK | SIO_SET_DTR_HIGH);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set DTR mask", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters N,8,1
	err = ftdi_set_line_property(&p->ftdic, BITS_8, STOP_BIT_1, NONE);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties N,8,1", drvthis->name);
		goto err_ftdi;
	}

	// disable flow control
	err = ftdi_setflowctrl(&p->ftdic, SIO_DISABLE_FLOW_CTRL);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot disable flow control", drvthis->name);
		goto err_ftdi;
	}

	// turn off the event character (only works with patched libftdi)
//	err = ftdi_set_event_char(&p->ftdic, 0x00);
//	if (err < 0) {
//		report(RPT_ERR, "%s: cannot turn off event character ", drvthis->name);
//		goto err_ftdi;
//	}
//
//	timing_uPause(2*16000);

	// not sure about this
	buffer[0] = 0xA0;
	err = lis_ftdi_write_command(drvthis, buffer, 1);
	if (err < 0) {
		report(RPT_WARNING, "%s: lis_ftdi_clear: "
			"lis_ftdi_write_command failed", drvthis->name);
	}

	timing_uPause(10*16000);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

//	lis_test(drvthis);	 /*DEBUG*/

	return 0;

err_ftdi:
	ftdi_usb_close(&p->ftdic);
	ftdi_deinit(&p->ftdic);
	if(p->line_flags)
		free(p->line_flags);
err_framebuf:
	free(p->framebuf);
err_begin:

	if (p->parent_flag) {			// terminate the child
		p->child_flag = 1;
		while(p->parent_flag)
			timing_uPause(5*16000);
	}
	return -1;
}

/**
 * API: Clean-up
 */
MODULE_EXPORT void
lis_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	report(RPT_DEBUG, "%s: closing driver", drvthis->name);
	if (p != NULL) {
		if (p->parent_flag) {	/* terminate the child */
			p->child_flag = 1;
			while(p->parent_flag)
				timing_uPause(5*16000);
		}
		ftdi_usb_purge_buffers(&p->ftdic);
		ftdi_usb_close(&p->ftdic);
		ftdi_deinit(&p->ftdic);

		if (p->framebuf != NULL)
			free(p->framebuf);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

/**
 * API: Returns the display width
 */
MODULE_EXPORT int
lis_width (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->width;
}

/**
 * API: Returns the display height
 */
MODULE_EXPORT int
lis_height(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->height;
}

/**
 * API: Clear the framebuffer
 */
MODULE_EXPORT void
lis_clear(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int line;

	report(RPT_DEBUG, "%s: Clearing display", drvthis->name);
	for (line = 0; line < p->height; line++) {
		memset(p->framebuf + (line * p->width), ' ', p->width);
		p->line_flags[line] = 1;
	}
	lis_standard_custom_chars(drvthis);
}

/**
 * API: Place a character in the framebuffer
 */
MODULE_EXPORT void
lis_chr(Driver *drvthis, int x, int y, unsigned char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	/* ignore out-of-range */
	if (y > p->height || x > p->width) {
		report(RPT_WARNING, "%s: Writing char %x at %d,%d"
		       " ignored out of range %d,%d",
		       drvthis->name, ch, x, y, p->height, p->width);
		return;
	}

	y--;
	x--;

	if (p->framebuf[(y * p->width) + x] != ch) {
		p->framebuf[(y * p->width) + x] = ch;

		p->line_flags[y] = 1;
		report(RPT_DEBUG, "%s: Caching char %x at %d,%d",
		       drvthis->name, ch, x, y);
	}
}

/**
 * API: Place a string in the framebuffer
 */
MODULE_EXPORT void
lis_string(Driver *drvthis, int x, int y, char *s)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x--;			/* Convert 1-based coords to 0-based */
	y--;

	report(RPT_DEBUG, "%s: Write string to framebuffer  %d,%d \"%s\"",
	       drvthis->name, x, y, s);

	for (i = 0; s[i]; i++) {
		/* Check for buffer overflows... */
		if ((y * p->width) + x + i > (p->width * p->height)) {
			report(RPT_WARNING, "%s: Writing string ignored, out of range",
			       drvthis->name, x, y);
			break;
		}

		if (p->framebuf[(y * p->width) + x + i] != s[i]) {
			p->framebuf[(y * p->width) + x + i] = s[i];
			p->line_flags[((y * p->width) + x + i) / p->width] = 1;	/* dirty */
		}
	}
}


/**
 * API: Place an icon on the screen. The uPD16314 comes with a lot of icons
 * already in CGROM, so use them.
 */
MODULE_EXPORT int
lis_icon(Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char ch;

	switch (icon) {
		case ICON_BLOCK_FILLED:
			ch = 0xff;
			break;
		case ICON_HEART_FILLED:
			ch = 0x9D;
			break;
		case ICON_HEART_OPEN:
			ch = 0x9C;
			break;
		case ICON_ARROW_UP:
			ch = 0x18;
			break;
		case ICON_ARROW_DOWN:
			ch = 0x19;
			break;
		case ICON_ARROW_LEFT:
			ch = 0x1B;
			break;
		case ICON_ARROW_RIGHT:
			ch = 0x1A;
			break;
		case ICON_CHECKBOX_OFF:
			ch = 0x6F;
			break;
		case ICON_CHECKBOX_ON:
			ch = 0xC7;
			break;
		case ICON_CHECKBOX_GRAY:
			ch = 2;
			if (p->ccmode != standard)
				lis_standard_custom_chars(drvthis);
			break;
		case ICON_STOP:
			ch = 0x16;
			break;
		case ICON_PAUSE:
			ch = 0xA0;
			break;
		case ICON_PLAY:
			ch = 0x10;
			break;
		case ICON_PLAYR:
			ch = 0x11;
			break;
		case ICON_FF:
			ch = 0xBB;
			break;
		case ICON_FR:
			ch = 0xBC;
			break;
		case ICON_NEXT:
			ch = 0x1D;
			break;
		case ICON_PREV:
			ch = 0x1C;
			break;
		case ICON_REC:
			ch = 0xAE;
			break;
		default:
			return -1; /* Let the core do other icons */

	}
	report(RPT_DEBUG, "%s: Writing icon #%d (%x) @ %d,%d",
 		drvthis->name, icon, ch, x, y);
	lis_chr(drvthis, x, y, ch);

	return 0;
}


#if 0
void
lis_test(Driver *drvthis)
{
	int	x, y, i;


	x = 0;
	y = 1;
	lis_clear(drvthis);

	for (i = 0; i < 1001; i++) {
		lis_vbar(drvthis, 1, 2, 2, i, 0);
		lis_flush(drvthis);
	}
}
#endif


/**
 * Define a custom character and write it to the LCD.
 * \param drvthis  Pointer to driver structure.
 * \param n        Custom character to define [0 - (NUM_CCs-1)].
 * \param dat      Array of 8(=cellheight) bytes, each representing a pixel row
 *                 starting from the top to bottom.
 *                 The bits in each byte represent the pixels where the LSB
 *                 (least significant bit) is the rightmost pixel in each pixel row.
 */
MODULE_EXPORT void
lis_set_char(Driver *drvthis, int n, unsigned char *dat)
{
	PrivateData *p = drvthis->private_data;
	unsigned char mask = (1 << p->cellwidth) - 1;
	int row;


	if ((n < 0) || (n >= NUM_CCs))
		return;
	if (dat == NULL)
		return;

	for (row = 0; row < p->cellheight; row++) {
		int letter = 0;

		if (p->lastline || (row < p->cellheight - 1))
			letter = dat[row] & mask;

		if (p->cc[n].cache[row] != letter) {
			p->cc[n].clean = 0;	/* only mark dirty if really
						 * different */
		}
		p->cc[n].cache[row] = letter;
	}
	report(RPT_DEBUG, "%s: cached custom character #%d", drvthis->name, n);
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
lis_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != vbar) {
		unsigned char vBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: vbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}
		p->ccmode = vbar;

		memset(vBar, 0x00, sizeof(vBar));

		for (i = 2; i <= p->cellheight; i++) {
			// add pixel line per pixel line ...
			vBar[p->cellheight - i + 1] = 0x1F;
			lis_set_char(drvthis, i, vBar);
		}
	}

	report(RPT_DEBUG, "%s: vbar @ %d,%d len %d, %d/1000",
		drvthis->name, x, y, len, promille);

	lib_vbar_static(drvthis, x, y, len, promille, options, p->cellheight, 2);
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
lis_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;

	if (p->ccmode != hbar) {
		unsigned char hBar[p->cellheight];
		int i;

		if (p->ccmode != standard) {
			/* Not supported(yet) */
			report(RPT_WARNING, "%s: hbar: cannot combine two modes using user-defined characters",
				drvthis->name);
			return;
		}

		p->ccmode = hbar;

		for (i = 1; i <= p->cellwidth; i++) {
			// fill pixel columns from left to right.
			memset(hBar, 0xFF & ~((1 << (p->cellwidth - i)) - 1), sizeof(hBar));
			lis_set_char(drvthis, i + 2, hBar);
		}
	}
	report(RPT_DEBUG, "%s: hbar @ %d,%d len %d, %d/1000",
		drvthis->name, x, y, len, promille);

	lib_hbar_static(drvthis, x, y, len, promille, options, p->cellwidth, 2);
}


/**
 * Write a big number to the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param num      Character to write (0 - 10 with 10 representing ':')
 */
MODULE_EXPORT void
lis_num(Driver *drvthis, int x, int num)
{
	PrivateData *p = drvthis->private_data;
	int do_init = 0;

	if ((num < 0) || (num > 10))
		return;

	if (p->ccmode != bignum) {
		if (p->ccmode != standard) {
			/* Not supported (yet) */
			report(RPT_WARNING, "%s: num: cannot combine two modes using user-defined characters",
			       drvthis->name);
			return;
		}

		p->ccmode = bignum;

		do_init = 1;
	}
	report(RPT_DEBUG, "%s: big number %d @ %d", drvthis->name, x, num);

	/*
	 * Lib_adv_bignum does everything needed to show the bignumbers.
	 * offset by 2 because driver uses first three custom characters
	 */
	lib_adv_bignum(drvthis, x, num, 3, do_init);
}


/**
 * Get total number of custom characters available.
 * \param drvthis  Pointer to driver structure.
 * \return  Number of custom characters.
 */
MODULE_EXPORT int
lis_get_free_chars(Driver *drvthis)
{
	return NUM_CCs - 3;	/* first three are reserved */
}

/* EOF */
