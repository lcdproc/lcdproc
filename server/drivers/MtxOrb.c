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
//#include "drv_base.h"

// I don't want to break anything here so let's do it step by step
//#define USE_REPORT
#ifdef USE_REPORT
//#define DEBUG
#include "shared/report.h"
//#undef DEBUG
#else
#include "shared/debug.h"
#endif

#include "shared/str.h"
#include "input.h"

#define IS_LCD_DISPLAY	(MtxOrb_type == MTXORB_LCD)
#define IS_LKD_DISPLAY	(MtxOrb_type == MTXORB_LKD)
#define IS_VFD_DISPLAY	(MtxOrb_type == MTXORB_VFD)
#define IS_VKD_DISPLAY	(MtxOrb_type == MTXORB_VKD)

#define NotEnoughArgs (i + 1 > argc)

// NOTE: This does not appear to make use of the
//       hbar and vbar functions present in the LKD202-25.
//       Why I do not know.
// RESP: Because software emulated hbar/vbar permit simultaneous use.

#ifdef USE_REPORT
#else
extern int debug_level;
#endif

// TODO: Remove this custom_type if not in use anymore.
typedef enum {
	bar = 2,
	bign = 4,
	beat = 8
} custom_type;

#define DIRTY_CHAR 254
#define START_ICON 22
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
	empty_heart = 22,
	filled_heart = 23,
	ellipsis = 24,
	barw = 32,
	barb = 255
} bar_type;

static int custom = 0;
static enum {MTXORB_LCD, MTXORB_LKD, MTXORB_VFD, MTXORB_VKD} MtxOrb_type;
static int fd;
static int clear = 1;

static int def[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int use[9] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *framebuf = NULL;
static int width = LCD_DEFAULT_WIDTH;
static int height = LCD_DEFAULT_HEIGHT;
static int cellwidth = LCD_DEFAULT_CELLWIDTH;
static int cellheight = LCD_DEFAULT_CELLHEIGHT;
static int contrast = DEFAULT_CONTRAST;

// Vars for the server core
MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "MtxOrb_";


static int  MtxOrb_ask_bar (Driver *drvthis, int type);
static void MtxOrb_set_known_char (Driver * drvthis, int car, int type);
static void MtxOrb_linewrap (Driver *drvthis, int on);
static void MtxOrb_autoscroll (Driver *drvthis, int on);
static void MtxOrb_cursorblink (Driver *drvthis, int on);

// Very private function that clear internal definition.
static void
MtxOrb_clear_custom (Driver *drvthis)
{
	int pos;

	for (pos = 0; pos < 9; pos++) {
		def[pos] = -1;		// Not in use.
		use[pos] = 0;		// Not in use.
		}
}

static int
MtxOrb_parse_type (char * str) {
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
MtxOrb_parse_speed (char *arg) {
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
MtxOrb_parse_contrast (char * str) {
	int contrast;

	contrast = atoi (str);
	if ((contrast < 0) || (contrast > 255)) {
		fprintf(stderr, "MtxOrb_init: argument must between 0 and 255 (found %s). Using default contrast value of %d.\n", str, DEFAULT_CONTRAST);
		contrast = DEFAULT_CONTRAST;
	}
	return contrast;
}

// TODO:  Get the frame buffers working right

/////////////////////////////////////////////////////////////////
// Opens com port and sets baud correctly...
//
// Called to initialize driver settings
//
int
MtxOrb_init (Driver *drvthis, char *args)
{
	char *argv[64];		// Notice: 64 arguments - overflows?
	int argc;
	struct termios portset;
	int i;
	//int tmp;

	int contrast = DEFAULT_CONTRAST;
	char device[256] = DEFAULT_DEVICE;
	int speed = DEFAULT_SPEED;
	//char c;

	MtxOrb_type = MTXORB_LKD;  // Assume it's an LCD w/keypad

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
				speed = MtxOrb_parse_speed(optarg);
				break;
			case 'c':
				contrast = MtxOrb_parse_contrast(optarg);
				break;
			case 'h':
				MtxOrb_usage();
				return -1;
			case 't':
				MtxOrb_type = MtxOrb_parse_type(optarg);
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
					contrast = MtxOrb_parse_contrast (argv[++i]);
					break;
				case 's':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					speed = MtxOrb_parse_speed (argv[++i]);
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

						width = wid;
						height = hgt;
						}
					break;
				case 'b':
					if (i + 1 > argc) {
						fprintf (stderr, "MtxOrb_init: %s requires an argument\n", argv[i]);
						return -1;
					}
					i++;
					MtxOrb_type = MtxOrb_parse_type(argv[i]);
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

	// Make sure the frame buffer is there...
	if (!framebuf)
		framebuf = (unsigned char *)
			malloc (width * height);
	memset (framebuf, ' ', width * height);

	/*
	 * Configure display
	 */

	MtxOrb_linewrap (drvthis, DEFAULT_LINEWRAP);
	MtxOrb_autoscroll (drvthis, DEFAULT_AUTOSCROLL);
	MtxOrb_cursorblink (drvthis, DEFAULT_CURSORBLINK);
	MtxOrb_set_contrast (drvthis, contrast);

	/*
	 * Configure the display functions
	 */

	// Set variables for server
	drvthis->api_version = api_version;
	drvthis->stay_in_foreground = &stay_in_foreground;
	drvthis->supports_multiple = &supports_multiple;

	// Set the functions the driver supports
	drvthis->clear = MtxOrb_clear;
	drvthis->string = MtxOrb_string;
	drvthis->chr = MtxOrb_chr;
	drvthis->old_vbar = MtxOrb_vbar;
	drvthis->init_vbar = MtxOrb_init_vbar;
	drvthis->old_hbar = MtxOrb_hbar;
	drvthis->init_hbar = MtxOrb_init_hbar;
	drvthis->num = MtxOrb_num;
	drvthis->init_num = MtxOrb_init_num;

	drvthis->init = MtxOrb_init;
	drvthis->close = MtxOrb_close;
	drvthis->width = MtxOrb_width;
	drvthis->height = MtxOrb_height;
	drvthis->flush = MtxOrb_flush;
	drvthis->get_contrast = MtxOrb_get_contrast;
	drvthis->set_contrast = MtxOrb_set_contrast;
	drvthis->backlight = MtxOrb_backlight;
	drvthis->output = MtxOrb_output;
	drvthis->set_char = MtxOrb_set_char;
	drvthis->old_icon = MtxOrb_icon;

	drvthis->getkey = MtxOrb_getkey;
	drvthis->get_info = MtxOrb_get_info;
	drvthis->heartbeat = MtxOrb_heartbeat;

	return 0;
}

#define ValidX(x) if ((x) > width) { (x) = width; } else (x) = (x) < 1 ? 1 : (x);
#define ValidY(y) if ((y) > height) { (y) = height; } else (y) = (y) < 1 ? 1 : (y);

// TODO: Check this quick hack to detect clear of the screen.
/////////////////////////////////////////////////////////////////
// Clear: catch up when the screen get clear to be able to
//  forget bar caracter not in use anymore and reuse the
//  slot for another bar caracter.
//
MODULE_EXPORT void
MtxOrb_clear (Driver *drvthis)
{
	if (framebuf != NULL)
		memset (framebuf, ' ', (width * height));

	//write(fd, "\x0FE" "X", 2);  // instant clear...
	clear = 1;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: cleared screen");
#else
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: cleared screen");
#endif
}

/////////////////////////////////////////////////////////////////
// Clean-up
//
MODULE_EXPORT void
MtxOrb_close (Driver *drvthis)
{
	close (fd);

	if (framebuf)
		free (framebuf);
	framebuf = NULL;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: closed");
#else
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: closed");
#endif
}

/////////////////////////////////////////////////////////////////
// Returns the display width
//
MODULE_EXPORT int
MtxOrb_width (Driver *drvthis)
{
	return width;
}

/////////////////////////////////////////////////////////////////
// Returns the display height
//
MODULE_EXPORT int
MtxOrb_height (Driver *drvthis)
{
	return height;
}

MODULE_EXPORT void
MtxOrb_string (Driver *drvthis, int x, int y, char *string)
{
	int offset, siz;

	ValidX(x);
	ValidY(y);

	x--; y--; // Convert 1-based coords to 0-based...
	offset = (y * width) + x;
	siz = (width * height) - offset - 1;
	siz = siz > strlen(string) ? strlen(string) : siz;

	memcpy(framebuf + offset, string, siz);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: printed string at (%d,%d)", x, y);
#else
	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: printed string at (%d,%d)", x, y);
#endif
}

MODULE_EXPORT void
MtxOrb_flush (Driver *drvthis)
{
	char out[12];
	int i,j,mv = 1;
	static char *old = NULL;
	char *p, *q;

	if (old == NULL) {
		old = malloc(width * height);

		write(fd, "\x0FEG\x01\x01", 4);
		write(fd, framebuf, width * height);

		strncpy(old, framebuf, width * height);

		return;

	} else {
		/* CODE TEMPORARY DISABLED (joris)
		   UNSURE IF IT STILL WORKS NOW
		if (! new_framebuf(drvthis, old))
			return;
		*/
	}

	p = framebuf;
	q = old;

	for (i = 1; i <= height; i++) {
		for (j = 1; j <= width; j++) {

			if ((*p == *q) && (*p > 8))
				mv = 1;
			else {
				// Draw characters that have changed, as well
				// as custom characters.  We know not if a custom
				// character has changed.

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


	//for (i = 0; i < height; i++) {
	//	snprintf (out, sizeof(out), "\x0FEG\x001%c", i + 1);
	//	write (fd, out, 4);
	//	write (fd, framebuf + (width * i), width);
	//}

	strncpy(old, framebuf, width * height);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: frame buffer flushed");
#else
	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: frame buffer flushed");
#endif
}

/////////////////////////////////////////////////////////////////
// Prints a character on the lcd display, at position (x,y).  The
// upper-left is (1,1), and the lower right should be (20,4).
//
MODULE_EXPORT void
MtxOrb_chr (Driver *drvthis, int x, int y, char c)
{
#ifdef USE_REPORT
#else
	char buf[64]; // char out[10];
#endif
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
	offset = (y * width) + x;
	framebuf[offset] = c;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "writing character %02X to position (%d,%d)", c, x, y);
	debug(RPT_DEBUG, "MtxOrb: printed a char at (%d,%d)", x, y);
#else
	if (debug_level > 2) {
		snprintf(buf, sizeof(buf), "writing character %02X to position (%d,%d)", c, x, y);
		syslog(LOG_DEBUG, buf);

	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: printed a char at (%d,%d)", x, y);
	}
#endif
}

MODULE_EXPORT int
MtxOrb_get_contrast (Driver *drvthis)
{
	return contrast;
}

/////////////////////////////////////////////////////////////////
// Changes screen contrast (0-255; 140 seems good)
// note: works only for LCD displays
// Is it better to use the brightness for VFD/VKD displays ?
//
MODULE_EXPORT void
MtxOrb_set_contrast (Driver *drvthis, int promille)
{
	char out[4];
	int real_contrast;

	// Check it
	if( promille < 0 || promille > 1000 )
		return;

	// Store it
	contrast = promille;

	real_contrast = (int) ((long)promille * 255 / 1000 );

	// And do it

	if (IS_LCD_DISPLAY || IS_LKD_DISPLAY) {
		snprintf (out, sizeof(out), "\x0FEP%c", real_contrast);
		write (fd, out, 3);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: contrast set to %d", real_contrast);
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: contrast set to %d", real_contrast);
#endif
	} else {
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: contrast not set to %d - not LCD or LKD display", real_contrast);
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: contrast not set to %d - not LCD or LKD display", real_contrast);
#endif
	}
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

MODULE_EXPORT void
MtxOrb_backlight (Driver *drvthis, int on)
{
	static int backlight_state = 1;

	if (backlight_state == on)
		return;

	backlight_state = on;

	switch (on) {
		case BACKLIGHT_ON:
			write (fd, "\x0FE" "F", 2);
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: backlight turned on");
#else
			if (debug_level > 3)
				syslog(LOG_DEBUG, "MtxOrb: backlight turned on");
#endif
			break;
		case BACKLIGHT_OFF:
			if (IS_VKD_DISPLAY || IS_VFD_DISPLAY) {
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: backlight ignored - not LCD or LKD display");
#else
				if (debug_level > 3)
					syslog(LOG_DEBUG, "MtxOrb: backlight ignored - not LCD or LKD display");
#endif
				; // turns display off entirely (whoops!)
			} else {
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: backlight turned off");
#else
				if (debug_level > 3)
					syslog(LOG_DEBUG, "MtxOrb: backlight turned off");
#endif
				write (fd, "\x0FE" "B" "\x000", 3);
			}
			break;
		default: // ignored...
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: backlight - invalid setting");
#else
			if (debug_level > 3)
				syslog(LOG_DEBUG, "MtxOrb: backlight - invalid setting");
#endif
			break;
		}
}

/////////////////////////////////////////////////////////////////
// Sets output port on or off
// displays with keypad have 6 outputs but the one without kepad
// have only one output
// NOTE: length of command are different
MODULE_EXPORT void
MtxOrb_output (Driver *drvthis, int on)
{
	char out[5];
	static int output_state = -1;

	on = on & 077;	// strip to six bits

	if (output_state == on)
		return;

	output_state = on;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: output pins set: %04X", on);
#else
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: output pins set: %04X", on);
#endif

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
MtxOrb_linewrap (Driver *drvthis, int on)
{
	if (on) {
		write (fd, "\x0FE" "C", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: linewrap turned on");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: linewrap turned on");
#endif
	} else {
		write (fd, "\x0FE" "D", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: linewrap turned off");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: linewrap turned off");
#endif
	}
}

/////////////////////////////////////////////////////////////////
// Toggle the built-in automatic scrolling feature
//
static void
MtxOrb_autoscroll (Driver *drvthis, int on)
{
	if (on) {
		write (fd, "\x0FEQ", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: autoscroll turned on");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: autoscroll turned on");
#endif
	} else {
		write (fd, "\x0FER", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: autoscroll turned off");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: autoscroll turned off");
#endif
	}
}

// TODO: make sure this doesn't mess up non-VFD displays
/////////////////////////////////////////////////////////////////
// Toggle cursor blink on/off
//
static void
MtxOrb_cursorblink (Driver *drvthis, int on)
{
	if (on) {
		write (fd, "\x0FES", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: cursorblink turned on");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: cursorblink turned on");
#endif
	} else {
		write (fd, "\x0FET", 2);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: cursorblink turned off");
#else
		if (debug_level > 3)
			syslog(LOG_DEBUG, "MtxOrb: cursorblink turned off");
#endif
	}
}

/////////////////////////////////////////////////////////////////
// Sets up for vertical bars.  Call before lcd.vbar()
//
MODULE_EXPORT void
MtxOrb_init_vbar (Driver *drvthis)
{
	custom = bar;
}

/////////////////////////////////////////////////////////////////
// Inits horizontal bars...
//
MODULE_EXPORT void
MtxOrb_init_hbar (Driver *drvthis)
{
	custom = bar;
}

/////////////////////////////////////////////////////////////////
// Returns string with general information about the display
//
MODULE_EXPORT char *
MtxOrb_get_info (Driver *drvthis)
{
	char in = 0;
	static char info[255];
	char tmp[255], buf[64];
	//int i = 0;
	fd_set rfds;

	struct timeval tv;
	int retval;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: get_info");
#else
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: get_info");
#endif

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
MODULE_EXPORT void
MtxOrb_vbar (Driver *drvthis, int x, int len)
{
	unsigned char mapu[9] = { barw, baru1, baru2, baru3, baru4, baru5, baru6, baru7, barb };
	unsigned char mapd[9] = { barw, bard1, bard2, bard3, bard4, bard5, bard6, bard7, barb };

	int y;

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: vertical bar at %d set to %d", x, len);
#else
	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: vertical bar at %d set to %d", x, len);
#endif

// REMOVE THE NEXT LINE FOR TESTING ONLY...
//  len=-len;
// REMOVE THE PREVIOUS LINE FOR TESTING ONLY...

	if (len > 0) {
		for (y = height; y > 0 && len > 0; y--) {
			if (len >= cellheight)
				MtxOrb_chr (drvthis, x, y, 255);
			else
				MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, mapu[len]));

			len -= cellheight;
		}
	} else {
		len = -len;
		for (y = 2; y <= height && len > 0; y++) {
			if (len >= cellheight)
				MtxOrb_chr (drvthis, x, y, 255);
			else
				MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, mapd[len]));

			len -= cellheight;
		}
	}

}

// TODO: Finish the support for bar growing reverse way.
/////////////////////////////////////////////////////////////////
// Draws a horizontal bar to the right.
// This is the new version ussing dynamic icon alocation
//
MODULE_EXPORT void
MtxOrb_hbar (Driver *drvthis, int x, int y, int len)
{
	unsigned char mapr[6] = { barw, barr1, barr2, barr3, barr4, barb };
	unsigned char mapl[6] = { barw, barl1, barl2, barl3, barl4, barb };

	ValidX(x);
	ValidY(y);

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: horizontal bar at %d set to %d", x, len);
#else
	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: horizontal bar at %d set to %d", x, len);
#endif

	if (len > 0) {
		for (; x <= width && len > 0; x++) {
			if (len >= cellwidth)
				MtxOrb_chr (drvthis, x, y, 255);
			else
				MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, mapr[len]));

			len -= cellwidth;

		}
	} else {
		len = -len;
		for (; x > 0 && len > 0; x--) {
			if (len >= cellwidth)
				MtxOrb_chr (drvthis, x, y, 255);
			else
				MtxOrb_chr (drvthis, x, y, MtxOrb_ask_bar (drvthis, mapl[len]));

			len -= cellwidth;

		}
	}

}

// TODO: Might not work, bignum is untested... an untested with dynamic bar.
//
// TODO: Rather than to use the hardware BigNum we should use software
// emulation, this will make it work simultaniously as hbar/vbar. GLU
//
/////////////////////////////////////////////////////////////////
// Sets up for big numbers.
//
MODULE_EXPORT void
MtxOrb_init_num (Driver *drvthis)
{
#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: init for big numbers");
#else
	if (debug_level > 3)
		syslog(LOG_DEBUG, "MtxOrb: init for big numbers");
#endif

	if (custom != bign) {
		write (fd, "\x0FEn", 2);
		custom = bign;
		MtxOrb_clear_custom (drvthis);
	}

}

// TODO: MtxOrb_set_char is d ing the j b "real-time" as oppose
// to at flush time. Call to this function should be done in flush
// this mean in  raw_frame. GLU
//
// TODO: Rather than to use the hardware BigNum we should use software
// emulation, this will make it work simultaniously as hbar/vbar. GLU
//
// TODO: Before the desinitive solution we need to make the caracter
// hiden behind the hardware bignum dirty so that they get cleaned
// when draw_frame is called. There is no dirty char so I will use 254
// hoping nowone is using that char. GLU

/////////////////////////////////////////////////////////////////
// Writes a big number.
//
MODULE_EXPORT void
MtxOrb_num (Driver *drvthis, int x, int num)
{
	int y, dx;
	char out[5];

#ifdef USE_REPORT
	debug(RPT_DEBUG, "MtxOrb: write big number %d at %d", num, x);
#else
	if (debug_level > 4)
		syslog(LOG_DEBUG, "MtxOrb: write big number %d at %d", num, x);
#endif

	snprintf (out, sizeof(out), "\x0FE#%c%c", x, num);
	write (fd, out, 4);

// Make this space dirty as far as frame buffer knows.
	for (y = 1; y < 5; y++)
		for (dx = 0; dx < 3; dx++)
			MtxOrb_chr (drvthis, x + dx, y, DIRTY_CHAR);

}

// TODO: Every time we define a custom char within the LCD,
// we have to compute the binary value we are going to use.
// It is easy to keep the bitmap in this source file,
// but we compute that once rather than every time. GLU
//
// TODO: MtxOrb_set_char is doing the job "real-time" as oppose
// to at flush time. Call to this function should be done in flush
// this mean in draw_frame. GLU
//
// TODO: _icon should not call this directly, this is why we define
// so frequently the heartbeat custom char. GLU
//
// TODO: We make one 3 bytes write folowed by cellheight one byte
// write. This should be done in one single write. GLU

#define MAX_CUSTOM_CHARS 7

/////////////////////////////////////////////////////////////////
// Sets a custom character from 0-7...
//
// The input is just an array of characters...
//
MODULE_EXPORT void
MtxOrb_set_char (Driver *drvthis, int n, char *dat)
{
	char out[4];
	int row, col;
	int letter;

	if (n < 0 || n > MAX_CUSTOM_CHARS)
		return;
	if (!dat)
		return;

	snprintf (out, sizeof(out), "\x0FEN%c", n);
	write (fd, out, 3);

	for (row = 0; row < cellheight; row++) {
		letter = 0;
		for (col = 0; col < cellwidth; col++) {
			// shift to make room for new scan line data
			letter <<= 1;
			// Now read a single bit of data
			// -- one entry in dat[] --
			// and add it to the binary data in "letter"
			letter |= (dat[(row * cellwidth) + col] > 0);
		}
		write (fd, &letter, 1); // write one character for each row
	}
}

// TODO (DONE): All the icon are now define at the end of the custom char
// used for hbar/vbar. GLU
//
// TODO (DONE): Don't make direct call to caracter definition if the caracter is
// already defined. GLU
MODULE_EXPORT void
MtxOrb_icon (Driver *drvthis, int which, char dest)
{
	if (custom == bign)
		custom = beat;
	MtxOrb_set_known_char (drvthis, dest, START_ICON+which);
}

// TODO: Recover the code for I2C connectivity to MtxOrb
// and don't query the LCD if it does not support keypad.
// Otherwise crash of the LCD and/or I2C bus.
//
/////////////////////////////////////////////////////////////
// returns one character from the keypad...
// (A-Z) on success, 0 on failure...
//
MODULE_EXPORT char
MtxOrb_getkey (Driver *drvthis)
{
	char in = 0;

	read (fd, &in, 1);
	switch (in) {
		case KEY_LEFT:
			in = INPUT_BACK_KEY;
			break;
		case KEY_RIGHT:
			in = INPUT_FORWARD_KEY;
			break;
		case KEY_DOWN:
			in = INPUT_MAIN_MENU_KEY;
			break;
		case KEY_F1:
			in = INPUT_PAUSE_KEY;
			break;
		/*TODO: add more translations here (if neccessary)*/
		default:
			in = 0;
			break;
	}

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
MtxOrb_ask_bar (Driver *drvthis, int type)
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
			MtxOrb_set_known_char (drvthis, pos, type);	// Define a new graphic caracter.
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
MODULE_EXPORT void
MtxOrb_heartbeat (Driver *drvthis, int type)
{
	int the_icon=255;
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
//		MtxOrb_icon (whichIcon, 0);
		the_icon=MtxOrb_ask_bar (drvthis, whichIcon+START_ICON);

		// Put character on screen...
		MtxOrb_chr (drvthis, width, 1, the_icon);

		// change display...
		MtxOrb_flush (drvthis);
	}

	timer++;
	timer &= 0x0f;
}

/////////////////////////////////////////////////////////////////
// Sets up a well known character for use.
//
static void
MtxOrb_set_known_char (Driver *drvthis, int car, int type)
{
	char all_bar[25][5 * 8] = {
		{
		0, 0, 0, 0, 0,	//  char u1[] =
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	//  char u2[] =
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		}, {
		0, 0, 0, 0, 0,	//  char u3[] =
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

	MtxOrb_set_char (drvthis, car, &all_bar[type][0]);
}

