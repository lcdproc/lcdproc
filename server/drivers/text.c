/*
 * TextMode driver
 *
 * Displays LCD screens, one after another; suitable for hard-copy
 * terminals.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <syslog.h>

#include "lcd.h"
#include "text.h"
#include "drv_base.h"

lcd_logical_driver *text;

//////////////////////////////////////////////////////////////////////////
////////////////////// For Text-Mode Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define LCD_DEFAULT_WIDTH 20
#define LCD_DEFAULT_HEIGHT 4
#define LCD_DEFAULT_CELL_WIDTH 5
#define LCD_DEFAULT_CELL_HEIGHT 8

// TODO: When using the text driver, ^C fails to interrupt!
// Why?  Fix it...

int
text_init (lcd_logical_driver * driver, char *args)
{
	text = driver;

	text->wid = LCD_DEFAULT_WIDTH;
	text->hgt = LCD_DEFAULT_HEIGHT;
	text->cellwid = LCD_DEFAULT_CELL_WIDTH;
	text->cellhgt = LCD_DEFAULT_CELL_HEIGHT;

	text->clear = text_clear;
	text->string = text_string;
	text->chr = text_chr;
	text->vbar = text_vbar;
	//text->init_vbar = NULL;
	text->hbar = text_hbar;
	//text->init_hbar = NULL;
	text->num = text_num;
	//text->init_num = NULL;

	text->init = text_init;
	text->close = text_close;
	text->flush = text_flush;
	//text->flush_box = NULL;
	//text->contrast = NULL;
	//text->backlight = NULL;
	//text->set_char = NULL;
	//text->icon = NULL;
	text->draw_frame = text_draw_frame;

	//text->getkey = NULL;

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

void
text_close ()
{
	if (text->framebuf != NULL)
		free (text->framebuf);

	text->framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
text_clear ()
{
	memset (text->framebuf, ' ', text->wid * text->hgt);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
text_flush ()
{
	text_draw_frame (text->framebuf);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
text_string (int x, int y, char string[])
{
	int i;

	x--; y--; // Convert 1-based coords to 0-based...

	for (i = 0; string[i]; i++) {
		text->framebuf[(y * text->wid) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
text_chr (int x, int y, char c)
{
	y--; x--;

	text->framebuf[(y * text->wid) + x] = c;
}

int
text_contrast (int contrast)
{
//  printf("Contrast: %i\n", contrast);
	return 0;
}

void
text_backlight (int on)
{
/*
  if(on)
  {
    printf("Backlight ON\n");
  }
  else
  {
    printf("Backlight OFF\n");
  }
*/
}

void
text_init_vbar ()
{
//  printf("Vertical bars.\n");
}

void
text_init_hbar ()
{
//  printf("Horizontal bars.\n");
}

void
text_init_num ()
{
//  printf("Big Numbers.\n");
}

void
text_num (int x, int num)
{
//  printf("BigNum(%i, %i)\n", x, num);
}

void
text_set_char (int n, char *dat)
{
//  printf("Set Character %i\n", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
text_vbar (int x, int len)
{
	int y;
	for (y = text->hgt; y > 0 && len > 0; y--) {
		text_chr (x, y, '|');

		len -= text->cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
text_hbar (int x, int y, int len)
{
	for (; x <= text->wid && len > 0; x++) {
		text_chr (x, y, '-');

		len -= text->cellwid;
	}

}

void
text_flush_box (int lft, int top, int rgt, int bot)
{
	text_flush ();

}

void
text_draw_frame (char *dat)
{
	int i, j;

	char out[LCD_MAX_WIDTH];

	if (!dat)
		return;

//  printf("Frame (%ix%i): \n%s\n", lcd.wid, lcd.hgt, dat);

	for (i = 0; i < text->wid; i++) {
		out[i] = '-';
	}
	out[text->wid] = 0;
	printf ("+%s+\n", out);

	for (i = 0; i < text->hgt; i++) {
		for (j = 0; j < text->wid; j++) {
			out[j] = dat[j + (i * text->wid)];
		}
		out[text->wid] = 0;
		printf ("|%s|\n", out);

	}

	for (i = 0; i < text->wid; i++) {
		out[i] = '-';
	}
	out[text->wid] = 0;
	printf ("+%s+\n", out);

}
