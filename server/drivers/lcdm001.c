/*lcdm001.c*/
//////////////////////////////////////////////////////////////
//// Driver for the LCDM001 device from kernelconcepts.de ////
////          written by Rene Wagner <reenoo@gmx.de>      ////
//////////////////////////////////////////////////////////////

/*LCDM001 does NOT support custom chars
   So the output may look a bit strange
   Most of the "strange output" has been fixed by
   using ASCII workarounds*/

/*All the functions neccessary for running "normal" clients
   have been implemented.
   Thanks to the curses driver ;) heartbeat works as well now.
   The chars that are used instead of the heartbeat-icons can
   be set in lcdm001.h*/

/*Most of the code has been stolen from MtxOrb.c ;)
   So if you make changes here, do the same with MtxOrb.c*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif

#include "lcd.h"
#include "lcdm001.h"
#include "shared/str.h"
#include "shared/report.h"
#include "configfile.h"
#include "render.h"

// Moved here from lcdm001.h to reduce the number of warning.
static void lcdm001_close ();
static void lcdm001_clear ();
static void lcdm001_flush ();
static void lcdm001_string (int x, int y, char string[]);
static void lcdm001_chr (int x, int y, char c);
static void lcdm001_output (int on);
static void lcdm001_vbar (int x, int len);
static void lcdm001_hbar (int x, int y, int len);
static void lcdm001_num (int x, int num);
static void lcdm001_icon (int which, char dest);
static void lcdm001_flush_box (int lft, int top, int rgt, int bot);
static void lcdm001_draw_frame (char *dat);
static char lcdm001_getkey ();
// End of extract from lcdm001.h by David GLAUDE.
static void lcdm001_heartbeat (int type);

#define NotEnoughArgs (i + 1 > argc)

lcd_logical_driver *lcdm001;
int fd;
static int clear = 1;
static char icon_char = '@';


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

static void lcdm001_cursorblink (int on);
static void lcdm001_string (int x, int y, char *string);

#define ValidX(x) if ((x) > lcdm001->wid) { (x) = lcdm001->wid; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > lcdm001->hgt) { (y) = lcdm001->hgt; } else (y) = (y) < 1 ? 1 : (y);

// Set cursorblink on/off
//
static void
lcdm001_cursorblink (int on)
{
	if (on) {
		write (fd, "~K1", 3);
		debug(RPT_INFO, "LCDM001: cursorblink turned on");
	} else {
		write (fd, "~K0", 3);
		debug(RPT_INFO, "LCDM001: cursorblink turned off");
	}
}


// TODO: Get lcd.framebuf to properly work as whatever driver is running...

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
int
lcdm001_init (struct lcd_logical_driver *driver, char *args)
{
        char device[256];
        int speed=B38400;
        struct termios portset;

	char out[5]="";

	lcdm001 = driver;

	debug( RPT_INFO, "LCDM001: init(%p,%s)", driver, args );

	driver->wid = 20;
	driver->hgt = 4;

	// You must use driver->framebuf here, but may use lcd.framebuf later.
	if (!driver->framebuf) {
		driver->framebuf = malloc (driver->wid * driver->hgt);
	}

	if (!driver->framebuf) {
		lcdm001_close ();
                report(RPT_ERR, "\nError: unable to create LCDM001 framebuffer.\n");
		return -1;
	}
// Debugging...
//  if(lcd.framebuf) printf("Frame buffer: %i\n", (int)lcd.framebuf);

	memset (driver->framebuf, ' ', driver->wid * driver->hgt);
//  lcdm001_clear();

	driver->cellwid = 5;
	driver->cellhgt = 8;

	// TODO: replace DriverName with driver->name when that field exists.
	#define DriverName "lcdm001"

	//READ CONFIG FILE:

	//which serial device should be used
	strncpy(device, config_get_string ( DriverName , "Device" , 0 , "/dev/lcd"),255);
	if (strlen(device)>254) strncat(device, "\0", 1);
	report (RPT_INFO,"LCDM001: Using device: %s", device);

	// Set up io port correctly, and open it...
	debug( RPT_DEBUG, "LCDM001: Opening serial device: %s", device);
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		switch (errno) {
			case ENOENT: report( RPT_ERR, "LCDM001: lcdm001_init() failed: Device file missing: %s\n", device);
				break;
			case EACCES: report( RPT_ERR, "LCDM001: lcdm001_init() failed: Could not open device: %s\n", device);
				report( RPT_ERR, "LCDM001: lcdm001_init() failed: Make sure you have rw access to %s!\n", device);
				break;
			default: report( RPT_ERR, "LCDM001: lcdm001_init() failed (%s)\n", strerror (errno));
				break;
		}
  		return -1;
	} else {
		report (RPT_INFO, "opened LCDM001 display on %s\n", device);
	}
	tcgetattr(fd, &portset);
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
	cfsetospeed(&portset, speed);
	cfsetispeed(&portset, speed);
	tcsetattr(fd, TCSANOW, &portset);
	tcflush(fd, TCIOFLUSH);

	// Reset and clear the LCDM001
	write (fd, "~C", 2);
	//Set cursorblink default
	lcdm001_cursorblink (DEFAULT_CURSORBLINK);
	// Turn all LEDs off
	snprintf (out, sizeof(out), "\%cL%c%c", 126, 0, 0);
	write (fd, out, 4);

        /*
         * Configure the display functions
        */

	driver->clear = lcdm001_clear;
	driver->string = lcdm001_string;
	driver->chr = lcdm001_chr;
	driver->vbar =lcdm001_vbar;
	//init_vbar is not needed
	driver->hbar = lcdm001_hbar;
	//init_hbar is not needed
	driver->num = lcdm001_num;
	//init_num is not needed

	driver->init = lcdm001_init;
	driver->close = lcdm001_close;
	driver->flush = lcdm001_flush;
	driver->flush_box = lcdm001_flush_box;
	//contrast and backlight are not implemented as
	//changing the contrast or the state of the backlight
	//is not supported by the device
	//Well ... you could make use of your screw driver and
	//soldering iron ;)
	driver->output = lcdm001_output;
	//set_char is not implemented as custom chars are not
	//supported by the device
	driver->icon = lcdm001_icon;
	driver->draw_frame = lcdm001_draw_frame;

	driver->getkey = lcdm001_getkey;
	driver->heartbeat = lcdm001_heartbeat;

	return fd;
}

// Below here, you may use either lcd.framebuf or driver->framebuf..
// lcd.framebuf will be set to the appropriate buffer before calling
// your driver.

static void
lcdm001_close ()
{
	char out[5];
	if (lcdm001->framebuf != NULL)
		free (lcdm001->framebuf);

	lcdm001->framebuf = NULL;
	//switch off all LEDs
	snprintf (out, sizeof(out), "\%cL%c%c", 126, 0, 0);
	write (fd, out, 4);
	close (fd);

        report (RPT_INFO, "LCDM001: closed");
}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
static void
lcdm001_clear ()
{
        if (lcdm001->framebuf != NULL)
                memset (lcdm001->framebuf, ' ', (lcdm001->wid * lcdm001->hgt));

	write (fd, "~C", 2); // instant clear...
        clear = 1;

	debug (RPT_DEBUG, "LCDM001: cleared screen");
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
lcdm001_flush ()
{
	lcdm001_draw_frame(lcdm001->framebuf);

        debug (RPT_DEBUG, "LCDM001: frame buffer flushed");
}

//////////////////////////////////////////////////////////////////////
// Send a rectangular area to the display.
//

static void
lcdm001_flush_box (int lft, int top, int rgt, int bot)
{
	int y;
	char out[LCD_MAX_WIDTH];

	for (y = top; y <= bot; y++) {
		snprintf (out, sizeof(out), "%cP%c%c", 126, lft, y);
		write (fd, out, 4);
		write (fd, lcdm001->framebuf + (y * lcdm001->wid) + lft, rgt - lft + 1);
	}
	debug (RPT_DEBUG, "LCDM001: frame buffer box flushed");
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
static void
lcdm001_chr (int x, int y, char c)
{
	char buf[64]; // char out[10];
	int offset;

	ValidX(x);
	ValidY(y);

        if (c==0) {
                c = icon_char;   //heartbeat workaround
        }

	// write to frame buffer
	y--; x--; // translate to 0-coords

	offset = (y * lcdm001->wid) + x;
	lcdm001->framebuf[offset] = c;

	snprintf(buf, sizeof(buf), "LCDM001: writing character %02X to position (%d,%d)", c, x, y);
	debug (RPT_DEBUG, buf);
}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
static void
lcdm001_string (int x, int y, char string[])
{
	int offset, siz;

	ValidX(x);
	ValidY(y);

	x--; y--; // Convert 1-based coords to 0-based...
	offset = (y * lcdm001->wid) + x;
	siz = (lcdm001->wid * lcdm001->hgt) - offset - 1;
	siz = siz > strlen(string) ? strlen(string) : siz;

	memcpy(lcdm001->framebuf + offset, string, siz);

	debug (RPT_DEBUG, "LCDM001: printed string at (%d,%d)", x, y);
}

/////////////////////////////////////////////////////////////////
// Controls LEDs
static void
lcdm001_output (int on)
{
	char out[5];
	int one = 0, two = 0;

	if (on<=255)
	{
		one=on;
		two=0;
	}
	else
	{
		one = on & 0xff;
		two = (on >> 8) & 0xff;
	}
        snprintf (out, sizeof(out), "~L%c%c",one,two);
        write(fd,out,4);

        debug (RPT_DEBUG, "LCDM001: current LED state: %d", on);
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
static void
lcdm001_vbar(int x, int len)
{
   int y = 4;

   debug (RPT_DEBUG , "LCDM001: vertical bar at %d set to %d", x, len);

   while (len >= 8)
     {
       lcdm001_chr(x, y, 0xFF);
       len -= 8;
       y--;
     }

   if(!len)
     return;

  //TODO: Distinguish between len>=4 and len<4

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
static void
lcdm001_hbar(int x, int y, int len)
{

  ValidX(x);
  ValidY(y);

  debug (RPT_DEBUG, "LCDM001: horizontal bar at %d set to %d", x, len);

  //TODO: Improve this function

  while((x <= lcdm001->wid) && (len > 0))
  {
    if(len < lcdm001->cellwid)
      {
	//lcdm001_chr(x, y, 0x98 + len);
	break;
      }

    lcdm001_chr(x, y, 0xFF);
    len -= lcdm001->cellwid;
    x++;
  }

  return;
}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
static void lcdm001_num (int x, int num)
{
	int y, dx;

	debug (RPT_DEBUG, "LCDM001: Writing big number \"%d\" at x = %d", num, x);

	/*This function uses an ASCII emulation of big numbers*/

	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			lcdm001_chr (x + dx, y, num_icon[num][y-1][dx]);
}

/////////////////////////////////////////////////////////////////
// Sets character 0 to an icon...
//
void
lcdm001_icon (int which, char dest)
{

/*Heartbeat workaround:
   As custom chars are not supported OPEN_HEART
   and FILLED_HEART are displayed instead.
   Change them in lcdm001.h*/

	if (dest == 0)
		switch (which) {
			case 0:
				icon_char = OPEN_HEART;
				break;
			case 1:
				icon_char = FILLED_HEART;
				break;
			default:
				icon_char = PAD;
				break;
		}
}

//////////////////////////////////////////////////////////////////////
// Draws the framebuffer on the display.
//
// The commented-out code is from the text driver.
//
void
lcdm001_draw_frame (char *dat)
{

        //TODO: Check whether this is still correct

	write(fd,lcdm001->framebuf,80);
}

//////////////////////////////////////////////////////////////////////
// Tries to read a character from an input device...
//
// Return 0 for "nothing available".
//
static char
lcdm001_getkey ()
{
        char in = 0;
        read (fd, &in, 1);
	/*debug: if(in) fprintf(stderr,"key: %c",in); */
        return in;
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
static void
lcdm001_heartbeat (int type)
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
		lcdm001_icon (whichIcon, 0);

		// Put character on screen...
		lcdm001_chr (lcdm001->wid, 1, 0);

		// change display...
		lcdm001_flush ();
	}

	timer++;
	timer &= 0x0f;
}
