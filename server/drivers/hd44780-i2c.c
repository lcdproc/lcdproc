/** \file server/drivers/hd44780-i2c.c
 * \c i2c connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 4 bit-mode to be connected to the 8 bit-port
 * of a single PCF8574(A) or PCA9554(A) that is accessed by the server via the i2c bus.
 */

/* Copyright (c) 2005 Matthias Goebl <matthias.goebl@goebl.net>
 *		  2000, 1999, 1995 Benjamin Tse <blt@Comports.com>
 *		  2001 Joris Robijn <joris@robijn.net>
 *		  1999 Andrew McMeikan <andrewm@engineer.com>
 *		  1998 Richard Rognlie <rrognlie@gamerz.net>
 *		  1997 Matthias Prinke <m.prinke@trashcan.mcnet.de>
 *
 * The connections are:
 * PCF8574AP  Bit	LCD
 * P0 (4)	  0x01  D4 (11)
 * P1 (5)	  0x02  D5 (12)
 * P2 (6)	  0x04  D6 (13)
 * P3 (7)	  0x08  D7 (14)
 * P4 (9)	  0x10  RS (4)
 * P5 (10)	  0x20  RW (5)
 * P6 (11)	  0x40  EN (6)
 * P7 (12)	  0x80  /backlight (optional, active-low)
 *
 * Configuration:
 \verbatim
   alternative wiring example:
   PCF8574AP: P0 P1 P2 P3 P4 P5 P6 P7
              |  |  |  |  |  |  |  |
   HD44780:   RS RW EN BL D4 D5 D6 D7
   
   in LCDd.conf we then need to define
    i2c_line_RS=0x01
    i2c_line_RW=0x02
    i2c_line_EN=0x04
    i2c_line_BL=0x80
    i2c_line_D4=0x10
    i2c_line_D5=0x20
    i2c_line_D6=0x40
    i2c_line_D7=0x80
    Backlight=yes
    BacklightInvert=yes

	The Backlight Invert is used if a 0 turns the backlight on, and 1 turns it off, i.e. npn transistor
\endverbatim

 *
 * Configuration:
 * device=/dev/i2c-0   # the device file of the i2c bus
 * port=0x20   # the i2c address of the i2c port expander
 *
 *  Attention: Bit 8 of the address given in port is special:
 *  It tells the driver to treat the device as PCA9554 or similar,
 *  a device that needs a 2-byte command, and it will be stripped
 *  off the address.
 *  So we have:
 *  port=0x20..0x27   PCF8574  with A[012]=0..7
 *  port=0x38..0x3f   PCF8574A with A[012]=0..7
 *  port=0xa0..0xa7   PCA9554  with A[012]=0..7
 *  port=0xa0..0xa7   PCA9554A with A[012]=0..7
 *
 * When using this driver, DON'T load the i2c chip module (e.g. pcf8574),
 * you only need the i2c bus driver module!
 *
 *
 * Based mostly on the hd44780-4bit module, see there for a complete history.
 * Suggestions for PCA9554 support from Tonu Samuel <tonu@jes.ee>.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "hd44780-i2c.h"
#include "hd44780-low.h"

#include "shared/report.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "i2c.h"

// Generally, any function that accesses the LCD control lines needs to be
// implemented separately for each HW design. This is typically (but not
// restricted to):
// HD44780_senddata
// HD44780_readkeypad

void i2c_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void i2c_HD44780_backlight(PrivateData *p, unsigned char state);
void i2c_HD44780_close(PrivateData *p);

#define RS	0x10
#define RW	0x20
#define EN	0x40
#define BL	0x80
#define D4	0x01
#define D5	0x02
#define D6	0x04
#define D7	0x08
#define BL_INVERT  0
// note that the above bits are all meant for the data port of PCF8574

#define I2C_ADDR_MASK 0x7f
#define I2C_PCAX_MASK 0x80

static void
i2c_out(PrivateData *p, unsigned char val)
{
	unsigned char data[2];
	int datalen;
	static int no_more_errormsgs=0;

	if (p->port & I2C_PCAX_MASK) { // we have a PCA9554 or similar, that needs a 2-byte command
		data[0]=1; // command: read/write output port register
		data[1]=val;
		datalen=2;
	} else { // we have a PCF8574 or similar, that needs a 1-byte command
		data[0]=val;
		datalen=1;
	}

	if (i2c_write(p->i2c, data, datalen) < 0) {
		p->hd44780_functions->drv_report(no_more_errormsgs?RPT_DEBUG:RPT_ERR, "HD44780: I2C: i2c write data %u failed: %s",
			val, strerror(errno));
		no_more_errormsgs=1;
	}
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
hd_init_i2c(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	HD44780_functions *hd44780_functions = p->hd44780_functions;
	char device[256] = I2C_DEFAULT_DEVICE;

	p->i2c_backlight_invert = drvthis->config_get_bool(drvthis->name, "BacklightInvert", 0, BL_INVERT);
	p->i2c_line_RS = drvthis->config_get_int(drvthis->name, "i2c_line_RS", 0, RS);
	p->i2c_line_RW = drvthis->config_get_int(drvthis->name, "i2c_line_RW", 0, RW);
	p->i2c_line_EN = drvthis->config_get_int(drvthis->name, "i2c_line_EN", 0, EN);
	p->i2c_line_BL = drvthis->config_get_int(drvthis->name, "i2c_line_BL", 0, BL);
	p->i2c_line_D4 = drvthis->config_get_int(drvthis->name, "i2c_line_D4", 0, D4);
	p->i2c_line_D5 = drvthis->config_get_int(drvthis->name, "i2c_line_D5", 0, D5);
	p->i2c_line_D6 = drvthis->config_get_int(drvthis->name, "i2c_line_D6", 0, D6);
	p->i2c_line_D7 = drvthis->config_get_int(drvthis->name, "i2c_line_D7", 0, D7);
	
	report(RPT_INFO, "HD44780: I2C: Init using D4 and D5, and or'd lines, invert", p->i2c_line_RS);
	report(RPT_INFO, "HD44780: I2C: Pin RS mapped to 0x%02X", p->i2c_line_RS);
	report(RPT_INFO, "HD44780: I2C: Pin RW mapped to 0x%02X", p->i2c_line_RW);
	report(RPT_INFO, "HD44780: I2C: Pin EN mapped to 0x%02X", p->i2c_line_EN);
	report(RPT_INFO, "HD44780: I2C: Pin BL mapped to 0x%02X", p->i2c_line_BL);
	report(RPT_INFO, "HD44780: I2C: Pin D4 mapped to 0x%02X", p->i2c_line_D4);
	report(RPT_INFO, "HD44780: I2C: Pin D5 mapped to 0x%02X", p->i2c_line_D5);
	report(RPT_INFO, "HD44780: I2C: Pin D6 mapped to 0x%02X", p->i2c_line_D6);
	report(RPT_INFO, "HD44780: I2C: Pin D7 mapped to 0x%02X", p->i2c_line_D7);
	report(RPT_INFO, "HD44780: I2C: Invert Backlight %d", p->i2c_backlight_invert);
	
	p->backlight_bit = p->i2c_line_BL;

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, I2C_DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device)-1] = '\0';
	report(RPT_INFO,"HD44780: I2C: Using device '%s' and address 0x%02X for a %s",
		device, p->port & I2C_ADDR_MASK, (p->port & I2C_PCAX_MASK) ? "PCA9554(A)" : "PCF8574(A)");

	p->i2c = i2c_open(device, p->port & I2C_ADDR_MASK);
	if (!p->i2c) {
		report(RPT_ERR, "HD44780: I2C: connecting to device '%s' slave 0x%02X failed:", device, p->port & I2C_ADDR_MASK, strerror(errno));
		return(-1);
	}

	if (p->port & I2C_PCAX_MASK) { // we have a PCA9554 or similar, that needs special config
		unsigned char data[2];
		data[0] = 2; // command: set polarity inversion
		data[1] = 0; // -> no polarity inversion
		if (i2c_write(p->i2c, data, 2) < 0) {
			report(RPT_ERR, "HD44780: I2C: i2c set polarity inversion failed: %s", strerror(errno));
		}
		data[0] = 3; // command: set output direction
		data[1] = 0; // -> all pins are outputs
		if (i2c_write(p->i2c, data, 2) <0) {
			report(RPT_ERR, "HD44780: I2C: i2c set output direction failed: %s", strerror(errno));
		}
	}

	hd44780_functions->senddata = i2c_HD44780_senddata;
	hd44780_functions->backlight = i2c_HD44780_backlight;
	hd44780_functions->close = i2c_HD44780_close;

	// powerup the lcd now
	/* We'll now send 0x03 a couple of times,
	 * which is in fact (FUNCSET | IF_8BIT) >> 4
	 */
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	hd44780_functions->uPause(p, 15000);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	hd44780_functions->uPause(p, 5000);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	hd44780_functions->uPause(p, 100);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	hd44780_functions->uPause(p, 100);

	// now in 8-bit mode...  set 4-bit mode
	/*
	OLD   (FUNCSET | IF_4BIT) >> 4 0x02
	ALT   (FUNCSET | IF_4BIT)      0x20
	*/
	i2c_out(p, p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D5);
	if (p->delayBus)
		hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D5);
	hd44780_functions->uPause(p, 100);

	// Set up two-line, small character (5x8) mode
	hd44780_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
	hd44780_functions->uPause(p, 40);

	common_init(p, IF_4BIT);

	return 0;
}

void
i2c_HD44780_close(PrivateData *p) {
	if (p->i2c >= 0)
		i2c_close(p->i2c);
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
i2c_HD44780_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char portControl = 0;
	unsigned char h=0;
	unsigned char l=0;
	if( ch & 0x80 ) h |= p->i2c_line_D7;
	if( ch & 0x40 ) h |= p->i2c_line_D6;
	if( ch & 0x20 ) h |= p->i2c_line_D5;
	if( ch & 0x10 ) h |= p->i2c_line_D4;
	if( ch & 0x08 ) l |= p->i2c_line_D7;
	if( ch & 0x04 ) l |= p->i2c_line_D6;
	if( ch & 0x02 ) l |= p->i2c_line_D5;
	if( ch & 0x01 ) l |= p->i2c_line_D4;
	if (flags == RS_INSTR)
		portControl = 0;
	else //if (flags == RS_DATA)
		portControl = p->i2c_line_RS;

	portControl |= p->backlight_bit;

	i2c_out(p, portControl | h);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_EN | portControl | h);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, 1);
	i2c_out(p, portControl | h);

	i2c_out(p, portControl | l);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_EN | portControl | l);
	if (p->delayBus)
		p->hd44780_functions->uPause(p, 1);
	i2c_out(p, portControl | l);
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void i2c_HD44780_backlight(PrivateData *p, unsigned char state)
{
	static unsigned char old_state=0;
	if ( p->i2c_backlight_invert == 0 )
		p->backlight_bit = ((!p->have_backlight||state) ? 0 : p->i2c_line_BL);
	else // Inverted backlight - npn transistor
		p->backlight_bit = ((p->have_backlight && state) ? p->i2c_line_BL : 0);
	i2c_out(p, p->backlight_bit);
	if (state!=old_state)
	{
		    if (state == BACKLIGHT_ON)
		    {
    			/*100% brightness*/
			p->hd44780_functions->senddata(p, 0, RS_INSTR, 0x28 );
			p->hd44780_functions->uPause(p, 150);
		    }
		    else
		    {
    			/*25% brightness*/
			p->hd44780_functions->senddata(p, 0, RS_INSTR, 0x2B );
			p->hd44780_functions->uPause(p, 150);
		    }
	}
	else
	{ /*no need to update state*/
	}
	/*save old state*/
	old_state = state;
}
