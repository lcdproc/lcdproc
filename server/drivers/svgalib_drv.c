/** \file server/drivers/svgalib_drv.c
 * LCDd \c svga driver for displaying on SVGA screens.
 */

/*-
   This is a hack of the curses driver.  It's my first attempt at SVGALIB
   programming so please send comments to <smh@remove_this@dr.com>.

   Simon Harrison.  27 Dec 1999.

   Changes:
   9 Jan 2000:  Figured out svgalib driver needs a hack to main.c just like
                curses.  Maybe this should be fixed with another function call
                added to the API?
   4 Jan 2000:  Added 5x7 font, fixed flicker.
   26 Jul 2005: adapted better to 0.5 API; changed font from 5x7 to 6x8;
   		take options from the config file: mode, size, brightness,
		contrast, ...
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vga.h>
#include <vgagl.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "report.h"
#include "svgalib_drv.h"



/* Small font */

const unsigned char simple_font6x8[] = {
/* ASCII   0 (NUL) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   1 (SOH) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   2 (STX) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   3 (ETX) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   4 (EOT) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   5 (ENQ) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   6 (ACK) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   7 (BEL) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   8 (BS)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII   9 (HT)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  10 (LF)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  11 (VT)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  12 (FF)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  13 (CR)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  14 (SO)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  15 (SI)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  16 (DLE) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  17 (DC1) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  18 (DC2) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  19 (DC3) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  20 (DC4) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  21 (NAK) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  22 (SYN) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  23 (ETB) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  24 (CAN) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  25 (EM)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  26 (SUB) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  27 (ESC) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  28 (FS)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  29 (GS)) */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  30 (RS)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  31 (US)  */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  32 (' ') */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  33 ('!') */	0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04, 0x00,
/* ASCII  34 ('"') */	0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  35 ('#') */	0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A, 0x00,
/* ASCII  36 ('$') */	0x04, 0x1E, 0x05, 0x0E, 0x14, 0x0F, 0x04, 0x00,
/* ASCII  37 ('%') */	0x03, 0x13, 0x08, 0x04, 0x02, 0x19, 0x18, 0x00,
/* ASCII  38 ('&') */	0x06, 0x09, 0x05, 0x02, 0x15, 0x09, 0x16, 0x00,
/* ASCII  39 (''') */	0x06, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  40 ('(') */	0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00,
/* ASCII  41 (')') */	0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00,
/* ASCII  42 ('*') */	0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00, 0x00,
/* ASCII  43 ('+') */	0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00,
/* ASCII  44 (',') */	0x00, 0x00, 0x00, 0x00, 0x06, 0x04, 0x02, 0x00,
/* ASCII  45 ('-') */	0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
/* ASCII  46 ('.') */	0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00,
/* ASCII  47 ('/') */	0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00,
/* ASCII  48 ('0') */	0x0E, 0x11, 0x19, 0x15, 0x13, 0x11, 0x0E, 0x00,
/* ASCII  49 ('1') */	0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
/* ASCII  50 ('2') */	0x0E, 0x11, 0x10, 0x08, 0x04, 0x02, 0x1F, 0x00,
/* ASCII  51 ('3') */	0x1F, 0x08, 0x04, 0x08, 0x10, 0x11, 0x0E, 0x00,
/* ASCII  52 ('4') */	0x08, 0x0C, 0x0A, 0x09, 0x1F, 0x08, 0x08, 0x00,
/* ASCII  53 ('5') */	0x1F, 0x01, 0x0F, 0x10, 0x10, 0x11, 0x0E, 0x00,
/* ASCII  54 ('6') */	0x0C, 0x02, 0x01, 0x0F, 0x11, 0x11, 0x0E, 0x00,
/* ASCII  55 ('7') */	0x1F, 0x10, 0x08, 0x04, 0x02, 0x02, 0x02, 0x00,
/* ASCII  56 ('8') */	0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00,
/* ASCII  57 ('9') */	0x0E, 0x11, 0x11, 0x1E, 0x10, 0x08, 0x06, 0x00,
/* ASCII  58 (':') */	0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00, 0x00,
/* ASCII  59 (';') */	0x00, 0x06, 0x06, 0x00, 0x06, 0x04, 0x02, 0x00,
/* ASCII  60 ('<') */	0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00,
/* ASCII  61 ('=') */	0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00,
/* ASCII  62 ('>') */	0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00,
/* ASCII  63 ('?') */	0x0E, 0x11, 0x10, 0x08, 0x04, 0x00, 0x04, 0x00,
/* ASCII  64 ('@') */	0x0E, 0x11, 0x10, 0x16, 0x15, 0x15, 0x0E, 0x00,
/* ASCII  65 ('A') */	0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00,
/* ASCII  66 ('B') */	0x0F, 0x11, 0x11, 0x0F, 0x11, 0x11, 0x0F, 0x00,
/* ASCII  67 ('C') */	0x0E, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0E, 0x00,
/* ASCII  68 ('D') */	0x07, 0x09, 0x11, 0x11, 0x11, 0x09, 0x07, 0x00,
/* ASCII  69 ('E') */	0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F, 0x00,
/* ASCII  70 ('F') */	0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x01, 0x00,
/* ASCII  71 ('G') */	0x0E, 0x11, 0x01, 0x1D, 0x11, 0x11, 0x1E, 0x00,
/* ASCII  72 ('H') */	0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00,
/* ASCII  73 ('I') */	0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
/* ASCII  74 ('J') */	0x1C, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06, 0x00,
/* ASCII  75 ('K') */	0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11, 0x00,
/* ASCII  76 ('L') */	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1F, 0x00,
/* ASCII  77 ('M') */	0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00,
/* ASCII  78 ('N') */	0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00,
/* ASCII  79 ('O') */	0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
/* ASCII  80 ('P') */	0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00,
/* ASCII  81 ('Q') */	0x0E, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16, 0x00,
/* ASCII  82 ('R') */	0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00,
/* ASCII  83 ('S') */	0x1E, 0x01, 0x01, 0x0E, 0x10, 0x10, 0x0F, 0x00,
/* ASCII  84 ('T') */	0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
/* ASCII  85 ('U') */	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
/* ASCII  86 ('V') */	0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00,
/* ASCII  87 ('W') */	0x11, 0x11, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00,
/* ASCII  88 ('X') */	0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00,
/* ASCII  89 ('Y') */	0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00,
/* ASCII  90 ('Z') */	0x1F, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1F, 0x00,
/* ASCII  91 ('[') */	0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E, 0x00,
/* ASCII  92 ('\') */	0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00,
/* ASCII  93 (']') */	0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E, 0x00,
/* ASCII  94 ('^') */	0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  95 ('_') */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00,
/* ASCII  96 ('`') */	0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
/* ASCII  97 ('a') */	0x00, 0x00, 0x0E, 0x10, 0x1E, 0x11, 0x1E, 0x00,
/* ASCII  98 ('b') */	0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F, 0x00,
/* ASCII  99 ('c') */	0x00, 0x00, 0x0E, 0x01, 0x01, 0x11, 0x0E, 0x00,
/* ASCII 100 ('d') */	0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E, 0x00,
/* ASCII 101 ('e') */	0x00, 0x00, 0x0E, 0x11, 0x1F, 0x01, 0x0E, 0x00,
/* ASCII 102 ('f') */	0x0C, 0x12, 0x02, 0x07, 0x02, 0x02, 0x02, 0x00,
/* ASCII 103 ('g') */	0x00, 0x00, 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x0E,
/* ASCII 104 ('h') */	0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
/* ASCII 105 ('i') */	0x04, 0x00, 0x06, 0x04, 0x04, 0x04, 0x0E, 0x00,
/* ASCII 106 ('j') */	0x08, 0x00, 0x0C, 0x08, 0x08, 0x08, 0x09, 0x06,
/* ASCII 107 ('k') */	0x01, 0x01, 0x09, 0x05, 0x03, 0x05, 0x09, 0x00,
/* ASCII 108 ('l') */	0x06, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
/* ASCII 109 ('m') */	0x00, 0x00, 0x0B, 0x15, 0x15, 0x11, 0x11, 0x00,
/* ASCII 110 ('n') */	0x00, 0x00, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
/* ASCII 111 ('o') */	0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00,
/* ASCII 112 ('p') */	0x00, 0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01,
/* ASCII 113 ('q') */	0x00, 0x00, 0x16, 0x19, 0x19, 0x1E, 0x10, 0x10,
/* ASCII 114 ('r') */	0x00, 0x00, 0x0D, 0x13, 0x01, 0x01, 0x01, 0x00,
/* ASCII 115 ('s') */	0x00, 0x00, 0x0E, 0x01, 0x0E, 0x10, 0x0F, 0x00,
/* ASCII 116 ('t') */	0x02, 0x02, 0x07, 0x02, 0x02, 0x12, 0x0C, 0x00,
/* ASCII 117 ('u') */	0x00, 0x00, 0x11, 0x11, 0x11, 0x19, 0x16, 0x00,
/* ASCII 118 ('v') */	0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00,
/* ASCII 119 ('w') */	0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00,
/* ASCII 120 ('x') */	0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00,
/* ASCII 121 ('y') */	0x00, 0x00, 0x11, 0x11, 0x11, 0x1E, 0x10, 0x0E,
/* ASCII 122 ('z') */	0x00, 0x00, 0x1F, 0x08, 0x04, 0x02, 0x1F, 0x00,
/* ASCII 123 ('{') */	0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08, 0x00,
/* ASCII 124 ('|') */	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
/* ASCII 125 ('}') */	0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02, 0x00,
/* ASCII 126 ('~') */	0x00, 0x04, 0x08, 0x1F, 0x08, 0x04, 0x00, 0x00,
};


/* No, I don't understand SVGALIB key mappings or a neat way of doing this
 * Cursor keys manifest themselves as 3 byte excapes in svgalib: 27,91
 * followed by one of the codes below.
 */
#define VGAKEY_UP	65
#define VGAKEY_DOWN	66
#define VGAKEY_RIGHT	67
#define VGAKEY_LEFT	68


/*************************************************************************
********************** For Output on SVGALIB screen **********************
*************************************************************************/

static void
ExpandGroovyFont (int w, int ht, unsigned char col, const unsigned char *fnt, unsigned char *ptr)
/* Expand groovy 6x8 font into an area of memory */
{
	int n;
	unsigned char *p = ptr;

	for (n = 0; n < 127; n++) {
		int y;

		for (y = 0; y < ht; y++) {
			unsigned char mask = 1;
			unsigned char base = fnt[n * ht + y];
			int x;

			for (x = 0; x < w; x++) {
				*p++ = (base & mask) ? col : 0;
				mask <<= 1;
			}
		}
	}
}


static char icon_char = '@';

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "svga_";


/**
 * Init driver
 */
MODULE_EXPORT int
svga_init (Driver *drvthis)
{
	char modestr[LCD_MAX_WIDTH+1] = DEFAULT_MODESTR;
	char size[LCD_MAX_WIDTH+1] = DEFAULT_SIZE;
	vga_modeinfo *modeinfo;
	int tmp;

	PrivateData *p;

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* Initialize the PrivateData structure */

	p->cellwidth = CELLWIDTH;
	p->cellheight = CELLHEIGHT;
	p->contrast = DEFAULT_CONTRAST;
	p->brightness = DEFAULT_BRIGHTNESS;
	p->offbrightness = DEFAULT_OFFBRIGHTNESS;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	/* Read config file */

	/* Which size */
	if (drvthis->config_has_key(drvthis->name, "Size")) {
		int w;
		int h;

		strncpy(size, drvthis->config_get_string(drvthis->name, "Size",
							 0, DEFAULT_SIZE), sizeof(size));
		size[sizeof(size) - 1] = '\0';
		debug(RPT_INFO, "%s: Size (in config) is '%s'", __FUNCTION__, size);
		if ((sscanf(size, "%dx%d", &w, &h) != 2) ||
		    (w <= 0) || (w > LCD_MAX_WIDTH) ||
		    (h <= 0) || (h > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
					drvthis->name, size, DEFAULT_SIZE);
			sscanf(DEFAULT_SIZE, "%dx%d", &w, &h);
		}
		p->width = w;
		p->height = h;
	}
	else {
		/* Determine the size of the screen */
		p->width = drvthis->request_display_width();
		p->height = drvthis->request_display_height();
		if ((p->width <= 0) || (p->width >= LCD_MAX_WIDTH) ||
		    (p->height <= 0) || (p->height >= LCD_MAX_HEIGHT)) {
			p->width = LCD_DEFAULT_WIDTH;
			p->height = LCD_DEFAULT_HEIGHT;
		}
	}
	report(RPT_INFO, "%s: using Size %dx%d", drvthis->name, p->width, p->height);

	/* Which backlight brightness */
	tmp = drvthis->config_get_int(drvthis->name, "Brightness", 0, DEFAULT_BRIGHTNESS);
	debug(RPT_INFO, "%s: Brightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: Brightness must be between 0 and 1000; using default %d",
			drvthis->name, DEFAULT_BRIGHTNESS);
		tmp = DEFAULT_BRIGHTNESS;
	}
	p->brightness = tmp;

	/* Which backlight-off "brightness" */
	tmp = drvthis->config_get_int(drvthis->name, "OffBrightness", 0, DEFAULT_OFFBRIGHTNESS);
	debug(RPT_INFO, "%s: OffBrightness (in config) is '%d'", __FUNCTION__, tmp);
	if ((tmp < 0) || (tmp > 1000)) {
		report(RPT_WARNING, "%s: OffBrightness must be between 0 and 1000. Using default %d",
			drvthis->name, DEFAULT_OFFBRIGHTNESS);
		tmp = DEFAULT_OFFBRIGHTNESS;
	}
	p->offbrightness = tmp;

	/* which mode */
	strncpy(modestr, drvthis->config_get_string(drvthis->name, "Mode",
						    0, DEFAULT_MODESTR), sizeof(modestr));
	modestr[sizeof(modestr) - 1] = '\0';
	debug(RPT_INFO, "%s: Mode (in config) is '%s'", __FUNCTION__, modestr);

	/* initialize svgalib library */
	if (vga_init() != 0) {
		report(RPT_ERR, "%s: vga_init() failed", drvthis->name);
		return -1;
	}

	/* check for legal VGA mode */
	tmp = vga_getmodenumber(modestr);
	if (tmp <= 0) {
		report(RPT_ERR, "%s: illegal VGA mode %s", drvthis->name, modestr);
		return -1;
	}
	p->mode = tmp;

	/* switch to selected VGA mode if it is available */
	if (!vga_hasmode (p->mode)) {
		report(RPT_ERR, "%s: VGA mode %s not available", drvthis->name, modestr);
		return -1;
	}

	modeinfo = vga_getmodeinfo(p->mode);

	/* make sure width and height fit into selected resolution */
	if (p->width * p->cellwidth > modeinfo->width)
		p->width = modeinfo->width / p->cellwidth;
	if (p->height * p->cellheight > modeinfo->height)
		p->height = modeinfo->height / p->cellheight;

	/* center display on screen */
	p->xoffs = p->cellwidth + (modeinfo->width - p->width * p->cellwidth) / 2;
	p->yoffs = p->cellheight + (modeinfo->height - p->height * p->cellheight) / 2;

	if (vga_setmode (p->mode) < 0) {
		report(RPT_ERR, "%s: unable to switch to mode %s", drvthis->name, modestr);
		return -1;
	}
	gl_setcontextvga(p->mode);	/* Physical screen context. */
	gl_setrgbpalette();

	/* allocate space, expand and install the font */
	p->font = malloc(256 * p->cellheight * p->cellwidth * modeinfo->bytesperpixel);
	if (p->font == NULL) {
		report(RPT_ERR, "%s: unable to allocate font memory", drvthis->name);
		return -1;
	}

	tmp = (p->brightness * 255) / 1000;
	if (tmp <= 0)
		tmp = 1;
	ExpandGroovyFont(p->cellwidth, p->cellheight, gl_rgbcolor(tmp, tmp, tmp), simple_font6x8, p->font);
	gl_setfont(p->cellwidth, p->cellheight, p->font);

	gl_clearscreen(gl_rgbcolor (0, 0, 0));

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close down driver
 */
MODULE_EXPORT void
svga_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	if (p != NULL) {
		if (p->font != NULL)
			free(p->font);
		p->font = NULL;

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);

	vga_setmode(TEXT);
}


/**
 * Return width
 */
MODULE_EXPORT int
svga_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}


/**
 * Return height
 */
MODULE_EXPORT int
svga_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Return cellwidth
 */
MODULE_EXPORT int
svga_cellwidth (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellwidth;
}


/**
 * Return cellheight
 */
MODULE_EXPORT int
svga_cellheight (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->cellheight;
}


/**
 * Clear screen
 */
MODULE_EXPORT void
svga_clear (Driver *drvthis)
{
	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	//vga_waitretrace ();
	gl_clearscreen(gl_rgbcolor(0, 0, 0));
}


/**
 * Flush framebuffer to screen
 */
MODULE_EXPORT void
svga_flush (Driver *drvthis)
{
	/* It's already on the screen ! */
}


/**
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (p->width,p->height).
 */
MODULE_EXPORT void
svga_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;
	char *buffer = strdup(string);
	char *ptr;

	debug(RPT_DEBUG, "%s(%p, %d, %d, \"%s\")", __FUNCTION__, drvthis, x, y, string);

	for (ptr = buffer; *ptr != '\0'; ptr++) {

		if ((unsigned char) *ptr == 255)	// TODO: Is this still necessary ?
			*ptr = '#';
	}
	gl_writen(x * p->cellwidth + p->xoffs, y * p->cellheight + p->yoffs, (ptr - buffer), buffer);
	free(buffer);
}


/**
 * Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (p->width,p->height).
 */
MODULE_EXPORT void
svga_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;
	char buffer[2];

	debug(RPT_DEBUG, "%s(%p, %d, %d, \'%c\')", __FUNCTION__, drvthis, x, y, c);

	switch ((unsigned char) c) {		// TODO: is this still necessary ?
		case '\0':
			c = icon_char;
			break;
		case 255:
			c = '#';
			break;
	}
	buffer[0] = c;
	buffer[1] = '\0';
	gl_writen(x * p->cellwidth + p->xoffs, y * p->cellheight + p->yoffs, 1, buffer);
}


/**
 * Writes a big number, but not.  A bit like the curses driver.
 */
MODULE_EXPORT void
svga_num (Driver *drvthis, int x, int num)
{
	int y, dx;
	char c;

	debug(RPT_DEBUG, "%s(%p, %d, %d)", __FUNCTION__, drvthis, x, num);

	if ((num < 0) || (num > 10))
		return;

	c = (num >= 10) ? ':' : ('0' + num);

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			svga_chr(drvthis, x + dx, y, c);
}


/**
 * Draws a vertical bar; erases entire column onscreen.
 */
MODULE_EXPORT void
svga_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, pattern);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			svga_chr(drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
svga_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	debug(RPT_DEBUG, "%s(%p, %d, %d, %d, %d, %02x)", __FUNCTION__, drvthis, x, y, len, promille, pattern);

	for (pos = 0; pos < len; pos++) {
		if (2 * pos < ((long) promille * len / 500 + 1)) {
			svga_chr(drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}


/**
 * Return a keypress
 */
MODULE_EXPORT const char *
svga_get_key (Driver *drvthis)
{
	static char buf[2] = " ";
	int key = vga_getkey ();

	debug(RPT_DEBUG, "%s(%p)", __FUNCTION__, drvthis);

	if (key <= 0)	/* no key */
		return NULL;

	switch (key) {
		case 0x1B:	/* ESC may introduce special key sequence */
			key = vga_getkey();

			if (key == 0)	/* alone it is "Escape" */
				return "Escape";

			if (key == 0x5B) {	/* 0x1B 0x5B 0x??: cursor keys */
				key = vga_getkey();
				switch (key) {
					case VGAKEY_LEFT:
						return "Left";
					case VGAKEY_UP:
						return "Up";
					case VGAKEY_DOWN:
						return "Down";
					case VGAKEY_RIGHT:
						return "Right";
				}
			}
			/* otherwise key not recognised; ignore it */
			return NULL;
		case '\t':	/* TAB, LF and CR serve as "Enter" */
		case 0x0A:
		case 0x0D:
			return "Enter";
		default:
			buf[0] = (char) key & 0xFF;	/* make sure it fits into a char */
			buf[1] = '\0';
			return (buf[0] != '\0') ? buf : NULL;
	}
	return NULL;
}


/**
 * Returns current contrast (in promille)
 * This is only the locally stored contrast.
 */
MODULE_EXPORT int
svga_get_contrast (Driver *drvthis)
{
        PrivateData *p = drvthis->private_data;

        return p->contrast;
}


/**
 *  Changes screen contrast (in promille)
 */
MODULE_EXPORT void
svga_set_contrast (Driver *drvthis, int promille)
{
	PrivateData *p = drvthis->private_data;
	int contrast;

	/* Check it */
	if ((promille < 0) || (promille > 1000))
		return;

	/* store the software value since there is not get */
	p->contrast = promille;

	/* map range [0, 1000] to [0, 255] */
	contrast = (p->contrast * 255) / 1000;

	/* What to do with it ? */
}


/**
 * Retrieves brightness (in promille)
 */
MODULE_EXPORT int
svga_get_brightness(Driver *drvthis, int state)
{
	PrivateData *p = drvthis->private_data;

	return (state == BACKLIGHT_ON) ? p->brightness : p->offbrightness;
}


/**
 * Sets on/off brightness (in promille)
 */
MODULE_EXPORT void
svga_set_brightness(Driver *drvthis, int state, int promille)
{
	PrivateData *p = drvthis->private_data;

	/* Check it */
	if ((promille < 0) || (promille > 1000))
		return;

	/* store the software value since there is no get */
	if (state == BACKLIGHT_ON) {
		p->brightness = promille;
	}
	else {
		p->offbrightness = promille;
	}
}


/**
 * Sets the backlight on or off.
 * The hardware support any value between 0 and 100.
 */
MODULE_EXPORT void
svga_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;
	int value = (on == BACKLIGHT_ON) ? p->brightness : p->offbrightness;

	/* map range [0, 1000] -> [1, 255] */
	value = value * 255 / 1000;
	if (value <= 0)
		value = 1;

	/* set font color */
	gl_colorfont(p->cellwidth, p->cellheight, gl_rgbcolor(value, value, value), p->font);
}

