#ifndef HD44780_SERIAL_H
#define HD44780_SERIAL_H

#include "lcd.h"					  /* for Driver */
#include "hd44780-low.h"

#define SERIALIF_NAME_LENGTH 20

#define DEFAULT_DEVICE       "/dev/lcd"

typedef struct SerialInterface {
	int          connectiontype;
	char         instruction_escape;
	char         data_escape;
	char         data_escape_min; /* escaped data lower limit */
	char         data_escape_max; /* escaped data upper limit */
	unsigned int default_bitrate;
	char         if_bits;
	char         keypad;
	char         keypad_escape;
	char         backlight;
        /*
          if both escape and on/off codes are set means that display
          can't set brightness, but can only switch light on or off and
          needs both escape and on/off bytes (see pertelian,
          http://pertelian.com/index.php?option=com_content&task=view&id=27&Itemid=33)
        */
	char         backlight_escape; /* leave to 0 is the interface uses on/off codes   */
	char         backlight_off;
	char         backlight_on;     /* leave these two to 0 is backlight_escape is set */
	char         multiple_displays;
	char         end_code;         /* code to send on shutdown */
} SerialInterface;

/* List of connectiontypes managed by this driver, if you change
   something here, remember also to change hd44780-drivers.h */
static const SerialInterface serial_interfaces[] = {
	/*    type         instr data     v     ^ bitrate bits  K   esc  B  Besc  Boff   Bon Multi  End */
	{ HD44780_CT_PICANLCD,      0x11, 0x12, 0x00, 0x20,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LCDSERIALIZER, 0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_LOS_PANEL,     0xFE,    0, 0x00, 0x00,   9600,   4, 1, 0xFE, 1, 0xFF,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_LCD,       0xFE,    0, 0x00, 0x00,   9600,   4, 0, 0x00, 0,    0,    0,    0,   0,    0 },
	{ HD44780_CT_VDR_WAKEUP,    0xC0, 0xC4, 0xC0, 0xD0,   9600,   4, 0, 0x00, 1,    0, 0xC9, 0xC8,   1, 0xCF },
	{ HD44780_CT_PERTELIAN,     0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 1, 0xFE, 0x02, 0x03,   0,    0 },
	{ HD44780_CT_UNKNOWN, 0x00, 0, 0x00, 0x00, 0, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0, 0 }
};

/* initialize this particular driver */
int hd_init_serial(Driver *drvthis);

#endif
