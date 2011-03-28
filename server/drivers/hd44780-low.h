/** \file server/drivers/hd44780-low.h
 * Interface to low level code to export from hd44780.c to "lower" HW
 * implementation dependent files.
 */

#ifndef HD_LOW_H
#define HD_LOW_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(HAVE_LIBUSB)
# include <usb.h>
#endif

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

#ifdef HAVE_LIBFTDI
# include <ftdi.h>
#endif

// symbolic names for connection types
#define HD44780_CT_UNKNOWN		0
#define HD44780_CT_4BIT			1
#define HD44780_CT_8BIT			2
#define HD44780_CT_SERIALLPT		3
#define HD44780_CT_WINAMP		4
#define HD44780_CT_PICANLCD		5
#define HD44780_CT_LCDSERIALIZER	6
#define HD44780_CT_LOS_PANEL		7
#define HD44780_CT_VDR_LCD		8
#define HD44780_CT_VDR_WAKEUP		9
#define HD44780_CT_PERTELIAN		10
#define HD44780_CT_LIS2			11
#define HD44780_CT_MPLAY		12
#define HD44780_CT_BWCTUSB		13
#define HD44780_CT_LCD2USB		14
#define HD44780_CT_FTDI			15
#define HD44780_CT_I2C			16
#define HD44780_CT_ETHLCD		17
#define HD44780_CT_USS720		18
#define HD44780_CT_USBLCD		19
#define HD44780_CT_USBTINY		20

// symbolic names for interface types
#define IF_TYPE_UNKNOWN		0
#define IF_TYPE_PARPORT		1
#define IF_TYPE_SERIAL		2
#define IF_TYPE_USB		3
#define IF_TYPE_I2C		4
#define IF_TYPE_TCP		5

// symbolic default values
#define DEFAULT_CONTRAST	800
#define DEFAULT_BRIGHTNESS	800
#define DEFAULT_OFFBRIGHTNESS	300


// Maximum sizes of the keypad
// DO NOT CHANGE THESE 2 VALUES, unless you change the functions too
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 11

/* Constants for userdefchar_mode */
#define NUM_CCs 8 /* number of custom characters */


typedef struct cgram_cache {
	unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
	int clean;
} CGram;


typedef struct ConnectionMapping {
	char *name;
	int connectiontype;
	int if_type;
	int (*init_fn)(Driver *drvthis);
} ConnectionMapping;


/**
 * Transmit buffer. This structure provides a generic transmit buffer for
 * collecting command or data bytes before sending them to the display. The
 * connection driver is responsible for allocating memory and resetting
 * \c use_count. \c type may also be driver dependent.
 */
typedef struct tx_buffer_t {
	unsigned char *buffer;		/**< Pointer to buffer */
	int type;			/**< data or command */
	int use_count;			/**< currently used byte */
} tx_buffer;


/** private data for the \c hd44780 driver */
typedef struct hd44780_private_data {
	// parallel connection typeS
	unsigned int port;		/* parallel port */

	// serial connection types
	int fd;				/* file handle to serial device */
	int serial_type;		/* type of device for hd44780-serial */

#if defined(HAVE_LIBUSB)
	// USB connection types
	usb_dev_handle *usbHandle;	/* USB device handle */
	int usbIndex;			/* USB interface index */
#endif

#ifdef HAVE_LIBFTDI
	// FTDI connection type
	struct ftdi_context ftdic, ftdic2;

	int ftdi_mode;
	int ftdi_line_RS;
	int ftdi_line_RW;
	int ftdi_line_EN;
	int ftdi_line_backlight;
#endif

	int sock;			/* socket for TCP devices */

	int charmap;

	int width, height;
	int cellwidth, cellheight;

	// The framebuffer
	unsigned char *framebuf;

	// For incremental updates store last lcd contents
	unsigned char *backingstore;

	// The definable characters
	CGram cc[NUM_CCs];
	CGmode ccmode;

	// Connection type data
	int connectiontype;
	struct hwDependentFns *hd44780_functions;

	// spanList[line number] = display line number is in
	int *spanList;
	int numLines;

	// dispVOffset is a cumulative sized array of line numbers for each display.
	// use this to determine the vertical positioning on a given display
	int *dispVOffset;
	int numDisplays;

	// dispSizes is the vertical size of each display. This is the same as the
	// input span list but is kept to save some cpu cycles.
	int *dispSizes;

	// Keypad, backlight extended interface and delay options
	char have_keypad;	// off by default
	char have_backlight;	// off by default
	char have_output;	// have extra output port (off by default)
	char ext_mode;		// use of extended mode required for some weird controllers
	int line_address; 	// address of the next line in ext_mode (linear addressing)
	int delayMult;	 // Delay multiplier for slow displays
	char delayBus;	 // Delay if the computer can send data too fast over
				 // its bus to LPT port
        char lastline;   // lastline controls the use of the last line, if pixel addressable (true, default) or
                         // underline effect (false). To avoid the underline effect, last line is always zeroed
                         // for whatever redefined character

	// keyMapDirect contains an array of the ascii-codes that should be generated
	// when a directly connected key is pressed (not in matrix).
	char *keyMapDirect[KEYPAD_MAXX];

	// keyMapMatrix contrains an array with arrays of the ascii-codes that should be generated
	// when a key in the matrix is pressed.
	char *keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX];

	char *pressed_key;
	int pressed_key_repetitions;
	struct timeval pressed_key_time;

	int stuckinputs;

	int backlight_bit;

	// force full refresh of display
	time_t nextrefresh;
	int refreshdisplay;     // When >0 make a full display update every <refreshdisplay> seconds
	time_t nextkeepalive;
	int keepalivedisplay;   // When >0 refresh upper left char every <keepalivedisplay> seconds to keep display alive

	int output_state;	// what was most recently output to the output port

	int contrast;		// Contrast setting (range 0 - 1000)
	int brightness;		// Brightness when backlight is "on" (range 0 - 1000)
	int offbrightness;	// Brightness when backlight is "off" (range 0 - 1000)
	int backlightstate;	// Saves the last backlight state

	tx_buffer tx_buf;	// Output buffer
} PrivateData;


// Structures holding pointers to HD44780 specific functions
typedef struct hwDependentFns {
	// microsec pauses
	void (*uPause)(PrivateData *p, int usecs);

	// report and debug helper: set by global hd44780 init
	void (*drv_report)(const int level, const char *format, .../*args*/);
	void (*drv_debug)(const int level, const char *format, .../*args*/);

	// Senddata to the LCD
	// dispID     - display to send data to (0 = all displays)
	// flags      - data or instruction command (RS_DATA | RS_INSTR)
	// ch	      - character to display or instruction value
	void (*senddata)(PrivateData *p, unsigned char dispID, unsigned char flags, unsigned char ch);

	// Flush data to the display. To be used by subdrivers that
	// queue from senddata internally.
	void (*flush)(PrivateData *p);

	// Switch the backlight on or off
	// state      - to be or not to be on
	void (*backlight)(PrivateData *p, unsigned char state);

	// Set the contrast
	// value      - new value to be set
	void (*set_contrast)(PrivateData *p, unsigned char value);

	// Read the keypad
	// Ydata      - the up to 11 bits that should be put on the Y side of the matrix
	// return     - the up to 5 bits that are read out on the X side of the matrix
	unsigned char (*readkeypad)(PrivateData *p, unsigned int Ydata);

	// Scan the keypad and return a scancode.
	// The code is the Yvalue in the high nibble and the Xvalue in the low nibble.
	// A subdriver should do only one of two things:
	// - set readkeypad; or
	// - override scankeypad.
	unsigned char (*scankeypad)(PrivateData *p);

	// Output "data" to output latch if there is one
	void (*output)(PrivateData *p, int data);

        // Close the interface on shutdown
        void (*close)(PrivateData *p);


} HD44780_functions;				  /* for want of a better name :-) */


void common_init(PrivateData *p, unsigned char if_bit);


// commands for senddata
#define RS_DATA		0x00
#define RS_INSTR	0x01

#define CLEAR		0x01

#define HOMECURSOR	0x02

#define ENTRYMODE	0x04
#define E_MOVERIGHT	0x02
#define E_MOVELEFT	0x00
#define EDGESCROLL	0x01
#define NOSCROLL	0x00

#define ONOFFCTRL	0x08	/* Only reachable with EXTREG clear */
#define DISPON		0x04
#define DISPOFF		0x00
#define CURSORON	0x02
#define CURSOROFF	0x00
#define CURSORBLINK	0x01
#define CURSORNOBLINK	0x00

#define EXTMODESET	0x08	/* Only reachable with EXTREG set */
#define FONT6WIDE	0x04
#define INVCURSOR	0x02
#define FOURLINE	0x01

#define CURSORSHIFT	0x10	/* Only reachable with EXTREG clear */
#define SCROLLDISP	0x08
#define MOVECURSOR	0x00
#define MOVERIGHT	0x04
#define MOVELEFT	0x00

#define HSCROLLEN	0x10	/* Only reachable with EXTREG set */

#define FUNCSET		0x20
#define IF_8BIT		0x10
#define IF_4BIT		0x00
#define TWOLINE		0x08
#define ONELINE		0x00
#define LARGECHAR	0x04	/* 5x11 characters */
#define SMALLCHAR	0x00	/* 5x8 characters */
#define EXTREG		0x04	/* Select ext. registers (Yes, the same bits)*/
#define SEGBLINK	0x02	/* Only reachable with EXTREG set */
#define POWERDOWN	0x01	/* Only reachable with EXTREG set */

#define SETCHAR		0x40	/* Only reachable with EXTREG clear */

#define SETSEG		0x40	/* Only reachable with EXTREG set */

#define POSITION	0x80	/* Only reachable with EXTREG clear */

#define HSCROLLAMOUNT	0x80	/* Only reachable with EXTREG set */

#define LADDR		0x20	/* Default Lineaddress in ext_mode */

#endif
