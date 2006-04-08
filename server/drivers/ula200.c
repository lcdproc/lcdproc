// Description:
//
// This is a driver for the ULA-200 from ELV (http://www.elv.de).  It's a small
// board which can be connected to the PC via USB interface and that allows to
// connect a HD44780-compatible display.  It also has a connection for up to 6
// input buttons.
//
// It uses libftdi (which uses libusb) for communication, so no kernel driver
// is needed. You have to take care that ftdi_sio module doesn't claim the ELV
// device. If you didn't change the ids in the driver (ftdi_sio.c), you don't
// have to matter.
//
//
// Features:
//
//  - text display is supported
//  - standard symbols (heart, checkbox) are supported
//  - backlight is supported
//  - input buttons are supported
//  - bars are NOT supported
//
//
// Configuration options (the values are the default values):
//
//  - Size=20x4
//  - KeyMap_A=Up
//  - KeyMap_B=Down
//  - KeyMap_C=Left
//  - KeyMap_D=Right
//  - KeyMap_E=Enter
//  - KeyMap_F=Escape
//
//
// Known problems:
//
// Sometimes the display hangs (the ACK response is not received) on shutdown.
// Reconnect the display in that case. The same if it hangs while starting
// up (only happens if it was not the first time lcdproc talked to the
// display).
//
// I tried several hours to fix this, I simply find the reason for this problem.
// 
//
// Implementation note:
//
// The ULA-200 talks a text protocol which allows to display text using a
// high-level language, i.e. STX 's' <len> <char0> <char1> ... ETX.  It also
// allows low-level register access to the HD44780. So in theory, it would be
// possible to write a hd44780 extension and let the hd44780 core do the rest.
// I tried this. It was slow and didn't work with user-specific characters
// (the hd44780 frequently changes this characters which seems to confuse the
// microcontroller, at least I cannot explain why it didn't work, there was
// garbare).
//
// So I wrote a own driver (this here) which uses the high-level language and
// should work for displays with all sizes. I only tested 20x4, so maybe for
// other sizes the positioning code may be adapted.
//
// As I mentioned, there were problems with frequently changing the
// user-definable characters. I also tried to implement bar code in the ula200
// driver with similar effects. I gave it up because I don't need it personally
// and it can be done later. However, standard icons are implemented. The
// user-definable characters are set in startup and are not changed. This
// works like a charm. It is not possible to use character 0 with the
// high-level language (or at least it isn't documented how to escape it).
// It could be done with hd44780 code, but I replaced the character with
// a standard character which looks good.
//
//
// Author
//
// Bernhard Walle <bernhard.walle@gmx.de>
// Feel free to contact me if there are problems with this driver.
//

/*  Copyright (C) 2006 Bernhard Walle <bernhard.walle@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include <usb.h>
#include <ftdi.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "hd44780-charmap.h"
#include "lcd.h"
#include "ula200.h"
#include "report.h"
#include "lcd_lib.h"
#include "timing.h"


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "ula200_";


///////////////////////////////////////////////////////////////////////////////
// constants
//
#define DISPLAY_VENDOR_ID    0x0403
#define DISPLAY_PRODUCT_ID   0xf06d

#define CH_STX  0x02
#define CH_ETX  0x03
#define CH_ENQ  0x05
#define CH_ACK  0x06
#define CH_NAK  0x15
#define CH_DC2  0x12
#define CH_DC3  0x13

#define MAX_KEY_MAP 6
static char *default_key_map[MAX_KEY_MAP] = { "Up", "Down", "Left", "Right", "Enter", "Escape" };

#define CELLWIDTH  5
#define CELLHEIGHT 8

/* repeat conts for responses */
#define MAX_REPEATS 20


///////////////////////////////////////////////////////////////////////////////
// Key ring implementation from CFontz633io
//

/* KeyRing management */
#define KEYRINGSIZE	16

typedef struct {
	unsigned char contents[KEYRINGSIZE];
	int head;
	int tail;
} KeyRing;



/*
 * KeyRing handling functions.
 * This separates the producer from the consumer.
 * It is just a small fifo of unsigned char.
 */

/** initialize/empty key ring by resetting its read & write pointers */
void EmptyKeyRing(KeyRing *kr)
{
	kr->head = kr->tail = 0;
}


/** add byte to key ring; return success (byte added) / failure (key ring is full) */
int AddKeyToKeyRing(KeyRing *kr, unsigned char key)
{
	if (((kr->head + 1) % KEYRINGSIZE) != (kr->tail % KEYRINGSIZE)) {
 		/* fprintf(stderr, "We add key: %d\n", key); */

	        kr->contents[kr->head % KEYRINGSIZE] = key;
  		kr->head = (kr->head + 1) % KEYRINGSIZE;
		return 1;
	}
	
	/* KeyRing overflow: do not accept extra key */
	return 0;
}


/** get byte from key ring (or '\\0' if key ring is empty) */
unsigned char GetKeyFromKeyRing(KeyRing *kr)
{
	unsigned char retval = '\0';

	kr->tail %= KEYRINGSIZE;

	if ((kr->head % KEYRINGSIZE) != kr->tail) {
		retval = kr->contents[kr->tail];
	        kr->tail = (kr->tail + 1) % KEYRINGSIZE;
	}

	/*  if (retval) fprintf(stderr, "We remove key: %d\n", retval); */
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
// for HD 44780 compatibility
//

// commands for senddata
#define RS_DATA     0x00
#define RS_INSTR    0x01

#define SETCHAR     0x40	/* Only reachable with EXTREG clear */


///////////////////////////////////////////////////////////////////////////////
// private data types
//
typedef struct {

    // the handle for the USB FTDI library
    struct ftdi_context ftdic;

    // the width and the height (in number of characters) of the library
	int width, height;

	// The framebuffer and the framebuffer for the last contents (incr. update)
	unsigned char *framebuf, *lcd_contents;

    // first time => all is dirty
    unsigned char all_dirty;

    // backlight (-1 = unset, 0 = off, 1 = on)
    int backlight;

    // the keyring
    KeyRing keyring;
    
	// the keymap
	char *key_map[MAX_KEY_MAP];
    
} PrivateData;


///////////////////////////////////////////////////////////////////////////////
// Reads a USB characters 
static inline int
ula200_ftdi_usb_read(PrivateData *p)
{
    unsigned char buffer[1];
    int err;
    
    while ((err = ftdi_read_data(&p->ftdic, buffer, 1)) == 0);

    if (err < 0) {
        return -1;
    }
    else {
        return buffer[0];
    }
}


///////////////////////////////////////////////////////////////////////////////
// Reads the response
//
// @return true if ACK was responded, false if NACk was responded
//
static bool 
ula200_ftdi_read_response(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
    bool result = false;
    bool answer_read = false;
    int ret;
    int ch;
    
    while (!answer_read)
    {
        /* wait until STX */
        while (((ret = ula200_ftdi_usb_read(p)) != CH_STX) && (ret > 0));
        if (ret < 0) {
            return false;
        }

        /* read next char */
        ch = ula200_ftdi_usb_read(p);

        switch (ch) {
            case 't':
                ch = ula200_ftdi_usb_read(p);
                AddKeyToKeyRing(&p->keyring, (unsigned char)(ch - 0x40));
                break;

            case CH_ACK:
                answer_read = true;
                result = true;
                break;

            case CH_NAK:
                answer_read = true;
                break;

            default:
                answer_read = true;
                report(RPT_INFO, "%s: read invalid answer (0x%02X)", drvthis->name, ch);
        }

        /* wait until ETX */
        while (((ret = ula200_ftdi_usb_read(p)) != CH_ETX) && (ret > 0));
        if (ret < 0) {
            return false;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Write a command to the display. Adds the STX and ETX header/trailer.
//
// @param p the private data
// @param data the data bytes
// @param length the number of bytes in @p data which are valid
// @param escape if the data should be escaped (see the User's Guide of the 
//        ULA-200)
// @return 0 on success, negative value on error
//
static int
ula200_ftdi_write_command(Driver *drvthis, unsigned char *data, int length, bool escape)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
    int i, err;
    int repeat_count = 0;
    int pos = 0;
    unsigned char buffer[1024];

    if (length > 512) {
        return -EINVAL;
    }

    /* fill the array */
    buffer[pos++] = CH_STX;
    for (i = 0;  i < length; i++) {
        if (escape) {
            if (data[i] == CH_STX) {
                buffer[pos++] = CH_ENQ;
                buffer[pos++] = CH_DC2;
            }
            else if (data[i] == CH_ETX) {
                buffer[pos++] = CH_ENQ;
                buffer[pos++] = CH_DC3;
            }
            else if (data[i] == CH_ENQ) {
                buffer[pos++] = CH_ENQ;
                buffer[pos++] = CH_NAK;
            }
            else {
                buffer[pos++] = data[i];
            }
        }
        else {
            buffer[pos++] = data[i];
        }
    }
    buffer[pos++] = CH_ETX;

    do {
        /* bytes */
        err = ftdi_write_data(&p->ftdic, buffer, pos);
        if (err < 0) {
            report(RPT_WARNING, "%s: ftdi_write_data failed", drvthis->name);
            return -1;
        }
    }
    while (!ula200_ftdi_read_response(drvthis) && (repeat_count++ < MAX_REPEATS));

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Clear the display
//
static int
ula200_ftdi_clear(Driver *drvthis)
{
	//PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned char command[1];
    int err;

    command[0] = 'l';
    err = ula200_ftdi_write_command(drvthis, command, 1, true);
    if (err < 0) {
        report(RPT_WARNING, "%s: ula200_ftdi_clear: "
                            "ula200_ftdi_write_command failed", drvthis->name);
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
// Set the position
//
static int
ula200_ftdi_position(Driver *drvthis, int x, int y)
{
    PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned char command[5];
    int err;

    if (y >= 2) {
        y -= 2;
        x += p->width;
    }
    
    command[0] = 'p';
    command[1] = x;
    command[2] = y;
    err = ula200_ftdi_write_command(drvthis, command, 3, true);
    if (err < 0) {
        report(RPT_WARNING, "%s: ula200_ftdi_position(%d,%d): "
                            "ula200_ftdi_write_command failed",
			    drvthis->name, x, y);
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
// Send raw data
//
static int
ula200_ftdi_rawdata(Driver *drvthis, unsigned char flags, unsigned char ch)
{
    //PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned char command[3];
    unsigned int err;

    command[0] = 'R';
    command[1] = flags == (RS_DATA) ? '2' : '0';
    command[2] = ch;
    err = ula200_ftdi_write_command(drvthis, command, 3, false);
    if (err < 0) {
		report(RPT_ERR, "%s: ftdi_write_command() failed", drvthis->name);
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
// Displays a string
//
static int
ula200_ftdi_string(Driver *drvthis, unsigned char *string, int len)
{
    //PrivateData *p = (PrivateData *) drvthis->private_data;
    unsigned char buffer[128];
    int err;
    int i;

    if (len > 80) {
        return -EINVAL;
    }

    buffer[0] = 's';
    buffer[1] = len;
    for (i = 0; i < len; i++) {
        buffer[i+2] = HD44780_charmap[(unsigned char)string[i]];
    }
    
    err = ula200_ftdi_write_command(drvthis, buffer, len+2, true);
    if (err < 0) {
        report(RPT_WARNING, "%s: ula200_ftdi_string: "
                            "ula200_ftdi_write_command() failed", drvthis->name);
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
// Enables the raw register access mode.
//
static int 
ula200_ftdi_enable_raw_mode(Driver *drvthis)
{
    unsigned char command[3];

    report(RPT_DEBUG, "%s: enable raw mode", drvthis->name);

    command[0] = 'R';
    command[1] = 'E';
    command[2] = '1';
    return ula200_ftdi_write_command(drvthis, command, 3, false);
}

///////////////////////////////////////////////////////////////////////////////
// Loads custom characters in the display
//
static int
ula200_load_curstom_chars(Driver *drvthis)
{
    int i, col, row;
    int err = 0;
	char custom_chars[8][CELLHEIGHT*CELLHEIGHT] = {
       { 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1 },
       { 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1 },
       { 0, 0, 1, 0, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0 },
       { 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0,
		 0, 0, 1, 0, 0,
		 0, 0, 0, 0, 0 },
       { 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 },
       { 0, 0, 1, 0, 0,
		 0, 0, 1, 0, 0,
		 1, 1, 1, 0, 1,
		 1, 0, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 0, 0, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 },
       { 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 0, 1, 0, 1,
		 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0 }};

    for (i = 0; i < 8 && err == 0; i++)
    {
        /* Tell the HD44780 we will redefine char number i */
        ula200_ftdi_rawdata(drvthis, RS_INSTR, SETCHAR | i * 8);
        if (err < 0) {
            report(RPT_WARNING, "%s: ula200_ftdi_rawdata failed", drvthis->name);
            break;
        }

        /* Send the subsequent rows */
        for (row = 0; row < CELLHEIGHT; row++) {
            int value = 0;
	    
            for (col = 0; col < CELLWIDTH; col++) {
		value <<= 1;
		value |= (custom_chars[i][(row * CELLWIDTH) + col] > 0) ? 1 : 0;
            }
            err = ula200_ftdi_rawdata(drvthis, RS_DATA, value);
            if (err < 0) {
                report(RPT_WARNING, "%s: ula200_ftdi_rawdata failed", drvthis->name);
                break;
            }
        }
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
// Init the driver and display
//
MODULE_EXPORT int
ula200_init(Driver *drvthis)
{
	PrivateData *p;
	int err, i;
	char *s;

	// Alocate and store private data
	p = (PrivateData *) malloc( sizeof( PrivateData) );
	if (p == NULL) {
		return -1;
	}
	if (drvthis->store_private_ptr(drvthis, p)) {
		return -1;
	}

	p->backlight = -1;
	p->all_dirty = 1;
	EmptyKeyRing(&p->keyring);

	// Get and parse size
	s = drvthis->config_get_string( drvthis->name, "size", 0, "20x4");
	if ((sscanf(s, "%dx%d", &(p->width), &(p->height)) != 2)
	    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
	    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
		report(RPT_ERR, "%s: cannot read Size %s", drvthis->name, s);
		return -1;
	}

	// read the keymap
	for (i = 0; i < MAX_KEY_MAP; i++) {
		char buf[40];

		// First fill with default value 
		p->key_map[i] = default_key_map[i];

		// Read config value 
		sprintf(buf, "KeyMap_%c", i+'A');
		s = drvthis->config_get_string(drvthis->name, buf, 0, NULL);

		// Was a key specified in the config file ? 
		if (s != NULL) {
			p->key_map[i] = strdup(s);
			report(RPT_INFO, "%s: Key '%c' mapped to \"%s\"",
					drvthis->name, i+'A', s );
		}
	}

	/* End of config file parsing */

    
	// Allocate framebuffer
	p->framebuf = (unsigned char *) malloc(p->width * p->height);
	if (p->framebuf == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
		goto err_begin;
	}

	// Allocate and clear the buffer for incremental updates
	p->lcd_contents = (unsigned char *) malloc(p->width * p->height);
	if (p->lcd_contents == NULL) {
		report(RPT_ERR, "%s: unable to allocate framebuffer backing store", drvthis->name);
		goto err_framebuf;
	}
	memset(p->lcd_contents, 0, p->width * p->height);

	// open the FTDI library
	ftdi_init(&p->ftdic);
	(&p->ftdic)->usb_write_timeout = 20;
	(&p->ftdic)->usb_read_timeout = 20;

	// open the device
	err = ftdi_usb_open(&p->ftdic, DISPLAY_VENDOR_ID, DISPLAY_PRODUCT_ID);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot open USB device", drvthis->name);
		goto err_lcd;
	}

	// set the baudrate
	err = ftdi_set_baudrate(&p->ftdic, 19200);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set baudrate", drvthis->name);
		goto err_ftdi;
	}

	// set communication parameters
	err = ftdi_set_line_property(&p->ftdic, BITS_8, STOP_BIT_1, EVEN);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot set line properties", drvthis->name);
		goto err_ftdi;
	}

	// user is able to write commands
	err = ula200_ftdi_enable_raw_mode(drvthis);
	if (err < 0) {
		report(RPT_ERR, "%s: unable to enable the raw mode", drvthis->name);
		goto err_ftdi;
	}

	// load the chars
	err = ula200_load_curstom_chars(drvthis);
	if (err < 0) {
		report(RPT_ERR, "%s: unable to write the custom characters", drvthis->name);
		goto err_ftdi;
	}

	report(RPT_DEBUG, "%s: init() done", drvthis->name);
        
	return 0;

err_ftdi:
	ftdi_usb_close(&p->ftdic);
	ftdi_deinit(&p->ftdic);
err_framebuf:
	free(p->framebuf);
err_lcd:
	free(p->lcd_contents);
err_begin:

	return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
ula200_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p != NULL) {
		ftdi_usb_purge_buffers(&p->ftdic);
		ftdi_usb_close(&p->ftdic);
		ftdi_deinit(&p->ftdic);

		if (p->framebuf != NULL)
        		free(p->framebuf);
    
		if (p->lcd_contents != NULL)
        		free(p->lcd_contents);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
ula200_width (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->width;
}

///////////////////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
ula200_height (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	return p->height;
}

///////////////////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
MODULE_EXPORT void
ula200_clear (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	memset(p->framebuf, ' ', p->width * p->height);
}

///////////////////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
MODULE_EXPORT void
ula200_chr (Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	y--;
	x--;

	p->framebuf[ (y * p->width) + x] = ch;
}

///////////////////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
MODULE_EXPORT void
ula200_string (Driver *drvthis, int x, int y, char *s)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x --;  // Convert 1-based coords to 0-based
	y --;

	for (i = 0; s[i]; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;
		p->framebuf[(y*p->width) + x + i] = s[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
// Sets the backlight on or off
//
MODULE_EXPORT void
ula200_backlight (Driver *drvthis, int on)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	unsigned char command[2];
	int err;

	if (p->backlight != on) {
        	p->backlight = on;

        	command[0] = 'h';
	        command[1] = (on) ? '1' : '0';
	        err = ula200_ftdi_write_command(drvthis, command, 2, false);
        	if (err < 0)
			report(RPT_WARNING, "%s: error in ula200_ftdi_write_command",
					drvthis->name);
		else
			report(RPT_INFO, "%s: turn backlight %s",
					drvthis->name, (on) ? "on" : "off");
	}
}

///////////////////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
MODULE_EXPORT void
ula200_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int x, y;
	int wid = p->width;
	char ch;
	char drawing;
	int count;
	int firstdiff;
	int lastdiff;

	if (p->all_dirty) {
		ula200_ftdi_clear(drvthis);
		p->all_dirty = 0;
	}

	// Update LCD incrementally by comparing with last contents
	count = 0;
	for (y = 0; y < p->height; y++) {
		drawing = 0;
		firstdiff = -1;
		lastdiff = 0;
		for (x = 0 ; x < wid; x++) {
			ch = p->framebuf[(y * wid) + x];
			if (ch != p->lcd_contents[(y*wid)+x]) {
                		p->lcd_contents[(y*wid)+x] = ch;
                		if (firstdiff == -1) {
                    			firstdiff = x;
				}
				lastdiff = x;
			}
		}

		if (firstdiff >= 0) {
			ula200_ftdi_position(drvthis, firstdiff, y);
			ula200_ftdi_string(drvthis, p->framebuf + (y*wid) + firstdiff, 
                               lastdiff - firstdiff + 1);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Set default icon into a userdef char
//
MODULE_EXPORT int
ula200_icon (Driver *drvthis, int x, int y, int icon)
{
	/* Yes I know, this is a VERY BAD implementation */
	switch (icon) {
		case ICON_BLOCK_FILLED:
			ula200_chr(drvthis, x, y, 0xff);
			break;
		case ICON_HEART_FILLED:
			ula200_chr(drvthis, x, y, 2);
			break;
		case ICON_HEART_OPEN:
			ula200_chr(drvthis, x, y, 1);
			break;
		case ICON_ARROW_UP:
			ula200_chr(drvthis, x, y, 3);
			break;
		case ICON_ARROW_DOWN:
			ula200_chr(drvthis, x, y, 4);
			break;
		case ICON_ARROW_LEFT:
			ula200_chr(drvthis, x, y, 0x7F);
			break;
		case ICON_ARROW_RIGHT:
			ula200_chr(drvthis, x, y, 0x7E);
			break;
		case ICON_CHECKBOX_OFF:
			ula200_chr(drvthis, x, y, 5);
			break;
		case ICON_CHECKBOX_ON:
			ula200_chr(drvthis, x, y, 6);
			break;
		case ICON_CHECKBOX_GRAY:
			ula200_chr(drvthis, x, y, 7);
			break;
		default:
			return -1; /* Let the core do other icons */
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Set default icon into a userdef char
//
MODULE_EXPORT const char *
ula200_get_key (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	unsigned char key;
	int i;

	// The libftdi has no non-blocking read (`select' system call), so we force
	// a read that could not block by updating one character on the display.
	// As long as lcdproc is single-threaded, we can write to the display because
	// we're not inside a read here.
	ula200_ftdi_position(drvthis, 0, 0);
	ula200_ftdi_string(drvthis, p->lcd_contents, 1);

	key = GetKeyFromKeyRing(&p->keyring);

	// search the bit that was set by the hardware
	for (i = 0; i < MAX_KEY_MAP; i++) {
		if (key & (1 << i))
			return p->key_map[i];
	}
	
	if (key != '\0') {
		report(RPT_INFO, "%s: Untreated key 0x%02X", drvthis->name, key);
	}
	return NULL;
}


