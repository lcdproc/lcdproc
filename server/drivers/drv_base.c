#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#include "lcd.h"
#include "drv_base.h"

//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////

lcd_logical_driver *drv_base;

// TODO: Get lcd.framebuf to properly work as whatever driver is running...

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
int
drv_base_init (struct lcd_logical_driver *driver, char *args)
{
//  printf("drv_base_init()\n");

	drv_base = driver;

	driver->wid = 20;
	driver->hgt = 4;

	// You must use driver->framebuf here, but may use lcd.framebuf later.
	if (!driver->framebuf)
		driver->framebuf = malloc (driver->wid * driver->hgt);

	if (!driver->framebuf) {
		drv_base_close ();
		return -1;
	}
// Debugging...
//  if(lcd.framebuf) printf("Frame buffer: %i\n", (int)lcd.framebuf);

	memset (driver->framebuf, ' ', driver->wid * driver->hgt);
//  drv_base_clear();

	driver->cellwid = 5;
	driver->cellhgt = 8;

	driver->clear = drv_base_clear;
	driver->string = drv_base_string;
	driver->chr = drv_base_chr;
	driver->vbar = drv_base_vbar;
	driver->init_vbar = drv_base_init_vbar;
	driver->hbar = drv_base_hbar;
	driver->init_hbar = drv_base_init_hbar;
	driver->num = drv_base_num;
	driver->init_num = drv_base_init_num;

	driver->init = drv_base_init;
	driver->close = drv_base_close;
	driver->flush = drv_base_flush;
	driver->flush_box = drv_base_flush_box;
	driver->contrast = drv_base_contrast;
	driver->backlight = drv_base_backlight;
	driver->set_char = drv_base_set_char;
	driver->icon = drv_base_icon;
	driver->draw_frame = drv_base_draw_frame;

	driver->getkey = drv_base_getkey;

	return 200;						  // 200 is arbitrary.  (must be 1 or more)
}

// Below here, you may use either lcd.framebuf or driver->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

void
drv_base_close ()
{
	if (lcd.framebuf != NULL)
		free (lcd.framebuf);

	lcd.framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
drv_base_clear ()
{
	memset (lcd.framebuf, ' ', lcd.wid * lcd.hgt);

}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
drv_base_flush ()
{
	//lcd.draw_frame(lcd.framebuf);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
drv_base_string (int x, int y, char string[])
{
	int i;

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

	for (i = 0; string[i]; i++) {
		// Check for buffer overflows...
		if ((y * lcd.wid) + x + i > (lcd.wid * lcd.hgt))
			break;
		lcd.framebuf[(y * lcd.wid) + x + i] = string[i];
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
drv_base_chr (int x, int y, char c)
{
	y--;
	x--;

	lcd.framebuf[(y * lcd.wid) + x] = c;
}

//////////////////////////////////////////////////////////////////////
// Sets the contrast of the display.  Value is 0-255, where 140 is
// what I consider "just right".
//
int
drv_base_contrast (int contrast)
{
//  printf("Contrast: %i\n", contrast);
	return -1;
}

//////////////////////////////////////////////////////////////////////
// Turns the lcd backlight on or off...
//
void
drv_base_backlight (int on)
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

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for vertical bargraphs.
//
void
drv_base_init_vbar ()
{
//  printf("Vertical bars.\n");
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for horizontal bargraphs.
//
void
drv_base_init_hbar ()
{
//  printf("Horizontal bars.\n");
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for big numbers, if possible.
//
void
drv_base_init_num ()
{
//  printf("Big Numbers.\n");
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
void
drv_base_num (int x, int num)
{
//  printf("BigNum(%i, %i)\n", x, num);
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
void
drv_base_set_char (int n, char *dat)
{
//  printf("Set Character %i\n", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
void
drv_base_vbar (int x, int len)
{
	int y;
	for (y = lcd.hgt; y > 0 && len > 0; y--) {
		drv_base_chr (x, y, '|');

		len -= lcd.cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
drv_base_hbar (int x, int y, int len)
{
	for (; x <= lcd.wid && len > 0; x++) {
		drv_base_chr (x, y, '-');

		len -= lcd.cellwid;
	}

}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
drv_base_icon (int which, char dest)
{
//  printf("Char %i set to icon %i\n", dest, which);
}

//////////////////////////////////////////////////////////////////////
// Send a rectangular area to the display.
//
// I've just called drv_base_flush() because there's not much point yet
// in flushing less than the entire framebuffer.
//
void
drv_base_flush_box (int lft, int top, int rgt, int bot)
{
	//drv_base_flush();

}

//////////////////////////////////////////////////////////////////////
// Draws the framebuffer on the display.
//
// The commented-out code is from the text driver.
//
void
drv_base_draw_frame (char *dat)
{
/*
   int i, j;

   char out[LCD_MAX_WIDTH];

   if(!dat) return;

   for(i=0; i<lcd.wid; i++)
   {
      out[i] = '-';
   }
   out[lcd.wid] = 0;
   printf("+%s+\n", out);
  
  
   for(i=0; i<lcd.hgt; i++)
   {
      for(j=0; j<lcd.wid; j++)
      {
	 out[j] = dat[j+(i*lcd.wid)];
      }
      out[lcd.wid] = 0;
      printf("|%s|\n", out);
      
   }
  
   for(i=0; i<lcd.wid; i++)
   {
      out[i] = '-';
   }
   out[lcd.wid] = 0;
   printf("+%s+\n", out);
*/
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
char
drv_base_getkey ()
{
	return 0;
}
