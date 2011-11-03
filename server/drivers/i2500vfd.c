/** \file server/drivers/i2500vfd.c
 * LCDd \c i2500vfd driver for Intra2net's Intranator 2500 VFD displays.
 */

//////////////////////////////////////////////////////////////////////////
// This is a driver for the Intra2net Intranator 2500 VFD display       //
//                                                                      //
// The display features:                                                //
// - B/W and two additional grayscale colors                            //
// - USB data transfer                                                  //
// - Animated boot logo                                                 //
// - Hardware double buffering, limit is 27 FPS                         //
// - Adjustable brightness / brightness of single colors                //
//                                                                      //
// (C) 2003,2007 Intra2net AG                                           //
//                                                                      //
// The HD44780 font in i2500vfdfm.c was taken from                      //
// Michael Reinelt / lcd4linux and is (C) 2000 by him.                  //
//                                                                      //
// Code here is basend on sed1520.c:                                    //
// (C) 2001,2002 Robin Adams ( robin@adams-online.de )                  //
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
#include <ftdi.h>

#include "lcd.h"
#include "i2500vfd.h"
#include "glcd_font5x8.h"
#include "report.h"

// The display itself stores three pixels in one byte
// We waste a little memory as we store one pixel per byte
// as we want to keep the drawing code simple.
// Take a look at i2500vfd_flush for the conversion

#define INTRA2NET_VFD_XSIZE 140
#define INTRA2NET_VFD_YSIZE 32
#define INTRA2NET_VFD_SCREENSIZE INTRA2NET_VFD_XSIZE*INTRA2NET_VFD_YSIZE
#define INTRA2NET_VFD_PACKEDSIZE 47*32
#define INTRA2NET_VFD_XSHIFT 0

#define WIDTH           23
#define HEIGHT          4
#define CELLWIDTH	6
#define CELLHEIGHT	8

/** private data for the \c i2500vfd driver */
typedef struct i2500vfd_private_data {
    struct ftdi_context ftdi;
    unsigned char *framebuf;
    int changed;
} PrivateData;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "i2500vfd_";

/////////////////////////////////////////////////////////////////
// draws char z from fontmap to the framebuffer at position
// x,y. These are zero-based textmode positions.
// The Fontmap is stored in rows while the framebuffer is stored
// in columns, so we need a little conversion.
//
void
drawchar2fb (Driver *drvthis, int x, int y, unsigned char z)
{
    PrivateData *p = drvthis->private_data;

    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    x++;

    int font_x, font_y;
    for (font_y = 0; font_y < 8; font_y++) {
        for (font_x = 5; font_x > -1; font_x--) {
            if ((glcd_iso8859_1[z][font_y] & 1<<font_x) == 1<<font_x)
                p->framebuf[INTRA2NET_VFD_XSHIFT+x*6-font_x + (y*8+font_y)*140] = 1;
            else
                p->framebuf[INTRA2NET_VFD_XSHIFT+x*6-font_x + (y*8+font_y)*140] = 0;
        }
    }

    p->changed = 1;
}

/////////////////////////////////////////////////////////////////
// This initialises the stuff.
//
MODULE_EXPORT int
i2500vfd_init (Driver *drvthis)
{
    PrivateData *p;
    int i;
    unsigned char c;

    /* Allocate and store private data */
    p = (PrivateData *) calloc(1, sizeof(PrivateData));
    if (p == NULL)
        return -1;
    if (drvthis->store_private_ptr(drvthis, p))
        return -1;

    if (ftdi_init (&p->ftdi) < 0) {
        report (RPT_ERR, "ftdi_init failed. Out of memory?");
        return -1;
    }

    i = ftdi_usb_open (&p->ftdi, 0x0403, 0xF8A8);
    if (i != 0 && i != -5) {
        report (RPT_ERR, "Unable to find i2500 VFD display on USB bus. Aborting");
        return -1;
    }

    // Allocate our framebuffer
    p->framebuf = (unsigned char *) malloc(INTRA2NET_VFD_SCREENSIZE * 2 + INTRA2NET_VFD_PACKEDSIZE + 1);
    if (p->framebuf == NULL) {
        report(RPT_ERR, "%s: unable to allocate framebuffer", drvthis->name);
        i2500vfd_close (drvthis);
        return -1;
    }

    // Fade out (set brightness to zero)
    c = 4|64;
    ftdi_write_data (&p->ftdi, &c, 1);
    c = 0|64;
    ftdi_write_data (&p->ftdi, &c, 1);
    sleep (1);

    // Blank display
    c = 2|64;
    ftdi_write_data (&p->ftdi, &c, 1);

    // Bring voltage up again
    c = 4|64;
    ftdi_write_data (&p->ftdi, &c, 1);
    c = 63|64;
    ftdi_write_data (&p->ftdi, &c, 1);

    // Flip to blank page
    c = 64;
    ftdi_write_data (&p->ftdi, &c, 1);
    sleep (1);

    // Clear internal screen
    i2500vfd_clear(drvthis);

    // Unblank display
    c = 3|64;
    ftdi_write_data (&p->ftdi, &c, 1);

    report(RPT_DEBUG, "%s: init() done", drvthis->name);
    return 0;
}

/////////////////////////////////////////////////////////////////
// Frees the frambuffer and exits the driver.
//
MODULE_EXPORT void
i2500vfd_close (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (p) {
        ftdi_usb_close (&p->ftdi);
        ftdi_deinit(&p->ftdi);

        if (p->framebuf)
            free(p->framebuf);

        free(p);
    }
    drvthis->store_private_ptr(drvthis, NULL);
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
i2500vfd_width (Driver *drvthis)
{
    return WIDTH;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
i2500vfd_height (Driver *drvthis)
{
    return HEIGHT;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
i2500vfd_cellwidth (Driver *drvthis)
{
    return CELLWIDTH;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
i2500vfd_cellheight (Driver *drvthis)
{
    return CELLHEIGHT;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
i2500vfd_clear (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    memset(p->framebuf, 0, INTRA2NET_VFD_SCREENSIZE);
    p->changed = 1;
}

/////////////////////////////////////////////////////////////////
//
// Flushes all output to the VFD...
//
MODULE_EXPORT void
i2500vfd_flush (Driver *drvthis)
{
    PrivateData *p = drvthis->private_data;

    if (!p->changed)
        return;

    // Grayscales are currently discarded.
    // Could be useful for antialised fonts
    int packed_begin = INTRA2NET_VFD_SCREENSIZE*2;
    int packed_offset = packed_begin, offset = 0, pixpos = 0, xpos = 0;
    memset (p->framebuf+packed_begin, 0, INTRA2NET_VFD_PACKEDSIZE);
    while (offset != INTRA2NET_VFD_SCREENSIZE) {
        if (p->framebuf[offset] != 0) {
            switch(pixpos) {
                case 0:
                    p->framebuf[packed_offset] = 3;
                    break;
                case 1:
                    p->framebuf[packed_offset] |= 3<<2;
                    break;
                case 2:
                    p->framebuf[packed_offset] |= 3<<4;
                    break;
            }
        }

        xpos++;
        offset++;
        pixpos++;
        if (pixpos == 3) {
            pixpos = 0;
            packed_offset++;
        }
        // Special: The display is organized in 3-pixel columns, but the last column got only 2 pixels
        if (xpos == 140) {
            packed_offset++;
            pixpos = 0;
            xpos = 0;
        }
    }

    // Page flip command
    p->framebuf[INTRA2NET_VFD_SCREENSIZE * 2 + INTRA2NET_VFD_PACKEDSIZE] = 64;

    // Write data to display
    ftdi_write_data (&p->ftdi, p->framebuf+packed_begin, INTRA2NET_VFD_PACKEDSIZE+1);

    p->changed = 0;
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lc display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
i2500vfd_string (Driver *drvthis, int x, int y, const char string[])
{
    int i;
    x--;            // Convert 1-based coords to 0-based
    y--;

    for (i = 0; string[i]; i++)
        drawchar2fb (drvthis, x + i, y, string[i]);
}

/////////////////////////////////////////////////////////////////
// Writes  char c at position x,y into the framebuffer.
// x and y are 1-based textmode coordinates.
//
MODULE_EXPORT void
i2500vfd_chr (Driver *drvthis, int x, int y, char c)
{
    y--;
    x--;
    drawchar2fb(drvthis, x, y, c);
}

/////////////////////////////////////////////////////////////////
// Changes the font of character n to a pattern given by *dat.
// HD44780 Controllers only posses 8 programmable chars. But
// we store the fontmap completely in RAM, so every character
// can be altered. !Important: Characters have to be redrawn
// by drawchar2fb() to show their new shape. Because we use
// a non-standard 6x8 font a *dat not calculated from
// width and height will fail.
//
MODULE_EXPORT void
i2500vfd_set_char (Driver *drvthis, int n, char *dat)
{
    int row;
    unsigned char mask = (1 << CELLWIDTH) - 1;

    if (n < 0 || n > 255)
        return;
    if (!dat)
        return;

    for (row = 0; row < CELLHEIGHT; row++) {
        glcd_iso8859_1[n][row] = dat[row] & mask;
    }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical from the bottom up to the last 3 rows of the
// framebuffer at 1-based position x. len is given in pixels.
//
MODULE_EXPORT void
i2500vfd_vbar(Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
    PrivateData *p = drvthis->private_data;
    unsigned int offset;
    int i, j, pixels;

    x--;
    // don't do y-- as we draw bottom up

    if (x < 0 || y < 1 || x >= WIDTH || y > HEIGHT || len > HEIGHT) {
        report(RPT_DEBUG, "%s: [vbar ERROR] x: %d, y: %d, len: %d", drvthis->name, x, y, len);
        return;
    }

    offset = INTRA2NET_VFD_XSHIFT + x*CELLWIDTH + y*INTRA2NET_VFD_XSIZE*CELLHEIGHT;
    pixels = len*CELLHEIGHT*promille/1000;

    // printf("[vbar] x: %d, y: %d, len: %d, offset: %d, pixels: %d\n", x, y, len, offset, pixels);
    for (i = 0; i < pixels; i++) {
        for (j = 0; j < CELLWIDTH; j++) {
            p->framebuf[offset+j] = 1;
        }
        // go to next y-line
        offset -= INTRA2NET_VFD_XSIZE;
    }

    p->changed = 1;
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar from left to right at 1-based position
// x,y into the framebuffer. len is given in characters;
//
MODULE_EXPORT void
i2500vfd_hbar(Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
    PrivateData *p = drvthis->private_data;
    unsigned int offset;
    int i, j, pixels;

    x--;
    y--;

    if (y < 0 || y >= HEIGHT || x < 0 || len < 0 || x + len > WIDTH) {
        return;
    }

    offset = INTRA2NET_VFD_XSHIFT + 2 + x*CELLWIDTH + (y*CELLHEIGHT+1)*INTRA2NET_VFD_XSIZE;

    // calculate length of bar
    pixels = len*CELLWIDTH*promille/1000;

    for (i = 0; i < CELLHEIGHT-1; i++) {
        for (j = 0; j < pixels; j++) {
            p->framebuf[offset+j] = 1;
        }
        // go to next y-line
        offset += INTRA2NET_VFD_XSIZE;
    }

    p->changed = 1;
}

/////////////////////////////////////////////////////////////////
// Gets an icon character from the font definition and calls
// chr() to draw it.
//
MODULE_EXPORT int
i2500vfd_icon (Driver *drvthis, int x, int y, int icon)
{
    int icon_char;

    if ((icon_char = glcd_icon5x8(icon)) != -1) {
        i2500vfd_chr(drvthis, x, y, icon_char);
        return 0;
    }
    return -1;
}
