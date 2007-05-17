/*
 * LIS driver for lcdproc
 * For the L.I.S MCE vacuum fluorescent display from vlsys.co.kr
 * 20x2 display using NEC UPD16314 driver and FTDI FT232RQ USB-to-serial
 * converter.
 *
 * This driver uses the libftdi library to interface to the FTDI chip. Make
 * sure kernel module ftdi_sio doesn't claim the device.
 *
 * Based on the ula200 driver Copyright (C) 2006, Bernhard Walle
 * and the CrystalFontz 633 driver Copyright (C) 2002 David GLAUDE
 *
 * Copyright (c)  2007, Daryl Fonseca-Holt <wyatt@prairieturtle.ca>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * 
 * 
 *
 */

/*
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

#include <sys/syscall.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lis.h"
#include "report.h"
#include "lcd_lib.h"
#include "timing.h"

void lis_test(Driver *drvthis);

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 1;
MODULE_EXPORT char *symbol_prefix = "lis_";


///////////////////////////////////////////////////////////////////////////////
// constants
//
#define DISPLAY_VENDOR_ID    0x0403
#define DISPLAY_PRODUCT_ID   0x6001

#define CELLWIDTH  5
#define CELLHEIGHT 8

#define DEFAULT_BRIGHTNESS 750

/*
 * Charactar mapping for UPD16314 device by Daryl Fonseca-Holt
 * <wyatt@prairieturtle.ca> taken from Mark Haemmerling's HD44780 table.
 * Character mapping for HD44780 devices by Mark Haemmerling <mail@markh.de>.
 *
 * Translates ISO 8859-1 to any HD44780 charset.
 *
 * Charmap selector (C) 2006 Pillon Matteo <matteo.pillon@email.it>
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

/*
 * UPD16314 table
 *
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
  160,  33, 236, 237, 164,  92, 124, 167,
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
  248, 249, 250, 251, 252, 253, 254, 3 
};


///////////////////////////////////////////////////////////////////////////////
// private data types
//
typedef struct {

    // the handle for the USB FTDI library
    struct ftdi_context ftdic;

    // the width and the height (in number of characters) of the library
	int width, height;

	// The framebuffer and the framebuffer for the last contents (incr. update)
	unsigned char *framebuf;

    // dirty line flags
	unsigned int *line_flags;

	// child thread flag
	int child_flag;

	// parent thread flag
	int parent_flag;

	// display brightness 0-1000
	int brightness;

	// custom characters loaded
	int	cc_flag;

} PrivateData;


///////////////////////////////////////////////////////////////////////////////
// Write a command to the display.
//
// @param p the private data
// @param data the data bytes
// @param length the number of bytes in @p data which are valid
// @return 0 on success, negative value on error
//
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

//////////////////////////////////////////////////////////////////////////////
// Displays a string at line n (n typically 1 or 2)
//
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


///////////////////////////////////////////////////////////////////////////////
// Flush the framebuffer to the display
//
MODULE_EXPORT void
lis_flush(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	int line;


	for (line = 0; line < p->height; line++) {
		if (p->line_flags[line]) {
			lis_ftdi_line_to_display(drvthis, line+1, p->framebuf + (line * p->width), p->width);
			p->line_flags[line] = 0;	// clean
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Loads custom characters in the display
//
static int
lis_load_custom_chars(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
    int i, col, row, err;
	unsigned char buffer[65];
	char custom_chars[8][CELLHEIGHT*CELLHEIGHT] = {
       { 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0 },
       { 1, 0, 0, 0, 0,			/* 1-5 characters used by hbar() */
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0,
		 1, 0, 0, 0, 0 },
       { 1, 1, 0, 0, 0,			
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0,
		 1, 1, 0, 0, 0 },
       { 1, 1, 1, 0, 0,			
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0,
		 1, 1, 1, 0, 0 },
       { 1, 1, 1, 1, 0,			
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0,
		 1, 1, 1, 1, 0 },
       { 1, 1, 1, 1, 1,			
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1 },
       { 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 1, 0, 0, 0, 1,
		 1, 0, 0, 0, 1,
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

	buffer[0] = 0xAD;		// command to write CGRAM at 0

    for (i = 0; i < 8; i++)
    {

        /* build the buffer */
        for (row = 0; row < CELLHEIGHT; row++) {
            unsigned char value = 0;

            for (col = 0; col < CELLWIDTH; col++) {
				value <<= 1;
				value |= (custom_chars[i][(row * CELLWIDTH) + col] > 0) ? 1 : 0;
            }
			buffer[(i*8)+row+1] = value;
        }
    }
    err = lis_ftdi_write_command(drvthis, buffer, 65);
    if (err < 0) {
        report(RPT_WARNING, "%s: lis_load_custom_chars(): "
                            "lis_ftdi_write_command() failed", drvthis->name);
    }
	else
		p->cc_flag = 1;

    return err;
}

//////////////////////////////////////////////////////////////////////////////
// Seperate thread to keep a read up on the USB device at all times
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
			return size;
			p->parent_flag = 0;
		}
	}
	p->parent_flag = 0;
	return 0;

}

///////////////////////////////////////////////////////////////////////////////
// Control the display brightness
//
MODULE_EXPORT int
lis_set_brightness(Driver *drvthis, int state, int promille)
{
	unsigned char buffer[2];
	int err;
	PrivateData *p = (PrivateData *) drvthis->private_data;
	
	if (promille < 0 || promille > 1000) 
        return -EINVAL;

	buffer[0] = 0xA5;
	if ( promille < 251 )
		buffer[1] = 0x3;		// 25%
	else if ( promille < 501 )
		buffer[1] = 0x2;		// 50%
	else if ( promille < 751 )
		buffer[1] = 0x1;		// 75%
	else
		buffer[1] = 0x0;		// 100%

    err = ftdi_write_data(&p->ftdic, buffer, 2);
    if (err < 0) {
        report(RPT_WARNING, "%s: lis_set_brightness(): ftdi_write_data failed with %d", drvthis->name, err);
        return err;
    }
	else
		p->brightness = promille;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Get the brightness setting
MODULE_EXPORT int
lis_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->brightness;
}

///////////////////////////////////////////////////////////////////////////////
// Init the driver and display
//
MODULE_EXPORT int
lis_init(Driver *drvthis)
{
	PrivateData *p;
	int err;
	const char *s;
	unsigned char buffer[64], *thread_stack;
	int count;

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
	p->cc_flag = 0;

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
		p->line_flags[count] = 1;					// dirty!

	/* Which display brightness */
	count = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	if ((count < 0) || (count > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		count = DEFAULT_BRIGHTNESS;
	}
	p->brightness = count;

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
	err = ftdi_usb_open(&p->ftdic, DISPLAY_VENDOR_ID, DISPLAY_PRODUCT_ID);
	if (err < 0) {
		report(RPT_ERR, "%s: cannot open USB device", drvthis->name);
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

	// clone a thread to keep a read up on the device
	thread_stack = calloc(4096, 1);
	if(! thread_stack) {
		report(RPT_ERR, "%s: cannot create thread stack", drvthis->name);
		goto err_framebuf;
	}

	err = clone(lis_read_thread, thread_stack+4092, CLONE_VM | CLONE_THREAD| CLONE_SIGHAND, drvthis);
	if (err == -1) {
		report(RPT_ERR, "%s: clone() - %s", drvthis->name, strerror(errno));
		goto err_framebuf;
	}
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

	lis_load_custom_chars(drvthis);

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

///////////////////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
lis_close(Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	if (p != NULL) {
		if (p->parent_flag) {			// terminate the child
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

///////////////////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
lis_width (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->width;
}

///////////////////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
lis_height (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	return p->height;
}

///////////////////////////////////////////////////////////////////////////////
// Clear the framebuffer
//
MODULE_EXPORT void
lis_clear (Driver *drvthis)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int line;

	for (line = 0; line < p->height; line++) {
		memset(p->framebuf + (line * p->width), ' ', p->width);
		p->line_flags[line] = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Place a character in the framebuffer
//
MODULE_EXPORT void
lis_chr (Driver *drvthis, int x, int y, char ch)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;

	// ignore out-of-range
	if (y > p->height || x > p->width)
		return;

	y--;
	x--;

	if ( p->framebuf[ (y * p->width) + x] != ch) {
		p->framebuf[ (y * p->width) + x] = ch;

		p->line_flags[y] = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Place a string in the framebuffer
//
MODULE_EXPORT void
lis_string (Driver *drvthis, int x, int y, char *s)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	int i;

	x --;  // Convert 1-based coords to 0-based
	y --;

	for (i = 0; s[i]; i++) {
		// Check for buffer overflows...
		if ((y * p->width) + x + i > (p->width * p->height))
			break;

		if ( p->framebuf[(y*p->width) + x + i] != s[i] ) {
			p->framebuf[(y*p->width) + x + i] = s[i];
			p->line_flags[((y*p->width) + x + i ) / p->width ] = 1;  // dirty
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Set default icon into a userdef char
//
MODULE_EXPORT int
lis_icon (Driver *drvthis, int x, int y, int icon)
{
	char ch;

	switch (icon) {
		case ICON_BLOCK_FILLED:
			ch = 0xFF;
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
			ch = 7;
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

		lis_chr(drvthis, x, y, ch);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Draw a horizontal bar
//
// options ignored
// truncates to fit display if needed
MODULE_EXPORT void
lis_hbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = (PrivateData *) drvthis->private_data;
	char	buf[p->width+1];
	int		per_pel;
	char	*bp;

	if (! p->cc_flag)
		lis_load_custom_chars(drvthis);

	if (x + len > p->width)
		len = p->width - x;

	if (len < 1 || promille < 0 || promille > 1000 )
		return;

	// blank area for bar
	memset(buf, ' ', len);

	if( promille == 0)
		return;

	per_pel = 1000 / (len * CELLWIDTH);

	bp = buf;

	while(promille > per_pel) {
		if( promille >= per_pel * CELLWIDTH) {
			*bp = 0x05;		// full width
			promille -= CELLWIDTH*per_pel;
		} else {
			*bp = (promille / per_pel);
			promille = 0;
		}
		bp++;
	}
	*bp = 0x00;

	lis_string(drvthis, x, y, buf);
}


void
lis_test(Driver *drvthis)
{
	int	x, y, i;


	x = 0;
	y = 1;
	lis_clear(drvthis);

	for (i = 0; i < 1001; i++) {
		lis_hbar(drvthis, 1, 1, 20, i, 0);
		lis_flush(drvthis);
	}
}
