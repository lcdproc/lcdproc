/*
 * CrystalFontz driver
 *
 * http://www.crystalfontz.com
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "CFontz.h"
#include "drv_base.h"
#include "render.h"
#include "shared/debug.h"
#include "shared/str.h"

#define DEFAULT_CELL_WIDTH 6
#define DEFAULT_CELL_HEIGHT 8
#define DEFAULT_CONTRAST 140
#define DEFAULT_DEVICE "/dev/lcd"
#define DEFAULT_SPEED B9600

static int custom = 0;
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
	beat = 8
} custom_type;

static int fd;

static void CFontz_linewrap (int on);
static void CFontz_autoscroll (int on);
static void CFontz_hidecursor ();
static void CFontz_reboot ();

// TODO:  Get rid of this variable?
lcd_logical_driver *CFontz;
// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
CFontz_init (lcd_logical_driver * driver, char *args)
{
	char *argv[64];
	int argc;
	struct termios portset;
	int i;
	int tmp;
	int reboot = 0;

	int contrast = DEFAULT_CONTRAST;
	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;

	CFontz = driver;

	//debug("CFontz_init: Args(all): %s\n", args);

	argc = get_args (argv, args, 64);

	/*
	   for(i=0; i<argc; i++)
	   {
	   printf("Arg(%i): %s\n", i, argv[i]);
	   }
	 */

	for (i = 0; i < argc; i++) {
		//printf("Arg(%i): %s\n", i, argv[i]);
		if (0 == strcmp (argv[i], "-d") || 0 == strcmp (argv[i], "--device")) {
			if (i + 1 > argc) {
				fprintf (stderr, "CFontz_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strcpy (device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-c") || 0 == strcmp (argv[i], "--contrast")) {
			if (i + 1 > argc) {
				fprintf (stderr, "CFontz_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			tmp = atoi (argv[++i]);
			if ((tmp < 0) || (tmp > 255)) {
				fprintf (stderr, "CFontz_init: %s argument must between 0 and 255. Using default value.\n", argv[i]);
			} else
				contrast = tmp;
		} else if (0 == strcmp (argv[i], "-b") || 0 == strcmp (argv[i], "--brightness")) {
			if (i + 1 > argc) {
				fprintf (stderr, "CFontz_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			tmp = atoi (argv[++i]);
			if ((tmp < 0) || (tmp > 255)) {
				fprintf (stderr, "CFontz_init: %s argument must between 0 and 255. Using default value.\n", argv[i]);
			} else
				backlight_brightness = tmp;
		} else if (0 == strcmp (argv[i], "-o") || 0 == strcmp (argv[i], "--off-bright")) {
			if (i + 1 > argc) {
				fprintf (stderr, "CFontz_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			tmp = atoi (argv[++i]);
			if ((tmp < 0) || (tmp > 255)) {
				fprintf (stderr, "CFontz_init: %s argument must between 0 and 255. Using default value.\n", argv[i]);
			} else
				backlight_off_brightness = tmp;
		} else if (0 == strcmp (argv[i], "-s") || 0 == strcmp (argv[i], "--speed")) {
			if (i + 1 > argc) {
				fprintf (stderr, "CFontz_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			tmp = atoi (argv[++i]);
			if (tmp == 1200)
				speed = B1200;
			else if (tmp == 2400)
				speed = B2400;
			else if (tmp == 9600)
				speed = B9600;
			else {
				fprintf (stderr, "CFontz_init: %s argument must be 1200, 2400, or 9600. Using default value.\n", argv[i]);
			}
		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			printf ("LCDproc CrystalFontz LCD driver\n" "\t-d\t--device\tSelect the output device to use [/dev/lcd]\n" "\t-t\t--type\t\tSelect the LCD type (size) [20x4]\n" "\t-c\t--contrast\tSet the initial contrast [140]\n" "\t-b\t--brightness\tSet the initial brightness [255]\n" "\t-o\t--off-bright\tSet the initial brightness [0]\n" "\t-s\t--speed\t\tSet the communication speed [9600]\n" "\t-r\t--reboot\tReinitialize the LCD's BIOS\n" "\t-h\t--help\t\tShow this help information\n");
			return -1;
		} else if (0 == strcmp (argv[i], "-r") || 0 == strcmp (argv[i], "--reboot")) {
			printf ("LCDd: rebooting CrystalFontz LCD...\n");
			reboot = 1;
		} else {
			printf ("Invalid parameter: %s\n", argv[i]);
		}

	}

	// Set up io port correctly, and open it...
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		fprintf (stderr, "CFontz_init: failed (%s)\n", strerror (errno));
		return -1;
	}
	//else fprintf(stderr, "CFontz_init: opened device %s\n", device);
	tcgetattr (fd, &portset);

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
	tcsetattr (fd, TCSANOW, &portset);

	// Set display-specific stuff..
	if (reboot) {
		CFontz_reboot ();
		sleep (4);
		reboot = 0;
	}
	sleep (1);
	CFontz_hidecursor ();
	CFontz_linewrap (1);
	CFontz_autoscroll (0);
	CFontz_backlight (backlight_brightness);

	// Set the functions the driver supports...

	driver->clear = CFontz_clear;
	driver->string = CFontz_string;
	driver->chr = CFontz_chr;
	driver->vbar = CFontz_vbar;
	driver->init_vbar = CFontz_init_vbar;
	driver->hbar = CFontz_hbar;
	driver->init_hbar = CFontz_init_hbar;
	driver->num = CFontz_num;
	driver->init_num = CFontz_init_num;

	driver->init = CFontz_init;
	driver->close = CFontz_close;
	driver->flush = CFontz_flush;
	driver->flush_box = CFontz_flush_box;
	driver->contrast = CFontz_contrast;
	driver->backlight = CFontz_backlight;
	driver->set_char = CFontz_set_char;
	driver->icon = CFontz_icon;
	driver->draw_frame = CFontz_draw_frame;

	CFontz_contrast (contrast);

	driver->cellwid = DEFAULT_CELL_WIDTH;
	driver->cellhgt = DEFAULT_CELL_HEIGHT;

	debug ("CFontz: foo!\n");

	return fd;
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
void
CFontz_close ()
{
	close (fd);

	if (CFontz->framebuf)
		free (CFontz->framebuf);

	CFontz->framebuf = NULL;
}

void
CFontz_flush ()
{
	CFontz_draw_frame (CFontz->framebuf);
}

void
CFontz_flush_box (int lft, int top, int rgt, int bot)
{
	int y;
	char out[LCD_MAX_WIDTH];

//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

	for (y = top; y <= bot; y++) {
		snprintf (out, sizeof(out), "%c%c%c", 17, lft, y);
		write (fd, out, 4);
		write (fd, CFontz->framebuf + (y * CFontz->wid) + lft, rgt - lft + 1);

	}

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
CFontz_chr (int x, int y, char c)
{
	y--;
	x--;

	if (c < 32 && c >= 0)
		c += 128;
	CFontz->framebuf[(y * CFontz->wid) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
//
int
CFontz_contrast (int contrast)
{
	int realcontrast;
	char out[4];
	static int status = 140;

	if (contrast > 0) {
		status = contrast;
		realcontrast = (((int) (status)) * 100) / 255;
		snprintf (out, sizeof(out), "%c%c", 15, realcontrast);
		write (fd, out, 3);
	}

	return status;
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
void
CFontz_backlight (int on)
{
	char out[4];
	if (on) {
		snprintf (out, sizeof(out), "%c%c", 14, (unsigned char) (on * 100 / 255));
		//snprintf(out, sizeof(out), "%c%c", 14, 100);
	} else {
		snprintf (out, sizeof(out), "%c%c", 14, 0);
	}
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
CFontz_linewrap (int on)
{
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 23);
	else
		snprintf (out, sizeof(out), "%c", 24);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
CFontz_autoscroll (int on)
{
	char out[4];
	if (on)
		snprintf (out, sizeof(out), "%c", 19);
	else
		snprintf (out, sizeof(out), "%c", 20);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Get rid of the blinking curson
//
static void
CFontz_hidecursor ()
{
	char out[4];
	snprintf (out, sizeof(out), "%c", 4);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Reset the display bios
//
static void
CFontz_reboot ()
{
	char out[4];
	snprintf (out, sizeof(out), "%c", 26);
	write (fd, out, 1);
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before CFontz->vbar()
//
void
CFontz_init_vbar ()
{
	char a[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1,
	};

	if (custom != vbar) {
		CFontz_set_char (1, a);
		CFontz_set_char (2, b);
		CFontz_set_char (3, c);
		CFontz_set_char (4, d);
		CFontz_set_char (5, e);
		CFontz_set_char (6, f);
		CFontz_set_char (7, g);
		custom = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
CFontz_init_hbar ()
{

	char a[] = {
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 0, 0,
	};
	char e[] = {
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0,
	};
	char f[] = {
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1,
	};

	if (custom != hbar) {
		CFontz_set_char (1, a);
		CFontz_set_char (2, b);
		CFontz_set_char (3, c);
		CFontz_set_char (4, d);
		CFontz_set_char (5, e);
		CFontz_set_char (6, f);
		custom = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//
void
CFontz_vbar (int x, int len)
{
	char map[9] = { 32, 1, 2, 3, 4, 5, 6, 7, 255 };

	int y;
	for (y = CFontz->hgt; y > 0 && len > 0; y--) {
		if (len >= CFontz->cellhgt)
			CFontz_chr (x, y, 255);
		else
			CFontz_chr (x, y, map[len]);

		len -= CFontz->cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
CFontz_hbar (int x, int y, int len)
{
	char map[7] = { 32, 1, 2, 3, 4, 5, 6 };

	for (; x <= CFontz->wid && len > 0; x++) {
		if (len >= CFontz->cellwid)
			CFontz_chr (x, y, map[6]);
		else
			CFontz_chr (x, y, map[len]);

		len -= CFontz->cellwid;

	}

}

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
void
CFontz_init_num ()
{
}

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
void
CFontz_num (int x, int num)
{
	char out[5];
	snprintf (out, sizeof(out), "%c%c%c", 28, x, num);
	write (fd, out, 3);
}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void
CFontz_set_char (int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	snprintf (out, sizeof(out), "%c%c", 25, n);
	write (fd, out, 2);

	for (row = 0; row < CFontz->cellhgt; row++) {
		letter = 0;
		for (col = 0; col < CFontz->cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * CFontz->cellwid) + col] > 0);
		}
		write (fd, &letter, 1);
	}
}

void
CFontz_icon (int which, char dest)
{
	char icons[3][6 * 8] = {
		{
		 1, 1, 1, 1, 1, 1,		  // Empty Heart
		 1, 0, 1, 0, 1, 1,
		 0, 0, 0, 0, 0, 1,
		 0, 0, 0, 0, 0, 1,
		 0, 0, 0, 0, 0, 1,
		 1, 0, 0, 0, 1, 1,
		 1, 1, 0, 1, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1, 1,		  // Filled Heart
		 1, 0, 1, 0, 1, 1,
		 0, 1, 0, 1, 0, 1,
		 0, 1, 1, 1, 0, 1,
		 0, 1, 1, 1, 0, 1,
		 1, 0, 1, 0, 1, 1,
		 1, 1, 0, 1, 1, 1,
		 1, 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0, 0,		  // Ellipsis
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1, 0,
		 },

	};

	if (custom == bign)
		custom = beat;
	CFontz_set_char (dest, &icons[which][0]);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized CFontz->wid*CFontz->hgt
//
void
CFontz_draw_frame (char *dat)
{
	char out[LCD_MAX_WIDTH * LCD_MAX_HEIGHT];
	int i;

	if (!dat)
		return;

	// Custom characters start at 128, not at 0.
	/*
	   for(i=0; i<CFontz->wid*CFontz->hgt; i++)
	   {
	   if(dat[i] < 32  &&  dat[i] >= 0) dat[i] += 128;
	   }
	 */

	for (i = 0; i < CFontz->hgt; i++) {
		snprintf (out, sizeof(out), "%c%c%c", 17, 0, i);
		write (fd, out, 3);
		write (fd, dat + (CFontz->wid * i), CFontz->wid);
	}
	/*
	   snprintf(out, sizeof(out), "%c", 1);
	   write(fd, out, 1);
	   write(fd, dat, CFontz->wid*CFontz->hgt);
	 */

}

/////////////////////////////////////////////////////////////////
// Clears the LCD screen
//
void
CFontz_clear ()
{
	memset (CFontz->framebuf, ' ', CFontz->wid * CFontz->hgt);

}

/////////////////////////////////////////////////////////////////
// Prints a string on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
CFontz_string (int x, int y, char string[])
{
	int i;

	x -= 1;							  // Convert 1-based coords to 0-based...
	y -= 1;

	for (i = 0; string[i]; i++) {
		// Check for buffer overflows...
		if ((y * CFontz->wid) + x + i > (CFontz->wid * CFontz->hgt))
			break;
		CFontz->framebuf[(y * CFontz->wid) + x + i] = string[i];
	}
}

