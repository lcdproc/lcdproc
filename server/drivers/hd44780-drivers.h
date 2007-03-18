/*
 * Low-level driver types, headers and names.
 *
 * To add support for a new driver in this file:
 *  1. include your header file
 *  2. Add a new connectionType
 *  3. Add an entry in the connectionMapping structure
 */

#ifndef HD44780_DRIVERS_H
#define HD44780_DRIVERS_H

// hd44780 specific header files
#ifdef HAVE_PCSTYLE_LPT_CONTROL
# include "hd44780-4bit.h"
# include "hd44780-ext8bit.h"
# include "hd44780-serialLpt.h"
# include "hd44780-winamp.h"
#endif
#ifdef HAVE_LIBUSB
# include "hd44780-bwct-usb.h"
#endif
#include "hd44780-serial.h"
#include "hd44780-lis2.h"
#ifdef HAVE_I2C
# include "hd44780-i2c.h"
#endif
// add new connection type header files here

static const ConnectionMapping connectionMapping[] = {
	// connectionType enumerator
	// string to identify connection on command line
	// your initialisation function
	// help string for your particular connection
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	{"4bit", hd_init_4bit, "\tnone\n"},
	{"8bit", hd_init_ext8bit, "\tnone\n"},
	{"serialLpt", hd_init_serialLpt, "\tnone\n"},
	{"winamp", hd_init_winamp, "\tnone\n"},
#endif
	/* Serial connectiontypes */
	{"picanlcd", hd_init_serial, "\tnone\n"},
	{"lcdserializer", hd_init_serial, "\tnone\n"},
	{"los-panel", hd_init_serial, "\tnone\n"},
	{"vdr-lcd", hd_init_serial, "\tnone\n"},
	{"vdr-wakeup", hd_init_serial, "\tnone\n"},
	{"pertelian", hd_init_serial, "\tnone\n"},
	/* End serial connectiontypes */
	{"lis2", hd_init_lis2, "\tnone\n"},
#ifdef HAVE_LIBUSB
	{"bwctusb", hd_init_bwct_usb, "\tnone\n"},
#endif
#ifdef HAVE_I2C
	{"i2c", hd_init_i2c, "\tnone\n"},
#endif
		 // add new connection types and their string specifier here
		 // default, end of structure element (do not delete)
	{NULL, NULL, NULL}
};

#endif
