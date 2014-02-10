/** \file server/drivers/hd44780-low.h
 * Interface to low level code to export from hd44780.c to "lower" hardware
 * implementation dependent files.
 */

#ifndef HD_LOW_H
#define HD_LOW_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_LIBUSB
# include <usb.h>
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

#ifdef HAVE_LIBFTDI
# include <ftdi.h>
#endif

/** \name Symbolic names for connection types
 *@{*/
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
#define HD44780_CT_USB4ALL		21
#define HD44780_CT_RASPBERRYPI		22
#define HD44780_CT_PIPLATE		23
#define HD44780_CT_SPI			24
#define HD44780_CT_PIFACECAD		25
/**@}*/

/** \name Symbolic names for interface types
 *@{*/
#define IF_TYPE_UNKNOWN		0
#define IF_TYPE_PARPORT		1
#define IF_TYPE_SERIAL		2
#define IF_TYPE_USB		3
#define IF_TYPE_I2C		4
#define IF_TYPE_TCP		5
#define IF_TYPE_SPI		6
/**@}*/

/** \name Symbolic default values
 *@{*/
#define DEFAULT_CONTRAST	800
#define DEFAULT_BRIGHTNESS	800
#define DEFAULT_OFFBRIGHTNESS	300
/**@}*/

/** \name Maximum sizes of the keypad
 *@{*/
/* DO NOT CHANGE THESE VALUES, unless you change the functions too! */
#define KEYPAD_MAXX 5
#define KEYPAD_MAXY 11
/**@}*/

/** number of custom characters */
#define NUM_CCs 8

/**
 * One entry of the custom character cache consists of 8 bytes of cache data
 * and a clean flag.
 */
typedef struct cgram_cache {
	unsigned char cache[LCD_DEFAULT_CELLHEIGHT];
	int clean;
} CGram;


/**
 * Provides necessary data to initialize a connection type (sub-driver).
 */
typedef struct ConnectionMapping {
	/** Name (string) of connection type as used in LCDd.conf */
	char *name;
	/** Registered number of connection type, see above */
	int connectiontype;
	/** Numeric type (serial, parallel, etc), see above */
	int if_type;
	/** Pointer to connection type's initialization function */
	int (*init_fn) (Driver *drvthis);
} ConnectionMapping;


/**
 * Transmit buffer. This structure provides a generic transmit buffer for
 * collecting command or data bytes before sending them to the display. The
 * connection driver is responsible for allocating memory and resetting
 * \c use_count. \c type may also be driver dependent.
 */
typedef struct tx_buffer_t {
	unsigned char *buffer;	/**< Pointer to buffer */
	int type;		/**< Data or Command indicator */
	int use_count;		/**< Number of currently used bytes */
} tx_buffer;


/** private data for the \c hd44780 driver */
typedef struct hd44780_private_data {
	/* parallel connection types */
	unsigned int port;	/**< parallel port */

	/* serial connection types */
	int fd;			/**< file handle to serial device */
	int serial_type;	/**< type of device for hd44780-serial */

#ifdef HAVE_LIBUSB
	/* libusb-based connection types */
	usb_dev_handle *usbHandle;	/**< USB device handle */
	int usbIndex;		/**< USB interface index */
	int usbMode;		/**< USB write mode (bulk or interrupt)*/
	int usbEpOut;		/**< USB Endpoint Out */
	int usbEpIn;		/**< USB Endpoint In */
	tx_buffer rx_buf;	/**< Input buffer */
#endif

#ifdef HAVE_LIBFTDI
	/* libftdi-based connection types */
	struct ftdi_context ftdic, ftdic2;

	int ftdi_mode;
	int ftdi_line_RS;
	int ftdi_line_RW;
	int ftdi_line_EN;
	int ftdi_line_backlight;
#endif

#ifdef WITH_ETHLCD
	int sock;		/**< socket for TCP devices */
#endif
#ifdef WITH_RASPBERRYPI
	struct rpi_gpio_map *rpi_gpio;	/**< GPIO pin mapping for Raspberry Pi */
#endif

	int charmap;		/**< index of currently used charmap */

	int width, height;	/**< size of display (characters) */
	int cellwidth, cellheight;	/**< size a one cell (pixels) */

	unsigned char *framebuf;	/**< the framebuffer */
	unsigned char *backingstore;	/**< buffer for incremental updates */

	CGram cc[NUM_CCs];	/**< the custom character cache */
	CGmode ccmode;		/**< character mode of the current screen */

	/* Connection type data */
	int connectiontype;
	struct hwDependentFns *hd44780_functions;

	/** \name Screens spanning several display
	 *@{*/
	/**
	 * \c spanlist is an array which stores the number of the display a
	 * line is shown own. (disp=spanList[line])
	 */
	int *spanList;
	int numLines;		/**< total number of lines for screen */

	/**
	 * \c dispVOffset is an array that stores for each display its offset
	 * aka start line within the (virtual) screen. Use this to determine
	 * the vertical positioning on a given display.
	 */
	int *dispVOffset;
	int numDisplays;	/**< total number of displays */

	/** Array storing the vertical size of each display. */
	int *dispSizes;
	/**@}*/

	/** \name Display features
	 *@{*/
	char have_keypad;
	char have_backlight;
	char have_output;
	/**@}*/

	char ext_mode;		/**< use extended mode on some weird controllers */
	int line_address;	/**< address of the next line in extended mode  */

	int delayMult;		/**< Delay multiplier for slow displays */
	char delayBus;		/**< Delay if data is sent too fast over LPT port */

	/**
	 * lastline controls the use of the last line, if pixel addressable
	 * (true, default) or underline effect (false). To avoid the
	 * underline effect, last line is always zeroed for whatever
	 * redefined character.
	 */
	char lastline;

	/** \name Keypad
	 *@{*/
	/**
	 * keyMapDirect contains an array of strings (ASCII-codes) that
	 * should be generated when a directly connected key is pressed (not
	 * in matrix).
	 */
	char *keyMapDirect[KEYPAD_MAXX];

	/**
	 * keyMapMatrix contains an array with arrays of strings
	 * (ASCII-codes) that should be generated when a key in the matrix is
	 * pressed.
	 */
	char *keyMapMatrix[KEYPAD_MAXY][KEYPAD_MAXX];

	char *pressed_key;		/**< The last pressed key */
	int pressed_key_repetitions;	/**< Number of repeated key presses */
	struct timeval pressed_key_time;/**< Time the key was pressed first */
	int stuckinputs;		/**< Value on the parallel port input if no keys are pressed */
	/**@}*/

	int backlight_bit;		/**< shorthand for the value of the BL bit if it is set */

	/** \name Forced screen updates
	 *@{*/
	time_t nextrefresh;	/**< Time when the next refresh is due. */
	int refreshdisplay;	/**< Seconds after which a complete display update is forced. */
	/**@}*/

	/** \name Keepalive
	 *@{*/
	time_t nextkeepalive;	/**< Time the next keep-alive is due. */
	int keepalivedisplay;	/**< Refresh upper left char every \c keepalivedisplay seconds. */
	/**@}*/

	int output_state;	/**< most recently output to the output port */

	/** \name Contrast and brightness
	 *@{*/
	int contrast;		/**< Contrast value (range 0 - 1000) */
	int brightness;		/**< Brightness when backlight is "on" (range 0 - 1000) */
	int offbrightness;	/**< Brightness when backlight is "off" (range 0 - 1000) */
	int backlightstate;	/**< Saves the last backlight state (on/off) */
	/**@}*/

	/** Output buffer to collect command or data bytes */
	tx_buffer tx_buf;
} PrivateData;


/** Structure holding pointers to HD44780 specific functions */
typedef struct hwDependentFns {
	/** Wait for some micro seconds.
	 * \param p      pointer to private date structure
	 * \param usecs  micro seconds to wait
	 */
	void (*uPause) (PrivateData *p, int usecs);

	/** \name Report and debug helper: set by global hd44780 init */
	/**@{*/
	void (*drv_report) (const int level, const char *format,... /* args */ );
	void (*drv_debug) (const int level, const char *format,... /* args */ );
	/**@}*/

	/** Send a single byte of data to the LCD.
	 * \param p       pointer to private date structure
	 * \param dispID  display to send data to (0 = all displays)
	 * \param flags   data or instruction command (RS_DATA | RS_INSTR)
	 * \param ch      character to display or instruction value
	 */
	void (*senddata) (PrivateData *p, unsigned char dispID, unsigned char flags, unsigned char ch);

	/**
	 * Flush data to the display. To be used by sub-drivers that queue from
	 * senddata internally.
	 */
	void (*flush) (PrivateData *p);

	/** Switch the backlight on or off
	 * \param p      pointer to private date structure
	 * \param state  to be or not to be on
	 */
	void (*backlight) (PrivateData *p, unsigned char state);

	/** Set the contrast
	 * \param p      pointer to private date structure
	 * \param value  new value to be set
	 */
	void (*set_contrast) (PrivateData *p, unsigned char value);

	/** Read the keypad
	 * \param p      pointer to private date structure
	 * \param Ydata  Up to 11 bits that should be put on the Y side
	 *               of the matrix.
	 * \return  Up to 5 bits that are read out on the X side of the matrix.
	 */
	unsigned char (*readkeypad) (PrivateData *p, unsigned int Ydata);

	/** Scan the keypad and return a scancode.
	 * \param p  pointer to private date structure
	 * \return   Y-value in the high nibble and X-value in the low nibble.
	 *
	 * \note  A sub-driver should either implement \c readkeypad or
	 *        override \c scankeypad.
	 */
	unsigned char (*scankeypad) (PrivateData *p);

	/** Output "data" to output latch if there is one.
	 * \param p      pointer to private date structure
	 * \param data   Value to put on the output latch (up to 32 bit)
	 */
	void (*output) (PrivateData *p, int data);

	/** Close the interface on shutdown */
	void (*close) (PrivateData *p);
} HD44780_functions;


/* Prototypes */
void common_init(PrivateData *p, unsigned char if_bit);


/* commands for senddata */
#define RS_DATA		0x00
#define RS_INSTR	0x01

/** \name ---------------- The HD44780 Instruction Set -------------------- */
/**@{*/
/*
 * The instruction set below is for the original HD44780 and compatibel and
 * those that support extended function registers (like the KS0073). For
 * extended function registers two instruction sets exist but we assume
 * "instruction set 2" below.
 *
 * Note: RE=0 means EXTREG bit has not been set, RE=1 means EXTREG has been
 * set. The term 'RE' is used in the datasheets so I use it here as well.
 */

/** Clear display */
#define CLEAR		0x01

/** Return home */
#define HOMECURSOR	0x02

/** Entry mode set */
#define ENTRYMODE	0x04
#define E_MOVERIGHT	0x02
#define E_MOVELEFT	0x00
#define EDGESCROLL	0x01
#define NOSCROLL	0x00

/** Display on/off control (RE=0) */
#define ONOFFCTRL	0x08
#define DISPON		0x04
#define DISPOFF		0x00
#define CURSORON	0x02
#define CURSOROFF	0x00
#define CURSORBLINK	0x01
#define CURSORNOBLINK	0x00

/** Extended function set (RE=1) */
#define EXTMODESET	0x08
#define FONT6WIDE	0x04
#define FONT5WIDE	0x00
#define INVCURSOR	0x02
#define FOURLINE	0x01	/**< If not set, ONELINE or TWOLINE is used */

/** Cursor or display shift (RE=0) */
#define CURSORSHIFT	0x10
#define SCROLLDISP	0x08
#define MOVECURSOR	0x00
#define MOVERIGHT	0x04
#define MOVELEFT	0x00

/** Shift or scroll enable (RE=1) */
#define HSCROLLEN	0x10

/** Function set (RE=0) */
#define FUNCSET		0x20
#define IF_8BIT		0x10
#define IF_4BIT		0x00
#define TWOLINE		0x08
#define ONELINE		0x00
#define LARGECHAR	0x04	/**< 5x11 characters */
#define SMALLCHAR	0x00	/**< 5x8 characters */
#define EXTREG		0x04	/**< Select ext. registers (Yes, the same bits) */
#define SEGBLINK	0x02	/**< CGRAM/SEGRAM blink, only if RE=1 */

/** Set CGRAM address (RE=0) */
#define SETCHAR		0x40

/** Set SEGRAM address (RE=1) */
#define SETSEG		0x40

/** Set DDRAM address (RE=0) */
#define POSITION	0x80

/** Set scroll quantity (RE=1) */
#define HSCROLLAMOUNT	0x80

/** Default Lineaddress in 4 line mode (RE=1) */
#define LADDR		0x20

/**@}*/
#endif
