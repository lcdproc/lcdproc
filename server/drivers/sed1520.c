//////////////////////////////////////////////////////////////////////////
// This is a driver for 122x32 pixel graphic displays based on the      //
// SED1520 Controller connected to the parallel port. Check             //
// www.adams-online.de/lcd for where to buy                             //
// and how to build the hardware. This Controller has no built in       //
// character generator. Therefore all fonts and pixels are generated    //
// by this driver.                                                      //
//                                                                      //
// This driver is based on drv_base.c and hd44780.c.                    //
// The HD44780 font in sed1520fm.c was shamelessly stolen from          //
// Michael Reinelt / lcd4linux and is (C) 2000 by him.                  //
// The rest of fontmap.c and this driver is                             //
//                                                                      //
// Moved the delay timing code by Charles Steinkuehler to timing.h.     //
// Guillaume Filion <gfk@logidac.com>, December 2001                    //
//                                                                      //
// (C) 2001 Robin Adams ( robin@adams-online.de )                       //
//                                                                      //
// This driver is released under the GPL. See file COPYING in this      //
// package for further details.                                         //
//////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <time.h>
#include "port.h"
#include "timing.h"
#define uPause timing_uPause

#include "sed1520fm.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef LPTPORT
#define LPTPORT 0x378
#endif

#define A0 0x08
#define CS2 0x04
#define CS1 0x02
#define WR 0x01
#define IODELAY 500

#include "shared/str.h"
#include "lcd.h"
#include "sed1520.h"


unsigned int sed1520_lptport = LPTPORT;
char *framebuf = NULL;
int width = LCD_DEFAULT_WIDTH;
int height = LCD_DEFAULT_HEIGHT;
int cellwidth = LCD_DEFAULT_CELLWIDTH;
int cellheight = LCD_DEFAULT_CELLHEIGHT;


// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "sed1520_";

/////////////////////////////////////////////////////////////////
// writes command value to one or both sed1520 selected by chip
//
void
writecommand (int value, int chip)
{
    port_out ( sed1520_lptport,value);
    port_out ( sed1520_lptport + 2,WR + CS1 - (chip & CS1) + (chip & CS2));
    port_out ( sed1520_lptport + 2,CS1 - (chip & CS1) + (chip & CS2));
    uPause (IODELAY);
    port_out ( sed1520_lptport + 2,WR + CS1 - (chip & CS1) + (chip & CS2));
    uPause (IODELAY);
}

/////////////////////////////////////////////////////////////////
// writes data value to one or both sed 1520 selected by chip
//
void
writedata (int value, int chip)
{
    port_out ( sed1520_lptport,value);
    port_out ( sed1520_lptport + 2,A0 + WR + CS1 - (chip & CS1) + (chip & CS2));
    port_out ( sed1520_lptport + 2,A0 + CS1 - (chip & CS1) + (chip & CS2));
    uPause (IODELAY);
    port_out ( sed1520_lptport + 2,A0 + WR + CS1 - (chip & CS1) + (chip & CS2));
    uPause (IODELAY);
}

/////////////////////////////////////////////////////////////////
// selects a page (=row) on both sed1520s
//
void
selectpage (int page)
{
    writecommand (0xB8 + (page & 3), CS1 + CS2);
}

/////////////////////////////////////////////////////////////////
// selects a column on the sed1520s specified by chip
//
void
selectcolumn (int column, int chip)
{
    writecommand ((column & 0x7F), chip);
}

/////////////////////////////////////////////////////////////////
// draws  char z from fontmap to the framebuffer at position
// x,y. These are zero-based textmode positions.
// The Fontmap is stored in rows while the framebuffer is stored
// in columns, so we need a little conversion.
//
void
drawchar2fb (int x, int y, unsigned char z)
{
    int i, j, k;

    if (x < 0 || x > 19 || y < 0 || y > 3)
	return;

    for (i = 6; i > 0; i--)
      {
	  k = 0;
	  for (j = 0; j < 7; j++)
	    {
		k = k +
		    (((fontmap[(int) z][j] * 2) & (1 << i)) / (1 << i)) *
		    (1 << j);
	    }
	  framebuf[(y * 122) + (x * 6) + (6 - i)] = k;
      }

}

/////////////////////////////////////////////////////////////////
// This initialises the stuff. We support supplying port as
// a command line argument.
//
MODULE_EXPORT int
sed1520_init (Driver *drvthis, char *args)
{
    char *argv[64], *str;
    int argc, i;

    if (args)
	if ((str = (char *) malloc (strlen (args) + 1)))
	    strcpy (str, args);
	else
	  {
	      fprintf (stderr, "Error mallocing\n");
	      return -1;
	  }
    else
	str = NULL;

    argc = get_args (argv, args, 64);
    for (i = 0; i < argc; i++)
      {
	  if (0 == strcmp (argv[i], "-p")
	      || 0 == strcmp (argv[i], "--port\0"))
	    {
		if (i + 1 >= argc)
		  {
		      fprintf (stderr,
			       "sed1520_init: %s requires an argument\n",
			       argv[i]);
		      return -1;
		  }
		else
		  {
		      int myport;
		      if (sscanf (argv[i + 1], "%i", &myport) != 1)
			{
			    fprintf (stderr,
				     "sed1520_init: Couldn't read port address -"
				     " using default value 0x%x\n", sed1520_lptport);
			    return -1;
			}
		      else
			{
			    sed1520_lptport = myport;
			    ++i;
			}
		  }
	    }
	  else if (0 == strcmp (argv[i], "-h")
		   || 0 == strcmp (argv[i], "--help"))
	    {
		//int i;
		printf
		    ("LCDproc sed1520 driver\n\t-p n\t--port n\tSelect the output device to use port n\n");
		printf ("put the options in quotes like this:  '-p 0x278'\n");
		printf ("\t-h\t--help\t\tShow this help information\n");
		return -1;
	    }
      }

    // driver->wid = 20;
    // driver->hgt = 4;

	if (timing_init() == -1)
		return -1;

    // Allocate our framebuffer
    framebuf = malloc (122 * 4);
    if (!framebuf)
      {
	  // sed1520_close ();
	  return -1;
      }

    // clear screen
    memset (framebuf, 0, 122 * 4);

    // Initialize the Port and the sed1520s
    if(port_access(sed1520_lptport)) return -1;
    if(port_access(sed1520_lptport+2)) return -1;

    port_out (sed1520_lptport,0);
    port_out (sed1520_lptport +2,WR + CS2);
    writecommand (0xE2, CS1 + CS2);
    writecommand (0xAF, CS1 + CS2);
    writecommand (0xC0, CS1 + CS2);
    selectpage (3);

    cellwidth = 6;
    cellheight = 8;

    return 0;
}

/////////////////////////////////////////////////////////////////
// Frees the frambuffer and exits the driver.
//
MODULE_EXPORT void
sed1520_close (Driver *drvthis)
{
    if (framebuf != NULL)
	free (framebuf);
    framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
sed1520_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
sed1520_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
sed1520_clear (Driver *drvthis)
{
    memset (framebuf, 0, 488);
}

/////////////////////////////////////////////////////////////////
//
// Flushes all output to the lcd...
//
MODULE_EXPORT void
sed1520_flush (Driver *drvthis)
{
    int i, j;
    for (i = 0; i < 4; i++)
      {
	  selectpage (i);
	  selectcolumn (0, CS2) ;
	  for (j = 0; j < 61; j++)
	      writedata (framebuf[j + (i * 122)], CS2);
	  selectcolumn (0, CS1) ;
	  for (j = 61; j < 122; j++)
	      writedata (framebuf[j + (i * 122)], CS1);
      }
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lc display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
sed1520_string (Driver *drvthis, int x, int y, char string[])
{
    int i;
    x--;			// Convert 1-based coords to 0-based...
    y--;

    for (i = 0; string[i]; i++)
      {
	  drawchar2fb (x + i, y, string[i]);
      }
}

/////////////////////////////////////////////////////////////////
// Writes  char c at position x,y into the framebuffer.
// x and y are 1-based textmode coordinates.
//
MODULE_EXPORT void
sed1520_chr (Driver *drvthis, int x, int y, char c)
{
    y--;
    x--;
    drawchar2fb (x, y, c);
}

/////////////////////////////////////////////////////////////////
// This function draws a number num into the last 3 rows of the
// framebuffer at 1-based position x. It should draw a 4-row font,
// but methinks this would look a little stretched. When
// num=10 a colon is drawn.
// FIXME: make big numbers use less memory
//
MODULE_EXPORT void
sed1520_num (Driver *drvthis, int x, int num)
{
    int z, c, i, s;
    x--;

    if (x < 0 || x > 19 || num < 0 || num > 10)
	return;
    if (num != 10 && (x < 0 || x > 17))
	return;
    if (num == 10 && (x < 0 || x > 19))
	return;

    if (num == 10)
      {				// Doppelpunkt
	  for (z = 0; z < 3; z++)
	    {			// Zeilen a 8 Punkte
		for (c = 0; c < 6; c++)
		  {		// 6 Spalten
		      s = 0;
		      for (i = 0; i < 8; i++)
			{	// 8 bits aus zeilen
			    s >>= 1;
			    if (*(fontbigdp[(z * 8) + i] + c) == '.')
				s += 128;
			}
		      framebuf[(z * 122) + 122 + (x * 6) + c] = s;
		  }
	    }
      }
    else
      {

	  for (z = 0; z < 3; z++)
	    {			// Zeilen a 8 Punkte
		for (c = 0; c < 18; c++)
		  {		// 18 Spalten
		      s = 0;
		      for (i = 0; i < 8; i++)
			{	// 8 bits aus zeilen
			    s >>= 1;
			    if (*(fontbignum[num][z * 8 + i] + c) == '.')
				s += 128;
			}
		      framebuf[(z * 122) + 122 + (x * 6) + c] = s;
		  }
	    }
      }
}


/////////////////////////////////////////////////////////////////
// Changes the font of character n to a pattern given by *dat.
// HD44780 Controllers only posses 8 programmable chars. But
// we store the fontmap completely in RAM, so every character
// can be altered. !Important: Characters have to be redraw
// by drawchar2fb() to show their new shape. Because we use
// a non-standard 6x8 font a *dat not calculated from
// widthth and sed1520->height will fail.
//
MODULE_EXPORT void
sed1520_set_char (Driver *drvthis, int n, char *dat)
{

    int row, col, i;
    if (n < 0 || n > 255)
	return;
    if (!dat)
	return;
    for (row = 0; row < 8; row++)
      {
	  i = 0;
	  for (col = 0; col < 6; col++)
	    {
		i <<= 1;
		i |= (dat[(row * 6) + col] > 0);
	    }
	  fontmap[n][row] = i;
      }
}


/////////////////////////////////////////////////////////////////
// Draws a vertical from the bottom up to the last 3 rows of the
// framebuffer at 1-based position x. len is given in pixels.
//
MODULE_EXPORT void
sed1520_old_vbar (Driver *drvthis, int x, int len)
{
    int i, j, k;
    x--;


    for (j = 0; j < 3; j++)
      {
	  i = 0;
	  k = 0;
	  for (i = 0; i < 8; i++)
	    {
		if (len > i)
		    k += 1 << (7 - i);
	    }

	  framebuf[((3 - j) * 122) + (x * 6)] = 0;
	  framebuf[((3 - j) * 122) + (x * 6) + 1] = 0;
	  framebuf[((3 - j) * 122) + (x * 6) + 2] = k;
	  framebuf[((3 - j) * 122) + (x * 6) + 3] = k;
	  framebuf[((3 - j) * 122) + (x * 6) + 4] = k;
	  framebuf[((3 - j) * 122) + (x * 6) + 5] = 0;
	  len -= 8;
      }

}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar from left to right at 1-based position
// x,y into the framebuffer. len is given in pixels.
//
MODULE_EXPORT void
sed1520_old_hbar (Driver *drvthis, int x, int y, int len)
{
    int i;
    x--;
    y--;

    if (y < 0 || y > 3 || x < 0 || len < 0 || (x + (len / 6)) > 19)
	return;

    for (i = 0; i < len; i++)
	framebuf[(y * 122) + (x * 6) + i] = 0x3C;
}

/////////////////////////////////////////////////////////////////
// Reprogrammes character dest to contain an icon given by
// which. Calls set_char() to do this.
//
MODULE_EXPORT int
sed1520_icon (Driver *drvthis, int x, int y, int icon)
{
  static char heart_open[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 0, 0, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1 };

  static char heart_filled[] = {
    1, 1, 1, 1, 1,
    1, 0, 1, 0, 1,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 1, 1, 0,
    1, 0, 1, 0, 1,
    1, 1, 0, 1, 1,
    1, 1, 1, 1, 1 };

  switch( icon )
  {
    case ICON_BLOCK_FILLED:
      sed1520_chr( drvthis, x, y, 255 );
      break;
    case ICON_HEART_FILLED:
      sed1520_set_char( drvthis, 0, heart_filled );
      sed1520_chr( drvthis, x, y, 0 );
      break;
    case ICON_HEART_OPEN:
      sed1520_set_char( drvthis, 0, heart_open );
      sed1520_chr( drvthis, x, y, 0 );
      break;
    default:
      return -1;
  }
  return 0;
}

