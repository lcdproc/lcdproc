/** \file server/drivers/hd44780-serial.h
 * Contains configuration data for serial attached HD44780 displays supported
 * by the \c hd44780 driver.
 */

#ifndef HD44780_SERIAL_H
#define HD44780_SERIAL_H

#define SERIALIF_NAME_LENGTH 20
#define DEFAULT_DEVICE       "/dev/lcd"

/** Declares one configuration enty in the serial_interfaces table */
struct hd44780_SerialInterface {
	int          connectiontype;	/**< Connection type from hd44780 config */
	/** Command escape character. This is always sent, even if 0x00 */
	char         instruction_escape;
	/** Data escape character. Only sent if not NUL data is within range
	 * configure by data_escape_min and data_escape_max. */
	char         data_escape;
	char         data_escape_min;	/**< Escaped data lower limit (inclusive) */
	char         data_escape_max;	/**< Escaped data upper limit (exclusive) */
	unsigned int default_bitrate;	/**< Bitrate device is set to by default */
	char         if_bits;		/**< Initialize to 8 or 4 bit interface */
	char         keypad;		/**< Flag: keypad available */
	char         keypad_escape;	/**< Keys are escaped with this character */
	char         backlight;		/**< Flag: backlight available */
	/** Escape character to send to indicate a backlight state change */
	char         backlight_escape;
	char         backlight_off;	/**< Character sent to set display off */
	char         backlight_on;	/**< Character sent to set display on */
	/** Flag: Device has multiple controllers. If enabled, the displayID
	 * is added to data escape */
	char         multiple_displays;
	char         end_code;         /**< Code to send on shutdown */
};

/**
 * List of connectiontypes managed by this driver. If you change something
 * here, remember also to change hd44780-drivers.h as well.
 */
static const struct hd44780_SerialInterface serial_interfaces[] = {
	/*    type                  instr data     v     ^ bitrate bits  K   esc  B  Besc  Boff   Bon Multi  End */
	{ HD44780_CT_PICANLCD,      0x11, 0x12, 0x00, 0x20,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LCDSERIALIZER, 0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LOS_PANEL,     0xFE,    0, 0x00, 0x00,   9600,   4, 1, 0xFE, 1, 0xFD,    0, 0xFF,   0,    0 },
	{ HD44780_CT_VDR_LCD,       0xFE,    0, 0x00, 0x00,   9600,   4, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_WAKEUP,    0xC0, 0xC4, 0xC0, 0xD0,   9600,   4, 0, 0x00, 1,    0, 0xC9, 0xC8,   1, 0xCF },
	{ HD44780_CT_PERTELIAN,     0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 1, 0xFE, 0x02, 0x03,   0,    0 },
	{ HD44780_CT_UNKNOWN, 0x00, 0, 0x00, 0x00, 0, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0, 0 }
};

/* initialize this particular driver */
int hd_init_serial(Driver *drvthis);

#endif
