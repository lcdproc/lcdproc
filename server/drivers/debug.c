#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#include "lcd.h"
#include "debug.h"


//////////////////////////////////////////////////////////////////////////
////////////////////// For Debugging Output //////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO: somehow allow access to the driver->framebuffer to each
// function...


int
debug_init (struct lcd_logical_driver *driver, char *args)
{
   printf ("debug_init(%s)\n", device);

   if (!driver->framebuf) {
      printf ("Allocating frame buffer (%ix%i)\n", lcd.wid, lcd.hgt);
      driver->framebuf = malloc (lcd.wid * lcd.hgt);
   }


   if (!driver->framebuf) {
      debug_close ();
      return -1;
   }

   if (driver->framebuf)
      printf ("Frame buffer: %i\n", (int) driver->framebuf);

   debug_clear ();

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


   return 200;			// 200 is arbitrary.  (must be 1 or more)
}

void
debug_close ()
{
// Ack!  This shouldn't crash the program, but does..  Why??


   printf ("debug_close()\n");

// This is the line which crashes.
   if (driver->framebuf)
      free (driver->framebuf);

   if (driver->framebuf)
      printf ("Frame buffer: %i\n", (int) driver->framebuf);
   driver->framebuf = NULL;
//  printf("debug_close() finished\n");
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
debug_clear ()
{
   printf ("clear()\n");

   memset (driver->framebuf, ' ', lcd.wid * lcd.hgt);

}


//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
debug_flush ()
{
   printf ("flush()\n");

   lcd.draw_frame ();
}


/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
debug_string (int x, int y, char string[])
{

   int i;

   printf ("string(%i, %i):%s \n", x, y, string);


   x -= 1;			// Convert 1-based coords to 0-based...
   y -= 1;

   for (i = 0; string[i]; i++) {
      driver->framebuf[(y * lcd.wid) + x + i] = string[i];
   }
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
debug_chr (int x, int y, char c)
{
   printf ("character(%i, %i):%c\n", x, y, c);


   y--;
   x--;

   lcd.framebuf[(y * lcd.wid) + x] = c;
}



void
debug_contrast (int contrast)
{
   printf ("Contrast: %i\n", contrast);
}

void
debug_backlight (int on)
{
   if (on) {
      printf ("Backlight ON\n");
   } else {
      printf ("Backlight OFF\n");
   }
}

void
debug_init_vbar ()
{
   printf ("Vertical bars.\n");
}

void
debug_init_hbar ()
{
   printf ("Horizontal bars.\n");
}

void
debug_init_num ()
{
   printf ("Big Numbers.\n");
}

void
debug_num (int x, int num)
{
   printf ("BigNum(%i, %i)\n", x, num);
}

void
debug_set_char (int n, char *dat)
{
   printf ("Set Character %i\n", n);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar; erases entire column onscreen.
//
void
debug_vbar (int x, int len)
{
   int y;

   printf ("Vbar(%i, %i)\n", x, len);


   for (y = lcd.hgt; y > 0 && len > 0; y--) {
      debug_chr (x, y, '|');

      len -= lcd.cellhgt;
   }

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
debug_hbar (int x, int y, int len)
{
   printf ("Hbar(%i, %i, %i)\n", x, y, len);


   for (; x < lcd.wid && len > 0; x++) {
      debug_chr (x, y, '-');

      len -= lcd.cellwid;
   }

}


/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
debug_icon (int which, char dest)
{
   printf ("Char %i is icon %i\n", dest, which);
}


void
debug_flush_box (int lft, int top, int rgt, int bot)
{
   printf ("Flush Box(%i, %i)-(%i, %i)\n", lft, top, rgt, bot);


   debug_flush ();

}


void
debug_draw_frame (char *dat)
{
   int i, j;

   char out[LCD_MAX_WIDTH];

   printf ("draw_frame()\n");


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


char
debug_getkey ()
{
   printf ("Trying to grab keypress.\n");
   return 0;
}
