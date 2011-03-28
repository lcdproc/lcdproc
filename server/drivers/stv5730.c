/** \file server/drivers/stv5730.c
 * LCDd \c stv5730 driver for the STV5730A on-screen display chip
 * connected to a parallel port.
 */

//////////////////////////////////////////////////////////////////////////
// This is a driver for the STV5730A on-screen display chip in con-     //
// junction with a parallel port interface. Check                       //
// http://www.usblcd.de/lcdproc/ for where to buy iand how to build     //
// the hardware.                                                        //
// The STV3730 displays 11 rows with 28 characters. The characters are  //
// fixed and can not be reprogrammed. Luckily the chraracter set con-   //
// tains a heartbeat icon and some characters that can be used as       //
// hbars / vbars.                                                       //
//                                                                      //
// Moved the delay timing code by Charles Steinkuehler to timing.h.     //
// Guillaume Filion <gfk@logidac.com>, December 2001                    //
//                                                                      //
// (C) 2001 Robin Adams ( robin@adams-online.de )                       //
//                                                                      //
// This driver is released under the GPL. See file COPYING in this      //
// package for further details.                                         //
//////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "lcd.h"
#include "stv5730.h"
#include "report.h"
#include "port.h"
#include "timing.h"

#ifndef LPTPORT
#define LPTPORT 0x378
#endif

#define STV5730_TEST_O	0x01
#define STV5730_BAR	0x02
#define STV5730_CLK	0x04
#define STV5730_CSN	0x08
#define STV5730_DATA	0x10

#define STV5730_TEST_I	0x40
#define STV5730_MUTE	0x80

// If it doesn't work try increasing this value
#define IODELAY		400


// Change that to NTSC if you are from the US...
#define	PAL


#define STV5730_HGT	11
#define STV5730_WID	28
#define STV5730_ATTRIB	0x800

#define STV5730_REG_ZOOM	0xCC
#define STV5730_REG_COLOR	0xCD
#define STV5730_REG_CONTROL	0xCE
#define STV5730_REG_POSITION	0xCF
#define STV5730_REG_MODE	0xD0

// Choose Colors: FLINE: First line text color, TEXT: Text color, CBACK: Character Background Color
//                CBORD: Character Border Color, SBACK: Screen Background color
// 0:Black, 1: Blue, 2:Green, 3: Cyan, 4: Red, 5: Magenta, 6: Yellow, 7: White
#define STV5730_COL_FLINE	4
#define STV5730_COL_TEXT	1
#define STV5730_COL_CBACK	3
#define STV5730_COL_CBORD	0
#define STV5730_COL_SBACK	2


/** private data for the \c stv5730 driver */
typedef struct stv5730_private_data {
    unsigned int port;
    unsigned int charattrib;
    unsigned int flags;
    char *framebuf;
} PrivateData;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "stv5730_";


// Translation map ascii->stv5730 charset
unsigned char stv5730_to_ascii[256] =
    {
	/* 00 - 7F */
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x29,	/*               ' */
	0x0B, 0x0B, 0x5F, 0x5E, 0x27, 0x0A, 0x27, 0x28,	/*     * + . - . / */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* 0 1 2 3 4 5 6 7 */
	0x08, 0x09, 0x26, 0x26, 0x62, 0x78, 0x61, 0x70,	/* 8 9 : : « = » ? */
	0x6c, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,	/* @ A B C D E F G */
	0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,	/* H I J K L M N O */
	0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,	/* P Q R S T U V W */
	0x23, 0x24, 0x25, 0x0B, 0x0B, 0x0B, 0x0B, 0x72,	/* X Y Z         _ */
	0x0B, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,	/*   a b c d e f g */
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,	/* h i j k l m n o */
	0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,	/* p q r s t u v w */
	0x41, 0x42, 0x43, 0x6E, 0x6C, 0x71, 0x79, 0x7F,	/* x y z # # # # # */
	/* 80 - FF */
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x58, 0x69, 0x0B, 0x0B,	/*         Ä Å     */
	0x0B, 0x6B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,	/*   É             */
	0x0B, 0x6D, 0x0B, 0x0B, 0x0B, 0x0B, 0x59, 0x0B,	/*   Ñ         Ö   */
	0x0B, 0x0B, 0x0B, 0x0B, 0x5A, 0x0B, 0x0B, 0x5C,	/*         Ü     ß */
	0x44, 0x47, 0x45, 0x0B, 0x46, 0x6A, 0x0B, 0x58,	/* à á â   ä å   ç */
	0x49, 0x48, 0x4B, 0x4A, 0x4C, 0x4F, 0x4D, 0x4E,	/* è é ê ë ì í î ï */
	0x0B, 0x5D, 0x50, 0x53, 0x51, 0x0B, 0x52, 0x0B,	/*   ñ ò ó ô   ö   */
	0x0B, 0x54, 0x57, 0x55, 0x56, 0x0B, 0x0B, 0x77	/*   ù ú û ü     # */
    };


//static void stv5730_upause(int delayCalls);
#define stv5730_upause timing_uPause

/////////////////////////////////////////////////////////////////
// This function returns true if a powered and working STV5730
// hardware is present at p->port
static int
stv5730_detect (unsigned int port)
{
    int i;

    for (i = 0; i < 10; i++) {
	port_out(port, STV5730_TEST_O);
	stv5730_upause(IODELAY);
	if ((port_in(port + 1) & STV5730_TEST_I) == 0)
	    return -1;
	port_out(port, 0);
	stv5730_upause(IODELAY);
	if ((port_in(port + 1) & STV5730_TEST_I) != 0)
	    return -1;
      }
    return 0;
}

/////////////////////////////////////////////////////////////////
// returns 0 if a valid video signal is connected to the video
// input
static int
stv5730_is_mute (unsigned int port)
{
    stv5730_upause(IODELAY);
    return ((port_in(port + 1) & STV5730_MUTE) ? 0 : 1);
}

/////////////////////////////////////////////////////////////////
// stv5730_write16bit, stv5730_write8bit, stv5730_write0bit
// this family of functions write commands or data to the stv5730
// 8 bit writes repeat the high byte, 0 byte writes repeat the last
// written word
static void
stv5730_write16bit (unsigned int port, unsigned int flags, unsigned int value)
{
    int i;

    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CLK + flags);

    for (i = 15; i >= 0; i--) {
	char databit = ((value & (1 << i)) != 0) ? STV5730_DATA : 0;

	port_out(port, databit + STV5730_CLK + flags);
	stv5730_upause(IODELAY);
	port_out(port, databit + flags);
	stv5730_upause(IODELAY);
	port_out(port, databit + STV5730_CLK + flags);
	stv5730_upause(IODELAY);
    }

    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
    stv5730_upause(IODELAY);
}

static void
stv5730_write8bit (unsigned int port, unsigned int flags, unsigned int value)
{
    int i;

    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CLK + flags);

    for (i = 7; i >= 0; i--) {
	char databit = ((value & (1 << i)) != 0) ? STV5730_DATA : 0;

	port_out(port, databit + STV5730_CLK + flags);
	stv5730_upause(IODELAY);
	port_out(port, databit + flags);
	stv5730_upause(IODELAY);
	port_out(port, databit + STV5730_CLK + flags);
	stv5730_upause(IODELAY);
    }

    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
}

static void
stv5730_write0bit (unsigned int port, unsigned int flags)
{
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CLK + flags);

    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + STV5730_CLK + flags);
    stv5730_upause(IODELAY);
    port_out(port, STV5730_CSN + flags);
}


/////////////////////////////////////////////////////////////////
// sets the memory pointer inside the stv5730 to the position
// row, col.
static void
stv5730_locate (unsigned int port, unsigned int flags, int row, int col)
{
    if (row < 0 || row >= STV5730_HGT || col < 0 || col >= STV5730_WID)
	return;

    stv5730_write16bit(port, flags, (row << 8) + col);
}

/////////////////////////////////////////////////////////////////
// draws  char z from fontmap to the framebuffer at position
// x,y. These are zero-based textmode positions.
// We need a conversion map to translate from ascii to the
// non-standard STV5730 charset.
//
static void
stv5730_drawchar2fb (Driver *drvthis, int x, int y, unsigned char z)
{
    PrivateData *p = drvthis->private_data;

    if ((x >= 0) && (x < STV5730_WID) && (y >= 0) && (y < STV5730_HGT))
      p->framebuf[(y * STV5730_WID) + x] = stv5730_to_ascii[(unsigned int) z];
}

/////////////////////////////////////////////////////////////////
// This initialises the stuff. We support supplying port as
// a command line argument.
//
MODULE_EXPORT int
stv5730_init (Driver *drvthis)
{
    PrivateData *p;
    int i;

    /* Allocate and store private data */
    p = (PrivateData *) calloc(1, sizeof(PrivateData));
    if (p == NULL)
   	return -1;
    if (drvthis->store_private_ptr(drvthis, p))
	return -1;

    /* initialize private data */
    p->port = LPTPORT;
    p->charattrib = STV5730_ATTRIB;
    p->flags = 0;
    p->framebuf = NULL;

    /* Read config file */

    /* What port to use */
    p->port = drvthis->config_get_int(drvthis->name, "Port", 0, LPTPORT);

    /* End of config file parsing */

    if (timing_init() == -1) {
	report(RPT_ERR, "%s: timing_init() failed (%s)", drvthis->name, strerror(errno));
	return -1;
    }

    // Initialize the Port and the stv5730
    if (port_access(p->port) || port_access(p->port + 1)) {
	  report(RPT_ERR,
	      "%s: cannot get IO-permission for 0x%03X! Are we running as root?",
	       drvthis->name, p->port);
	  return -1;
    }

    if (stv5730_detect(p->port)) {
	  report(RPT_ERR, "%s: no STV5730 hardware found at 0x%03X ",
			  drvthis->name, p->port);
	  return -1;
    }

    port_out(p->port, 0);

    // Reset the STV5730
    stv5730_write16bit(p->port, p->flags, 0x3000);
    stv5730_write16bit(p->port, p->flags, 0x3000);
    stv5730_write16bit(p->port, p->flags, 0x00db);
    stv5730_write16bit(p->port, p->flags, 0x1000);

    // Setup Mode + Control Register for video detection
    stv5730_write16bit(p->port, p->flags, STV5730_REG_MODE);
    stv5730_write16bit(p->port, p->flags, 0x1576);

    stv5730_write16bit(p->port, p->flags, STV5730_REG_CONTROL);
    stv5730_write16bit(p->port, p->flags, 0x1FF4);

    report(RPT_INFO, "%s: detecting video signal: ", drvthis->name);
    usleep (50000);

    if (stv5730_is_mute(p->port)) {
	  report(RPT_INFO, "%s: no video signal found; using full page mode", drvthis->name);
	  // Setup Mode + Control for full page mode
	  p->charattrib = STV5730_ATTRIB;
	  stv5730_write16bit(p->port, p->flags, STV5730_REG_MODE);
	  stv5730_write16bit(p->port, p->flags, 0x15A6);

	  stv5730_write16bit(p->port, p->flags, STV5730_REG_CONTROL);
#ifdef PAL
	  stv5730_write16bit(p->port, p->flags, 0x1FD5);
#endif
#ifdef NTSC
	  stv5730_write16bit(p->port, p->flags, 0x1ED4);
#endif

    }
    else {
	  report(RPT_INFO, "%s: video signal found, using mixed mode (B&W)", drvthis->name);
	  // Setup Mode + Control for mixed mode, disable color
	  p->charattrib = 0;
	  stv5730_write16bit(p->port, p->flags, STV5730_REG_MODE);
	  stv5730_write16bit(p->port, p->flags, 0x1576);

	  stv5730_write16bit(p->port, p->flags, STV5730_REG_CONTROL);
#ifdef PAL
	  stv5730_write16bit(p->port, p->flags, 0x1DD4);
#endif
#ifdef NTSC
	  stv5730_write16bit(p->port, p->flags, 0x1CF4);
#endif
      }

    // Position Register
    stv5730_write16bit(p->port, p->flags, STV5730_REG_POSITION);
    stv5730_write16bit(p->port, p->flags, 0x1000 + 64 * 30 + 30);

    // Color Register
    stv5730_write16bit(p->port, p->flags, STV5730_REG_COLOR);
    stv5730_write16bit(p->port, p->flags, 0x1000 + (STV5730_COL_SBACK << 9) +
			(STV5730_COL_CBORD << 6) + STV5730_COL_CBACK);

    // Zoom Register: Zoom first line
    stv5730_write16bit(p->port, p->flags, STV5730_REG_ZOOM);
    stv5730_write16bit(p->port, p->flags, 0x1000 + 4);

    // Set the Row Attributes
    for (i = 0; i <= 10; i++) {
	  stv5730_write16bit(p->port, p->flags, 0x00C0 + i);
	  stv5730_write16bit(p->port, p->flags, 0x10C0);
    }

    // Allocate our own framebuffer
    p->framebuf = malloc(STV5730_WID * STV5730_HGT);
    if (p->framebuf == NULL) {
	  report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
	  stv5730_close(drvthis);
	  return -1;
    }

    // clear screen
    memset(p->framebuf, 0, STV5730_WID * STV5730_HGT);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);

    return 0;
}

/////////////////////////////////////////////////////////////////
// Frees the framebuffer and exits the driver.
//
MODULE_EXPORT void
stv5730_close (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (p != NULL) {
	if (p->framebuf != NULL)
	    free(p->framebuf);

	free(p);
    }
    drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
stv5730_width (Driver *drvthis)
{
    return STV5730_WID;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
stv5730_height (Driver *drvthis)
{
    return STV5730_HGT;
}

/////////////////////////////////////////////////////////////////
// Returns the number of pixels a character is wide
//
MODULE_EXPORT int
stv5730_cellwidth (Driver *drvthis)
{
    return 4;
}

/////////////////////////////////////////////////////////////////
// Returns the number of pixels a character is high
//
MODULE_EXPORT int
stv5730_cellheight (Driver *drvthis)
{
    return 6;
}
// cellwidth and cellheight are only needed for old_vbar.
// Therefor these values are now hardcoded into these functions.
// When old_vbar is not used anymore, these two functions can be removed.


/////////////////////////////////////////////////////////////////
// Clears the screen
//
MODULE_EXPORT void
stv5730_clear (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->framebuf, 0x0B, STV5730_WID * STV5730_HGT);
}

/////////////////////////////////////////////////////////////////
//
// Flushes all output to the lcd...
//
MODULE_EXPORT void
stv5730_flush (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;
    int i, j, atr;

    stv5730_locate(p->port, p->flags, 0, 0);

    for (i = 0; i < STV5730_HGT; i++) {
	if (i == 0)
	    atr = (STV5730_COL_FLINE << 8);
	else
	    atr = (STV5730_COL_TEXT << 8);
	stv5730_write16bit(p->port, p->flags, 0x1000 + atr + p->framebuf[i * STV5730_WID] +
			      p->charattrib);
	for (j = 1; j < STV5730_WID; j++) {
	    if (p->framebuf[j + (i * STV5730_WID) - 1] !=
		p->framebuf[j + (i * STV5730_WID)])
		stv5730_write8bit(p->port, p->flags, p->framebuf[j + (i * STV5730_WID)]);
	    else
		stv5730_write0bit(p->port, p->flags);
	}
    }
}

/////////////////////////////////////////////////////////////////
// Prints a string on the screen, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (28,11).
//
MODULE_EXPORT void
stv5730_string (Driver *drvthis, int x, int y, const char string[])
{
    //PrivateData *p = drvthis->private_data;
    int i;

    x--;			// Convert 1-based coords to 0-based...
    y--;

    for (i = 0; string[i] != '\0'; i++)
	stv5730_drawchar2fb(drvthis, x + i, y, string[i]);
}

/////////////////////////////////////////////////////////////////
// Writes  char c at position x,y into the framebuffer.
// x and y are 1-based textmode coordinates.
//
MODULE_EXPORT void
stv5730_chr (Driver *drvthis, int x, int y, char c)
{
    //PrivateData *p = drvthis->private_data;

    y--;
    x--;
    stv5730_drawchar2fb(drvthis, x, y, c);
}

/////////////////////////////////////////////////////////////////
// This function draws ugly big numbers. We could use the zoom
// feature of the stv5730 if we'd know when big numbers start
// and stop.
MODULE_EXPORT void
stv5730_num (Driver *drvthis, int x, int num)
{
    //PrivateData *p = drvthis->private_data;
    int i, j;

    x--;

    if ((x >= STV5730_WID) || (num < 0) || (num > 10))
	return;

    for (j = 1; j < 10; j++) {
	if (num != 10) {
	    for (i = 0; i < 3; i++)
		stv5730_drawchar2fb(drvthis, x + i, j, '0' + num);
	}
	else {
	    stv5730_drawchar2fb(drvthis, x, j, ':');
	}
    }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar from the bottom up at 1-based position x.
//
MODULE_EXPORT void
stv5730_vbar(Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = drvthis->private_data;
    int i;			/* number of pixels drawn */
    int pixels;

    x--;

    if (x < 0 || len < 0 || (len) >= STV5730_HGT)
	return;

    pixels = ((long) 2 * len * 6) * promille / 2000;

    for (i = 0; i <= pixels; i += 6) {
	if (pixels >= (i + 6))		/* 6 = cellheight */
		p->framebuf[((10 - (i / 6)) * STV5730_WID) + x] = 0x77;
	else
		p->framebuf[((10 - (i / 6)) * STV5730_WID) + x] = 0x72 + (pixels % 6);
    }
}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar from left to right at 1-based position
// x,y into the framebuffer.
// It uses the STV5730 'channel-tuning' chars(0x64-0x68) to do
// this.
MODULE_EXPORT void
stv5730_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
    PrivateData *p = drvthis->private_data;
    int i;
    int pixels;

    x--;
    y--;

    if (y < 0 || y >= STV5730_HGT || x < 0 || len < 0 || (x + len) >= STV5730_WID)
	return;

    pixels = ((long) 2 * len * 4) * promille / 2000;

    for (i = 0; i <= pixels; i += 5) {
	if (pixels >= (i + 4))		/* 4 = cellwidth */
	    p->framebuf[(y * STV5730_WID) + x + (i / 5)] = 0x64;
	else
	    p->framebuf[(y * STV5730_WID) + x + (i / 5)] = 0x65 + (len % 5);
    }
}

/////////////////////////////////////////////////////////////////
// Reprogrammes character dest to contain an icon given by
// which.
// The STV5730 has no programmable chars. The charset is very
// limited, it doesn't even contain a '%' char. But wait...
// It contains a heartbeat char ! :-)
MODULE_EXPORT int
stv5730_icon(Driver *drvthis, int x, int y, int icon)
{
    PrivateData *p = drvthis->private_data;

    switch (icon) {
      case ICON_BLOCK_FILLED:
	p->framebuf[(y * STV5730_WID) + x] = 0x77;
	break;
      case ICON_HEART_OPEN:
	p->framebuf[(y * STV5730_WID) + x] = 0x0B;
	break;
      case ICON_HEART_FILLED:
	p->framebuf[(y * STV5730_WID) + x] = 0x71;
	break;
      case ICON_ARROW_UP:
	p->framebuf[(y * STV5730_WID) + x] = 0x7C;
	break;
      case ICON_ARROW_DOWN:
	p->framebuf[(y * STV5730_WID) + x] = 0x7D;
	break;
      case ICON_ARROW_LEFT:
	p->framebuf[(y * STV5730_WID) + x] = 0x7B;
	break;
      case ICON_ARROW_RIGHT:
	p->framebuf[(y * STV5730_WID) + x] = 0x7A;
	break;
      default:
	return -1;		/* Let the core do the others */
    }

    return 0;
}
