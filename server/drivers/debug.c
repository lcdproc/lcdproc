#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#include "lcd.h"
#include "debug.h"
#include "shared/report.h"

//////////////////////////////////////////////////////////////////////////
////////////////////// For Debugging Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO: somehow allow access to the driver->framebuffer to each
// function...

static lcd_logical_driver *debug_drv;

int
debug_init (struct lcd_logical_driver *driver, char *args)
{
	report (RPT_INFO, "debug_init()");

	debug_drv = driver;

	debug_clear ();

	driver->daemonize = 0;

	driver->clear = debug_clear;
	driver->string = debug_string;
	driver->chr = debug_chr;
	driver->vbar = debug_vbar;
	driver->hbar = debug_hbar;
	driver->init_num = debug_init_num;
	driver->num = debug_num;

	driver->init = debug_init;
	driver->close = debug_close;
	driver->flush = debug_flush;
	driver->flush_box = debug_flush_box;
	driver->contrast = debug_contrast;
	driver->backlight = debug_backlight;
	driver->set_char = debug_set_char;
	driver->icon = debug_icon;
	driver->init_vbar = debug_init_vbar;
	driver->init_hbar = debug_init_hbar;
	driver->draw_frame = debug_draw_frame;

	driver->getkey = debug_getkey;

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

void
debug_close ()
{
	report (RPT_INFO, "debug_close()");

	if (debug_drv->framebuf) {
		report (RPT_DEBUG, "frame buffer: %010X", (int) debug_drv->framebuf);
		free (debug_drv->framebuf);
		}

	debug_drv->framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
debug_clear ()
{
	report (RPT_INFO, "clear()");

	memset (debug_drv->framebuf, ' ', debug_drv->wid * debug_drv->hgt);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
debug_flush ()
{
	report (RPT_INFO, "flush()");

	debug_drv->draw_frame ();
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
debug_string (int x, int y, char string[])
{

	int i;

	report(RPT_INFO, "string(%i,%i,%.40s)", x, y, string);

	y --; x --;  // Convert 1-based coords to 0-based...

	for (i = 0; string[i]; i++) {
		debug_drv->framebuf[(y * debug_drv->wid) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
debug_chr (int x, int y, char c)
{
	report (RPT_DEBUG, "char(%i,%i,%c)", x, y, c);

	x--; y--;
	debug_drv->framebuf[(y * debug_drv->wid) + x] = c;
}

int
debug_contrast (int contrast)
{
	report (RPT_INFO, "contrast(%i)", contrast);
	return 0;
}

void
debug_backlight (int on)
{
	report (RPT_INFO, "backlight(%i)", on);
}

void
debug_init_vbar ()
{
	report (LOG_INFO, "init_vbar()");
}

void
debug_init_hbar ()
{
	report (RPT_INFO, "init_hbar()");
}

void
debug_init_num ()
{
	report (RPT_INFO, "init_bignum()");
}

void
debug_num (int x, int num)
{
	report (RPT_INFO, "big number(%i,%i)", x, num);
}

void
debug_set_char (int n, char *dat)
{
	report (RPT_INFO, "set_char(%i,data)", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
debug_vbar (int x, int len)
{
	int y;

	report (RPT_INFO, "vbar(%i,%i)", x, len);

	for (y = debug_drv->hgt; y > 0 && len > 0; y--) {
		debug_chr (x, y, '|');

		len -= debug_drv->cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
debug_hbar (int x, int y, int len)
{
	report (RPT_INFO, "hbar(%i,%i,%i)", x, y, len);

	for (; x < debug_drv->wid && len > 0; x++) {
		debug_chr (x, y, '-');

		len -= debug_drv->cellwid;
	}

}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
debug_icon (int which, char dest)
{
	report (RPT_INFO, "icon(%i,%i", which, dest);
}

void
debug_flush_box (int lft, int top, int rgt, int bot)
{
	report (RPT_INFO, "flush_box(%i,%i,%i,%i)", lft, top, rgt, bot);

	debug_flush ();
}

void
debug_draw_frame (char *dat)
{
	int i, j;

	char out[LCD_MAX_WIDTH];

	report (RPT_INFO, "draw_frame(data)");

	if (!dat)
		return;

//  report (RPT_DEBUG, "Frame (%ix%i): %s", debug_drv->wid, debug_drv->hgt, dat);

	for (i = 0; i < debug_drv->wid; i++) {
		out[i] = '-';
	}
	out[debug_drv->wid] = 0;
	//report (RPT_DEBUG, "+%s+", out);

	for (i = 0; i < debug_drv->hgt; i++) {
		for (j = 0; j < debug_drv->wid; j++) {
			out[j] = dat[j + (i * debug_drv->wid)];
		}
		out[debug_drv->wid] = 0;
		//report (RPT_DEBUG, "|%s|", out);

	}

	for (i = 0; i < debug_drv->wid; i++) {
		out[i] = '-';
	}
	out[debug_drv->wid] = 0;
	//report (RPT_DEBUG, "+%s+", out);

}

char
debug_getkey ()
{
	report (RPT_INFO, "getkey()");
	return 0;
}
