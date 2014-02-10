/** \file server/drivers/hd44780-serial.h
 * Contains configuration data for serial attached HD44780 displays supported
 * by the \c hd44780 driver.
 */

#ifndef HD44780_SERIAL_H
#define HD44780_SERIAL_H

#define SERIALIF_NAME_LENGTH 20
#define DEFAULT_DEVICE       "/dev/lcd"

/** Declares one configuration entry in the serial_interfaces table */
struct hd44780_SerialInterface {
	int           connectiontype;	/**< Connection type from hd44780 config */

	/** \name Instruction / data escape sequence
	 * Determines if escape characters have to be sent for instruction or
	 * data values. The instruction escape character is always sent, even
	 * if its value is 0x00. The data escape character is only sent it it
	 * is not NUL and data is within range set by data_escape_min and
	 * data_escape_max.
	 *@{*/
	unsigned char instruction_escape;	/**< Instruction escape character. */
	unsigned char data_escape;	/**< Data escape character. */
	unsigned char data_escape_min;	/**< Escaped data lower limit (inclusive) */
	unsigned char data_escape_max;	/**< Escaped data upper limit (inclusive) */
	/**@}*/

	unsigned int  default_bitrate;	/**< Bitrate device is set to by default */
	char          if_bits;		/**< Initialize to 8 or 4 bit interface */

	/** \name Keypad settings
	 *@{*/
	char          keypad;		/**< Flag: keypad available */
	unsigned char keypad_escape;	/**< Keys are escaped with this character */
	/**@}*/

	/** \name Backlight options
	 * The backlight flag determines the type of backlight available.
	 * If the backlight is just switchable, the backlight_off or
	 * backlight_on characters are sent according to backlight state.
	 * If the backlight is adjustable, these characters must define a range
	 * of values that are understood as different brightness levels by
	 * the display. The 'brightness' and 'offbrightness' values from config
	 * are used according to the backlight state and mapped to this range.
	 *@{*/
	char          backlight;	/**< Flag: backlight available
					 * 0 = none, 1 = switchable, 2 = adjustable */
	unsigned char backlight_escape;	/**< Escape character to send to indicate
					 * a backlight state change */
	unsigned char backlight_off;	/**< Character sent to set display off
					 * or minimum value if adjustable */
	unsigned char backlight_on;	/**< Character sent to set display on
					 * or maximum value if adjustable */
	/**@}*/

	/** Flag: Device has multiple controllers. If enabled, the displayID
	 * is added to data_escape and it is always sent. */
	char          multiple_displays;
	unsigned char end_code;         /**< Code to send on shutdown */
};

/**
 * List of connectiontypes managed by this driver. If you change something
 * here, remember also to change hd44780-drivers.h as well.
 */
static const struct hd44780_SerialInterface serial_interfaces[] = {
	/*    type                  instr data     v     ^ bitrate bits  K   esc  B  Besc  Boff   Bon Multi  End */
	{ HD44780_CT_PICANLCD,      0x11, 0x12, 0x00, 0x1F,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LCDSERIALIZER, 0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LOS_PANEL,     0xFE,    0, 0x00, 0x00,   9600,   4, 1, 0xFE, 1, 0xFD,    0, 0xFF,   0,    0 },
	{ HD44780_CT_VDR_LCD,       0xFE,    0, 0x00, 0x00,   9600,   4, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_WAKEUP,    0xC0, 0xC4, 0xC0, 0xCF,   9600,   4, 0, 0x00, 1,    0, 0xC9, 0xC8,   1, 0xCF },
	{ HD44780_CT_PERTELIAN,     0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 1, 0xFE, 0x02, 0x03,   0,    0 },
	{ HD44780_CT_UNKNOWN, 0x00, 0, 0x00, 0x00, 0, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0, 0 }
};

/* initialize this particular driver */
int hd_init_serial(Driver *drvthis);

#endif
