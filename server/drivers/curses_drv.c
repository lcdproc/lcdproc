/*  This is the LCDproc driver for ncurses.
    It displays an emulated LCD display at top left of terminal screen
    using ncurses.

    Copyright (C) ?????? ?????????

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 */

/*Configfile support added by Rene Wagner (C) 2001*/

/*
Different implementations of (n)curses available on:
OpenBSD: 
 	http://www.openbsd.org/cgi-bin/cvsweb/src/lib/libcurses/
	ncurses
NetBSD: 
	http://cvsweb.netbsd.org/bsdweb.cgi/basesrc/lib/libcurses/
	curses : does not define ACS_S3, ACS_S7, wcolor_set() or redrawwin().
	it is possible to make a: 
		#define ACS_S3 (_acs_char['p'])
		#define ACS_S7 (_acs_char['r'])
FreeBSD:
	http://www.freebsd.org/cgi/cvsweb.cgi/src/
	ncurses
RedHat, Debian, (most distros) Linux:
	ncurses
SunOS (5.5.1):
	curses : does not define ACS_S3, ACS_S7 or wcolor_set().
	it is possible to make a: 
		#define ACS_S3 (acs_map['p'])
		#define ACS_S7 (acs_map['r'])
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

#include "render.h"
#include "lcd.h"
#include "curses_drv.h"
#include "shared/report.h"
#include "configfile.h"

// ACS_S9 and ACS_S1 are defined as part of XSI Curses standard, Issue 4.
// However, ACS_S3 and ACS_S7 are not; these definitions were created to support
// commonly available graphics found in many terminfo definitions.  The acsc character
// used for ACS_S3 is 'p'; the acsc character used for ACS_S7 is 'r'.
//
// Other systems may very likely support these characters; however,
// their names were invented for ncurses.
#ifndef ACS_S3
# ifdef CURSES_HAS_ACS_MAP
#  define ACS_S3 (acs_map['p'])
# else
#  ifdef CURSES_HAS__ACS_CHAR
#   define ACS_S3 (_acs_char['p'])
#  else
#   define ACS_S3 ACS_S1    // Last resort
#  endif
# endif 
#endif

#ifndef ACS_S7
# ifdef CURSES_HAS_ACS_MAP
#  define ACS_S7 (acs_map['r'])
# else
#  ifdef CURSES_HAS__ACS_CHAR
#   define ACS_S7 (_acs_char['r'])
#  else
#   define ACS_S7 ACS_S9    // Last resort
#  endif
# endif 
#endif

lcd_logical_driver *curses_drv;

// Character used for title bars...
#define PAD '#'
// #define PAD ACS_BLOCK

int ELLIPSIS = 7;
void curses_drv_restore_screen (void);

//////////////////////////////////////////////////////////////////////////
////////////////////// For Curses Terminal Output ////////////////////////
//////////////////////////////////////////////////////////////////////////

static char icon_char = '@';
static WINDOW *lcd_win;

/*this is really ugly ;) but works ;)*/
static char num_icon [10][4][3] = 	{{{' ','_',' '}, /*0*/
					  {'|',' ','|'},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ',' ',' '},/*1*/
					  {' ',' ','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*2*/
					  {' ','_','|'},
					  {'|','_',' '},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*3*/
					  {' ','_','|'},
					  {' ','_','|'},
					  {' ',' ',' '}},
					  {{' ',' ',' '},/*4*/
					  {'|','_','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*5*/
					  {'|','_',' '},
					  {' ','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*6*/
					  {'|','_',' '},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*7*/
					  {' ',' ','|'},
					  {' ',' ','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*8*/
					  {'|','_','|'},
					  {'|','_','|'},
					  {' ',' ',' '}},
					  {{' ','_',' '},/*9*/
					  {'|','_','|'},
					  {' ','_','|'},
					  {' ',' ',' '}}};
/*end of ugly code ;) Rene Wagner*/

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
	char buf[256];

	// Colors....
	chtype	back_color = DEFAULT_BACKGROUND_COLOR,
		fore_color = DEFAULT_FOREGROUND_COLOR,
		backlight_color = DEFAULT_BACKGROUND_COLOR;

	// Screen position (top left)
	int	screen_begx = CONF_DEF_TOP_LEFT_X,
		screen_begy = CONF_DEF_TOP_LEFT_Y;

	curses_drv = driver;

	// TODO: replace DriverName with driver->name when that field exists.
	#define DriverName "curses"

	/*Get settings from config file*/

	/*Get color settings*/
	/*foreground color*/
	strncpy(buf, config_get_string ( DriverName , "foreground" , 0 , CONF_DEF_FOREGR),sizeof(config_get_string ( DriverName , "foreground" , 0 , CONF_DEF_FOREGR)));
	buf[sizeof(buf)-1]=0;
	fore_color = set_foreground_color(buf);
	debug( RPT_DEBUG, "CURSES: using foreground color: %s", buf);
	/*background color*/
	strncpy(buf, config_get_string ( DriverName , "background" , 0 , CONF_DEF_BACKGR),sizeof(config_get_string ( DriverName , "background" , 0 , CONF_DEF_BACKGR)));
	buf[sizeof(buf)-1]=0;
	back_color = set_background_color(buf);
	debug( RPT_DEBUG, "CURSES: using background color: %s", buf);
	/*backlight color*/
	strncpy(buf, config_get_string ( DriverName , "backlight" , 0 , CONF_DEF_BACKLIGHT), sizeof(config_get_string ( DriverName , "backlight" , 0 , CONF_DEF_BACKLIGHT)));
	buf[sizeof(buf)-1]=0;
	backlight_color = set_background_color(buf);
	debug( RPT_DEBUG, "CURSES: using backlight color: %s", buf);

	//TODO: Make it possible to configure the backlight's "off" color and its "on" color
	//      Or maybe don't do so? - Rene Wagner

	/*Get size settings*/
	strncpy(buf, config_get_string ( DriverName , "size" , 0 , CONF_DEF_SIZE), sizeof(config_get_string ( DriverName , "size" , 0 , CONF_DEF_SIZE)));
	buf[sizeof(buf)-1]=0;
	int wid, hgt;
	if( sscanf(buf , "%dx%d", &wid, &hgt ) != 2
	|| (wid <= 0)
	|| (hgt <= 0)) {
		report (RPT_WARNING, "CURSES: Cannot read size: %s. Using default value.\n", buf);
		sscanf( CONF_DEF_SIZE , "%dx%d", &wid, &hgt );
	}
	driver->wid = wid;
	driver->hgt = hgt;

	/*Get position settings*/
	if (0<=config_get_int ( DriverName , "topleftx" , 0 , CONF_DEF_TOP_LEFT_X) && config_get_int ( DriverName , "topleftx" , 0 , CONF_DEF_TOP_LEFT_X) <= 255) {
		screen_begx = config_get_int ( DriverName , "topleftx" , 0 , CONF_DEF_TOP_LEFT_X);
	} else {
		report (RPT_WARNING, "CURSES: topleftx must between 0 and 255. Using default value %d.\n",CONF_DEF_TOP_LEFT_X);
	}
	if (0<=config_get_int ( DriverName , "toplefty" , 0 , CONF_DEF_TOP_LEFT_Y) && config_get_int ( DriverName , "toplefty" , 0 , CONF_DEF_TOP_LEFT_Y) <= 255) {
		screen_begy = config_get_int ( DriverName , "toplefty" , 0 , CONF_DEF_TOP_LEFT_Y);
	} else {
		report (RPT_WARNING, "CURSES: toplefty must between 0 and 255. Using default value %d.\n",CONF_DEF_TOP_LEFT_Y);
	}

	//debug: sleep(1);

	// Init curses...
	initscr ();
	cbreak ();
	noecho ();
	nonl ();

	nodelay (stdscr, TRUE);
	intrflush (stdscr, FALSE);
	keypad (stdscr, TRUE);

	lcd_win = newwin(curses_drv->hgt + 2,
			 curses_drv->wid + 2,
			 screen_begy,
			 screen_begx);

	//nodelay (lcd_win, TRUE);
	//intrflush (lcd_win, FALSE);
	//keypad (lcd_win, TRUE);

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
	driver->heartbeat = curses_drv_heartbeat;

	// Change the character used for "..."
	ELLIPSIS = '~';

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

static void
curses_drv_wborder (WINDOW *win) {
	//int x, y;
	//char buf[128];

#ifdef CURSES_HAS_WCOLOR_SET
	if (has_colors()) {
		wcolor_set(win, current_border_pair, NULL);
		//wattron(win, COLOR_PAIR(current_border_pair) | A_BOLD);
		wattron(win, A_BOLD);
	}
#endif

	box(win, 0, 0);

#ifdef CURSES_HAS_WCOLOR_SET
	if (has_colors()) {
		wcolor_set(win, current_color_pair, NULL);
		//wattron(win, COLOR_PAIR(current_color_pair));
		wattroff(win, A_BOLD);
	}
#endif
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
	curs_set(1);

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

#define ValidX(x) { if ((x) > curses_drv->wid) { (x) = curses_drv->wid; } else (x) = (x) < 1 ? 1 : x; }
#define ValidY(y) { if ((y) > curses_drv->hgt) { (y) = curses_drv->hgt; } else (y) = (y) < 1 ? 1 : y; }

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

	curses_drv_clear();
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
curses_drv_string (int x, int y, char *string)
{
	//int i;
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
	int ch;

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

	if ((ch = getch ()) != ERR)
		if (ch == 0x0C) {
			curses_drv_restore_screen();
			ungetch(ch);
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
	int y, dx;

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			curses_drv_chr (x + dx, y, num_icon[num][y-1][dx]);
//   printf("%1d",num);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
curses_drv_vbar (int x, int len)
{
	int y;
	char map[] = { ACS_S9, ACS_S9, ACS_S7, ACS_S7, ACS_S3, ACS_S3, ACS_S1, ACS_S1 };

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

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
void
curses_drv_heartbeat (int type)
{
	static int timer = 0;
	int whichIcon;
	static int saved_type = HEARTBEAT_ON;

	if (type)
		saved_type = type;

	if (type == HEARTBEAT_ON) {
		// Set this to pulsate like a real heart beat...
		whichIcon = (! ((timer + 4) & 5));

		// This defines a custom character EVERY time...
		// not efficient... is this necessary?
		curses_drv_icon (whichIcon, 0);

		// Put character on screen...
		curses_drv_chr (curses_drv->wid, 1, 0);

		// change display...
		curses_drv_flush ();
	}

	timer++;
	timer &= 0x0f;
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
	int c;

	if ((c = getch ()) != ERR)
		if (c == 0x0C) {
			curses_drv_restore_screen();
			ungetch (c);
		}

	curses_drv_wborder (lcd_win);
	wrefresh (lcd_win);
}

char
curses_drv_getkey ()
{
	int i;

	i = getch ();

	switch(i) {
		case 0x0C:
			curses_drv_restore_screen();
			return 0;
			break;
		case KEY_LEFT:
			return 'D';
			break;
		case KEY_UP:
			return 'B';
			break;
		case KEY_DOWN:
			return 'C';
			break;
		case KEY_RIGHT:
			return 'A';
			break;
		case ERR:
			return 0;
			break;
		default:
			return i;
			break;
	}
}

void
curses_drv_restore_screen () {
	erase();
	refresh();
#ifdef CURSES_HAS_REDRAWWIN
	redrawwin(lcd_win);
#endif
	wrefresh(lcd_win);
}
