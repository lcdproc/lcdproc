/*lcdm001.c*/
/////////////////////////////////////////////////////////////////////////////////////////////////
//// Driver for the LCDM001 device from kernelconcepts.de////
////          written by Rene Wagner <reenoo@gmx.de>          ////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*LCDM001 does NOT support custom chars
   So the output may look a bit strange*/

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
#include <time.h>

#include "lcd.h"
#include "lcdm001.h"
#include "drv_base.h"
#include "shared/debug.h"
#include "shared/str.h"

#define NotEnoughArgs (i + 1 > argc)

extern int debug_level;

lcd_logical_driver *lcdm001;
int fd;
static int clear = 1;
static char icon_char = '@';

static void lcdm001_cursorblink (int on);
static void lcdm001_string (int x, int y, char *string);
static void lcdm001_usage (void);


#define ValidX(a) { if (x > lcdm001->wid) { x = lcdm001->wid; } else x < 1 ? 1 : x; }
#define ValidY(a) { if (y > lcdm001->hgt) { y = lcdm001->hgt; } else y < 1 ? 1 : y; }

// Set cursorblink on/off
//
static void
lcdm001_cursorblink (int on)
{
	if (on) {
		write (fd, "~K1", 3);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "LCDM001: cursorblink turned on");
	} else {
		write (fd, "~K0", 3);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "LCDM001: cursorblink turned off");
	}
}

static void
lcdm001_usage (void) {
	printf ("LCDproc LCDM001 LCD driver\n"
		"\t-d\t\tSelect the output device to use [/dev/lcd]\n"
		"\t-h\t\tShow this help information\n");
}

// TODO: Get lcd.framebuf to properly work as whatever driver is running...

////////////////////////////////////////////////////////////
// init() should set up any device-specific stuff, and
// point all the function pointers.
int
lcdm001_init (struct lcd_logical_driver *driver, char *args)
{
        char device[256]="/dev/lcd"; 
        int speed=B38400;
        char *argv[64];
        int argc;
        int i;
        struct termios portset;
	char c;

	char out[5]="";
  //fprintf(stderr,"lcdm001_init()\n");

	lcdm001 = driver;

	driver->wid = 20;
	driver->hgt = 4;

	// You must use driver->framebuf here, but may use lcd.framebuf later.
	if (!driver->framebuf)
		driver->framebuf = malloc (driver->wid * driver->hgt);

	if (!driver->framebuf) {
		lcdm001_close ();
                fprintf(stderr, "\nError: unable to create LCDM001 framebuffer.\n");
		return -1;
	}
// Debugging...
//  if(lcd.framebuf) printf("Frame buffer: %i\n", (int)lcd.framebuf);

	memset (driver->framebuf, ' ', driver->wid * driver->hgt);
//  lcdm001_clear();

	driver->cellwid = 5;
	driver->cellhgt = 8;

	argc = get_args(argv, args, 64);

	/*
	for(i=0; i<argc; i++)
	{
		printf("Arg(%i): %s\n", i, argv[i]);
	}
	*/

#ifdef USE_GETOPT
        while ((c = getopt(argc, argv, "d:h")) > 0) {
		switch(c) {
			case 'd':
				strncpy(device, optarg, sizeof(device));
				break;
			case 'h':
				lcdm001_usage();
				return -1;
			default:
				lcdm001_usage();
				return -1;
		}
	}
#else
	for (i = 0; i < argc; i++) {
		char *p;

		p = argv[i];
		//printf("Arg(%i): %s\n", i, argv[i]);

		if (*p == '-') {

			p++;
			switch (*p) {
				case 'd':
					if (i + 1 > argc) {
						fprintf (stderr, "lcdm001_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					strcpy (device, argv[++i]);
					break;
				case 'h':
					lcdm001_usage();
					return -1;
					break;
				default:
					printf ("Invalid parameter: %s\n", argv[i]);
					break;
			}
		}
	}
#endif
	// Set up io port correctly, and open it...
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY); 
	if (fd == -1) 
	{
		switch (errno) {
			case ENOENT: fprintf(stderr, "lcdm001_init: %s device file missing!\n", device);
				break;
			case EACCES: fprintf(stderr, "lcdm001_init: %s device could not be opened...\n", device);
				fprintf(stderr, "lcdm001_init: make sure you have rw access to %s!\n", device);
				break;
			default: fprintf (stderr, "lcdm001_init: failed (%s)\n", strerror (errno));
				break;
		}
  		return -1;
	} else
		syslog(LOG_INFO, "opened LCDM001 display on %s\n", device);

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
	driver->init_vbar = lcdm001_init_vbar;
	driver->hbar = lcdm001_hbar;
	driver->init_hbar = lcdm001_init_hbar;
	driver->num = lcdm001_num;
	driver->init_num = lcdm001_num;

	driver->init = lcdm001_init;
	driver->close = lcdm001_close;
	driver->flush = lcdm001_flush;
	driver->flush_box = lcdm001_flush_box;
	driver->contrast = lcdm001_contrast;
	driver->backlight = lcdm001_backlight;
	driver->output = lcdm001_output;
	driver->set_char = lcdm001_set_char;
	driver->icon = lcdm001_icon;
	driver->draw_frame = lcdm001_draw_frame;

	driver->getkey = lcdm001_getkey;

	return fd;
}

/* DUMMY functions: */
static void lcdm001_num (int x, int num)
{
        /*TODO: find out what this function is supposed to do*/
}

static void lcdm001_init_num ()
{
        /*TODO: find out what this function is supposed to do*/
}

static void
lcdm001_backlight (int on)
{
        /*Go and get your soldering iron ...*/
}

static int
lcdm001_contrast (int contrast)
{
        /*Go and get your screwdriver ...*/
	return -1;
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
static void
lcdm001_set_char (int n, char *dat)
{
	/* custom chars are NOT supported */
}

/*End of DUMMY functions*/

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
	snprintf (out, sizeof(out), "\%cL%c%c", 126, 0, 0);
	write (fd, out, 4);
	close (fd);

        if (debug_level > 3)
                syslog(LOG_DEBUG, "LCDM001: closed");
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

	if (debug_level > 3)
		syslog(LOG_DEBUG, "lcdm001: cleared screen");
}

//////////////////////////////////////////////////////////////////
// Flushes all output to the lcd...
//
void
lcdm001_flush ()
{
	lcdm001_draw_frame(lcdm001->framebuf);

        if (debug_level > 4)
                syslog(LOG_DEBUG, "LCDM001: frame buffer flushed");

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

	if (debug_level > 4)
		syslog(LOG_DEBUG, "LCDM001: frame buffer box flushed");
	}
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
static void
lcdm001_chr (int x, int y, char c)
{
	char out[10], buf[64];
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

	if (debug_level > 2) {
		snprintf(buf, sizeof(buf), "writing character %02X to position (%d,%d)",
			c, x, y);
		syslog(LOG_DEBUG, buf);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "LCDM001: printed a char at (%d,%d)", x, y);
	}
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

	if (debug_level > 4)
		syslog(LOG_DEBUG, "LCDM001: printed string at (%d,%d)", x, y);
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

        if (debug_level > 3)
		syslog(LOG_DEBUG, "LCDM001: current LED state: %d", on);
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for vertical bargraphs.
//
static void lcdm001_init_vbar() 
{
  //printf("Init Vertical bars.\n");

  //Nothing to be done

  return;
}

//////////////////////////////////////////////////////////////////////
// Tells the driver to get ready for horizontal bargraphs.
//
static void lcdm001_init_hbar() 
{
  //printf("Init Horizontal bars.\n");  
  //nothing to be done!
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar, from the bottom of the screen up.
//
static void
lcdm001_vbar(int x, int len) 
{
   int y = 4;

   if (debug_level > 4)
		syslog(LOG_DEBUG, "LCDM001: vertical bar at %d set to %d", x, len);

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

  if (debug_level > 4)
		syslog(LOG_DEBUG, "LCDM001: horizontal bar at %d set to %d", x, len);

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
