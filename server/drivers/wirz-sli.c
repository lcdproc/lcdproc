/*	wirz-sli.c -- Source file for LCDproc Wirz SLI driver
	Copyright (C) 1999 Horizon Technologies-http://horizon.pair.com/
	Written by Bryan Rittmeyer <bryanr@pair.com> - Released under GPL
			
        LCD info: http://www.wirz.com/                               */

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

#include "lcd.h"
#include "wirz-sli.h"
#include "drv_base.h"

#include "shared/debug.h"
#include "shared/str.h"

static int custom = 0;
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
	beat = 8
} custom_type;

static int fd;
static char lastframe[32];

lcd_logical_driver *sli;

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
int
sli_init (lcd_logical_driver * driver, char *args)
{
	char *argv[64];
	int argc;
	struct termios portset;
	int i;
	int tmp;
	char out[2];

	char device[256] = "/dev/lcd";
	int speed = B19200;

	sli = driver;

	//debug("sli_init: Args(all): %s\n", args);

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
				fprintf (stderr, "sli_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			strcpy (device, argv[++i]);
		} else if (0 == strcmp (argv[i], "-s") || 0 == strcmp (argv[i], "--speed")) {
			if (i + 1 > argc) {
				fprintf (stderr, "sli_init: %s requires an argument\n", argv[i]);
				return -1;
			}
			tmp = atoi (argv[++i]);
			if (tmp == 1200)
				speed = B1200;
			else if (tmp == 2400)
				speed = B2400;
			else if (tmp == 9600)
				speed = B9600;
			else if (tmp == 19200)
				speed = B19200;
			else if (tmp == 38400)
				speed = B38400;
			else if (tmp == 57600)
				speed = B57600;
			else if (tmp == 115200)
				speed = B115200;
			else {
				fprintf (stderr, "sli_init: %s argument must be 1200, 2400, 9600, 19200, 38400, 57600, or 115200. Using default value (19200).\n", argv[i]);
			}
		} else if (0 == strcmp (argv[i], "-h") || 0 == strcmp (argv[i], "--help")) {
			printf ("LCDproc Wirz SLI LCD driver\n" "\t-d\t--device\tSelect the output device to use [/dev/lcd]\n" "\t-s\t--speed\t\tSet the communication speed [19200]\n" "\t-h\t--help\t\tShow this help information\n");
			return -1;
		} else {
			printf ("Invalid parameter: %s\n", argv[i]);
		}

	}

	// Set up io port correctly, and open it...
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		fprintf (stderr, "sli_init: failed (%s)\n", strerror (errno));
		return -1;
	}
	//else fprintf(stderr, "sli_init: opened device %s\n", device);
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

	/* Initialize SLI using autobaud detection, and then turn off cursor 
	   and clear screen */
	usleep (150000);				  /* 150ms delay to allow SLI to power on */
	out[0] = 13;					  /* CR for SLI autobaud */
	write (fd, out, 1);
	usleep (3000);					  /* 3ms delay.. wait for it to autobaud */
	out[0] = 0x0FE;
	out[1] = 0x00C;				  /* No cursor */
	write (fd, out, 2);
	out[0] = 0x0FE;
	out[1] = 0x001;				  /* Clear LCD, not sure if this belongs here */
	write (fd, out, 2);

	if (!driver->framebuf) {
		fprintf (stderr, "sli_init: No frame buffer.\n");
		driver->close ();
		return -1;
	}
	// Set LCD parameters (I use a 16x2 LCD) -- small but still useful
	// Its also much cheaper than the higher quality Matrix Orbital modules
	// Currently, $30 for interface kit and 16x2 non-backlit LCD...
	driver->wid = 15;
	driver->hgt = 2;

	// Set the functions the driver supports...

	driver->clear = (void *) -1;
	driver->string = (void *) -1;
	driver->chr = (void *) -1;
	driver->vbar = sli_vbar;
	driver->init_vbar = sli_init_vbar;
	driver->hbar = sli_hbar;
	driver->init_hbar = sli_init_hbar;
	driver->num = (void *) -1;
	driver->init_num = (void *) -1;

	driver->init = sli_init;
	driver->close = sli_close;
	driver->flush = sli_flush;
	driver->flush_box = sli_flush_box;
	driver->contrast = (void *) -1;
	driver->backlight = (void *) -1;
	driver->set_char = sli_set_char;
	driver->icon = sli_icon;
	driver->draw_frame = sli_draw_frame;

	driver->getkey = (void *) -1;

	return fd;
}

/* Clean-up */
void
sli_close ()
{
	close (fd);

	if (sli->framebuf)
		free (sli->framebuf);

	sli->framebuf = NULL;
}

void
sli_flush ()
{
	sli_draw_frame (lcd.framebuf);
}

/* no bounds checking is done in MtxOrb.c (which I shamelessly ripped)
   this is bad imho, so I added it.. may remove later
   speed is not a huge issue though, this isnt a 
   device driver or anything ;)                           */
void
sli_flush_box (int lft, int top, int rgt, int bot)
{
	int y;
	char out[2];					  /* Why does the matrix driver allocate so much here? */

	/* simple bounds checking */
	if ((top > lcd.hgt) | (bot > lcd.hgt))
		return;

	if ((lft > lcd.wid) | (rgt > lcd.wid))
		return;

//  printf("Flush (%i,%i)-(%i,%i)\n", lft, top, rgt, bot);

/* I like having hex, everywhere and all the time */
	for (y = top; y <= bot; y++) {
		if (y == 1)
			snprintf (out, sizeof(out), "%c%c", 0x0FE, 0x080 + lft);
		if (y == 2)
			snprintf (out, sizeof(out), "%c%c", 0x0FE, 0x0C0 + lft);
		write (fd, out, 0x002);
		write (fd, lcd.framebuf + (y * lcd.wid) + lft, rgt - lft + 1);
	}

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
void
sli_chr (int x, int y, char c)
{
	y--;
	x--;

	lcd.framebuf[(y * lcd.wid) + x] = c;
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//

/* Because of the way we do this (custom characters in CGRAM)
   we can't have both horizontal and vertical bars at once...
   this also appears to be a limitation of the Matrix Orbital
   modules so I will assume that all client coders know about it 

   I think it would be cool to use triangular stuff for the non-full
   characters, so that you can do both bar types at once.. maybe I 
   will release a new version of the SLI driver that attempts this */

void
sli_init_vbar ()
{
	char a[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
	};
	char b[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char c[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char d[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char e[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char f[] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	char g[] = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};

	if (custom != vbar) {
		sli_set_char (1, a);
		sli_set_char (2, b);
		sli_set_char (3, c);
		sli_set_char (4, d);
		sli_set_char (5, e);
		sli_set_char (6, f);
		sli_set_char (7, g);
		custom = vbar;
	}
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
void
sli_init_hbar ()
{

	char a[] = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
	};
	char b[] = {
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
		1, 1, 0, 0, 0,
	};
	char c[] = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	};
	char d[] = {
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 1, 0,
	};

	if (custom != hbar) {
		sli_set_char (1, a);
		sli_set_char (2, b);
		sli_set_char (3, c);
		sli_set_char (4, d);
		custom = hbar;
	}
}

/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
//

/* It would be cool if you could start a vertical bar at any y
   like the horizontal bar routine can start at any x... but
   since we only have 2 lines anyway this is rather pointless
   for me to add                                               */

void
sli_vbar (int x, int len)
{
	char map[9] = { 32, 1, 2, 3, 4, 5, 6, 7, 255 };

	int y;
	for (y = lcd.hgt; y > 0 && len > 0; y--) {
		if (len >= lcd.cellhgt)
			sli_chr (x, y, 255);
		else
			sli_chr (x, y, map[len]);

		len -= lcd.cellhgt;
	}

}

/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
//
void
sli_hbar (int x, int y, int len)
{
	char map[6] = { 32, 1, 2, 3, 4, 255 };

	for (; x <= lcd.wid && len > 0; x++) {
		if (len >= lcd.cellwid)
			sli_chr (x, y, 255);
		else
			sli_chr (x, y, map[len]);

		len -= lcd.cellwid;

	}

}

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
void
sli_set_char (int n, char *dat)
{
	char out[2];
	int row, col;
	int letter;

	/* SLI also has 8 user definable characters */
	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	/* Move cursor to CGRAM */
	out[0] = 0x0FE;
	out[1] = 0x040 + 8 * n;
	write (fd, out, 2);

	for (row = 0; row < lcd.cellhgt; row++) {
		letter = 0;
		for (col = 0; col < lcd.cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * lcd.cellwid) + col] > 0);
		}
		letter |= 0x020;			  /* SLI can't accept CR, LF, etc in this character! */
		write (fd, &letter, 1);
	}

	/* Move cursor back to DDRAM */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write (fd, out, 2);
}

void
sli_icon (int which, char dest)
{
	char icons[3][5 * 8] = {
		{
		 1, 1, 1, 1, 1,			  // Empty Heart
		 1, 0, 1, 0, 1,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 0, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 1, 1, 1, 1, 1,			  // Filled Heart
		 1, 0, 1, 0, 1,
		 0, 1, 0, 1, 0,
		 0, 1, 1, 1, 0,
		 0, 1, 1, 1, 0,
		 1, 0, 1, 0, 1,
		 1, 1, 0, 1, 1,
		 1, 1, 1, 1, 1,
		 },

		{
		 0, 0, 0, 0, 0,			  // Ellipsis
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 0, 1, 0, 1,
		 },

	};

	if (custom == bign)
		custom = beat;
	sli_set_char (dest, &icons[which][0]);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized lcd.wid*lcd.hgt
//
void
sli_draw_frame (char *dat)
{
	char out[2];					  /* Again, why does the Matrix driver allocate so much here? */
	int y;

	if (!dat)
		return;

	/*
	   out[0]=0x0FE;
	   out[1]=0x001;
	   write(fd, out, 2);
	 */

	/* Don't update if we have no new data
	   this keeps me from getting a migraine
	   (just like those copyleft penguin mints... mmmmmm)  */

	//   if (!strncmp(dat,lastframe,32)) /* Nothing has changed */
	//     return;

	/* Do the actual refresh */
	out[0] = 0x0FE;
	out[1] = 0x080;
	write (fd, out, 2);
	write (fd, &dat[0], 16);
	usleep (10);
	write (fd, &dat[16], 15);

	//   strncpy(lastframe,dat,32); // Update lastframe...

}
