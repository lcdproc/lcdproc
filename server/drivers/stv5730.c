//////////////////////////////////////////////////////////////////////////
// This is a driver for the STV5730A on-screen display chip in con-     //
// junction with a parallel port interface. Check                       //
// www.adams-online.de/lcd/stv5730.html for where to buy                //
// and how to build the hardware.                                       //
// The STV3730 displays 11 rows with 28 characters. The characters are  //
// fixed and can not be reprogrammed. Luckily the chraracter set con-   //
// tains a heartbeat icon and some characters that can be used as       //
// hbars / vbars.                                                       //
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
#include <sys/io.h>
#include <sched.h>
#include <time.h>
#include "port.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "shared/str.h"
#include "lcd.h"
#include "stv5730.h"


#ifndef LPTPORT
#define LPTPORT 0x378
#endif

#define STV5730_TEST_O	0x01
#define STV5730_BAR	0x02
#define STV5730_CLK	0x04
#define STV5730_CSN	0x08
#define STV5730_DATA	0x10

#define STV5730_TEST_I	0x40
#define STV5730_MUTE	0x80

// If it doesn't work try increasing this value
#define IODELAY		400


// Change that to NTSC if you are from the US...
#define	PAL


#define STV5730_HGT	11
#define STV5730_WID	28
#define STV5730_ATTRIB	0x800

#define STV5730_REG_ZOOM	0xCC
#define STV5730_REG_COLOR	0xCD
#define STV5730_REG_CONTROL	0xCE
#define STV5730_REG_POSITION	0xCF
#define STV5730_REG_MODE	0xD0

// Choose Colors: FLINE: First line text color, TEXT: Text color, CBACK: Character Background Color
//                CBORD: Character Border Color, SBACK: Screen Background color
// 0:Black, 1: Blue, 2:Green, 3: Cyan, 4: Red, 5: Magenta, 6: Yellow, 7: White 
#define STV5730_COL_FLINE	4
#define STV5730_COL_TEXT	1
#define STV5730_COL_CBACK	3
#define STV5730_COL_CBORD	0
#define STV5730_COL_SBACK	2



unsigned int stv5730_lptport = LPTPORT;
unsigned int stv5730_charattrib = STV5730_ATTRIB;
unsigned int stv5730_flags = 0;

// Translation map ascii->stv5730 charset
unsigned char stv5730_to_ascii[256] =
    { 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x27, 0x0B, 0x27, 0x28,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x26, 0x26,
    0x62, 0x78, 0x61, 0x70,
    0x6c, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x1A,
    0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x0B,
    0x0B, 0x0B, 0x0B, 0x72,
    0x0B, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38,
    0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x6E,
    0x6C, 0x71, 0x79, 0x7F,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x77
};


lcd_logical_driver *stv5730;

static void stv5730_upause (int delayCalls);

/////////////////////////////////////////////////////////////////
// This function returns true if a powered and working STV5730
// hardware is present at stv5730_lptport

int
stv5730_detect (void)
{
    int i;

    for (i = 0; i < 10; i++)
      {
	  port_out (stv5730_lptport, STV5730_TEST_O);
	  stv5730_upause (IODELAY);
	  if ((port_in (stv5730_lptport + 1) & STV5730_TEST_I) == 0)
	      return -1;
	  port_out (stv5730_lptport, 0);
	  stv5730_upause (IODELAY);
	  if ((port_in (stv5730_lptport + 1) & STV5730_TEST_I) != 0)
	      return -1;
      }
    return 0;
}

/////////////////////////////////////////////////////////////////
// returns 0 if a valid video signal is connected to the video
// input
int
stv5730_is_mute (void)
{
    stv5730_upause (IODELAY);
    return ((port_in (stv5730_lptport + 1) & STV5730_MUTE) ? 0 : 1);
}

/////////////////////////////////////////////////////////////////
// stv5730_write16bit, stv5730_write8bit, stv5730_write0bit
// this family of functions write commands or data to the stv5730
// 8 bit writes repeat the high byte, 0 byte writes repeat the last
// written word

void
stv5730_write16bit (unsigned int value)
{
    char i, databit;
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CLK + stv5730_flags);

    for (i = 15; i >= 0; i--)
      {
	  (value & (1 << i)) != 0 ? (databit = STV5730_DATA) : (databit = 0);
	  port_out (stv5730_lptport, databit + STV5730_CLK + stv5730_flags);
	  stv5730_upause (IODELAY);
	  port_out (stv5730_lptport, databit + stv5730_flags);
	  stv5730_upause (IODELAY);
	  port_out (stv5730_lptport, databit + STV5730_CLK + stv5730_flags);
	  stv5730_upause (IODELAY);
      }

    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
    stv5730_upause (IODELAY);
}

void
stv5730_write8bit (unsigned int value)
{
    char i, databit;
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CLK + stv5730_flags);

    for (i = 7; i >= 0; i--)
      {
	  (value & (1 << i)) != 0 ? (databit = STV5730_DATA) : (databit = 0);
	  port_out (stv5730_lptport, databit + STV5730_CLK + stv5730_flags);
	  stv5730_upause (IODELAY);
	  port_out (stv5730_lptport, databit + stv5730_flags);
	  stv5730_upause (IODELAY);
	  port_out (stv5730_lptport, databit + STV5730_CLK + stv5730_flags);
	  stv5730_upause (IODELAY);
      }

    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
}

void
stv5730_write0bit (void)
{
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CLK + stv5730_flags);

    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + STV5730_CLK + stv5730_flags);
    stv5730_upause (IODELAY);
    port_out (stv5730_lptport, STV5730_CSN + stv5730_flags);
}


/////////////////////////////////////////////////////////////////
// sets the memory pointer inside the stv5730 to the position
// row, col.
void
stv5730_locate (int row, int col)
{
    if (row < 0 || row >= STV5730_HGT || col < 0 || col >= STV5730_WID)
	return;

    stv5730_write16bit ((row << 8) + col);
}

/////////////////////////////////////////////////////////////////
// draws  char z from fontmap to the framebuffer at position
// x,y. These are zero-based textmode positions.
// We need a conversion map to translate from ascii to the
// non-standard STV5730 charset.
//
void
stv5730_drawchar2fb (int x, int y, unsigned char z)
{

    if (x < 0 || x >= STV5730_WID || y < 0 || y >= STV5730_HGT)
	return;
    stv5730->framebuf[(y * STV5730_WID) + x] = stv5730_to_ascii[(unsigned int) z];

}

/////////////////////////////////////////////////////////////////
// This initialises the stuff. We support supplying port as 
// a command line argument.
// 
int
stv5730_init (struct lcd_logical_driver *driver, char *args)
{
    char *argv[64], *str;
    int argc, i;

    stv5730 = driver;

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
			       "stv5730_init: %s requires an argument\n",
			       argv[i]);
		      return -1;
		  }
		else
		  {
		      int myport;
		      if (sscanf (argv[i + 1], "%i", &myport) != 1)
			{
			    fprintf (stderr,
				     "stv5730_init: Couldn't read port address -"
				     " using default value 0x%x\n",
				     stv5730_lptport);
			    return -1;
			}
		      else
			{
			    stv5730_lptport = myport;
			    ++i;
			}
		  }
	    }
	  else if (0 == strcmp (argv[i], "-h")
		   || 0 == strcmp (argv[i], "--help"))
	    {
		int i;
		printf
		    ("LCDproc stv5730 driver\n\t-p n\t--port n\tSelect the output device to use port n\n");
		printf ("put the options in quotes like this:  '-p 0x278'\n");
		printf ("\t-h\t--help\t\tShow this help information\n");
		return -1;
	    }
      }

    driver->wid = STV5730_WID;
    driver->hgt = STV5730_HGT;

    {
	struct sched_param param;
	param.sched_priority = 1;
	if ((sched_setscheduler (0, SCHED_RR, &param)) == -1)
	  {
	      fprintf (stderr, "stv5730_init: failed (%s)\n",
		       strerror (errno));
	      return -1;
	  }
    }

    // Initialize the Port and the stv5730
    if (port_access (stv5730_lptport) || port_access (stv5730_lptport + 1))
      {
	  printf
	      ("Couldn't get IO-permission for 0x%X ! Are we running as root ?\n ",
	       stv5730_lptport); return -1;
      };

    if (stv5730_detect ())
      {
	  printf ("No STV5730 hardware found at 0x%X !\n ", stv5730_lptport);
	  return -1;
      };

    port_out (stv5730_lptport, 0);

    // Reset the STV5730
    stv5730_write16bit (0x3000);
    stv5730_write16bit (0x3000);
    stv5730_write16bit (0x00db);
    stv5730_write16bit (0x1000);

    // Setup Mode + Control Register for video detection
    stv5730_write16bit (STV5730_REG_MODE);
    stv5730_write16bit (0x1576);

    stv5730_write16bit (STV5730_REG_CONTROL);
    stv5730_write16bit (0x1FF4);

    printf ("Detecting Video Signal: ");
    usleep (50000);

    if (stv5730_is_mute ())
      {
	  printf ("No Video Signal found, using full page mode.\n");
	  // Setup Mode + Control for full page mode
	  stv5730_charattrib = STV5730_ATTRIB;
	  stv5730_write16bit (STV5730_REG_MODE);
	  stv5730_write16bit (0x15A6);

	  stv5730_write16bit (STV5730_REG_CONTROL);
#ifdef PAL
	  stv5730_write16bit (0x1FD5);
#endif
#ifdef NTSC
	  stv5730_write16bit (0x1ED4);
#endif


      }
    else
      {
	  printf ("Video Signal found, using mixed mode (B&W).\n");
	  // Setup Mode + Control for mixed mode, disable color
	  stv5730_charattrib = 0;
	  stv5730_write16bit (STV5730_REG_MODE);
	  stv5730_write16bit (0x1576);

	  stv5730_write16bit (STV5730_REG_CONTROL);
#ifdef PAL
	  stv5730_write16bit (0x1DD4);
#endif
#ifdef NTSC
	  stv5730_write16bit (0x1CF4);
#endif
      }

    // Position Register
    stv5730_write16bit (STV5730_REG_POSITION);
    stv5730_write16bit (0x1000 + 64 * 30 + 30);

    // Color Register
    stv5730_write16bit (STV5730_REG_COLOR);
    stv5730_write16bit (0x1000 + (STV5730_COL_SBACK << 9) +
			(STV5730_COL_CBORD << 6) + STV5730_COL_CBACK);

    // Zoom Register: Zoom first line
    stv5730_write16bit (STV5730_REG_ZOOM);
    stv5730_write16bit (0x1000 + 4);

    // Set the Row Attributes
    for (i = 0; i <= 10; i++)
      {
	  stv5730_write16bit (0x00C0 + i);
	  stv5730_write16bit (0x10C0);
      }


    // The Framebuffer LCDproc allocates by default is too small,
    // so we free() it and allocate one of adequate size.
    if (!driver->framebuf)
	free (driver->framebuf);

    driver->framebuf = malloc (STV5730_WID * STV5730_HGT);
    if (!driver->framebuf)
      {
	  stv5730_close ();
	  return -1;
      }

    // clear screen
    memset (driver->framebuf, 0, STV5730_WID * STV5730_HGT);

    driver->cellwid = 4;
    driver->cellhgt = 6;	// FIXME: stv5730->cellwid always stays 5
    // regardless what it is set to here. This is
    // a bug but not inside this driver.
    driver->clear = stv5730_clear;
    driver->string = stv5730_string;
    driver->chr = stv5730_chr;
    driver->vbar = stv5730_vbar;
    driver->hbar = stv5730_hbar;
    driver->num = stv5730_num;
    driver->init = stv5730_init;
    driver->close = stv5730_close;
    driver->flush = stv5730_flush;
    driver->flush_box = stv5730_flush_box;
    // We dont't have any programmable chars.
    driver->set_char = NULL;

    driver->icon = stv5730_icon;
    driver->draw_frame = stv5730_draw_frame;
    // We dont't need init for vbar,hbar and friends.
    driver->init_hbar = NULL;
    driver->init_vbar = NULL;
    driver->init_num = NULL;
    // Neither contrast nor backlight are controllable.
    driver->contrast = NULL;
    driver->backlight = NULL;
    // There are some unused input lines that may be used for input,
    // but nothing is programmed so far.
    driver->getkey = NULL;
    return 200;			// 200 is arbitrary.  (must be 1 or more)
}

/////////////////////////////////////////////////////////////////
// Frees the framebuffer and exits the driver.
//
void
stv5730_close ()
{
    if (stv5730->framebuf != NULL)
	free (stv5730->framebuf);
    stv5730->framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Clears the screen
//
void
stv5730_clear ()
{
    memset (stv5730->framebuf, 0x0B, STV5730_WID * STV5730_HGT);
}

/////////////////////////////////////////////////////////////////
// 
// Flushes all output to the lcd...
//
void
stv5730_flush ()
{
    stv5730->draw_frame (stv5730->framebuf);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the screen, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (28,11).
//
void
stv5730_string (int x, int y, char string[])
{
    int i;
    x--;			// Convert 1-based coords to 0-based...
    y--;

    for (i = 0; string[i]; i++)
      {
	  stv5730_drawchar2fb (x + i, y, string[i]);
      }
}

/////////////////////////////////////////////////////////////////
// Writes  char c at position x,y into the framebuffer.
// x and y are 1-based textmode coordinates.
//
void
stv5730_chr (int x, int y, char c)
{
    y--;
    x--;
    stv5730_drawchar2fb (x, y, c);
}

/////////////////////////////////////////////////////////////////
// This function draws ugly big numbers. We could use the zoom
// feature of the stv5730 if we'd know when big numbers start
// and stop.
void
stv5730_num (int x, int num)
{

    int i, j;
    x--;

    if (x < 0 || x > 19 || num < 0 || num > 10)
	return;
    if (num != 10 && (x < 0 || x > 17))
	return;
    if (num == 10 && (x < 0 || x > 19))
	return;

    for (j = 1; j < 10; j++)
      {
	  if (num != 10)
	    {
		for (i = 0; i < 3; i++)
		    stv5730_drawchar2fb (x + i, j, '0' + num);
	    }
	  else
	    {
		stv5730_drawchar2fb (x, j, ':');
	    }
      }

}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar from the bottom up to the last 7 rows of the 
// framebuffer at 1-based position x. len is given in pixels.
// 
void
stv5730_vbar (int x, int len)
{

    int i;
    x--;

    if (x < 0 || len < 0 || (len / 6) >= STV5730_WID)
	return;

    for (i = 0; i <= len; i += 6)
      {

	  if (len >= (i + 6))
	    {
		stv5730->framebuf[((10 - (i / 6)) * STV5730_WID) + x] = 0x77;
	    }
	  else
	    {
		stv5730->framebuf[((10 - (i / 6)) * STV5730_WID) + x] =
		    0x72 + (len % 6);
	    }
      }

}


/////////////////////////////////////////////////////////////////
// Draws a horizontal bar from left to right at 1-based position 
// x,y into the framebuffer. len is given in pixels.
// It uses the STV5730 'channel-tuning' chars(0x64-0x68) to do 
// this.
void
stv5730_hbar (int x, int y, int len)
{
    int i;
    x--;
    y--;

    if (y < 0 || y >= STV5730_HGT || x < 0 || len < 0
	|| (x + (len / 5)) >= STV5730_WID)
	return;

    for (i = 0; i <= len; i += 5)
      {

	  if (len >= (i + 4))
	    {
		stv5730->framebuf[(y * STV5730_WID) + x + (i / 5)] = 0x64;
	    }
	  else
	    {
		stv5730->framebuf[(y * STV5730_WID) + x + (i / 5)] =
		    0x65 + (len % 5);
	    }
      }
}

/////////////////////////////////////////////////////////////////
// Reprogrammes character dest to contain an icon given by 
// which. 
// The STV5730 has no programmable chars. The charset is very
// limited, it doesn't even contain a '%' char. But wait...
// It contains a heartbeat char ! :-)
void
stv5730_icon (int which, char dest)
{
    switch (which)
      {
      case 0:			// 0:empty Heart
	  stv5730_to_ascii[(int) dest] = 0x71;
	  break;
      case 1:			// 1:Filled Heart
	  stv5730_to_ascii[(int) dest] = 0x0B;
	  break;
      case 2:			// 2:Ellipsis
	  stv5730_to_ascii[(int) dest] = 0x5F;
	  break;
      default:
	  stv5730_to_ascii[(int) dest] = 0x0B;
	  break;

      }

}

/////////////////////////////////////////////////////////////////
// Send a rectangular area from lft,top to rgt,bot to the display
// These coordinates are probably one-based, too. It's so fast to
// flush the whole display that it makes no sense to flush less then
// the whole display. Therefore this function redraws the whole
// display. 
// FIXME: Check if this function is worth implementing.
//
void
stv5730_flush_box (int lft, int top, int rgt, int bot)
{
    stv5730_flush ();
}

/////////////////////////////////////////////////////////////////
// Outputs the whole framebuffer *dat to the display.
// Attributes are set for every row only.
// The first line has special attributes.
void
stv5730_draw_frame (char *dat)
{
    int i, j, atr;
    if (!dat)
	return;

    stv5730_locate (0, 0);
    
    for (i = 0; i < STV5730_HGT; i++)
      {
	  if (i == 0)
	      atr = (STV5730_COL_FLINE << 8);
	  else
	      atr = (STV5730_COL_TEXT << 8);
	  stv5730_write16bit (0x1000 + atr + dat[i * STV5730_WID] +
			      stv5730_charattrib);
	  for (j = 1; j < STV5730_WID; j++)
	    {
		if (dat[j + (i * STV5730_WID) - 1] !=
		    dat[j + (i * STV5730_WID)])
		    stv5730_write8bit (dat[j + (i * STV5730_WID)]);
		else
		    stv5730_write0bit ();

	    };
      }
}

/////////////////////////////////////////////////////////////////
// This function delays a legth given by delayCalls
// x 10 Nanoseconds
//
void
stv5730_upause (int delayCalls)
{
    struct timespec delay, remaining;
    delay.tv_sec = 0;
    delay.tv_nsec = delayCalls * 10;
    while (nanosleep (&delay, &remaining) == -1)
      {
	  delay.tv_sec = remaining.tv_sec;
	  delay.tv_nsec = remaining.tv_nsec;
      }
}
