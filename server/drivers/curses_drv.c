/** \file server/drivers/curses_drv.c
 * LCDd \c curses driver for text-mode terminals using curses.
 * It displays an emulated LCD display of configurable size at
 * a configurable position of the terminal screen using (n)curses.
 */

/*  Copyright (C) ?????? ?????????

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 */

/*Configfile support added by Rene Wagner (C) 2001*/

/*
Different implementations of (n)curses available on:
  - OpenBSD:
	http://www.openbsd.org/cgi-bin/cvsweb/src/lib/libcurses/
	ncurses
  - NetBSD:
	http://cvsweb.netbsd.org/bsdweb.cgi/src/lib/libcurses/
	curses : does not define ACS_S3, ACS_S7, wcolor_set() or redrawwin().
	it is possible to make a:
		#define ACS_S3 (_acs_char['p'])
		#define ACS_S7 (_acs_char['r'])
  - FreeBSD:
	http://www.freebsd.org/cgi/cvsweb.cgi/src/
	ncurses
  - RedHat, Debian, (most distros) Linux:
	ncurses
  - SunOS (5.5.1):
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
#include <string.h>
#include <strings.h>
#include <ctype.h>
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "lcd.h"
#include "curses_drv.h"
#include "report.h"

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

/* A few different nice pairs of colors to use... */
#define DEFAULT_FOREGROUND_COLOR COLOR_CYAN
#define DEFAULT_BACKGROUND_COLOR COLOR_BLUE


/** private data for the \c curses driver */
typedef struct curses_private_data {
	WINDOW *win;

	int current_color_pair;
	int current_border_pair;
	int curses_backlight_state;

	int width;
	int height;
	int cellwidth;
	int cellheight;

	int xoffs;
	int yoffs;

	int useACS;

	int drawBorder;
} PrivateData;


/* Vars for the server core */
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "curses_";

/* local helper functions */
static void curses_wborder (Driver *drvthis);
static chtype get_color_by_name (char *colorname, chtype default_color);
static void curses_restore_screen (Driver *drvthis);


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int
curses_init (Driver *drvthis)
{
	PrivateData *p;
	char buf[256];
	int tmp;

	// Colors....
	chtype	back_color = DEFAULT_BACKGROUND_COLOR,
		fore_color = DEFAULT_FOREGROUND_COLOR,
		backlight_color = DEFAULT_BACKGROUND_COLOR;

	// Screen position (top left)

	/* Allocate and store private data */
	p = (PrivateData *) calloc(1, sizeof(PrivateData));
	if (p == NULL)
		return -1;
	if (drvthis->store_private_ptr(drvthis, p))
		return -1;

	/* initialize private data */
	p->win = NULL;
	p->current_color_pair = 2;
	p->current_border_pair = 3;
	p->curses_backlight_state = 0;
	p->xoffs = CONF_DEF_TOP_LEFT_X,
	p->yoffs = CONF_DEF_TOP_LEFT_Y;
	p->cellwidth = LCD_DEFAULT_CELLWIDTH;
	p->cellheight = LCD_DEFAULT_CELLHEIGHT;
	p->drawBorder = CONF_DEF_DRAWBORDER;

	/* Get settings from config file */

	/* Get color settings */

	/* foreground color */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Foreground", 0, CONF_DEF_FOREGR), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	fore_color = get_color_by_name(buf, DEFAULT_FOREGROUND_COLOR);
	debug(RPT_DEBUG, "%s: using foreground color %s", drvthis->name, buf);

	/* background color */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Background", 0, CONF_DEF_BACKGR), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	back_color = get_color_by_name(buf, DEFAULT_BACKGROUND_COLOR);
	debug(RPT_DEBUG, "%s: using background color %s", drvthis->name, buf);

	/* backlight color */
	strncpy(buf, drvthis->config_get_string(drvthis->name, "Backlight", 0, CONF_DEF_BACKLIGHT), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
	backlight_color = get_color_by_name(buf, DEFAULT_BACKGROUND_COLOR);
	debug(RPT_DEBUG, "%s: using backlight color %s", drvthis->name, buf);

	/* use ACS characters? */
	p->useACS = drvthis->config_get_bool(drvthis->name, "UseACS", 0, CONF_DEF_USEACS);
	debug(RPT_DEBUG, "%s: using ACS %s", drvthis->name, (p->useACS) ? "ON" : "OFF");

	/* draw border ? */
	p->drawBorder = drvthis->config_get_bool(drvthis->name, "DrawBorder", 0, CONF_DEF_DRAWBORDER);
	debug(RPT_DEBUG, "%s: drawing Border %s", drvthis->name, (p->drawBorder) ? "ON" : "OFF");

	/* Get size settings */
	if ((drvthis->request_display_width() > 0)
	    && (drvthis->request_display_height() > 0)) {
		/* If this driver is secondary driver, use size from primary driver */
		p->width = drvthis->request_display_width();
		p->height = drvthis->request_display_height();
	}
	else {
		/* Use our own size from config file */
		strncpy(buf, drvthis->config_get_string(drvthis->name, "Size", 0, CONF_DEF_SIZE), sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		if ((sscanf(buf , "%dx%d", &p->width, &p->height) != 2)
		    || (p->width <= 0) || (p->width > LCD_MAX_WIDTH)
		    || (p->height <= 0) || (p->height > LCD_MAX_HEIGHT)) {
			report(RPT_WARNING, "%s: cannot read Size: %s; using default %s",
					drvthis->name, buf, CONF_DEF_SIZE);
			sscanf(CONF_DEF_SIZE, "%dx%d", &p->width, &p->height);
		}
	}

	/*Get position settings*/
	tmp = drvthis->config_get_int(drvthis->name, "TopLeftX", 0, CONF_DEF_TOP_LEFT_X);
	if ((tmp < 0) || (tmp > 255)) {
		report(RPT_WARNING, "%s: TopLeftX must be between 0 and 255; using default %d",
				drvthis->name, CONF_DEF_TOP_LEFT_X);
		tmp = CONF_DEF_TOP_LEFT_X;
	}
	p->xoffs = tmp;

	tmp = drvthis->config_get_int(drvthis->name, "TopLeftY", 0, CONF_DEF_TOP_LEFT_Y);
	if ((tmp < 0) || (tmp > 255)) {
		report(RPT_WARNING, "%s: TopLeftY must be between 0 and 255; using default %d",
				drvthis->name, CONF_DEF_TOP_LEFT_Y);
		tmp = CONF_DEF_TOP_LEFT_Y;
	}
	p->yoffs = tmp;

	//debug: sleep(1);

	// Init curses...
	initscr();
	cbreak();
	noecho();
	nonl();

	nodelay(stdscr, TRUE);
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);

	if (p->drawBorder) {
		p->win = newwin(p->height + 2,	/* +2 for the border */
				p->width + 2,	/* +2 for the border */
				p->yoffs,
				p->xoffs);
	}
	else{
		p->win = newwin(p->height,
				p->width,
				p->yoffs,
				p->xoffs);
	}

	//nodelay(p->win, TRUE);
	//intrflush(p->win, FALSE);
	//keypad(p->win, TRUE);

	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, back_color, fore_color);
		init_pair(2, fore_color, back_color);
		init_pair(3, COLOR_WHITE, back_color);
		init_pair(4, fore_color, backlight_color);
		init_pair(5, COLOR_WHITE, backlight_color);
	}

	curses_clear(drvthis);

	report(RPT_DEBUG, "%s: init() done", drvthis->name);

	return 0;
}


/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
curses_close (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	// Note that the program leaves a screen on
	// the display to be left behind after closing;
	// so don't clear...

	if (p != NULL) {
		// Close curses
		wrefresh(p->win);
		delwin(p->win);

		move(0, 0);
		endwin();
		curs_set(1);

		free(p);
	}
	drvthis->store_private_ptr(drvthis, NULL);
}


/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int
curses_width (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->width;
}

/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int
curses_height (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	return p->height;
}


/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
curses_clear (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	wbkgdset(p->win, COLOR_PAIR(p->current_color_pair) | ' ');

	if (p->drawBorder)
		curses_wborder(drvthis);

	werase(p->win);
}


/**
 * Turn the LCD backlight on or off.
 * This is simulated by changing the background
 * colour of the characters displayed.
 * \param drvthis  Pointer to driver structure.
 * \param on       New backlight status.
 */
MODULE_EXPORT void
curses_backlight (Driver *drvthis, int on)
{
	PrivateData *p = drvthis->private_data;

	if (p->curses_backlight_state == on)
		return;

	// no backlight: pairs 2, 3
	// backlight:    pairs 4, 5

	p->curses_backlight_state = on;

	if (on) {
		p->current_color_pair = 4;
		p->current_border_pair = 5;
	}
	else {
		p->current_color_pair = 2;
		p->current_border_pair = 3;
	}

	curses_clear(drvthis);
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void
curses_string (Driver *drvthis, int x, int y, const char string[])
{
	PrivateData *p = drvthis->private_data;

	if ((x <= 0) || (y <= 0) || (x > p->width) || (y > p->height))
		return;

	if (!p->drawBorder) {
		x--;
		y--;
	}

	mvwaddstr(p->win, y, x, string);
}


/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void
curses_chr (Driver *drvthis, int x, int y, char c)
{
	PrivateData *p = drvthis->private_data;

	if ((x <= 0) || (y <= 0) || (x > p->width) || (y > p->height))
		return;

	if (!p->drawBorder) {
		x--;
		y--;
	}

	mvwaddch(p->win, y, x, c);
}


/**
 * Draw a vertical bar bottom-up.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is high at 100%
 * \param promille Current height level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
curses_vbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	// map
	char ACS_map[] = { ACS_S9, ACS_S9, ACS_S7, ACS_S7, ACS_S3, ACS_S3, ACS_S1, ACS_S1 };
	char ascii_map[] = { ' ', ' ', '-', '-', '=', '=', '#', '#' };
	char *map = (p->useACS) ? ACS_map : ascii_map;
	int pixels = ((long) 2 * len * p->cellheight) * promille / 2000;
	int pos;


	if ((x <= 0) || (y <= 0) || (x > p->width))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'up' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	for (pos = 0; pos < len; pos++) {

		if (y - pos <= 0)
			return;

		if (pixels >= p->cellheight) {
			/* write a "full" block to the screen... */
			curses_chr(drvthis, x, y-pos, (p->useACS) ? ACS_BLOCK : '#');
		}
		else if (pixels > 0) {
			// write a partial block...
			curses_chr(drvthis, x, y-pos, map[len-1]);
			break;
		}
		else {
			; // write nothing (not even a space)
		}

		pixels -= p->cellheight;
	}
}


/**
 * Draw a horizontal bar to the right.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column) of the starting point.
 * \param y        Vertical character position (row) of the starting point.
 * \param len      Number of characters that the bar is long at 100%
 * \param promille Current length level of the bar in promille.
 * \param options  Options (currently unused).
 */
MODULE_EXPORT void
curses_hbar (Driver *drvthis, int x, int y, int len, int promille, int options)
{
	PrivateData *p = drvthis->private_data;
	int pixels = ((long) 2 * len * p->cellwidth) * promille / 2000;
	int pos;

	if ((x <= 0) || (y <= 0) || (y > p->height))
		return;

	/* x and y are the start position of the bar.
	 * The bar by default grows in the 'right' direction
	 * (other direction not yet implemented).
	 * len is the number of characters that the bar is long at 100%
	 * promille is the number of promilles (0..1000) that the bar should be filled.
	 */

	for (pos = 0; pos < len; pos++) {

		if (x + pos > p->width)
			return;

		if (pixels >= p->cellwidth * 2/3) {
			/* write a "full" block to the screen... */
			curses_chr(drvthis, x+pos, y, '=');
		}
		else if (pixels > p->cellwidth * 1/3) {
			/* write a partial block... */
			curses_chr(drvthis, x+pos, y, '-');
			break;
		}
		else {
			; // write nothing (not even a space)
		}

		pixels -= p->cellwidth;
	}
}


/**
 * Place an icon on the screen.
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param icon     synbolic value representing the icon.
 * \retval 0       Icon has been successfully defined/written.
 * \retval <0      Server core shall define/write the icon.
 */
MODULE_EXPORT int
curses_icon (Driver *drvthis, int x, int y, int icon)
{
	PrivateData *p = drvthis->private_data;
	char ch = '?';

	switch (icon) {
		case ICON_BLOCK_FILLED:
			ch = (p->useACS) ? ACS_BLOCK : '#';
			break;
		case ICON_HEART_OPEN:
			ch = '-';
			break;
		case ICON_HEART_FILLED:
			ch = '+';
			break;
		case ICON_ARROW_UP:
			ch = (p->useACS) ? ACS_UARROW : '^';
			break;
		case ICON_ARROW_DOWN:
			ch = (p->useACS) ? ACS_DARROW : 'v';
			break;
		case ICON_ARROW_LEFT:
			ch = (p->useACS) ? ACS_LARROW : '<';
			break;
		case ICON_ARROW_RIGHT:
			ch = (p->useACS) ? ACS_RARROW : '>';
			break;
		case ICON_ELLIPSIS:
			ch = '~';
			break;
		default:
			return -1; /* Let the core do it */
	}
	curses_chr(drvthis, x, y, ch);

	return 0;
}


/**
 * Flush data on screen to the display.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void
curses_flush (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;
	int c;

	if ((c = getch()) != ERR) {
		if (c == 0x0C) {	/* ^L restores screen */
			curses_restore_screen(drvthis);
		}
		ungetch(c);
	}

	if (p->drawBorder)
		curses_wborder(drvthis);

	wrefresh(p->win);
}


/**
 * Handle input from keyboard.
 * \param drvthis  Pointer to driver structure.
 * \return         String representation of the key;
 *                 \c NULL if nothing available / unmapped key.
 */
MODULE_EXPORT const char *
curses_get_key (Driver *drvthis)
{
	//PrivateData *p = drvthis->private_data;
	static char ret_val[2] = {0,0};
	int key = getch();

	switch (key) {
		case ERR:
			return NULL;
		case 0x0C:
			/* internal: ^L restores screen */
			curses_restore_screen(drvthis);
			return NULL;
			break;
		case KEY_LEFT:
			return "Left";
		case KEY_UP:
			return "Up";
		case KEY_DOWN:
			return "Down";
		case KEY_RIGHT:
			return "Right";
		case KEY_ENTER:
		case 0x0D:
			return "Enter";
		case 0x1B:
			return "Escape";
		default:
			report(RPT_INFO, "%s: Unknown key 0x%02X", drvthis->name, key);
			ret_val[0] = (char) key & 0xFF;
			return (ret_val[0] != '\0') ? ret_val : NULL;
			break;
	}
}


/**
 * Provide some information about this driver.
 * \param drvthis  Pointer to driver structure.
 * \return         Constant string with information.
 */
MODULE_EXPORT const char *
curses_get_info (Driver *drvthis)
{
        static char *info_string = "curses driver";

        return info_string;
}


/* local helper function */

static void
curses_wborder (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

#ifdef CURSES_HAS_WCOLOR_SET
	if (has_colors()) {
		//wattron(p->win, COLOR_PAIR(p->current_border_pair) | A_BOLD);
		wcolor_set(p->win, p->current_border_pair, NULL);
		wattron(p->win, A_BOLD);
	}
#endif

	box(p->win, 0, 0);

#ifdef CURSES_HAS_WCOLOR_SET
	if (has_colors()) {
		//wattron(p->win, COLOR_PAIR(p->current_color_pair));
		wcolor_set(p->win, p->current_color_pair, NULL);
		wattroff(p->win, A_BOLD);
	}
#endif
}


static chtype
get_color_by_name (char *colorname, chtype default_color) {
	if (strcasecmp(colorname, "red") == 0)
		return COLOR_RED;
	else if (strcasecmp(colorname, "black") == 0)
		return COLOR_BLACK;
	else if (strcasecmp(colorname, "green") == 0)
		return COLOR_GREEN;
	else if (strcasecmp(colorname, "yellow") == 0)
		return COLOR_YELLOW;
	else if (strcasecmp(colorname, "blue") == 0)
		return COLOR_BLUE;
	else if (strcasecmp(colorname, "magenta") == 0)
		return COLOR_MAGENTA;
	else if (strcasecmp(colorname, "cyan") == 0)
		return COLOR_CYAN;
	else if (strcasecmp(colorname, "white") == 0)
		return COLOR_WHITE;

	return default_color;
}


static void
curses_restore_screen (Driver *drvthis)
{
	PrivateData *p = drvthis->private_data;

	erase();
	refresh();
#ifdef CURSES_HAS_REDRAWWIN
	redrawwin(p->win);
#endif
	wrefresh(p->win);
}

/* EOF */
