/*
 *
 * Chris Debenham - eSun Systems Engineer <chris.debenham@aus.sun.com>
 *
 * Heres a bit more info on the display.
 * It is the LB216 and is made by R.T.N. Australia
 * The web page for it is http://www.nollet.com.au/
 * It is a serial 16x2 LCD with software controllable backlight.
 * They also make 40x4 displays (which I'll be getting one of soon :-) )
 * 3 wire connection (5V,0V and serial), 2400 or 9600 bps.
 * 8 custom characters
 * 40*83.5MM size
 * made in australia :-)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//#ifdef HAVE_NCURSES_H
//# include <ncurses.h>
//#else
//# include <curses.h>
//#endif

#include "lcd.h"
#include "lb216.h"
#include "shared/str.h"
#include "report.h"
//#include "drv_base.h"

#define LB216_DEFAULT_DEVICE		"/dev/lcd"
#define LB216_DEFAULT_SPEED		9600
#define LB216_DEFAULT_BRIGHTNESS	255

static int custom=0;
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
        beat = 8 } custom_type;


static int fd;
static char *framebuf = NULL;
static int width = LCD_DEFAULT_WIDTH;
static int height = LCD_DEFAULT_HEIGHT;
static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;

static void LB216_hidecursor();
static void LB216_reboot();

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "LB216_";


// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
MODULE_EXPORT int
LB216_init(Driver * drvthis, char *args)
{
   struct termios portset;
   int reboot=0;
   char device[256] = LB216_DEFAULT_DEVICE;
   int speed = LB216_DEFAULT_SPEED;
   int backlight_brightness = LB216_DEFAULT_BRIGHTNESS;


  /* Read config file */

  /* What device should be used */
  strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0,
					     LB216_DEFAULT_DEVICE), sizeof(device));
  device[sizeof(device)-1] = '\0';

  /* What speed to use */
  speed = drvthis->config_get_int(drvthis->name, "Speed", 0, LB216_DEFAULT_SPEED);
  
  if (speed == 2400)       speed = B2400;
  else if (speed == 9600)  speed = B9600;
  else {
    report(RPT_WARNING, "lb216_init: Illegal speed: %d. Must be 2400 or 9600. Using default.\n", speed);
    speed = B9600;
  }

  /* Which backlight brightness */
  backlight_brightness = drvthis->config_get_int ( drvthis->name , "Brightness" , 0 , LB216_DEFAULT_BRIGHTNESS);
  if ((backlight_brightness < 0) || (backlight_brightness > 255)) {
    report (RPT_WARNING, "lb216_init: Brightness must be between 0 and 255. Using default value.\n");
    backlight_brightness = LB216_DEFAULT_BRIGHTNESS;
  }
      
  /* Reboot display? */
  reboot = drvthis->config_get_bool( drvthis->name , "Reboot", 0, 0);
  if (reboot)
    report (RPT_INFO, "LCDd: rebooting LB216 LCD...\n");

  /* End of config file parsing */
      
   // Set up io port correctly, and open it...
   fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1)
   {
      report(RPT_ERR, "lb216_init: open(%s) failed (%s)\n", device, strerror(errno));
      return -1;
   }
   report(RPT_DEBUG, "lb216_init: opened device %s\n", device);

   tcgetattr(fd, &portset);

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

   // Set port speed
   cfsetospeed (&portset, speed);
   cfsetispeed (&portset, B0);

   // Do it...
   tcsetattr(fd, TCSANOW, &portset);

   // Make sure the frame buffer is there...
   framebuf = (unsigned char *) malloc (width * height);
   if (framebuf == NULL) {
      report(RPT_ERR, "lb216_init: unable to create framebuffer.\n");
      return -1;
   }
   memset (framebuf, ' ', width * height);

   // Set display-specific stuff..
   if (reboot)
   {
      LB216_reboot();
      sleep(4);
      reboot=0;
   }
   sleep(1);
   LB216_hidecursor();
   LB216_backlight(drvthis, backlight_brightness);

   return 0;
}



/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
LB216_close(Driver * drvthis)
{
  close (fd);

  if (framebuf)
    free(framebuf);
  framebuf = NULL;
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
LB216_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
LB216_height (Driver *drvthis)
{
	return height;
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
MODULE_EXPORT void
LB216_clear (Driver * drvthis)
{
	memset (framebuf, ' ', width * height);
}


/////////////////////////////////////////////////////////////////
// Flushes the framebuffer to the LCD
//
MODULE_EXPORT void
LB216_flush(Driver * drvthis)
{
  char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
  int i,j;

  snprintf (out, sizeof(out), "%c%c", 254,80);
  write(fd, out, 2);

  for(j=0; j<height; j++) {
	if (j>=2) {
    	snprintf (out, sizeof(out),"%c%c",254,148+(64*(j-2)));
	} else {
    	snprintf (out, sizeof(out),"%c%c",254,128+(64*(j)));
	}
    write(fd, out, 2);
    for(i=0; i<width; i++) {
      write(fd, framebuf + i+(j*width), 1);
    }
  }
}



/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (16,2).
//
MODULE_EXPORT void
LB216_chr(Driver * drvthis, int x, int y, char c)
{
  //y--;
 // x--;

  //if(c < 32  &&  c >= 0) c += 128;
//  framebuf[(y*width) + x] = c;

//	char chr[1];
//	snprintf (chr, sizeof(chr), "%c", c);
// Above two lines are incorrect (Joris)

	char chr[2];
	chr[0] = c;
	chr[1] = 0;
	LB216_string (drvthis, x, y, chr);
}


/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
MODULE_EXPORT void
LB216_backlight(Driver * drvthis, int on)
{
  char out[4];
  if(on)
  {
    snprintf (out, sizeof(out), "%c%c", 254, 253);
  }
  else
  {
    snprintf (out, sizeof(out), "%c%c", 254, 252);
  }
    write(fd, out, 2);
}


/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void LB216_hidecursor()
{
  char out[4];
  snprintf (out, sizeof(out), "%c%c", 254,12);
  write(fd, out, 2);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void LB216_reboot()
{
  char out[4];
  snprintf (out, sizeof(out), "%c%c", 254,1);
  write(fd, out, 2);
}


MODULE_EXPORT void
LB216_string (Driver * drvthis, int x, int y, char string[])
{
   int i;
   char c;

//printf("%d,%d:%s\n",x,y,string);
   y--;x--;
   for(i=0; string[i]; i++)
   {
      c = string[i];
      switch(c)
      {
         case '\254': c = '#'; break;
      }
      framebuf[(y*width) + x+i] = c;
   }

}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before LB216->vbar()
//
MODULE_EXPORT void
LB216_init_vbar(Driver * drvthis)
{
  char a[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
  };
  char b[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char c[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char d[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char e[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char f[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };
  char g[] = {
    0,0,0,0,0,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };

  if(custom!=vbar) {
    LB216_set_char(drvthis, 1,a);
    LB216_set_char(drvthis, 2,b);
    LB216_set_char(drvthis, 3,c);
    LB216_set_char(drvthis, 4,d);
    LB216_set_char(drvthis, 5,e);
    LB216_set_char(drvthis, 6,f);
    LB216_set_char(drvthis, 7,g);
    custom=vbar;
  }
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
MODULE_EXPORT void
LB216_init_hbar(Driver * drvthis)
{

  char a[] = {
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
    1,0,0,0,0,
  };
  char b[] = {
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
    1,1,0,0,0,
  };
  char c[] = {
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
    1,1,1,0,0,
  };
  char d[] = {
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
    1,1,1,1,0,
  };
  char e[] = {
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
    1,1,1,1,1,
  };

  if(custom!=hbar) {
    LB216_set_char(drvthis, 1,a);
    LB216_set_char(drvthis, 2,b);
    LB216_set_char(drvthis, 3,c);
    LB216_set_char(drvthis, 4,d);
    LB216_set_char(drvthis, 5,e);
    custom=hbar;
  }
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
MODULE_EXPORT void
LB216_vbar(Driver * drvthis, int x, int len)
{
  char map[9] = {32, 1, 2, 3, 4, 5, 6, 7, 255 };


  int y;
  for(y=height; y > 0 && len>0; y--)
    {
      if(len >= cellheight) LB216_chr(drvthis, x, y, 255);
      else LB216_chr(drvthis, x, y, map[len]);

      len -= cellheight;
    }

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
MODULE_EXPORT void
LB216_hbar(Driver * drvthis, int x, int y, int len)
{
  char map[7] = { 32, 1, 2, 3, 4, 5 };

  for(; x<=width && len>0; x++)
    {
      if(len >= cellwidth) LB216_chr(drvthis, x,y,map[5]);
      else LB216_chr(drvthis, x, y, map[len]);

	 //printf ("%d,",len);
      len -= cellwidth;

    }
//	printf ("\n");

}


/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
MODULE_EXPORT void
LB216_set_char(Driver * drvthis, int n, char *dat)
{
  char out[4];
  int row, col;
  int letter;

  if(n < 0 || n > 7) return;
  n=64+(8*n);
  if(!dat) return;

  snprintf (out, sizeof(out), "%c%c", 254, n);
  write(fd, out, 2);

  for(row=0; row<cellheight; row++)
  {
    letter = 1;
    for(col=0; col<cellwidth; col++)
    {
      letter <<= 1;
      letter |= (dat[(row*cellwidth) + col] > 0);
    }
	snprintf (out, sizeof(out),"%c",letter);
    write(fd, out, 1);
  }
}

MODULE_EXPORT int
LB216_icon(Driver * drvthis, int x, int y, int icon)
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
      LB216_chr( drvthis, x, y, 255 );
      break;
    case ICON_HEART_FILLED:
      LB216_set_char( drvthis, 0, heart_filled );
      LB216_chr( drvthis, x, y, 0 );
      break;
    case ICON_HEART_OPEN:
      LB216_set_char( drvthis, 0, heart_open );
      LB216_chr( drvthis, x, y, 0 );
      break;
    default:
      return -1;
  }
  return 0;
}
