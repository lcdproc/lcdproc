/*
 * ncurses driver
 *
 * Display LCD display at top left of terminal screen
 * using ncurses.
 *
 */

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
#include <ctype.h>
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "shared/str.h"

#include "lcd.h"
#include "curses_drv.h"
#include "drv_base.h"

lcd_logical_driver *curses_drv;

// Character used for title bars...
#define PAD '#'
// #define PAD ACS_BLOCK

int ELLIPSIS = 7;

//////////////////////////////////////////////////////////////////////////
////////////////////// For Curses Terminal Output ////////////////////////
//////////////////////////////////////////////////////////////////////////

static char icon_char = '@';
static WINDOW *lcd_win;

chtype get_color (char *colorstr) {
	if (strcasecmp(colorstr, "red") == 0)
		return COLOR_RED;
	else if (strcasecmp(colorstr, "black") == 0)
		return COLOR_BLACK;
	else if (strcasecmp(colorstr, "green") == 0)
		return COLOR_GREEN;
	else if (strcasecmp(colorstr, "yellow") == 0)
		return COLOR_YELLOW;
	else if (strcasecmp(colorstr, "blue") == 0)
		return COLOR_BLUE;
	else if (strcasecmp(colorstr, "magenta") == 0)
		return COLOR_MAGENTA;
	else if (strcasecmp(colorstr, "cyan") == 0)
		return COLOR_CYAN;
	else if (strcasecmp(colorstr, "white") == 0)
		return COLOR_WHITE;
	else
		return -1;
}

/*
 * A few different nice pairs of colors to use...
 *
 */

#define DEFAULT_FOREGROUND_COLOR COLOR_CYAN
#define DEFAULT_BACKGROUND_COLOR COLOR_BLUE

// #define DEFAULT_FOREGROUND_COLOR COLOR_WHITE
// #define DEFAULT_BACKGROUND_COLOR COLOR_BLUE

// #define DEFAULT_FOREGROUND_COLOR COLOR_WHITE
// #define DEFAULT_BACKGROUND_COLOR COLOR_RED

chtype
set_foreground_color (char * buf) {
	chtype color;

	if ((color = get_color(buf)) < 0)
		color = DEFAULT_FOREGROUND_COLOR;

	return color;
}

chtype
set_background_color (char * buf) {
	chtype color;

	if ((color = get_color(buf)) < 0)
		color = DEFAULT_BACKGROUND_COLOR;

	return color;
}

/*
 * What position (X,Y) to start the left top corner at...
 *
 */

#define TOP_LEFT_X 7
#define TOP_LEFT_Y 7

int current_color_pair, current_border_pair, curses_backlight_state = 0;

int
curses_drv_init (struct lcd_logical_driver *driver, char *args)
{
	char *argv[64];
	int argc;
	int i;
	char buf[64], *p;

	// Colors....
	chtype	back_color = DEFAULT_BACKGROUND_COLOR,
		fore_color = DEFAULT_FOREGROUND_COLOR,
		backlight_color = DEFAULT_BACKGROUND_COLOR;

	// Screen position (top left)
	int	screen_begx = TOP_LEFT_X,
		screen_begy = TOP_LEFT_Y;

	curses_drv = driver;

	memset(&argv, '\0', sizeof(argv));
	argc = get_args (argv, args, 64);

	for (i = 0; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			p++;
			switch (*p) {
				case 'f':
					if (i + 1 >= argc) {
						fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					strncpy(buf, argv[++i], sizeof(buf));
					fore_color = set_foreground_color(buf);
					break;
				case 'b':
					if (i + 1 >= argc) {
						fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					strncpy(buf, argv[++i], sizeof(buf));
					back_color = set_background_color(buf);
					break;
				case 'B':
					if (i + 1 >= argc) {
						fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					strncpy(buf, argv[++i], sizeof(buf));
					backlight_color = set_background_color(buf);
					break;
				case 'h':
					printf ("LCDproc [n]curses driver\n"
							"\t-f\tChange the foreground color\n"
							"\t-b\tChange the background color\n"
							"\t-t\tChange the type of screen (20x2, etc.)\n"
							// "\t-b\t--backcolor\tChange the backlight's \"off\" color\n"
							// "\t-B\t--backlight\tChange the backlight's \"on\" color\n"
							"\t-h\t--help\t\tShow this help information\n");
					return -1;
					break;
				case 't':
					if (i + 1 > argc) {
						fprintf (stderr, "curses_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					i++;
					p = argv[i];

					if (isdigit((unsigned int) *p) && isdigit((unsigned int) *(p+1))) {
						int wid, hgt;

						wid = ((*p - '0') * 10) + (*(p+1) - '0');
						p += 2;

						if (*p != 'x')
							break;

						p++;
						if (!isdigit((unsigned int) *p))
							break;

						hgt = (*p - '0');

						curses_drv->wid = wid;
						curses_drv->hgt = hgt;
						}
					break;
				default:
					return -1;
					break;
			}
		}
	}

	// Init curses...
	initscr ();
	cbreak ();
	noecho ();
	nodelay (stdscr, TRUE);
	nonl ();
	intrflush (stdscr, FALSE);
	keypad (stdscr, TRUE);
	lcd_win = newwin(curses_drv->hgt + 2,
			 curses_drv->wid + 2,
			 screen_begy,
			 screen_begx);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, back_color, fore_color);
		init_pair(2, fore_color, back_color);
		init_pair(3, COLOR_WHITE, back_color);
		init_pair(4, fore_color, backlight_color);
		init_pair(5, COLOR_WHITE, backlight_color);

		current_color_pair = 2;
		current_border_pair = 3;
	}

	curses_drv_clear ();

	driver->daemonize = 0; // don't daemonize...

	// Override output functions...
	driver->clear = curses_drv_clear;
	driver->string = curses_drv_string;
	driver->chr = curses_drv_chr;
	driver->vbar = curses_drv_vbar;
	//driver->init_vbar = NULL;
	driver->hbar = curses_drv_hbar;
	//driver->init_hbar = NULL;
	driver->num = curses_drv_num;
	driver->init_num = curses_drv_init_num;

	driver->init = curses_drv_init;
	driver->close = curses_drv_close;
	driver->flush = curses_drv_flush;
	driver->flush_box = curses_drv_flush_box;
	//driver->contrast = NULL;
	driver->backlight = curses_drv_backlight;
	//driver->set_char = NULL;
	driver->icon = curses_drv_icon;
	driver->draw_frame = curses_drv_draw_frame;

	driver->getkey = curses_drv_getkey;

	// Change the character used for "..."
	ELLIPSIS = '~';

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

static void
curses_drv_wborder (WINDOW *win) {
	int x, y;
	char buf[128];

	if (has_colors()) {
		wcolor_set(win, current_border_pair, NULL);
		wattron(win, COLOR_PAIR(current_border_pair) | A_BOLD);
	}

	box(win, 0, 0);

	if (has_colors()) {
		wcolor_set(win, current_color_pair, NULL);
		wattron(win, COLOR_PAIR(current_color_pair));
		wattroff(win, A_BOLD);
	}
}

void
curses_drv_close ()
{
	// Note that the program leaves a screen on
	// the display to be left behind after closing;
	// so don't clear...
	//
	// Close curses
	wrefresh (lcd_win);
	delwin (lcd_win);

	move (0, 0);
	endwin ();

	if (curses_drv->framebuf != NULL)
		free (curses_drv->framebuf);

	curses_drv->framebuf = NULL;

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
curses_drv_clear ()
{
	wbkgdset(lcd_win, COLOR_PAIR(current_color_pair) | ' ');
	curses_drv_wborder (lcd_win);
	werase (lcd_win);
}

#define ValidX(a) { if (x > curses_drv->wid) { x = curses_drv->wid; } else x < 1 ? 1 : x; }
#define ValidY(a) { if (y > curses_drv->hgt) { y = curses_drv->hgt; } else y < 1 ? 1 : y; }

void
curses_drv_backlight (int on)
{
	if (curses_backlight_state == on)
		return;

	// no backlight: pairs 2, 3
	// backlight:    pairs 4, 5

	switch (on) {
		case 0:
			curses_backlight_state = 0;
			current_color_pair = 2;
			current_border_pair = 3;
			break;
		case 1:
			curses_backlight_state = 1;
			current_color_pair = 4;
			current_border_pair = 5;
			break;
		default:
			return;
			break;
	}
	curses_drv_clear;
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
curses_drv_string (int x, int y, char *string)
{
	int i;
	unsigned char *p;

	ValidX(x);
	ValidY(y);

	p = string;

	// Convert NULLs and 0xFF in string to
	// valid printables...
	while (*p) {
		switch (*p) {
			//case 0: -- can never happen?
			//	*p = icon_char;
			//	break;
			case 255:
				*p = PAD;
				break;
		}
		p++;
	}

	mvwaddstr (lcd_win, y, x, string);
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
curses_drv_chr (int x, int y, char c)
{
	ValidX(x);
	ValidY(y);

	switch (c) {
		case 0:
			c = icon_char;
			break;
		case -1:	// translates to 255 (ouch!)
			c = PAD;
			break;
		//default:
		//	normal character...
	}

	mvwaddch (lcd_win, y, x, c);
}

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void
curses_drv_init_num ()
{
	;
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
	//char map[] = "_.,,ooO8";
	char map[] = { ACS_S9, ACS_S9, ACS_S7, ACS_S7, ACS_S3, ACS_S3, ACS_S1, ACS_S1 };
	int y;

	ValidX(x);

#define MAX_LINES (curses_drv->cellhgt * curses_drv->hgt)

	len = len > (MAX_LINES - 1) ? (MAX_LINES - 1) : len;
	len = len < 0 ? 0 : len;

	// len is the length of the bar (in pixels/scanlines)
	// y is one character line (cellhgt pixels/scanlines)

	for (y = curses_drv->hgt; y > 0 && len > 0; y--) {

		if (len >= curses_drv->cellhgt) {
			// write a "full" block to the screen...
			//curses_drv_chr (x, y, '8');
			curses_drv_chr (x, y, ACS_BLOCK);
			len -= curses_drv->cellhgt;
		}
		else {
			// write a partial block...
			curses_drv_chr (x, y, map[len-1]);
			break;
		}

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
	for (; x <= curses_drv->wid && len > 0; x++) {
		if (len >= curses_drv->cellwid)
			curses_drv_chr (x, y, '=');
		else
			curses_drv_chr (x, y, '-');

		len -= curses_drv->cellwid;
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
				icon_char = PAD;
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
	curses_drv_wborder (lcd_win);
	wrefresh (lcd_win);
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
