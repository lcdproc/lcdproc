#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "../../shared/str.h"

#include "lcd.h"
#include "curses_drv.h"
#include "drv_base.h"

lcd_logical_driver *curses_drv;

int PAD = 255;
int ELLIPSIS = 7;

//////////////////////////////////////////////////////////////////////////
////////////////////// For Curses Terminal Output ////////////////////////
//////////////////////////////////////////////////////////////////////////

static char icon_char = '@';

int
curses_drv_init (struct lcd_logical_driver *driver, char *args)
{
	char *argv[64];
	int argc;
	int i, j;

	argc = get_args (argv, args, 64);

	for (i = 0; i < argc; i++) {
		if (0 == strcmp (argv[i], "-f") || 0 == strcmp (argv[i], "--forecolor")) {
			if (i + 1 > argc) {
				fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			// TODO:  color display in curses driver!
			printf ("Sorry, colors not yet implemented...\n");
			//strcpy(device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-b") || 0 == strcmp (argv[i], "--backcolor")) {
			if (i + 1 > argc) {
				fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			// TODO:  color display in curses driver!
			printf ("Sorry, colors not yet implemented...\n");
			//strcpy(device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-B") || 0 == strcmp (argv[i], "--backlight")) {
			if (i + 1 > argc) {
				fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			// TODO:  color display in curses driver!
			printf ("Sorry, colors not yet implemented...\n");
			//strcpy(device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			printf ("LCDproc [n]curses driver\n" "\t-f\t--forecolor\tChange the foreground color\n" "\t-b\t--backcolor\tChange the backlight's \"off\" color\n" "\t-B\t--backlight\tChange the backlight's \"on\" color\n" "\t-h\t--help\t\tShow this help information\n");
			return -1;
		} else {
			printf ("Invalid parameter: %s\n", argv[i]);
		}

	}

	curses_drv = driver;

	// Init curses...
	initscr ();
	cbreak ();
	noecho ();
	nodelay (stdscr, TRUE);
	nonl ();
	intrflush (stdscr, FALSE);
	keypad (stdscr, TRUE);

	curses_drv_clear ();

	// Override output functions...
	driver->clear = curses_drv_clear;
	driver->string = curses_drv_string;
	driver->chr = curses_drv_chr;
	driver->vbar = curses_drv_vbar;
	driver->init_vbar = NULL;
	driver->hbar = curses_drv_hbar;
	driver->init_hbar = NULL;
	driver->num = curses_drv_num;
	driver->init_num = curses_drv_init_num;

	driver->init = curses_drv_init;
	driver->close = curses_drv_close;
	driver->flush = curses_drv_flush;
	driver->flush_box = curses_drv_flush_box;
	driver->contrast = NULL;
	driver->backlight = NULL;
	driver->set_char = NULL;
	driver->icon = curses_drv_icon;
	driver->draw_frame = curses_drv_draw_frame;

	driver->getkey = curses_drv_getkey;

	// Change the character used for padding the title bars...
	PAD = '#';
	// Change the character used for "..."
	ELLIPSIS = '~';

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

void
curses_drv_close ()
{
	// Close curses
	clear ();
	move (0, 0);
	refresh ();
	endwin ();

	drv_base_close ();

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
curses_drv_clear ()
{
	clear ();

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
curses_drv_string (int x, int y, char string[])
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
	mvaddstr (y - 1, x - 1, string);
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
curses_drv_chr (int x, int y, char c)
{
	switch (c) {
	case 0:
		c = icon_char;
		break;
	case -1:
		c = '#';
		break;
	}

	mvaddch (y - 1, x - 1, c);
}

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void
curses_drv_init_num ()
{
}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
curses_drv_num (int x, int num)
{
	char c;
	int y, dx;

	c = '0' + num;
//   printf(&c,"%1d",num);

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			curses_drv_chr (x + dx, y, c);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
curses_drv_vbar (int x, int len)
{
	char map[] = "_.,,ooO8";

	int y;
	for (y = lcd.hgt; y > 0 && len > 0; y--) {
		if (len >= lcd.cellhgt)
			curses_drv_chr (x, y, '8');
		else
			curses_drv_chr (x, y, map[len - 1]);

		len -= lcd.cellhgt;
	}

//  move(4-len/lcd.cellhgt, x-1);
//  vline(0, len/lcd.cellhgt);

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
curses_drv_hbar (int x, int y, int len)
{
	for (; x <= lcd.wid && len > 0; x++) {
		if (len >= lcd.cellwid)
			curses_drv_chr (x, y, '=');
		else
			curses_drv_chr (x, y, '-');

		len -= lcd.cellwid;
	}

//  move(y-1, x-1);
//  hline(0, len/lcd.cellwid);
}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
curses_drv_icon (int which, char dest)
{
	if (dest == 0)
		switch (which) {
		case 0:
			icon_char = '+';
			break;
		case 1:
			icon_char = '*';
			break;
		default:
			icon_char = '#';
			break;
		}

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
curses_drv_flush ()
{
	curses_drv_draw_frame (curses_drv->framebuf);
}

void
curses_drv_flush_box (int lft, int top, int rgt, int bot)
{
	curses_drv_flush ();
}

void
curses_drv_draw_frame (char *dat)
{
//  border(0, 0, 0, 0,  0, 0, lcd.wid+1, lcd.hgt+1);

	refresh ();

}

char
curses_drv_getkey ()
{
	int i;

	i = getch ();

	if (i == KEY_LEFT)
		return 'D';
	if (i == KEY_UP)
		return 'B';
	if (i == KEY_DOWN)
		return 'C';
	if (i == KEY_RIGHT)
		return 'A';

	if (i != ERR)
		return i;
	else
		return 0;

}
