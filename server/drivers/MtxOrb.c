/*
 * Matrix Orbital driver
 *
 * For the Matrix Orbital LCD* LKD* VFD* and VKD* displays
 *
 * September 16, 2001
 *
 * NOTE: GLK displays have a different driver.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "lcd.h"
#include "lcd_lib.h"
#include "MtxOrb.h"
#include "drv_base.h"
#include "shared/debug.h"
#include "shared/str.h"

#define IS_LCD_DISPLAY	(MtxOrb_type == MTXORB_LCD)
#define IS_LKD_DISPLAY	(MtxOrb_type == MTXORB_LKD)
#define IS_VFD_DISPLAY	(MtxOrb_type == MTXORB_VFD)
#define IS_VKD_DISPLAY	(MtxOrb_type == MTXORB_VKD)

#define NotEnoughArgs (i + 1 > argc)

// NOTE: This does not appear to make use of the
//       hbar and vbar functions present in the LKD202-25.
//       Why I do not know.

static int custom = 0;
static enum {MTXORB_LCD, MTXORB_LKD, MTXORB_VFD, MTXORB_VKD} MtxOrb_type;
extern int debug_level;

// TODO: Remove this custom_type if not in use anymore.
typedef enum {
	hbar = 1,
	vbar = 2,
	bign = 4,
	beat = 8
} custom_type;

// TODO: This is my ugglyest piece of code.
typedef enum {
	baru1 = 0,
	baru2 = 1,
	baru3 = 2,
	baru4 = 3,
	baru5 = 4,
	baru6 = 5,
	baru7 = 6,
	bard1 = 7,
	bard2 = 8,
	bard3 = 9,
	bard4 = 10,
	bard5 = 11,
	bard6 = 12,
	bard7 = 13,
	barr1 = 14,
	barr2 = 15,
	barr3 = 16,
	barr4 = 17,
	barl1 = 18,
	barl2 = 19,
	barl3 = 20,
	barl4 = 21,
	barw = 32,
	barb = 255
} bar_type;

static int fd;
static int clear = 1;

static int def[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int use[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };

static void MtxOrb_linewrap (int on);
static void MtxOrb_autoscroll (int on);
static void MtxOrb_cursorblink (int on);
static void MtxOrb_string (int x, int y, char *string);

static int
MtxOrb_set_type (char * str) {
	char c;
	c = str[0];

	if (c == 'l') {
		if (strncasecmp(str, "lcd", 3) == 0) {
			return MTXORB_LCD;
		} else if (strncasecmp(str, "lkd", 3) == 0) {
			return MTXORB_LKD;
		} else {
			fprintf (stderr, "MtxOrb_init: unknwon display type %s; must be one of lcd, lkd, vfd, or vkd\n", str);
		}
	} else if (c == 'v') {
		if (strncasecmp (str, "vfd", 3) == 0) {
			return MTXORB_VFD;
		} else if (strncasecmp (str, "vkd", 3) == 0) {
			return MTXORB_VKD;
		} else {
			fprintf (stderr, "MtxOrb_init: unknwon display type %s; must be one of lcd, lkd, vfd, or vkd\n", str);
		}
	} else {
		fprintf (stderr, "MtxOrb_init: unknwon display type %s; must be one of lcd, lkd, vfd, or vkd\n", str);
	}
	return (-1);
}

static int
MtxOrb_get_speed (char *arg) {
	int speed;

	switch (atoi(arg)) {
		case 1200: speed = B1200; break;
		case 2400: speed = B2400; break;
		case 9600: speed = B9600; break;
		case 19200: speed = B19200; break;
		default:
			speed = DEFAULT_SPEED;
			fprintf (stderr, "MtxOrb_init: argument must be 1200, 2400, 9600 or 19200. Using default value");
			switch (speed) {
				case B1200: fprintf(stderr, " of 1200 baud.\n"); break;
				case B2400: fprintf(stderr, " of 2400 baud.\n"); break;
				case B9600: fprintf(stderr, " of 9600 baud.\n"); break;
				case B19200: fprintf(stderr, " of 19200 baud.\n"); break;
				default: fprintf(stderr, ".\n"); break;
			}
		}

	return speed;
	}

static void
MtxOrb_usage (void) {
	printf ("LCDproc Matrix-Orbital LCD driver\n"
		"\t-d\t\tSelect the output device to use [/dev/lcd]\n"
		"\t-t\t\tSelect the LCD type (size) [20x4]\n"
//		"\t-b\t--backlight\tSelect the backlight state [on]\n"
		"\t-c\t\tSet the initial contrast [140]\n"
		"\t-s\t\tSet the communication speed [19200]\n"
		"\t-h\t\tShow this help information\n"
		"\t-b\t\tdisplay type: lcd, lkd, vfd, vkd\n");
}

int
MtxOrb_set_contrast (char * str) {
	int contrast;

	contrast = atoi (str);
	if ((contrast < 0) || (contrast > 255)) {
		fprintf(stderr, "MtxOrb_init: argument must between 0 and 255 (found %s). Using default contrast value of %d.\n", str, DEFAULT_CONTRAST);
		contrast = DEFAULT_CONTRAST;
	}
	return contrast;
}

// TODO:  Get rid of this variable? Probably not...
lcd_logical_driver *MtxOrb;	// set by MtxOrb_init(); doesn't seem to be used anywhere
// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
// Called to initialize driver settings
//
int
MtxOrb_init (lcd_logical_driver * driver, char *args)
{
	char *argv[64];		// Notice: 64 arguments - overflows?
	int argc;
	struct termios portset;
	int i;
	int tmp;

	int contrast = DEFAULT_CONTRAST;
	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	char c;

	MtxOrb_type = MTXORB_LKD;  // Assume it's an LCD w/keypad

	MtxOrb = driver;

	//debug("MtxOrb_init: Args(all): %s\n", args);

	argc = get_args (argv, args, 64);

	/*
	   for(i=0; i<argc; i++)
	   {
	   printf("Arg(%i): %s\n", i, argv[i]);
	   }
	 */

#ifdef USE_GETOPT
	while ((c = getopt(argc, argv, "d:c:s:ht:")) > 0) {
		switch(c) {
			case 'd':
				strncpy(device, optarg, sizeof(device));
				break;
			case 's':
				speed = MtxOrb_get_speed(optarg);
				break;
			case 'c':
				contrast = MtxOrb_set_contrast(optarg);
				break;
			case 'h':
				MtxOrb_usage();
				return -1;
			case 't':
				MtxOrb_set_type(optarg);
			default:
				MtxOrb_usage();
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
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					strcpy (device, argv[++i]);
					break;
				case 'c':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					contrast = MtxOrb_set_contrast (argv[++i]);
					break;
				case 's':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					speed = MtxOrb_get_speed (argv[++i]);
					break;
				case 'h':
					MtxOrb_usage();
					return -1;
					break;
				case 't':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					i++;
					p = argv[i];

					if (isdigit((unsigned int) *p) && isdigit((unsigned int) *(p+1))) {
						int wid, hgt;

						wid = ((*p - '0') * 10) + (*(p+1) - '0');
						p += 2;

						if (*p != 'x')
							break;

						p++;
						if (!isdigit((unsigned int) *p))
							break;

						hgt = (*p - '0');

						MtxOrb->wid = wid;
						MtxOrb->hgt = hgt;
						}
					break;
				case 'b':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					i++;
					MtxOrb_type = MtxOrb_set_type(argv[i]);
					break;
				default:
					printf ("Invalid parameter: %s\n", argv[i]);
					break;
			}
		}
	}
#endif

	// Set up io port correctly, and open it...
	fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		switch (errno) {
			case ENOENT: fprintf(stderr, "MtxOrb_init: %s device file missing!\n", device);
				break;
			case EACCES: fprintf(stderr, "MtxOrb_init: %s device could not be opened...\n", device);
				fprintf(stderr, "MtxOrb_init: perhaps you should run LCDd as root?\n");
				break;
			default: fprintf (stderr, "MtxOrb_init: failed (%s)\n", strerror (errno));
				break;
		}
  		return -1;
	} else
		syslog(LOG_INFO, "opened Matrix Orbital display on %s\n", device);

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

	/*
	 * Configure display
	 */

	MtxOrb_linewrap (DEFAULT_LINEWRAP);
	MtxOrb_autoscroll (DEFAULT_AUTOSCROLL);
	MtxOrb_cursorblink (DEFAULT_CURSORBLINK);
	MtxOrb_contrast (contrast);

	/*
	 * Configure the display functions
	 */

	driver->clear = MtxOrb_clear;
	driver->string = MtxOrb_string;
	driver->chr = MtxOrb_chr;
	driver->vbar = MtxOrb_vbar;
	driver->init_vbar = MtxOrb_init_vbar;
	driver->hbar = MtxOrb_hbar;
	driver->init_hbar = MtxOrb_init_hbar;
	driver->num = MtxOrb_num;
	driver->init_num = MtxOrb_init_num;

	driver->init = MtxOrb_init;
	driver->close = MtxOrb_close;
	driver->flush = MtxOrb_flush;
	driver->flush_box = MtxOrb_flush_box;
	driver->contrast = MtxOrb_contrast;
	driver->backlight = MtxOrb_backlight;
	driver->output = MtxOrb_output;
	driver->set_char = MtxOrb_set_char;
	driver->icon = MtxOrb_icon;
	driver->draw_frame = MtxOrb_draw_frame;

	driver->getkey = MtxOrb_getkey;
	driver->getinfo = MtxOrb_getinfo;

	return fd;
}

#define ValidX(a) { if (x > MtxOrb->wid) { x = MtxOrb->wid; } else x < 1 ? 1 : x; }
#define ValidY(a) { if (y > MtxOrb->hgt) { y = MtxOrb->hgt; } else y < 1 ? 1 : y; }

// TODO: Check this quick hack to detect clear of the screen.
/////////////////////////////////////////////////////////////////
// Clear: catch up when the screen get clear to be able to
//  forget bar caracter not in use anymore and reuse the
//  slot for another bar caracter.
//
static void
MtxOrb_clear ()
{
	if (MtxOrb->framebuf != NULL)
		memset (MtxOrb->framebuf, ' ', (MtxOrb->wid * MtxOrb->hgt));

	write(fd, "\x0FE" "X", 2);  // instant clear...
	clear = 1;

	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: cleared screen");
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
static void
MtxOrb_close ()
{
	close (fd);

	if (MtxOrb->framebuf)
		free (MtxOrb->framebuf);

	MtxOrb->framebuf = NULL;

	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: closed");
}

static void
MtxOrb_string (int x, int y, char *string)
{
	int offset, siz;

	ValidX(x);
	ValidY(y);

	x--; y--; // Convert 1-based coords to 0-based...
	offset = (y * MtxOrb->wid) + x;
	siz = (MtxOrb->wid * MtxOrb->hgt) - offset - 1;
	siz = siz > strlen(string) ? strlen(string) : siz;

	memcpy(MtxOrb->framebuf + offset, string, siz);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: printed string at (%d,%d)", x, y);
}

static void
MtxOrb_flush ()
{
	MtxOrb_draw_frame (MtxOrb->framebuf);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: frame buffer flushed");
}

static void
MtxOrb_flush_box (int lft, int top, int rgt, int bot)
{
	int y;
	char out[LCD_MAX_WIDTH];

	for (y = top; y <= bot; y++) {
		snprintf (out, sizeof(out), "\x0FEG%c%c", lft, y);
		write (fd, out, 4);
		write (fd, MtxOrb->framebuf + (y * MtxOrb->wid) + lft, rgt - lft + 1);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: frame buffer box flushed");
	}

}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
static void
MtxOrb_chr (int x, int y, char c)
{
	char out[10], buf[64];
	int offset;

	// Characters may or may NOT be alphabetic; it appears
	// that characters 0..4 (or similar) are graphic fonts

	ValidX(x);
	ValidY(y);

	// write immediately to screen... this code was taken
	// from the LK202-25; should work for others, yes?
	// snprintf(out, sizeof(out), "\x0FEG%c%c%c", x, y, c);
	// write (fd, out, 4);

	// write to frame buffer
	y--; x--; // translate to 0-index
	offset = (y * MtxOrb->wid) + x;
	MtxOrb->framebuf[offset] = c;

	if (debug_level > 2) {
		snprintf(buf, sizeof(buf), "writing character %02X to position (%d,%d)",
			c, x, y);
		syslog(LOG_DEBUG, buf);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: printed a char at (%d,%d)", x, y);
	}
}

/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
// note: works only for LCD displays
// Is it better to use the brightness for VFD/VKD displays ?
//
static int
MtxOrb_contrast (int contrast)
{
	char out[4];

	// validate contrast value
	if (contrast > 255)
		contrast = 255;
	if (contrast < 0)
		contrast = 0;

	if (IS_LCD_DISPLAY || IS_LKD_DISPLAY) {
		snprintf (out, sizeof(out), "\x0FEP%c", contrast);
		write (fd, out, 3);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: contrast set to %d", contrast);
	} else {
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: contrast not set to %d - not LCD or LKD display", contrast);
	}

	return contrast;
}

/////////////////////////////////////////////////////////////////
// Sets the backlight on or off -- can be done quickly for
// an intermediate brightness...
//
// WARNING: off switches vfd/vkd displays off entirely
//	    so maybe it is best to start LCDd with -b on
//
// WARNING: there seems to be a movement afoot to add more
//          functions than just on/off to this..

#define BACKLIGHT_OFF 0
#define BACKLIGHT_ON 1

static void
MtxOrb_backlight (int on)
{
	static int backlight_state = 1;

	if (backlight_state == on)
		return;

	backlight_state = on;

	switch (on) {
		case BACKLIGHT_ON: 
			write (fd, "\x0FE" "F", 2);
			if (debug_level > 3)
				syslog(LOG_DEBUG, "MtxOrb: backlight turned on");
			break;
		case BACKLIGHT_OFF: 
			if (IS_VKD_DISPLAY || IS_VFD_DISPLAY) {
				if (debug_level > 3)
					syslog(LOG_DEBUG, "MtxOrb: backlight ignored - not LCD or LKD display");
				; // turns display off entirely (whoops!)
			} else {
				if (debug_level > 3)
					syslog(LOG_DEBUG, "MtxOrb: backlight turned off");
				write (fd, "\x0FE" "B" "\x000", 3);
			}
			break;
		default: // ignored...
			if (debug_level > 3)
				syslog(LOG_DEBUG, "MtxOrb: backlight - invalid setting");
			break;
		}
}

/////////////////////////////////////////////////////////////////
// Sets output port on or off
// displays with keypad have 6 outputs but the one without kepad
// have only one output
// NOTE: length of command are different
static void
MtxOrb_output (int on)
{
	char out[5];
	static int output_state = -1;

	on = on & 077;	// strip to six bits

	if (output_state == on)
		return;

	output_state = on;

	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: output pins set: %04X", on);

	if (IS_LCD_DISPLAY || IS_VFD_DISPLAY) {
		// LCD and VFD displays only have one output port
		(on) ?
			write (fd, "\x0FEW", 2) :
			write (fd, "\x0FEV", 2);
	} else {
		int i;

		// Other displays have six output ports;
		// the value "on" is a binary value determining which
		// ports are turned on (1) and off (0).

		for(i = 0; i < 6; i++) {
			(on & (1 << i)) ?
				snprintf (out, sizeof(out), "\x0FEW%c", i + 1) :
				snprintf (out, sizeof(out), "\x0FEV%c", i + 1);
			write (fd, out, 3);
		}
	}
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in linewrapping feature
//
static void
MtxOrb_linewrap (int on)
{
	if (on) {
		write (fd, "\x0FE" "C", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: linewrap turned on");
	} else {
		write (fd, "\x0FE" "D", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: linewrap turned off");
	}
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
MtxOrb_autoscroll (int on)
{
	if (on) {
		write (fd, "\x0FEQ", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: autoscroll turned on");
	} else {
		write (fd, "\x0FER", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: autoscroll turned off");
	}
}

// TODO: make sure this doesn't mess up non-VFD displays
/////////////////////////////////////////////////////////////////
// Toggle cursor blink on/off
//
static void
MtxOrb_cursorblink (int on)
{
	if (on) {
		write (fd, "\x0FES", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: cursorblink turned on");
	} else {
		write (fd, "\x0FET", 2);

		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: cursorblink turned off");
	}
}

//// TODO: Might not be needed anymore...
/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//
static void
MtxOrb_init_vbar ()
{
// Isn't this function supposed to go away?
	MtxOrb_init_all (vbar);
}

// TODO: Might not be needed anymore...
/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
static void
MtxOrb_init_hbar ()
{
// Isn't this function supposed to go away?
	MtxOrb_init_all (hbar);
}

/////////////////////////////////////////////////////////////////
// Returns string with general information about the display
//
static char *
MtxOrb_getinfo (void)
{
	char in = 0;
	static char info[255];
	char tmp[255], buf[64];
	int i = 0;
	fd_set rfds;

	struct timeval tv;
	int retval;

	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: getinfo");

	memset(info, '\0', sizeof(info));
	strcpy(info, "Matrix Orbital Driver ");

	/*
	 * Read type of display
	 */

	write(fd, "\x0FE" "7", 2);

	/* Watch fd to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		// seconds
	tv.tv_usec = 500;	// microseconds

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (fd, &in, 1) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			switch (in) {
				case '\x01': strcat(info, "LCD0821 "); break;
				case '\x03': strcat(info, "LCD2021 "); break;
				case '\x04': strcat(info, "LCD1641 "); break;
				case '\x05': strcat(info, "LCD2041 "); break;
				case '\x06': strcat(info, "LCD4021 "); break;
				case '\x07': strcat(info, "LCD4041 "); break;
				case '\x08': strcat(info, "LK202-25 "); break;
				case '\x09': strcat(info, "LK204-25 "); break;
				case '\x0A': strcat(info, "LK404-55 "); break;
				case '\x0B': strcat(info, "VFD2021 "); break;
				case '\x0C': strcat(info, "VFD2041 "); break;
				case '\x0D': strcat(info, "VFD4021 "); break;
				case '\x0E': strcat(info, "VK202-25 "); break;
				case '\x0F': strcat(info, "VK204-25 "); break;
				case '\x10': strcat(info, "GLC12232 "); break;
				case '\x11': strcat(info, "GLC12864 "); break;
				case '\x12': strcat(info, "GLC128128 "); break;
				case '\x13': strcat(info, "GLC24064 "); break;
				case '\x14': strcat(info, "GLK12864-25 "); break;
				case '\x15': strcat(info, "GLK24064-25 "); break;
				case '\x21': strcat(info, "GLK128128-25 "); break;
				case '\x22': strcat(info, "GLK12232-25 "); break;
				case '\x31': strcat(info, "LK404-AT "); break;
				case '\x32': strcat(info, "VFD1621 "); break;
				case '\x33': strcat(info, "LK402-12 "); break;
				case '\x34': strcat(info, "LK162-12 "); break;
				case '\x35': strcat(info, "LK204-25PC "); break;
				default: //snprintf(tmp, sizeof(tmp), "Unknown (%X) ", in); strcat(info, tmp);
					     break;
			}
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device type");

	/*
	 * Read serial number of display
	 */

	memset(tmp, '\0', sizeof(tmp));
	write(fd, "\x0FE" "5", 2);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		// seconds
	tv.tv_usec = 500;	// microseconds

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Serial No: %ld ", (long int) tmp);
			strcat(info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device serial number");

	/*
	 * Read firmware revision number
	 */

	memset(tmp, '\0', sizeof(tmp));
	write(fd, "\x0FE" "6", 2);

	/* Wait the specified amount of time. */
	tv.tv_sec = 0;		// seconds
	tv.tv_usec = 500;	// microseconds

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval) {
		if (read (fd, &tmp, 2) < 0) {
			syslog(LOG_WARNING, "MatrixOrbital driver: unable to read data");
		} else {
			snprintf(buf, sizeof(buf), "Firmware Rev. %ld ", (long int) tmp);
			strcat(info, buf);
		}
	} else
		syslog(LOG_WARNING, "MatrixOrbital driver: unable to read device firmware revision");

	return info;
}

// TODO: Finish the support for bar growing reverse way.
// TODO: Need a "y" as input also !!!
/////////////////////////////////////////////////////////////////
// Draws a vertical bar...
// This is the new version ussing dynamic icon alocation
//
static void
MtxOrb_vbar (int x, int len)
{
	unsigned char mapu[9] = { barw, baru1, baru2, baru3, baru4, baru5, baru6, baru7, barb };
	unsigned char mapd[9] = { barw, bard1, bard2, bard3, bard4, bard5, bard6, bard7, barb };

	int y;

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: vertical bar at %d set to %d", x, len);

// REMOVE THE NEXT LINE FOR TESTING ONLY...
//  len=-len;
// REMOVE THE PREVIOUS LINE FOR TESTING ONLY...

	if (len > 0) {
		for (y = MtxOrb->hgt; y > 0 && len > 0; y--) {
			if (len >= MtxOrb->cellhgt)
				MtxOrb_chr (x, y, 255);
			else
				MtxOrb_chr (x, y, MtxOrb_ask_bar (mapu[len]));

			len -= MtxOrb->cellhgt;
		}
	} else {
		len = -len;
		for (y = 2; y <= MtxOrb->hgt && len > 0; y++) {
			if (len >= MtxOrb->cellhgt)
				MtxOrb_chr (x, y, 255);
			else
				MtxOrb_chr (x, y, MtxOrb_ask_bar (mapd[len]));

			len -= MtxOrb->cellhgt;
		}
	}

}

// TODO: Finish the support for bar growing reverse way.
/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
// This is the new version ussing dynamic icon alocation
//
static void
MtxOrb_hbar (int x, int y, int len)
{
	unsigned char mapr[6] = { barw, barr1, barr2, barr3, barr4, barb };
	unsigned char mapl[6] = { barw, barl1, barl2, barl3, barl4, barb };

	ValidX(x);
	ValidY(y);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: horizontal bar at %d set to %d", x, len);

	if (len > 0) {
		for (; x <= MtxOrb->wid && len > 0; x++) {
			if (len >= MtxOrb->cellwid)
				MtxOrb_chr (x, y, 255);
			else
				MtxOrb_chr (x, y, MtxOrb_ask_bar (mapr[len]));

			len -= MtxOrb->cellwid;

		}
	} else {
		len = -len;
		for (; x > 0 && len > 0; x--) {
			if (len >= MtxOrb->cellwid)
				MtxOrb_chr (x, y, 255);
			else
				MtxOrb_chr (x, y, MtxOrb_ask_bar (mapl[len]));

			len -= MtxOrb->cellwid;

		}
	}

}

// TODO: Might not work, bignum is untested... an untested with dynamic bar.

/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
static void
MtxOrb_init_num ()
{
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: init for big numbers");

	if (custom != bign) {
		write (fd, "\x0FEn", 2);
		custom = bign;
	}
}

// TODO: Might not work, bignum is untested... an untested with dynamic bar.

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
static void
MtxOrb_num (int x, int num)
{
	char out[5];

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: write big number %d at %d", num, x);

	snprintf (out, sizeof(out), "\x0FE#%c%c", x, num);
	write (fd, out, 4);
}

// TODO: This could be higly optimised if data where to be pre-computed.

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// For input, values > 0 mean "on" and values <= 0 are "off".
//
// The input is just an array of characters...
//
static void
MtxOrb_set_char (int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

	if (n < 0 || n > 7)
		return;
	if (!dat)
		return;

	snprintf (out, sizeof(out), "\x0FEN%c", n);
	write (fd, out, 3);

	for (row = 0; row < MtxOrb->cellhgt; row++) {
		letter = 0;
		for (col = 0; col < MtxOrb->cellwid; col++) {
			letter <<= 1;
			letter |= (dat[(row * MtxOrb->cellwid) + col] > 0);
		}
		write (fd, &letter, 1);
	}
}

static void
MtxOrb_icon (int which, char dest)
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
	MtxOrb_set_char (dest, &icons[which][0]);
}

/////////////////////////////////////////////////////////////
// Blasts a single frame onscreen, to the lcd...
//
// Input is a character array, sized lcd.wid*lcd.hgt
//
static void
MtxOrb_draw_frame (char *dat)
{
	char out[12];
	int i,j,mv = 1;
	static char *old = NULL;
	char *p, *q;

	if (!dat)
		return;

	if (old == NULL) {
		old = malloc(MtxOrb->wid * MtxOrb->hgt);

		write(fd, "\x0FEG\x01\x01", 4);
		write(fd, dat, MtxOrb->wid * MtxOrb->hgt);

		strncpy(old, dat, MtxOrb->wid * MtxOrb->hgt);

		return;

	} else {
		if (! new_framebuf(MtxOrb, old))
			return;
	}

	p = dat;
	q = old;

	for (i = 1; i <= MtxOrb->hgt; i++) {
		for (j = 1; j <= MtxOrb->wid; j++) {

			if ((*p) == (*q))
				mv = 1;
			else {
				if (mv == 1) {
					snprintf(out, sizeof(out), "\x0FEG%c%c", j, i);
					write (fd, out, 4);
					mv = 0;
				}
				write (fd, p, 1);
			}
			p++;
			q++;
		}
	}


	//for (i = 0; i < MtxOrb->hgt; i++) {
	//	snprintf (out, sizeof(out), "\x0FEG\x001%c", i + 1);
	//	write (fd, out, 4);
	//	write (fd, dat + (MtxOrb->wid * i), MtxOrb->wid);
	//}

	strncpy(old, dat, MtxOrb->wid * MtxOrb->hgt);
}

/////////////////////////////////////////////////////////////
// returns one character from the keypad...
// (A-Z) on success, 0 on failure...
//
static char
MtxOrb_getkey ()
{
	char in = 0;

	read (fd, &in, 1);
	return in;
}

/////////////////////////////////////////////////////////////////
// Ask for dynamic allocation of a custom caracter to be
//  a well none bar graphic. The function is suppose to
//  return a value between 0 and 7 but 0 is reserver for
//  heart beat.
//  This function manadge a cache of graphical caracter in use.
//  I really hope it is working and bug-less because it is not
//  completely tested, just a quick hack.
//
static int
MtxOrb_ask_bar (int type)
{
	int i;
	int last_not_in_use;
	int pos;							  // 0 is icon, 1 to 7 are free, 8 is not found.
	// TODO: Reuse graphic caracter 0 if heartbeat is not in use.

	// REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar(%d).\n", type);
	// Check if the screen was clear.
	if (clear) {					  // If the screen was clear then graphic caracter are not in use.
		//REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| clear was set.\n");
		use[0] = 1;					  // Heartbeat is always in use (not true but it help).
		for (pos = 1; pos < 8; pos++) {
			use[pos] = 0;			  // Other are not in use.
		}
		clear = 0;					  // We made the special treatement.
	} else {
		// Nothing special if some caracter a curently in use (...) ?
	}

	// Search for a match with caracter already defined.
	pos = 8;							  // Not found.
	last_not_in_use = 8;			  // No empty slot to reuse.
	for (i = 1; i < 8; i++) {	  // For all but first (heartbeat).
		if (!use[i])
			last_not_in_use = i;	  // New empty slot.
		if (def[i] == type)
			pos = i;					  // Founded (should break now).
	}

	if (pos == 8) {
		// REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| not found.\n");
		pos = last_not_in_use;
		// TODO: Best match/deep search is no more graphic caracter are available.
	}

	if (pos != 8) {				  // A caracter is found (Best match could solve our problem).
		// REMOVE: fprintf(stderr, "GLU: MtxOrb_ask_bar| found at %d.\n", pos);
		if (def[pos] != type) {
			MtxOrb_set_known_char (pos, type);	// Define a new graphic caracter.
			def[pos] = type;		  // Remember that now the caracter is available.
		}
		if (!use[pos]) {			  // If the caracter is no yet in use (but defined).
			use[pos] = 1;			  // Remember it is in use (so protect it from re-use).
		}
	}

	if (pos == 8)					  // TODO: Choose a character to approximate the graph
	{
		//pos=65; // ("A")?
		switch (type) {
		case baru1:
			pos = '_';
			break;
		case baru2:
			pos = '.';
			break;
		case baru3:
			pos = ',';
			break;
		case baru4:
			pos = 'o';
			break;
		case baru5:
			pos = 'o';
			break;
		case baru6:
			pos = 'O';
			break;
		case baru7:
			pos = '8';
			break;

		case bard1:
			pos = '\'';
			break;
		case bard2:
			pos = '"';
			break;
		case bard3:
			pos = '^';
			break;
		case bard4:
			pos = '^';
			break;
		case bard5:
			pos = '*';
			break;
		case bard6:
			pos = 'O';
			break;
		case bard7:
			pos = '8';
			break;

		case barr1:
			pos = '-';
			break;
		case barr2:
			pos = '-';
			break;
		case barr3:
			pos = '=';
			break;
		case barr4:
			pos = '=';
			break;

		case barl1:
			pos = '-';
			break;
		case barl2:
			pos = '-';
			break;
		case barl3:
			pos = '=';
			break;
		case barl4:
			pos = '=';
			break;

		case barw:
			pos = ' ';
			break;

		case barb:
			pos = 255;
			break;

		default:
			pos = '?';
			break;
		}
	}

	return (pos);
}

/////////////////////////////////////////////////////////////
// Does the heartbeat...
//
static char
MtxOrb_heartbeat (int timer)
{
	MtxOrb_icon (!((timer + 4) & 5), 0);
	MtxOrb_chr (MtxOrb->wid, 1, 0);
	return (char) 0;
}

/////////////////////////////////////////////////////////////////
// Sets up a well known character for use.
//
static void
MtxOrb_set_known_char (int car, int type)
{
	char all_bar[25][5 * 8] = {
		{
		 0, 0, 0, 0, 0,			  //  char u1[] = 
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1,
		 }, {
			  0, 0, 0, 0, 0,		  //  char u2[] = 
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0,
			  0, 0, 0, 0, 0,
			  1, 1, 1, 1, 1,
			  1, 1, 1, 1, 1,
			  }, {
					0, 0, 0, 0, 0,	  //  char u3[] = 
					0, 0, 0, 0, 0,
					0, 0, 0, 0, 0,
					0, 0, 0, 0, 0,
					0, 0, 0, 0, 0,
					1, 1, 1, 1, 1,
					1, 1, 1, 1, 1,
					1, 1, 1, 1, 1,
					}, {
						 0, 0, 0, 0, 0,	//  char u4[] = 
						 0, 0, 0, 0, 0,
						 0, 0, 0, 0, 0,
						 0, 0, 0, 0, 0,
						 1, 1, 1, 1, 1,
						 1, 1, 1, 1, 1,
						 1, 1, 1, 1, 1,
						 1, 1, 1, 1, 1,
						 }, {
							  0, 0, 0, 0, 0,	//  char u5[] = 
							  0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0,
							  1, 1, 1, 1, 1,
							  1, 1, 1, 1, 1,
							  1, 1, 1, 1, 1,
							  1, 1, 1, 1, 1,
							  1, 1, 1, 1, 1,
							  }, {
									0, 0, 0, 0, 0,	//  char u6[] = 
									0, 0, 0, 0, 0,
									1, 1, 1, 1, 1,
									1, 1, 1, 1, 1,
									1, 1, 1, 1, 1,
									1, 1, 1, 1, 1,
									1, 1, 1, 1, 1,
									1, 1, 1, 1, 1,
									}, {
										 0, 0, 0, 0, 0,	//  char u7[] = 
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 1, 1, 1, 1, 1,
										 }, {
											  1, 1, 1, 1, 1,	//  char d1[] = 
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  0, 0, 0, 0, 0,
											  }, {
													1, 1, 1, 1, 1,	//  char d2[] = 
													1, 1, 1, 1, 1,
													0, 0, 0, 0, 0,
													0, 0, 0, 0, 0,
													0, 0, 0, 0, 0,
													0, 0, 0, 0, 0,
													0, 0, 0, 0, 0,
													0, 0, 0, 0, 0,
													}, {
														 1, 1, 1, 1, 1,	//  char d3[] = 
														 1, 1, 1, 1, 1,
														 1, 1, 1, 1, 1,
														 0, 0, 0, 0, 0,
														 0, 0, 0, 0, 0,
														 0, 0, 0, 0, 0,
														 0, 0, 0, 0, 0,
														 0, 0, 0, 0, 0,
														 }, {
															  1, 1, 1, 1, 1,	//  char d4[] = 
															  1, 1, 1, 1, 1,
															  1, 1, 1, 1, 1,
															  1, 1, 1, 1, 1,
															  0, 0, 0, 0, 0,
															  0, 0, 0, 0, 0,
															  0, 0, 0, 0, 0,
															  0, 0, 0, 0, 0,
															  }, {
																	1, 1, 1, 1, 1,	//  char d5[] = 
																	1, 1, 1, 1, 1,
																	1, 1, 1, 1, 1,
																	1, 1, 1, 1, 1,
																	1, 1, 1, 1, 1,
																	0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0,
																	0, 0, 0, 0, 0,
																	}, {
																		 1, 1, 1, 1, 1,	//  char d6[] = 
																		 1, 1, 1, 1, 1,
																		 1, 1, 1, 1, 1,
																		 1, 1, 1, 1, 1,
																		 1, 1, 1, 1, 1,
																		 1, 1, 1, 1, 1,
																		 0, 0, 0, 0, 0,
																		 0, 0, 0, 0, 0,
																		 }, {
																			  1, 1, 1, 1, 1,	//  char d7[] = 
																			  1, 1, 1, 1, 1,
																			  1, 1, 1, 1, 1,
																			  1, 1, 1, 1, 1,
																			  1, 1, 1, 1, 1,
																			  1, 1, 1, 1, 1,
																			  1, 1, 1, 1, 1,
																			  0, 0, 0, 0, 0,
																			  }, {
																					1, 0, 0, 0, 0,	//  char r1[] = 
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					1, 0, 0, 0, 0,
																					}, {
																						 1, 1, 0, 0, 0,	//  char r2[] = 
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 1, 1, 0, 0, 0,
																						 }, {
																							  1, 1, 1, 0, 0,	//  char r3[] = 
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  1, 1, 1, 0, 0,
																							  }, {
																									1, 1, 1, 1, 0,	//  char r4[] = 
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									1, 1, 1, 1, 0,
																									}, {
																										 0, 0, 0, 0, 1,	//  char l1[] = 
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 0, 0, 0, 0, 1,
																										 }, {
																											  0, 0, 0, 1, 1,	//  char l2[] = 
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  0, 0, 0, 1, 1,
																											  }, {
																													0, 0, 1, 1, 1,	//  char l3[] = 
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													0, 0, 1, 1, 1,
																													}, {
																														 0, 1, 1, 1, 1,	//  char l4[] = 
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 0, 1, 1, 1, 1,
																														 }, {
																															  1, 1, 1, 1, 1,	// Empty Heart
																															  1, 0, 1, 0, 1,
																															  0, 0, 0, 0, 0,
																															  0, 0, 0, 0, 0,
																															  0, 0, 0, 0, 0,
																															  1, 0, 0, 0, 1,
																															  1, 1, 0, 1, 1,
																															  1, 1, 1, 1, 1,
																															  }, {
																																	1, 1, 1, 1, 1,	// Filled Heart
																																	1, 0, 1, 0, 1,
																																	0, 1, 0, 1, 0,
																																	0, 1, 1, 1, 0,
																																	0, 1, 1, 1, 0,
																																	1, 0, 1, 0, 1,
																																	1, 1, 0, 1, 1,
																																	1, 1, 1, 1, 1,
																																	}, {
																																		 0, 0, 0, 0, 0,	// Ellipsis
																																		 0, 0, 0, 0, 0,
																																		 0, 0, 0, 0, 0,
																																		 0, 0, 0, 0, 0,
																																		 0, 0, 0, 0, 0,
																																		 0, 0, 0, 0, 0,
																																		 0, 0, 0, 0, 0,
																																		 1, 0, 1, 0, 1,
																																		 }
	};

	MtxOrb_set_char (car, &all_bar[type][0]);
}

/////////////////STOP READING --- TRASH IS AT THE END////////////////

// TODO: Remove this code wich was use for developpement.
// PS: There might be reference to this code left, so keep it for some time.
//
// MtxOrb_init_hbar and MtxOrb_init_vbar use it; it's prototyped in MtxOrb.h ...
static void
MtxOrb_init_all (int type)
{
}
