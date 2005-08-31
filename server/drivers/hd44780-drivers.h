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
#include "hd44780-picanlcd.h"
#include "hd44780-lcdserializer.h"
#include "hd44780-lis2.h"
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
	{"picanlcd", hd_init_picanlcd, "\tnone\n"},
	{"lcdserializer", hd_init_lcdserializer, "\tnone\n"},
	{"lis2", hd_init_lis2, "\tnone\n"},
#ifdef HAVE_LIBUSB
	{"bwctusb", hd_init_bwct_usb, "\tnone\n"},
#endif
		 // add new connection types and their string specifier here
		 // default, end of structure element (do not delete)
	{NULL, NULL, NULL}
};

#endif
