#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#include "lcd.h"
#include "text.h"
#include "drv_base.h"

lcd_logical_driver *text;

//////////////////////////////////////////////////////////////////////////
////////////////////// For Text-Mode Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
text_init (lcd_logical_driver * driver, char *args)
{
	text = driver;

	if (!driver->framebuf) {
		driver->close ();
		return -1;
	}

	driver->wid = 20;
	driver->hgt = 4;
	driver->cellwid = 5;
	driver->cellhgt = 8;

	driver->clear = (void *) -1;
	driver->string = (void *) -1;
	driver->chr = (void *) -1;
	driver->vbar = text_vbar;
	driver->init_vbar = NULL;
	driver->hbar = text_hbar;
	driver->init_hbar = NULL;
	driver->num = (void *) -1;
	driver->init_num = NULL;

	driver->init = text_init;
	driver->close = text_close;
	driver->flush = text_flush;
	driver->flush_box = NULL;
	driver->contrast = NULL;
	driver->backlight = NULL;
	driver->set_char = NULL;
	driver->icon = NULL;
	driver->draw_frame = text_draw_frame;

	driver->getkey = NULL;

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

void
text_close ()
{
	drv_base_close ();
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
text_clear ()
{
	memset (lcd.framebuf, ' ', lcd.wid * lcd.hgt);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
text_flush ()
{
	text_draw_frame (lcd.framebuf);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
text_string (int x, int y, char string[])
{
	int i;

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

	for (i = 0; string[i]; i++) {
		lcd.framebuf[(y * lcd.wid) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
text_chr (int x, int y, char c)
{
	y--;
	x--;

	lcd.framebuf[(y * lcd.wid) + x] = c;
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
	for (y = lcd.hgt; y > 0 && len > 0; y--) {
		text_chr (x, y, '|');

		len -= lcd.cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
text_hbar (int x, int y, int len)
{
	for (; x <= lcd.wid && len > 0; x++) {
		text_chr (x, y, '-');

		len -= lcd.cellwid;
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

	for (i = 0; i < lcd.wid; i++) {
		out[i] = '-';
	}
	out[lcd.wid] = 0;
	printf ("+%s+\n", out);

	for (i = 0; i < lcd.hgt; i++) {
		for (j = 0; j < lcd.wid; j++) {
			out[j] = dat[j + (i * lcd.wid)];
		}
		out[lcd.wid] = 0;
		printf ("|%s|\n", out);

	}

	for (i = 0; i < lcd.wid; i++) {
		out[i] = '-';
	}
	out[lcd.wid] = 0;
	printf ("+%s+\n", out);

}
