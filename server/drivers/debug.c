#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <syslog.h>

#include "lcd.h"
#include "debug.h"

//////////////////////////////////////////////////////////////////////////
////////////////////// For Debugging Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO: somehow allow access to the driver->framebuffer to each
// function...

static lcd_logical_driver *debug_drv;

int
debug_init (struct lcd_logical_driver *driver, char *args)
{
	syslog(LOG_DEBUG, "debug_init");

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
	syslog(LOG_DEBUG, "debug_close()");

	if (debug_drv->framebuf) {
		printf ("frame buffer: %010X", (int) debug_drv->framebuf);
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
	syslog(LOG_DEBUG, "clear()");

	memset (debug_drv->framebuf, ' ', debug_drv->wid * debug_drv->hgt);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
debug_flush ()
{
	syslog(LOG_DEBUG, "flush()");

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
	char buf[64];

	snprintf(buf, sizeof(buf), "string(%i,%i): %s", x, y, string);
	syslog (LOG_DEBUG, buf);

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
	char buf[64];

	snprintf(buf, sizeof(buf), "character(%i,%i): %c", x, y, c);
	syslog (LOG_DEBUG, buf);

	x--; y--;
	debug_drv->framebuf[(y * debug_drv->wid) + x] = c;
}

int
debug_contrast (int contrast)
{
	syslog(LOG_DEBUG, "contrast: %i", contrast);
	return 0;
}

void
debug_backlight (int on)
{
	if (on)
		syslog(LOG_DEBUG, "backlight turned on");
	else
		syslog(LOG_DEBUG, "backlight turned off");
}

void
debug_init_vbar ()
{
	syslog(LOG_DEBUG, "Init vertical bars");
}

void
debug_init_hbar ()
{
	syslog(LOG_DEBUG, "Init horizontal bars");
}

void
debug_init_num ()
{
	syslog(LOG_DEBUG, "Big numbers");
}

void
debug_num (int x, int num)
{
	char buf[64];

	snprintf(buf, sizeof(buf), "big number(%i): %i", x, num);
	syslog (LOG_DEBUG, buf);
}

void
debug_set_char (int n, char *dat)
{
	syslog(LOG_DEBUG, "set character %i", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
debug_vbar (int x, int len)
{
	int y;

	syslog (LOG_DEBUG, "vbar(%i): len = %i", x, len);

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
	syslog(LOG_DEBUG, "hbar(%i,%i): len = %i", x, y, len);

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
	syslog (LOG_DEBUG, "char %i = icon %i", dest, which);
}

void
debug_flush_box (int lft, int top, int rgt, int bot)
{
	syslog (LOG_DEBUG, "flush_box(%i,%i) - (%i,%i)", lft, top, rgt, bot);

	debug_flush ();
}

void
debug_draw_frame (char *dat)
{
	int i, j;

	char out[LCD_MAX_WIDTH];

	syslog (LOG_DEBUG, "draw_frame()");

	if (!dat)
		return;

//  printf("Frame (%ix%i): \n%s\n", debug_drv->wid, debug_drv->hgt, dat);

	for (i = 0; i < debug_drv->wid; i++) {
		out[i] = '-';
	}
	out[debug_drv->wid] = 0;
	//printf ("+%s+\n", out);

	for (i = 0; i < debug_drv->hgt; i++) {
		for (j = 0; j < debug_drv->wid; j++) {
			out[j] = dat[j + (i * debug_drv->wid)];
		}
		out[debug_drv->wid] = 0;
		//printf ("|%s|\n", out);

	}

	for (i = 0; i < debug_drv->wid; i++) {
		out[i] = '-';
	}
	out[debug_drv->wid] = 0;
	//printf ("+%s+\n", out);

}

char
debug_getkey ()
{
	syslog(LOG_DEBUG, "getkey");
	return 0;
}
