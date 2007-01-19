#ifndef HD44780_SERIAL_H
#define HD44780_SERIAL_H

#include "lcd.h"					  /* for Driver */

#define SERIALIF_NAME_LENGTH 20

#define DEFAULT_DEVICE       "/dev/lcd"

typedef struct SerialInterface {
	char         name[SERIALIF_NAME_LENGTH];
	char         instruction_escape;
	char         data_escape;
	char         data_escape_min; /* escaped data lower limit */
	char         data_escape_max; /* escaped data upper limit */
	unsigned int default_bitrate;
	char         if_bits;
	char         keypad;
	char         keypad_escape;
	char         backlight;
	char         backlight_escape;
} SerialInterface;

/* List of connectiontypes managed by this driver, if you change
   something here, remember also to change hd44780-drivers.h */
static const SerialInterface serial_interfaces[] = {
	/*    name         instr data     v     ^ bitrate bits  K   esc  B  esc */
	{ "picanlcd",      0x11, 0x12, 0x00, 0x20,   9600,   8, 0, 0x00, 0,   0 },
	{ "lcdserializer", 0xFE,    0, 0x00, 0x00,   9600,   8, 0, 0x00, 0,   0 },
	{ "los-panel",     0xFE,    0, 0x00, 0x00,   9600,   4, 1, 0xFE, 1,   0 },
	{ "vdr-lcd",       0xFE,    0, 0x00, 0x00,   9600,   4, 0, 0x00, 0,   0 }
	{ "vdr-wakeup",    0xC0, 0xC4, 0xC0, 0xD0,   9600,   4, 0, 0x00, 0,   0 }
};

/* initialize this particular driver */
int hd_init_serial (Driver *drvthis);

#endif
