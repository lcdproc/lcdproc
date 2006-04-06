/* Code file for BayRAD driver
 * for LCDproc LCD software
 * by Nathan Yawn, yawn@emacinc.com
 * 3/24/01
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef SOLARIS
#  include <strings.h>
#endif
#include "lcd.h"
#include "bayrad.h"
//#include "drv_base.h"
#include "shared/str.h"
#include "report.h"
#include "lcd_lib.h"

#define NUM_CCs 8 /* number of characters */
#define CCMODE_STANDARD 0 /* only char 0 is used for heartbeat */
#define CCMODE_VBAR 1
#define CCMODE_HBAR 2
#define CCMODE_BIGNUM 3

#define BAYRAD_DEFAULT_DEVICE	"/dev/lcd"

//////////////////////////////////////////////////////////////////////////
////////////////////// Base "class" to derive from ///////////////////////
//////////////////////////////////////////////////////////////////////////

static int fd;
static int width = 0;
static int height = 0;
static int cellwidth = 5;
static int cellheight = 8;
static char *framebuf = NULL;
static char ccmode = CCMODE_STANDARD;

  /////////////////////////////////////////////////////////////
 /* Declare a bunch of global, static custom character data */
/////////////////////////////////////////////////////////////

char bar_up[7][5*8] = {
  {
	 0,0,0,0,0, //  char u1[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
      },
      {
	 0,0,0,0,0, //  char u2[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u3[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u4[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u5[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u6[] =
	 0,0,0,0,0,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },{
	 0,0,0,0,0, //  char u7[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },};


char bar_down[7][5*8] = {  /* Presently, this is not used */
     {
	 1,1,1,1,1, //  char d1[] =
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d2[] =
	 1,1,1,1,1,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d3[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d4[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d5[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 0,0,0,0,0,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d6[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 0,0,0,0,0,
	 0,0,0,0,0,
      },{
	 1,1,1,1,1, //  char d7[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 0,0,0,0,0,
      },};

char bar_right[5][5*8] = {
  {
	 1,0,0,0,0, //  char r1[] =
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
	 1,0,0,0,0,
      },{
	 1,1,0,0,0, //  char r2[] =
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
	 1,1,0,0,0,
      },{
	 1,1,1,0,0, //  char r3[] =
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
	 1,1,1,0,0,
      },{
	 1,1,1,1,0, //  char r4[] =
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
	 1,1,1,1,0,
      },{
	 1,1,1,1,1, //  char r5[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },};

char bar_left[5][5*8] = {  /* Presently, this is not used. */
  {
	 0,0,0,0,1, //  char l1[] =
	 0,0,0,0,1,
	 0,0,0,0,1,
	 0,0,0,0,1,
	 0,0,0,0,1,
	 0,0,0,0,1,
	 0,0,0,0,1,
	 0,0,0,0,1,
      },{
	 0,0,0,1,1, //  char l2[] =
	 0,0,0,1,1,
	 0,0,0,1,1,
	 0,0,0,1,1,
	 0,0,0,1,1,
	 0,0,0,1,1,
	 0,0,0,1,1,
	 0,0,0,1,1,
      },{
	 0,0,1,1,1, //  char l3[] =
	 0,0,1,1,1,
	 0,0,1,1,1,
	 0,0,1,1,1,
	 0,0,1,1,1,
	 0,0,1,1,1,
	 0,0,1,1,1,
	 0,0,1,1,1,
      },{
	 0,1,1,1,1, //  char l4[] =
	 0,1,1,1,1,
	 0,1,1,1,1,
	 0,1,1,1,1,
	 0,1,1,1,1,
	 0,1,1,1,1,
	 0,1,1,1,1,
	 0,1,1,1,1,
      },{
	 1,1,1,1,1, //  char l5[] =
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
	 1,1,1,1,1,
      },};


char icons[3][5*8] = {
   {
     1,1,1,1,1,  // Empty Heart
     1,0,1,0,1,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,0,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },

   {
     1,1,1,1,1,  // Filled Heart
     1,0,1,0,1,
     0,1,0,1,0,
     0,1,1,1,0,
     0,1,1,1,0,
     1,0,1,0,1,
     1,1,0,1,1,
     1,1,1,1,1,
   },

   {
     0,0,0,0,0,  // Ellipsis
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     0,0,0,0,0,
     1,0,1,0,1,
   },

  };


// Vars for the server core
MODULE_EXPORT char * api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 1;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "bayrad_";


////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
MODULE_EXPORT int
bayrad_init(Driver *drvthis)
{

   char device[256] = BAYRAD_DEFAULT_DEVICE;
   int speed = B9600;
   struct termios portset;

   width = 20;
   height = 2;

  framebuf = malloc(width * height);

  if(!framebuf)
    {
      bayrad_close(drvthis);
      report(RPT_ERR, "bayrad_init: Error: unable to create BayRAD framebuffer.");
      return -1;
    }

  memset(framebuf, ' ', width * height);

  //cellheight = 5;
  //cellwidth = 8;

  /* Read config file */

  /* What device should be used */
  strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
						   BAYRAD_DEFAULT_DEVICE), sizeof(device));
  device[sizeof(device)-1] = '\0';

  /* What speed to use */
  speed = drvthis->config_get_int(drvthis->name, "Speed", 0, 9600);
  
  if (speed == 1200)       speed = B1200;
  else if (speed == 2400)  speed = B2400;
  else if (speed == 9600)  speed = B9600;
  else if (speed == 19200) speed = B19200;
  else {
    report(RPT_WARNING, "bayrad_init: Illegal speed: %d. Must be one of 1200, 2400, 9600 or 19200. Using default.\n", speed);
    speed = B9600;
  }

   // Set up io port correctly, and open it...
   fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1)
   {
      report(RPT_ERR, "bayrad_init: failed (%s)", strerror(errno));
      return -1;
   }

   //else debug(RPT_DEBUG, "bayrad_init: opened device %s\n", device);

   tcflush(fd, TCIOFLUSH);

   // We use RAW mode
#ifdef HAVE_CFMAKERAW
   // The easy way
   cfmakeraw( &portset );
#else
   // The hard way
   portset.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                         | INLCR | IGNCR | ICRNL | IXON );
   portset.c_oflag &= ~OPOST;
   portset.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
   portset.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
   portset.c_cflag |= CS8 | CREAD | CLOCAL ;
#endif

   portset.c_cc[VTIME] = 0;  // Don't use the timer, no workee
   portset.c_cc[VMIN] = 1;  // Need at least 1 char

   // Set port speed
   cfsetospeed(&portset, B9600);
   cfsetispeed(&portset, B0);

   // Do it...
   tcsetattr(fd, TCSANOW, &portset);
   tcflush(fd, TCIOFLUSH);

   /*------------------------------------*/

   /*** Open the port write-only, then fork off a process that reads chars ?!!? ***/


   /* Reset and clear the BayRAD */
   write(fd, "\x80\x86\x00\x1a\x1e", 5);  // sync,reset to type 0, clear screen, home

  return 0;
}


// Below here, you may use either lcd.framebuf or drvthis->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

MODULE_EXPORT void
bayrad_close(Driver * drvthis)
{
  //debug(RPT_DEBUG, "\nClosing BayRAD.\n");
  write(fd, "\x8e\x00", 2);  // Backlight OFF

  if(framebuf) free(framebuf);
  framebuf = NULL;

  close(fd);
}


/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
bayrad_width(Driver * drvthis)
{
	return width;
}


/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
bayrad_height(Driver * drvthis)
{
	return height;
}


/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
bayrad_clear(Driver * drvthis)
{
  memset(framebuf, ' ', width * height);
  ccmode = CCMODE_STANDARD;

}


//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
MODULE_EXPORT void
bayrad_flush(Driver * drvthis)
{

  //debug(RPT_DEBUG, "\nBayRAD flush");

  write(fd, "\x80\x1e", 2);  //sync, home
  write(fd, framebuf, 20);
  write(fd, "\x1e\x0a", 2);  //home, LF
  write(fd, framebuf+20, 20);

  return;
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
bayrad_string(Driver * drvthis, int x, int y, char string[])
{
  int i;
  unsigned char c;

  //debug(RPT_DEBUG, "\nPutting string %s at %i, %i", string, x, y);

  x -= 1;  // Convert 1-based coords to 0-based...
  y -= 1;

  for(i=0; string[i]; i++)
  {
     // Check for buffer overflows...
     if((y*width) + x + i  >  (width*height))
       break;

     c = (unsigned char) string[i];

     if(c> 0x7F && c < 0x98)
       {
	 //c &= 0x7F;
	 report(RPT_WARNING, "bayrad_strign: Illegal char %#x requested in bayrad_string()!", c);
	 c = ' ';

       }

     if(c < 8)      /* The custom characters are mapped at 0x98 - 0x9F, */
       c += 0x98;   /* as 0x07 makes a beep instead of printing a character */


     framebuf[(y*width) + x + i] = c;
  }
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,2).
//
MODULE_EXPORT void
bayrad_chr(Driver * drvthis, int x, int y, char c)
{
  unsigned char ch;

  //debug(RPT_DEBUG, "\nPutting char %c (%#x) at %i, %i", c, c, x, y);

  y--;
  x--;
  ch = (unsigned char) c;

  if(ch > 0x7F && ch < 0x98)
    {
      report(RPT_WARNING, "Illegal char %#x requested in bayrad_chr()!", ch);
      ch = ' ';
    }

  /* No shifting the custom chars here, so bayrad_chr() can beep */

  framebuf[(y*width) + x] = ch;
}

//////////////////////////////////////////////////////////////////////
// Turns the lcd backlight on or off...
//
MODULE_EXPORT void
bayrad_backlight(Driver * drvthis, int on)
{

  /* This violates the LCDd driver model, but it does leave the
   * backlight control entirely in the hands of the user via
   * BayRAd buttons, which is nice, since the backlights have
   * a finite lifespan... */

  if(on)
  {;
    //write(fd, "\x8e\x0f", 2);
    //debug(RPT_DEBUG, "Backlight ON\n");
  }
  else
  {;
    //write(fd, "\x8e\x00", 2);
    //debug(RPT_DEBUG, "Backlight OFF\n");
  }

}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for vertical bargraphs.
//
void
bayrad_init_vbar(Driver * drvthis)
{
  //debug(RPT_DEBUG,"Init Vertical bars.\n");

  if( ccmode == CCMODE_VBAR ) {
    /* Work already done */
    return;
  }

  if( ccmode != CCMODE_STANDARD ) {
    /* Not supported (yet) */
    report( RPT_WARNING, "bayrad_init_vbar: Cannot combine two modes using user defined characters" );
    return;
  }
  ccmode = CCMODE_VBAR;

  bayrad_set_char(drvthis, 1, bar_up[0]);
  bayrad_set_char(drvthis, 2, bar_up[1]);
  bayrad_set_char(drvthis, 3, bar_up[2]);
  bayrad_set_char(drvthis, 4, bar_up[3]);
  bayrad_set_char(drvthis, 5, bar_up[4]);
  bayrad_set_char(drvthis, 6, bar_up[5]);
  bayrad_set_char(drvthis, 7, bar_up[6]);


  return;
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for horizontal bargraphs.
//
void
bayrad_init_hbar(Driver * drvthis)
{
  //debug(RPT_DEBUG,"Init Horizontal bars.\n");

  if( ccmode == CCMODE_HBAR ) {
    /* Work already done */
    return;
  }

  if( ccmode != CCMODE_STANDARD ) {
    /* Not supported (yet) */
    report( RPT_WARNING, "bayrad_init_hbar: Cannot combine two modes using user defined characters" );
    return;
  }
  ccmode = CCMODE_HBAR;

  bayrad_set_char(drvthis, 1, bar_right[0]);
  bayrad_set_char(drvthis, 2, bar_right[1]);
  bayrad_set_char(drvthis, 3, bar_right[2]);
  bayrad_set_char(drvthis, 4, bar_right[3]);
  bayrad_set_char(drvthis, 5, bar_right[4]);

return;
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for big numbers, if possible.
//
void
bayrad_init_num(Driver * drvthis)
{
//  debug(RPT_DEBUG,"Big Numbers.\n");
}

//////////////////////////////////////////////////////////////////////
// Draws a big (4-row) number.
//
MODULE_EXPORT void
bayrad_num(Driver * drvthis, int x, int num)
{
//  debug(RPT_DEBUG,"BigNum(%i, %i)\n", x, num);
}

//////////////////////////////////////////////////////////////////////
// Changes the font data of character n.
//
MODULE_EXPORT void
bayrad_set_char(Driver * drvthis, int n, char *dat)
{
  char out[4];
  int row, col;
  char letter;

  //debug(RPT_DEBUG, "\nSet char %i", n);

  if(n < 0 || n > 7) /* Do we want to the aliased indexes as well (0x98 - 0x9F?) */
    return;


  if(!dat)
    return;

  n = 0x40 + (n * 8);  /* Set n to the proper location in CG RAM */

  /* Set the LCD to accept data for rewrite-able char n */
  snprintf(out, sizeof(out), "\x88%c", n);
  write(fd, out, 2);

  for(row=0; row<cellheight; row++)
  {
    letter = 0;
    for(col=0; col<cellwidth; col++)
    {
      letter <<= 1;
      letter |= (dat[(row*cellwidth) + col] > 0);
    }
    write(fd, &letter, 1);
  }

  /* return the LCD to normal operation */
  write(fd, "\x80", 1);

return;
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
MODULE_EXPORT void
bayrad_vbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
  //debug(RPT_DEBUG, "\nVbar at %i, length %i", x, len);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'up' direction
   * (other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles (0..1000) that the bar should be filled.
   */

  bayrad_init_vbar(drvthis);

  lib_vbar_static(drvthis, x, y, len, promille, options, cellheight, 0x98);
}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
bayrad_hbar(Driver * drvthis, int x, int y, int len, int promille, int options)
{
  //debug(RPT_DEBUG, "\nHbar at %i,%i; length %i", x, y, len);

  /* x and y are the start position of the bar.
   * The bar by default grows in the 'right' direction
   * (other direction not yet implemented).
   * len is the number of characters that the bar is long at 100%
   * promille is the number of promilles (0..1000) that the bar should be filled.
   */

  bayrad_init_hbar(drvthis);

  lib_hbar_static(drvthis, x, y, len, promille, options, cellwidth, 0x98);
}


/////////////////////////////////////////////////////////////////
// Places an icon on screen
//
MODULE_EXPORT int
bayrad_icon(Driver * drvthis, int x, int y, int icon)
{
  switch( icon ) {
    case ICON_BLOCK_FILLED:
      bayrad_chr( drvthis, x, y, 0xFF );
      break;
    default:
      return -1; /* Let the core do other icons */
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
MODULE_EXPORT const char *
bayrad_get_key(Driver * drvthis)
{
  fd_set brfdset;
  struct timeval twait;
  char readchar;
  int retval;
  static char ret_val[2] = {0,0};

  //debug(RPT_DEBUG, "\nBayRAD get_key...");

  /* Check for incoming data.  Turn backlight ON/OFF as needed */

  /* TODO: NEEDS TO BE ADAPTED TO RETURN REAL KEY DESCRIPTION STRINGS ! */

  FD_ZERO(&brfdset);
  FD_SET(fd, &brfdset);

  twait.tv_sec = 0;
  twait.tv_usec = 0;

  if(select(fd+1, &brfdset, NULL, NULL, &twait))
    {
      retval = read(fd, &readchar, 1);
      if(retval > 0)
	{
          debug(RPT_INFO, "bayrad_get_key: Got key: %c", readchar);

	  if(readchar == 'Y')
	    {
	      write(fd, "\x8e\x0f", 2);
	    }
	  else if(readchar == 'N')
	    {
	      write(fd, "\x8e\x00", 2);
	    }

	}  /* if read returned data */
      else
	{  /* Read error */
	  report(RPT_ERR, "bayrad_get_key: Read error in BayRAD getchar.");
	}
    }  /* if select */
  else
    {
      ;//debug(RPT_DEBUG, "No BayRAD data present.");
    }

  ret_val[0] = readchar;
  return ret_val;
}

