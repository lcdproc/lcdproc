/*
   This is a hack of the curses driver.  It's my first attempt at SVGALIB
   programming so please send comments to <smh@remove_this@dr.com>.

   Simon Harrison.  27 Dec 1999.

   IMPORTANT NOTE:  In order to make this work properly LCDd should not be
   allowed to fork (-f option), otherwise k/b input won't work.

   Changes:
   9 Jan 2000:  Figured out svgalib driver needs a hack to main.c just like
                curses.  Maybe this should be fixed with another function call
                added to the API?
   4 Jan 2000:  Added 5x7 font, fixed flicker.

*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <vga.h>
#include <vgagl.h>

#include "config.h"

#include "lcd.h"
#include "svgalib_drv.h"

/* Small font */

const unsigned char simple_font5x7[] = {
/*  ascii '0' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '1' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '2' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '3' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '4' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '5' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '6' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '7' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '8' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '9' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '10' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '11' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '12' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '13' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '14' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '15' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '16' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '17' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '18' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '19' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '20' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '21' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '22' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '23' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '24' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '25' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '26' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '27' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '28' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '29' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '30' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '31' ( ) */ 31, 31, 31, 31, 31, 31, 31,

/*  ascii '32' ( ) */ 31, 31, 31, 31, 31, 31, 31,
/*  ascii '33' (!) */ 27, 27, 27, 27, 31, 31, 27,
/*  ascii '34' (") */ 21, 21, 21, 31, 31, 31, 31,
/*  ascii '35' (#) */ 21, 21, 0, 21, 0, 21, 21,
/*  ascii '36' ($) */ 27, 1, 26, 17, 11, 16, 27,
/*  ascii '37' (%) */ 28, 12, 23, 27, 29, 6, 7,
/*  ascii '38' (&) */ 25, 22, 26, 29, 10, 22, 9,
/*  ascii '39' (') */ 25, 27, 29, 31, 31, 31, 31,
/*  ascii '40' (() */ 23, 27, 29, 29, 29, 27, 23,
/*  ascii '41' ()) */ 29, 27, 23, 23, 23, 27, 29,
/*  ascii '42' (*) */ 31, 27, 10, 17, 10, 27, 31,
/*  ascii '43' (+) */ 31, 27, 27, 0, 27, 27, 31,
/*  ascii '44' (,) */ 31, 31, 31, 31, 25, 27, 29,
/*  ascii '45' (-) */ 31, 31, 31, 0, 31, 31, 31,
/*  ascii '46' (.) */ 31, 31, 31, 31, 31, 25, 25,
/*  ascii '47' (/) */ 31, 15, 23, 27, 29, 30, 31,
/*  ascii '48' (0) */ 17, 14, 6, 10, 12, 14, 17,
/*  ascii '49' (1) */ 27, 25, 27, 27, 27, 27, 17,
/*  ascii '50' (2) */ 17, 14, 15, 23, 27, 29, 0,
/*  ascii '51' (3) */ 0, 23, 27, 23, 15, 14, 17,
/*  ascii '52' (4) */ 23, 19, 21, 22, 0, 23, 23,
/*  ascii '53' (5) */ 0, 30, 16, 15, 15, 14, 17,
/*  ascii '54' (6) */ 19, 29, 30, 16, 14, 14, 17,
/*  ascii '55' (7) */ 0, 15, 23, 27, 29, 29, 29,
/*  ascii '56' (8) */ 17, 14, 14, 17, 14, 14, 17,
/*  ascii '57' (9) */ 17, 14, 14, 1, 15, 23, 25,
/*  ascii '58' (:) */ 31, 25, 25, 31, 25, 25, 31,
/*  ascii '59' (;) */ 31, 25, 25, 31, 25, 27, 29,
/*  ascii '60' (<) */ 23, 27, 29, 30, 29, 27, 23,
/*  ascii '61' (=) */ 31, 31, 0, 31, 0, 31, 31,
/*  ascii '62' (>) */ 29, 27, 23, 15, 23, 27, 29,
/*  ascii '63' (?) */ 17, 14, 15, 23, 27, 31, 27,
/*  ascii '64' (@) */ 17, 14, 15, 9, 10, 10, 17,
/*  ascii '65' (A) */ 17, 14, 14, 14, 0, 14, 14,
/*  ascii '66' (B) */ 16, 14, 14, 16, 14, 14, 16,
/*  ascii '67' (C) */ 16, 14, 30, 30, 30, 14, 16,
/*  ascii '68' (D) */ 24, 22, 14, 14, 14, 22, 24,
/*  ascii '69' (E) */ 0, 30, 30, 16, 30, 30, 0,
/*  ascii '70' (F) */ 0, 30, 30, 16, 30, 30, 30,
/*  ascii '71' (G) */ 17, 14, 30, 2, 14, 14, 1,
/*  ascii '72' (H) */ 14, 14, 14, 0, 14, 14, 14,
/*  ascii '73' (I) */ 17, 27, 27, 27, 27, 27, 17,
/*  ascii '74' (J) */ 3, 23, 23, 23, 23, 22, 25,
/*  ascii '75' (K) */ 14, 22, 26, 28, 26, 22, 14,
/*  ascii '76' (L) */ 30, 30, 30, 30, 30, 30, 0,
/*  ascii '77' (M) */ 14, 4, 10, 10, 14, 14, 14,
/*  ascii '78' (N) */ 14, 14, 12, 10, 6, 14, 14,
/*  ascii '79' (O) */ 17, 14, 14, 14, 14, 14, 17,
/*  ascii '80' (P) */ 16, 14, 14, 16, 30, 30, 30,
/*  ascii '81' (Q) */ 17, 14, 14, 14, 10, 22, 9,
/*  ascii '82' (R) */ 16, 14, 14, 16, 26, 22, 14,
/*  ascii '83' (S) */ 1, 30, 30, 17, 15, 15, 16,
/*  ascii '84' (T) */ 0, 27, 27, 27, 27, 27, 27,
/*  ascii '85' (U) */ 14, 14, 14, 14, 14, 14, 17,
/*  ascii '86' (V) */ 14, 14, 14, 14, 14, 21, 27,
/*  ascii '87' (W) */ 14, 14, 14, 14, 10, 10, 21,
/*  ascii '88' (X) */ 14, 14, 21, 27, 21, 14, 14,
/*  ascii '89' (Y) */ 14, 14, 14, 21, 27, 27, 27,
/*  ascii '90' (Z) */ 0, 15, 23, 27, 29, 30, 0,
/*  ascii '91' ([) */ 17, 29, 29, 29, 29, 29, 17,
/*  ascii '92' (\) */ 31, 30, 29, 27, 23, 15, 31,
/*  ascii '93' (]) */ 17, 23, 23, 23, 23, 23, 17,
/*  ascii '94' (^) */ 27, 21, 14, 31, 31, 31, 31,
/*  ascii '95' (_) */ 31, 31, 31, 31, 31, 31, 0,
/*  ascii '96' (`) */ 29, 27, 23, 31, 31, 31, 31,
/*  ascii '97' (a) */ 31, 31, 17, 15, 1, 14, 1,
/*  ascii '98' (b) */ 30, 30, 18, 12, 14, 14, 16,
/*  ascii '99' (c) */ 31, 31, 17, 30, 30, 14, 17,
/*  ascii '100' (d) */ 15, 15, 9, 6, 14, 14, 1,
/*  ascii '101' (e) */ 31, 31, 17, 14, 0, 30, 17,
/*  ascii '102' (f) */ 19, 13, 29, 24, 29, 29, 29,
/*  ascii '103' (g) */ 31, 1, 14, 14, 1, 15, 17,
/*  ascii '104' (h) */ 30, 30, 18, 12, 14, 14, 14,
/*  ascii '105' (i) */ 27, 31, 25, 27, 27, 27, 17,
/*  ascii '106' (j) */ 23, 31, 19, 23, 23, 22, 25,
/*  ascii '107' (k) */ 30, 30, 22, 26, 28, 26, 22,
/*  ascii '108' (l) */ 25, 27, 27, 27, 27, 27, 17,
/*  ascii '109' (m) */ 31, 31, 20, 10, 10, 14, 14,
/*  ascii '110' (n) */ 31, 31, 18, 12, 14, 14, 14,
/*  ascii '111' (o) */ 31, 31, 17, 14, 14, 14, 17,
/*  ascii '112' (p) */ 31, 31, 16, 14, 16, 30, 30,
/*  ascii '113' (q) */ 31, 31, 9, 6, 1, 15, 15,
/*  ascii '114' (r) */ 31, 31, 18, 12, 30, 30, 30,
/*  ascii '115' (s) */ 31, 31, 17, 30, 17, 15, 16,
/*  ascii '116' (t) */ 29, 29, 24, 29, 29, 13, 19,
/*  ascii '117' (u) */ 31, 31, 14, 14, 14, 6, 9,
/*  ascii '118' (v) */ 31, 31, 14, 14, 14, 21, 27,
/*  ascii '119' (w) */ 31, 31, 14, 14, 10, 10, 21,
/*  ascii '120' (x) */ 31, 31, 14, 21, 27, 21, 14,
/*  ascii '121' (y) */ 31, 31, 14, 14, 1, 15, 17,
/*  ascii '122' (z) */ 31, 31, 0, 23, 27, 29, 0,
/*  ascii '123' ({) */ 23, 27, 27, 29, 27, 27, 23,
/*  ascii '124' (|) */ 27, 27, 27, 27, 27, 27, 27,
/*  ascii '125' (}) */ 29, 27, 27, 23, 27, 27, 29,
/*  ascii '126' (~) */ 31, 27, 23, 0, 23, 27, 31
};

/* <start user definable!!!> */
#define SVGALIB_FONT_VER 10	  /* vertical spacing between lines (pixels) */
#define SVGALIB_Y_OFFSET 40	  /* distance from the top of the screen (pixels) */
/* <end user defineable!!!> */

/* No, I don't understand SVGALIB key mappings or a neat way of doing this
 * Cursor keys manifest themselves as 3 byte excapes in svgalib: 27,91
 * followed by one of the codes below.
 */
#define VGAKEY_UP 65
#define VGAKEY_DOWN 66
#define VGAKEY_RIGHT 67
#define VGAKEY_LEFT 68

int SVGALIB_PAD = 255;
int SVGALIB_ELLIPSIS = 7;

void *SVGALIB_font;				  /* normal font   */
void *SVGALIB_highfont;			  /* highlighted font (not used yet) */
void *SVGALIB_warnfont;			  /* warning font (not used yet)    */

/*
  Setting SVGALIB_figure_mappings will trap all keypresses and write the
  vga_getkey() codes out to a file.  You can then mod the getkey() function
  at the bottom of the file to your liking.

  Ahem... don't do this unless you can get to another console to kill the
  process after.
*/
int SVGALIB_figure_mappings = 0;

int width, height;

/*************************************************************************
********************** For Output on SVGALIB screen **********************
*************************************************************************/

void
ExpandGroovyFont (int w, int ht, unsigned char col, const unsigned char *fnt, unsigned char *ptr)
/* Expand groovy 5x7 font into an area of memory */
{
	int x, n, y;
	unsigned char mask;
	unsigned char base;
	unsigned char *p;
	p = ptr;
	for (n = 0; n < 127; n++) {
		for (y = 0; y < ht; y++) {
			mask = 1;
			base = fnt[n * ht + y];
			for (x = 0; x < w; x++) {
				if (base & mask) {
					*p = 0;
				} else {
					*p = col;
				}
				p++;
				mask *= 2;
			}
		}
	}
}

void
spaced_gl_writen (int x, int y, int count, char *text)
/*
 * Like gl_write but this one gets the spacing correct.
 */
{
	int i;
	for (i = 0; i < count; i++) {
		gl_writen (20 + x + i * 6, y, 1, &text[i]);
	}
}

static char icon_char = '@';

/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "svgalib_drv_";


/******************************************************************************
 * Init driver
 */
MODULE_EXPORT int
svgalib_drv_init (Driver *drvthis)
{
	int VGAMODE;

	vga_init ();
	VGAMODE = G320x200x256;		  /* Default mode. */

	if (!vga_hasmode (VGAMODE)) {
		printf ("320x200x256 Mode not available.\n");
		return -1;
	} else {
		vga_setmode (VGAMODE);
		gl_setcontextvga (VGAMODE);	/* Physical screen context. */
		gl_setrgbpalette ();

		/* get the font */
		SVGALIB_font = malloc (256 * 8 * 8 * 1);
		ExpandGroovyFont (5, 7, gl_rgbcolor (0, 255, 0), simple_font5x7, SVGALIB_font);
		gl_setfont (5, 7, SVGALIB_font);

	}

	gl_clearscreen (gl_rgbcolor (0, 0, 0));

	/* Change the character used for padding the title bars... */
	SVGALIB_PAD = '#';
	/* Change the character used for "..." */
	SVGALIB_ELLIPSIS = '~';

	/* Determine the size of the screen */
	width = drvthis->request_display_width();
	height = drvthis->request_display_height();
	if( width <= 0 || height <= 0 ) {
		width = LCD_DEFAULT_WIDTH;
		height = LCD_DEFAULT_HEIGHT;
	}

	return 0;
}

/******************************************************************************
 * Close down driver
 */
MODULE_EXPORT void
svgalib_drv_close (Driver *drvthis)
{
	vga_setmode (TEXT);
}

/******************************************************************************
 * Return width
 */
MODULE_EXPORT int
svgalib_drv_width (Driver *drvthis)
{
	return width;
}

/******************************************************************************
 * Return height
 */
MODULE_EXPORT int
svgalib_drv_height (Driver *drvthis)
{
	return height;
}

/******************************************************************************
 * Clear screen
 */
MODULE_EXPORT void
svgalib_drv_clear (Driver * drvthis)
{
	vga_waitretrace ();
	gl_clearscreen (gl_rgbcolor (0, 0, 0));
}

/******************************************************************************
 * Flush framebuffer to screen
 */
MODULE_EXPORT void
svgalib_drv_flush (Driver *drvthis)
{
	/* It's already in screen ! */
}

/******************************************************************************
 * Prints a string on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
svgalib_drv_string (Driver *drvthis, int x, int y, char string[])
{
	int i;
	unsigned char *c;
	for (i = 0; string[i]; i++) {
		c = &string[i];
		switch (*c) {
		case 0:
			*c = icon_char;
			break;
		case 255:
			*c = '#';
			break;
		}
	}
	gl_setfont (5, 7, SVGALIB_font);
	spaced_gl_writen (x * 8, y * SVGALIB_FONT_VER + SVGALIB_Y_OFFSET, i, string);
}

/******************************************************************************
 * Prints a character on the lcd display, at position (x,y).  The
 * upper-left is (1,1), and the lower right should be (20,4).
 */
MODULE_EXPORT void
svgalib_drv_chr (Driver *drvthis, int x, int y, char c)
{
	char buffer[2];
	switch (c) {
	case 0:
		c = icon_char;
		break;
	case -1:
		c = '#';
		break;
	}
	buffer[0] = c;
	buffer[1] = 0;
	gl_setfont (5, 7, SVGALIB_font);
	spaced_gl_writen (x * 8, y * SVGALIB_FONT_VER + SVGALIB_Y_OFFSET, 1, buffer);
}

/******************************************************************************
 * Writes a big number, but not.  A bit like the curses driver.
 */
MODULE_EXPORT void
svgalib_drv_num (Driver *drvthis, int x, int num)
{
	char c;
	int y, dx;

	c = '0' + num;

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			svgalib_drv_chr (drvthis, x + dx, y, c);
}

/******************************************************************************
 * Draws a vertical bar; erases entire column onscreen.
 */
MODULE_EXPORT void
svgalib_drv_vbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			svgalib_drv_chr (drvthis, x, y-pos, '|');
		} else {
			; /* print nothing */
		}
	}
}

/******************************************************************************
 * Draws a horizontal bar to the right.
 */
MODULE_EXPORT void
svgalib_drv_hbar (Driver *drvthis, int x, int y, int len, int promille, int pattern)
{
	int pos;

	for ( pos=0; pos<len; pos++ ) {
		if( 2 * pos < ((long) promille * len / 500 + 1) ) {
			svgalib_drv_chr (drvthis, x+pos, y, '-');
		} else {
			; /* print nothing */
		}
	}
}

/******************************************************************************
 * Return a keypress
 */
MODULE_EXPORT char *
svgalib_drv_get_key (Driver *drvthis)
{
	int i;
	char *s;
	static char buf[2];

	s = "";

	i = vga_getkey ();
	if (i) {
		switch (i) {
			/* map the cursor keys to something sensible. */
		case 27:
			i = vga_getkey ();
			if (i == 91) {
				i = vga_getkey ();
				switch (i) {
				case VGAKEY_LEFT:
					s = "ArrowLeft";
					break;
				case VGAKEY_UP:
					s = "ArrowUp";
					break;
				case VGAKEY_DOWN:
					s = "ArrowDown";
					break;
				case VGAKEY_RIGHT:
					s = "ArrowRight";
					break;
				/* otherwise key not recognised */
				}
			}
			break;

			/*
			   Change this bit to your liking.  I only set these values because
			   they tie in with my keypads use of the keyboard controller chip
			   see http://www.gofree.co.uk/home/smh/mp3 for details.
			   Having said that try to make sure four keys map to D,B,C,A so you'll
			   be able to quit LCDproc from the emulation.

			 */
		case 13:
			s = "Enter";
			break;
		default:
			buf[0] = i;
			buf[1] = 0;
			s = buf;
		}
	}
	return s;
}
