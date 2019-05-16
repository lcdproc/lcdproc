// clang-format off


// clang-format on
/**
 * @file
 *
 * This file was automatically generated using `kdb gen elektra`.
 * Any changes will be overwritten, when the file is regenerated.
 *
 * @copyright BSD Zero Clause License
 *
 *     Copyright (C) 2019 Elektra Initiative (https://libelektra.org)
 *
 *     Permission to use, copy, modify, and/or distribute this software for any
 *     purpose with or without fee is hereby granted.
 *
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 *     REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *     FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 *     INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 *     LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 *     OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef ELEKTRAGEN_H
#define ELEKTRAGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <elektra.h>

#include <kdbhelper.h>
#include <string.h>



#define ELEKTRA_CONTEXT_SET(contextTag) elektraSetContextualValue##contextTag

// clang-format off

// clang-format on

typedef enum
{
	C_FONTZ_PACKET_MODEL_533 = 0,
	C_FONTZ_PACKET_MODEL_631 = 1,
	C_FONTZ_PACKET_MODEL_633 = 2,
	C_FONTZ_PACKET_MODEL_635 = 3,
} CFontzPacketModel;

typedef enum
{
	CURSES_COLOR_RED = 0,
	CURSES_COLOR_BLACK = 1,
	CURSES_COLOR_GREEN = 2,
	CURSES_COLOR_YELLOW = 3,
	CURSES_COLOR_BLUE = 4,
	CURSES_COLOR_MAGENTA = 5,
	CURSES_COLOR_CYAN = 6,
	CURSES_COLOR_WHITE = 7,
} CursesColor;

typedef enum
{
	CW_LNX_MODEL_12232 = 0,
	CW_LNX_MODEL_12832 = 1,
	CW_LNX_MODEL_1602 = 2,
} CwLnxModel;

typedef enum
{
	GLCD_CONNECTION_TYPE_T6963 = 0,
	GLCD_CONNECTION_TYPE_PNG = 1,
	GLCD_CONNECTION_TYPE_SERDISPLIB = 2,
	GLCD_CONNECTION_TYPE_GLCD2USB = 3,
	GLCD_CONNECTION_TYPE_X11 = 4,
	GLCD_CONNECTION_TYPE_PICOLCDGFX = 5,
	GLCD_CONNECTION_TYPE_XYZ = 6,
} GlcdConnectionType;

typedef enum
{
	H_D44780_BACKLIGHT_NONE = 0,
	H_D44780_BACKLIGHT_EXTERNAL = 1,
	H_D44780_BACKLIGHT_INTERNAL = 2,
	H_D44780_BACKLIGHT_INTERNAL_CMDS = 3,
} HD44780Backlight;

typedef enum
{
	H_D44780_CHARMAP_HD44780_DEFAULT = 0,
	H_D44780_CHARMAP_HD44780_EURO = 1,
	H_D44780_CHARMAP_EA_KS0073 = 2,
	H_D44780_CHARMAP_SED1278F_0B = 3,
	H_D44780_CHARMAP_HD44780_KOI8_R = 4,
	H_D44780_CHARMAP_HD44780_CP1251 = 5,
	H_D44780_CHARMAP_HD44780_8859_5 = 6,
	H_D44780_CHARMAP_UPD16314 = 7,
} HD44780Charmap;

typedef enum
{
	H_D44780_CONNECTION_TYPE_4BIT = 0,
	H_D44780_CONNECTION_TYPE_8BIT = 1,
	H_D44780_CONNECTION_TYPE_WINAMP = 2,
	H_D44780_CONNECTION_TYPE_LCM162 = 3,
	H_D44780_CONNECTION_TYPE_SERIAL_LPT = 4,
	H_D44780_CONNECTION_TYPE_PICANLCD = 5,
	H_D44780_CONNECTION_TYPE_LCDSERIALIZER = 6,
	H_D44780_CONNECTION_TYPE_LOS_PANEL = 7,
	H_D44780_CONNECTION_TYPE_VDR_LCD = 8,
	H_D44780_CONNECTION_TYPE_VDR_WAKEUP = 9,
	H_D44780_CONNECTION_TYPE_EZIO = 10,
	H_D44780_CONNECTION_TYPE_PERTELIAN = 11,
	H_D44780_CONNECTION_TYPE_LIS2 = 12,
	H_D44780_CONNECTION_TYPE_MPLAY = 13,
	H_D44780_CONNECTION_TYPE_USBLCD = 14,
	H_D44780_CONNECTION_TYPE_BWCTUSB = 15,
	H_D44780_CONNECTION_TYPE_LCD2USB = 16,
	H_D44780_CONNECTION_TYPE_USBTINY = 17,
	H_D44780_CONNECTION_TYPE_USS720 = 18,
	H_D44780_CONNECTION_TYPE_U_S_B_4_ALL = 19,
	H_D44780_CONNECTION_TYPE_FTDI = 20,
	H_D44780_CONNECTION_TYPE_I2C = 21,
	H_D44780_CONNECTION_TYPE_PIPLATE = 22,
	H_D44780_CONNECTION_TYPE_SPI = 23,
	H_D44780_CONNECTION_TYPE_PIFACECAD = 24,
	H_D44780_CONNECTION_TYPE_ETHLCD = 25,
	H_D44780_CONNECTION_TYPE_RASPBERRYPI = 26,
	H_D44780_CONNECTION_TYPE_GPIO = 27,
} HD44780ConnectionType;

typedef enum
{
	ELEKTRA_ENUM_HD44780_MODEL_STANDARD = 0,
	ELEKTRA_ENUM_HD44780_MODEL_EXTENDED = 1,
	ELEKTRA_ENUM_HD44780_MODEL_WINSTAR_OLED = 2,
	ELEKTRA_ENUM_HD44780_MODEL_PT6314_VFD = 3,
} ElektraEnumHd44780Model;

typedef enum
{
	I_MON_CHARMAP_HD44780_EURO = 0,
	I_MON_CHARMAP_UPD16314 = 1,
	I_MON_CHARMAP_HD44780_KOI8_R = 2,
	I_MON_CHARMAP_HD44780_CP1251 = 3,
	I_MON_CHARMAP_HD44780_8859_5 = 4,
	I_MON_CHARMAP_NONE = 5,
} IMonCharmap;

typedef enum
{
	I_MON_L_C_D_DISC_MODE_0 = 0,
	I_MON_L_C_D_DISC_MODE_1 = 1,
} IMonLCDDiscMode;

typedef enum
{
	LCDM001_KEYS_LEFT_KEY = 0,
	LCDM001_KEYS_RIGHT_KEY = 1,
	LCDM001_KEYS_UP_KEY = 2,
	LCDM001_KEYS_DOWN_KEY = 3,
} Lcdm001Keys;

typedef enum
{
	ELEKTRA_ENUM_MDM166A_CLOCK_NO = 0,
	ELEKTRA_ENUM_MDM166A_CLOCK_SMALL = 1,
	ELEKTRA_ENUM_MDM166A_CLOCK_BIG = 2,
} ElektraEnumMdm166aClock;

typedef enum
{
	ELEKTRA_ENUM_MTXORB_TYPE_LCD = 0,
	ELEKTRA_ENUM_MTXORB_TYPE_LKD = 1,
	ELEKTRA_ENUM_MTXORB_TYPE_VFD = 2,
	ELEKTRA_ENUM_MTXORB_TYPE_VKD = 3,
} ElektraEnumMtxorbType;

typedef enum
{
	ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_CLASSIC = 0,
	ELEKTRA_ENUM_SED1330_CONNECTIONTYPE_BITSHAKER = 1,
} ElektraEnumSed1330Connectiontype;

typedef enum
{
	ELEKTRA_ENUM_SED1330_TYPE_G321_D = 0,
	ELEKTRA_ENUM_SED1330_TYPE_G121_C = 1,
	ELEKTRA_ENUM_SED1330_TYPE_G242_C = 2,
	ELEKTRA_ENUM_SED1330_TYPE_G191_D = 3,
	ELEKTRA_ENUM_SED1330_TYPE_G2446 = 4,
	ELEKTRA_ENUM_SED1330_TYPE_S_P14_Q002 = 5,
} ElektraEnumSed1330Type;

typedef enum
{
	ELEKTRA_ENUM_SERIALPOS_TYPE_A_E_D_E_X = 0,
	ELEKTRA_ENUM_SERIALPOS_TYPE_C_D5220 = 1,
	ELEKTRA_ENUM_SERIALPOS_TYPE_EPSON = 2,
	ELEKTRA_ENUM_SERIALPOS_TYPE_EMAX = 3,
	ELEKTRA_ENUM_SERIALPOS_TYPE_LOGIC_CONTROLS = 4,
	ELEKTRA_ENUM_SERIALPOS_TYPE_ULTIMATE = 5,
} ElektraEnumSerialposType;

typedef enum
{
	ELEKTRA_ENUM_SERVER_BACKLIGHT_OFF = 0,
	ELEKTRA_ENUM_SERVER_BACKLIGHT_ON = 1,
	ELEKTRA_ENUM_SERVER_BACKLIGHT_OPEN = 2,
} ElektraEnumServerBacklight;

typedef enum
{
	ELEKTRA_ENUM_SERVER_HEARTBEAT_OFF = 0,
	ELEKTRA_ENUM_SERVER_HEARTBEAT_ON = 1,
	ELEKTRA_ENUM_SERVER_HEARTBEAT_OPEN = 2,
} ElektraEnumServerHeartbeat;

typedef enum
{
	ELEKTRA_ENUM_SERVER_SERVERSCREEN_OFF = 0,
	ELEKTRA_ENUM_SERVER_SERVERSCREEN_ON = 1,
	ELEKTRA_ENUM_SERVER_SERVERSCREEN_BLANK = 2,
} ElektraEnumServerServerscreen;


ELEKTRA_KEY_TO_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);
ELEKTRA_TO_STRING_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);

ELEKTRA_GET_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);
ELEKTRA_SET_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CFontzPacketModel, EnumCFontzPacketModel);

ELEKTRA_KEY_TO_SIGNATURE (CursesColor, EnumCursesColor);
ELEKTRA_TO_STRING_SIGNATURE (CursesColor, EnumCursesColor);

ELEKTRA_GET_SIGNATURE (CursesColor, EnumCursesColor);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CursesColor, EnumCursesColor);
ELEKTRA_SET_SIGNATURE (CursesColor, EnumCursesColor);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CursesColor, EnumCursesColor);

ELEKTRA_KEY_TO_SIGNATURE (CwLnxModel, EnumCwLnxModel);
ELEKTRA_TO_STRING_SIGNATURE (CwLnxModel, EnumCwLnxModel);

ELEKTRA_GET_SIGNATURE (CwLnxModel, EnumCwLnxModel);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (CwLnxModel, EnumCwLnxModel);
ELEKTRA_SET_SIGNATURE (CwLnxModel, EnumCwLnxModel);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (CwLnxModel, EnumCwLnxModel);

ELEKTRA_KEY_TO_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);
ELEKTRA_TO_STRING_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);

ELEKTRA_GET_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);
ELEKTRA_SET_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (GlcdConnectionType, EnumGlcdConnectionType);

ELEKTRA_KEY_TO_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);
ELEKTRA_TO_STRING_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);

ELEKTRA_GET_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);
ELEKTRA_SET_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780Backlight, EnumHD44780Backlight);

ELEKTRA_KEY_TO_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);
ELEKTRA_TO_STRING_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);

ELEKTRA_GET_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);
ELEKTRA_SET_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780Charmap, EnumHD44780Charmap);

ELEKTRA_KEY_TO_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);
ELEKTRA_TO_STRING_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);

ELEKTRA_GET_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);
ELEKTRA_SET_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (HD44780ConnectionType, EnumHD44780ConnectionType);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);

ELEKTRA_GET_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);
ELEKTRA_SET_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumHd44780Model, EnumHd44780Model);

ELEKTRA_KEY_TO_SIGNATURE (IMonCharmap, EnumIMonCharmap);
ELEKTRA_TO_STRING_SIGNATURE (IMonCharmap, EnumIMonCharmap);

ELEKTRA_GET_SIGNATURE (IMonCharmap, EnumIMonCharmap);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (IMonCharmap, EnumIMonCharmap);
ELEKTRA_SET_SIGNATURE (IMonCharmap, EnumIMonCharmap);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (IMonCharmap, EnumIMonCharmap);

ELEKTRA_KEY_TO_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);
ELEKTRA_TO_STRING_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);

ELEKTRA_GET_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);
ELEKTRA_SET_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (IMonLCDDiscMode, EnumIMonLCDDiscMode);

ELEKTRA_KEY_TO_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);
ELEKTRA_TO_STRING_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);

ELEKTRA_GET_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);
ELEKTRA_SET_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (Lcdm001Keys, EnumLcdm001Keys);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);

ELEKTRA_GET_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);
ELEKTRA_SET_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMdm166aClock, EnumMdm166aClock);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);

ELEKTRA_GET_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);
ELEKTRA_SET_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumMtxorbType, EnumMtxorbType);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);

ELEKTRA_GET_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);
ELEKTRA_SET_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Connectiontype, EnumSed1330Connectiontype);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);

ELEKTRA_GET_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);
ELEKTRA_SET_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSed1330Type, EnumSed1330Type);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);

ELEKTRA_GET_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);
ELEKTRA_SET_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumSerialposType, EnumSerialposType);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);

ELEKTRA_GET_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);
ELEKTRA_SET_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerBacklight, EnumServerBacklight);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);

ELEKTRA_GET_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);
ELEKTRA_SET_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerHeartbeat, EnumServerHeartbeat);

ELEKTRA_KEY_TO_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);
ELEKTRA_TO_STRING_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);

ELEKTRA_GET_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);
ELEKTRA_GET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);
ELEKTRA_SET_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (ElektraEnumServerServerscreen, EnumServerServerscreen);



// clang-format off

// clang-format on

#define ELEKTRA_STRUCT_FREE(typeName) ELEKTRA_CONCAT (elektraFree, typeName)
#define ELEKTRA_STRUCT_FREE_SIGNATURE(cType, typeName) void ELEKTRA_STRUCT_FREE (typeName) (cType * ptr)

typedef struct BayradDriverConfig
{
	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  speed;		

} BayradDriverConfig;

typedef struct CFontzDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_boolean_t  newfirmware;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 kdb_boolean_t  usb;		

} CFontzDriverConfig;

typedef struct CFontzPacketDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 CFontzPacketModel  model;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  oldfirmware;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 kdb_boolean_t  usb;		

} CFontzPacketDriverConfig;

typedef struct CursesDriverConfig
{
	
	 CursesColor  background;		

	
	 CursesColor  backlight;		

	
	 kdb_boolean_t  drawborder;		

	
	 const char *  file;		

	
	 CursesColor  foreground;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  topleftx;		

	
	 kdb_unsigned_short_t  toplefty;		

	
	 kdb_boolean_t  useacs;		

} CursesDriverConfig;

typedef struct CwLnxDriverConfig
{
	
	 const char *  device;		

	
	 const char *  file;		

	
	 const char *  keymapA;		

	
	 const char *  keymapB;		

	
	 const char *  keymapC;		

	
	 const char *  keymapD;		

	
	 const char *  keymapE;		

	
	 const char *  keymapF;		

	
	 kdb_boolean_t  keypad;		

	
	 kdb_boolean_t  keypadTestMode;		

	
	 CwLnxModel  model;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

} CwLnxDriverConfig;

typedef struct Ea65DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} Ea65DriverConfig;

typedef struct EyeboxOneDriverConfig
{
	
	 kdb_boolean_t  backlight;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  cursor;		

	
	 const char *  device;		

	
	 const char *  downkey;		

	
	 const char *  escapekey;		

	
	 const char *  file;		

	
	 kdb_boolean_t  keypadTestMode;		

	
	 const char *  leftkey;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  rightkey;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 const char *  upkey;		

} EyeboxOneDriverConfig;

typedef struct FutabaDriverConfig
{
	
	 const char *  file;		

} FutabaDriverConfig;

typedef struct G15DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} G15DriverConfig;

typedef struct GlcdDriverConfig
{
	
	 kdb_boolean_t  bidirectional;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 const char *  cellsize;		

	
	 GlcdConnectionType  connectiontype;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  delaybus;		

	
	 const char *  file;		

	
	 kdb_boolean_t  fonthasicons;		

	
	 const char *  keymapA;		

	
	 const char *  keymapB;		

	
	 const char *  keymapC;		

	
	 const char *  keymapD;		

	
	 const char *  keymapE;		

	
	 const char *  keymapF;		

	
	 kdb_unsigned_short_t  keyrepeatdelay;		

	
	 kdb_unsigned_short_t  keyrepeatinterval;		

	
	 const char *  normalFont;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  picolcdgfxInverted;		

	
	 const char *  picolcdgfxKeytimeout;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  serdispDevice;		

	
	 const char *  serdispName;		

	
	 const char *  serdispOptions;		

	
	 const char *  size;		

	
	 kdb_boolean_t  useft2;		

	
	 const char *  x11Backlightcolor;		

	
	 kdb_unsigned_long_t  x11Border;		

	
	 kdb_boolean_t  x11Inverted;		

	
	 const char *  x11Pixelcolor;		

	
	 const char *  x11Pixelsize;		

} GlcdDriverConfig;

typedef struct GlcdlibDriverConfig
{
	
	 const char *  CharEncoding;		

	
	 kdb_boolean_t  backlight;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  driver;		

	
	 const char *  file;		

	
	 const char *  fontfile;		

	
	 kdb_boolean_t  invert;		

	
	 const char *  minfontfacesize;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_short_t  pixelshiftx;		

	
	 kdb_short_t  pixelshifty;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_boolean_t  showbigborder;		

	
	 kdb_boolean_t  showdebugframe;		

	
	 kdb_boolean_t  showthinborder;		

	
	 const char *  textresolution;		

	
	 kdb_boolean_t  upsidedown;		

	
	 kdb_boolean_t  useft2;		

} GlcdlibDriverConfig;

typedef struct GlkDriverConfig
{
	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  speed;		

} GlkDriverConfig;

typedef struct Hd44780DriverConfig
{
	
	 HD44780Backlight  backlight;		

	
	 const char *  backlightcmdoff;		

	
	 const char *  backlightcmdon;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 HD44780Charmap  charmap;		

	
	 HD44780ConnectionType  connectiontype;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  delaybus;		

	
	 kdb_unsigned_short_t  delaymult;		

	
	 const char *  device;		

	
	 kdb_boolean_t  extendedmode;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  fontbank;		

	
	 kdb_unsigned_short_t  keepalivedisplay;		

	
	 const char *  keymatrix41;		

	
	 const char *  keymatrix42;		

	
	 const char *  keymatrix43;		

	
	 const char *  keymatrix44;		

	
	 kdb_boolean_t  keypad;		

	
	 kdb_boolean_t  lastline;		

	
	 const char *  lineaddress;		

	
	 ElektraEnumHd44780Model  model;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  outputport;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_unsigned_short_t  refreshdisplay;		

	
	 const char *  size;		

	
	 kdb_unsigned_long_t  speed;		

	
	 const char *  vspan;		

} Hd44780DriverConfig;

typedef struct Icp_a106DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} Icp_a106DriverConfig;

typedef struct ImonDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 IMonCharmap  charmap;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} ImonDriverConfig;

typedef struct ImonlcdDriverConfig
{
	
	 kdb_boolean_t  backlight;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 IMonLCDDiscMode  discmode;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_unsigned_short_t  onexit;		

	
	 const char *  protocol;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} ImonlcdDriverConfig;

typedef struct IOWarriorDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  extendedmode;		

	
	 const char *  file;		

	
	 kdb_boolean_t  lastline;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  serialnumber;		

	
	 const char *  size;		

} IOWarriorDriverConfig;

typedef struct IrManDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 const char *  config;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} IrManDriverConfig;

typedef struct IrtransDriverConfig
{
	
	 kdb_boolean_t  backlight;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 const char *  hostname;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} IrtransDriverConfig;

typedef struct JoyDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 const char *  mapAxis1neg;		

	
	 const char *  mapAxis1pos;		

	
	 const char *  mapAxis2neg;		

	
	 const char *  mapAxis2pos;		

	
	 const char *  mapButton1;		

	
	 const char *  mapButton2;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} JoyDriverConfig;

typedef struct Lb216DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_unsigned_short_t  speed;		

} Lb216DriverConfig;

typedef struct Lcdm001DriverConfig
{
	
	 Lcdm001Keys  backkey;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 Lcdm001Keys  forwardkey;		

	
	 Lcdm001Keys  mainmenukey;		

	
	 Lcdm001Keys  pausekey;		

} Lcdm001DriverConfig;

typedef struct LctermDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} LctermDriverConfig;

typedef struct Linux_inputDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} Linux_inputDriverConfig;

typedef struct LircDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 const char *  lircrc;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  prog;		

	
	 kdb_boolean_t  reboot;		

} LircDriverConfig;

typedef struct LisDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_boolean_t  lastline;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  productid;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 const char *  vendorid;		

} LisDriverConfig;

typedef struct MD8800DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} MD8800DriverConfig;

typedef struct Mdm166aDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 ElektraEnumMdm166aClock  clock;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  dimming;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  offdimming;		

	
	 kdb_boolean_t  reboot;		

} Mdm166aDriverConfig;

typedef struct Ms6931DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} Ms6931DriverConfig;

typedef struct Mtc_s16209xDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} Mtc_s16209xDriverConfig;

typedef struct MtxOrbDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_boolean_t  hasAdjustableBacklight;		

	
	 const char *  keymapA;		

	
	 const char *  keymapB;		

	
	 const char *  keymapC;		

	
	 const char *  keymapD;		

	
	 const char *  keymapE;		

	
	 const char *  keymapF;		

	
	 kdb_boolean_t  keypadTestMode;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 ElektraEnumMtxorbType  type;		

} MtxOrbDriverConfig;

typedef struct Mx5000DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_unsigned_long_t  waitafterrefresh;		

} Mx5000DriverConfig;

typedef struct NoritakeVFDDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_unsigned_long_t  parity;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_long_t  speed;		

} NoritakeVFDDriverConfig;

typedef struct Olimex_MOD_LCD1x9DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} Olimex_MOD_LCD1x9DriverConfig;

typedef struct PicolcdDriverConfig
{
	
	 kdb_boolean_t  backlight;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_boolean_t  key0light;		

	
	 kdb_boolean_t  key1light;		

	
	 kdb_boolean_t  key2light;		

	
	 kdb_boolean_t  key3light;		

	
	 kdb_boolean_t  key4light;		

	
	 kdb_boolean_t  key5light;		

	
	 kdb_boolean_t  keylights;		

	
	 kdb_unsigned_short_t  keyrepeatdelay;		

	
	 kdb_unsigned_short_t  keyrepeatinterval;		

	
	 kdb_unsigned_short_t  keytimeout;		

	
	 kdb_boolean_t  linklights;		

	
	 kdb_unsigned_long_t  lircflushthreshold;		

	
	 const char *  lirchost;		

	
	 kdb_unsigned_short_t  lircport;		

	
	 kdb_boolean_t  lirctimeUs;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} PicolcdDriverConfig;

typedef struct PyramidDriverConfig
{
	
	 const char *  device;		

	
	 const char *  file;		

} PyramidDriverConfig;

typedef struct RawserialDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_long_t  speed;		

	
	 kdb_float_t  updaterate;		

} RawserialDriverConfig;

typedef struct Sed1330DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 const char *  cellsize;		

	
	 ElektraEnumSed1330Connectiontype  connectiontype;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

	
	 ElektraEnumSed1330Type  type;		

} Sed1330DriverConfig;

typedef struct Sed1520DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_unsigned_short_t  delaymult;		

	
	 const char *  file;		

	
	 kdb_boolean_t  haveinverter;		

	
	 kdb_unsigned_short_t  interfacetype;		

	
	 kdb_boolean_t  invertedmapping;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_boolean_t  usehardreset;		

} Sed1520DriverConfig;

typedef struct SerialPOSDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 const char *  cellsize;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_unsigned_long_t  customChars;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 ElektraEnumSerialposType  type;		

} SerialPOSDriverConfig;

typedef struct SerialVFDDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_unsigned_long_t  custom_characters;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_boolean_t  iso88591;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  port;		

	
	 kdb_unsigned_short_t  portwait;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

	
	 kdb_unsigned_short_t  type;		

	
	 kdb_boolean_t  useParallel;		

} SerialVFDDriverConfig;

typedef struct SliDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 kdb_unsigned_short_t  speed;		

} SliDriverConfig;

typedef struct Stv5730DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

} Stv5730DriverConfig;

typedef struct SureElecDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 kdb_unsigned_short_t  edition;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} SureElecDriverConfig;

typedef struct SvgaDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 const char *  mode;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} SvgaDriverConfig;

typedef struct T6963DriverConfig
{
	
	 kdb_boolean_t  bidirectional;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_boolean_t  cleargraphic;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 kdb_boolean_t  delaybus;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  port;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} T6963DriverConfig;

typedef struct TextDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} TextDriverConfig;

typedef struct TyanDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

	
	 kdb_unsigned_short_t  speed;		

} TyanDriverConfig;

typedef struct Ula200DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 const char *  keymapA;		

	
	 const char *  keymapB;		

	
	 const char *  keymapC;		

	
	 const char *  keymapD;		

	
	 const char *  keymapE;		

	
	 const char *  keymapF;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} Ula200DriverConfig;

typedef struct Vlsys_m428DriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  device;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

} Vlsys_m428DriverConfig;

typedef struct XosdDriverConfig
{
	
	 const char *  Font;		

	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 const char *  offset;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} XosdDriverConfig;

typedef struct Yard2LCDDriverConfig
{
	
	 kdb_unsigned_short_t  brightness;		

	
	 kdb_unsigned_short_t  contrast;		

	
	 const char *  file;		

	
	 kdb_unsigned_short_t  offbrightness;		

	
	 kdb_boolean_t  reboot;		

	
	 const char *  size;		

} Yard2LCDDriverConfig;



ELEKTRA_GET_OUT_PTR_SIGNATURE (BayradDriverConfig, StructBayradDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (BayradDriverConfig, StructBayradDriverConfig);
ELEKTRA_SET_SIGNATURE (const BayradDriverConfig *, StructBayradDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const BayradDriverConfig *, StructBayradDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (CFontzDriverConfig, StructCFontzDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CFontzDriverConfig, StructCFontzDriverConfig);
ELEKTRA_SET_SIGNATURE (const CFontzDriverConfig *, StructCFontzDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CFontzDriverConfig *, StructCFontzDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (CFontzPacketDriverConfig, StructCFontzPacketDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CFontzPacketDriverConfig, StructCFontzPacketDriverConfig);
ELEKTRA_SET_SIGNATURE (const CFontzPacketDriverConfig *, StructCFontzPacketDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CFontzPacketDriverConfig *, StructCFontzPacketDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (CursesDriverConfig, StructCursesDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CursesDriverConfig, StructCursesDriverConfig);
ELEKTRA_SET_SIGNATURE (const CursesDriverConfig *, StructCursesDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CursesDriverConfig *, StructCursesDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (CwLnxDriverConfig, StructCwLnxDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (CwLnxDriverConfig, StructCwLnxDriverConfig);
ELEKTRA_SET_SIGNATURE (const CwLnxDriverConfig *, StructCwLnxDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const CwLnxDriverConfig *, StructCwLnxDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Ea65DriverConfig, StructEa65DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ea65DriverConfig, StructEa65DriverConfig);
ELEKTRA_SET_SIGNATURE (const Ea65DriverConfig *, StructEa65DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ea65DriverConfig *, StructEa65DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (EyeboxOneDriverConfig, StructEyeboxOneDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (EyeboxOneDriverConfig, StructEyeboxOneDriverConfig);
ELEKTRA_SET_SIGNATURE (const EyeboxOneDriverConfig *, StructEyeboxOneDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const EyeboxOneDriverConfig *, StructEyeboxOneDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (FutabaDriverConfig, StructFutabaDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (FutabaDriverConfig, StructFutabaDriverConfig);
ELEKTRA_SET_SIGNATURE (const FutabaDriverConfig *, StructFutabaDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const FutabaDriverConfig *, StructFutabaDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (G15DriverConfig, StructG15DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (G15DriverConfig, StructG15DriverConfig);
ELEKTRA_SET_SIGNATURE (const G15DriverConfig *, StructG15DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const G15DriverConfig *, StructG15DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (GlcdDriverConfig, StructGlcdDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlcdDriverConfig, StructGlcdDriverConfig);
ELEKTRA_SET_SIGNATURE (const GlcdDriverConfig *, StructGlcdDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlcdDriverConfig *, StructGlcdDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (GlcdlibDriverConfig, StructGlcdlibDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlcdlibDriverConfig, StructGlcdlibDriverConfig);
ELEKTRA_SET_SIGNATURE (const GlcdlibDriverConfig *, StructGlcdlibDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlcdlibDriverConfig *, StructGlcdlibDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (GlkDriverConfig, StructGlkDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (GlkDriverConfig, StructGlkDriverConfig);
ELEKTRA_SET_SIGNATURE (const GlkDriverConfig *, StructGlkDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const GlkDriverConfig *, StructGlkDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Hd44780DriverConfig, StructHd44780DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Hd44780DriverConfig, StructHd44780DriverConfig);
ELEKTRA_SET_SIGNATURE (const Hd44780DriverConfig *, StructHd44780DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Hd44780DriverConfig *, StructHd44780DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Icp_a106DriverConfig, StructIcp_a106DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Icp_a106DriverConfig, StructIcp_a106DriverConfig);
ELEKTRA_SET_SIGNATURE (const Icp_a106DriverConfig *, StructIcp_a106DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Icp_a106DriverConfig *, StructIcp_a106DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (ImonDriverConfig, StructImonDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (ImonDriverConfig, StructImonDriverConfig);
ELEKTRA_SET_SIGNATURE (const ImonDriverConfig *, StructImonDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const ImonDriverConfig *, StructImonDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (ImonlcdDriverConfig, StructImonlcdDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (ImonlcdDriverConfig, StructImonlcdDriverConfig);
ELEKTRA_SET_SIGNATURE (const ImonlcdDriverConfig *, StructImonlcdDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const ImonlcdDriverConfig *, StructImonlcdDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (IOWarriorDriverConfig, StructIOWarriorDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IOWarriorDriverConfig, StructIOWarriorDriverConfig);
ELEKTRA_SET_SIGNATURE (const IOWarriorDriverConfig *, StructIOWarriorDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IOWarriorDriverConfig *, StructIOWarriorDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (IrManDriverConfig, StructIrManDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IrManDriverConfig, StructIrManDriverConfig);
ELEKTRA_SET_SIGNATURE (const IrManDriverConfig *, StructIrManDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IrManDriverConfig *, StructIrManDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (IrtransDriverConfig, StructIrtransDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (IrtransDriverConfig, StructIrtransDriverConfig);
ELEKTRA_SET_SIGNATURE (const IrtransDriverConfig *, StructIrtransDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const IrtransDriverConfig *, StructIrtransDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (JoyDriverConfig, StructJoyDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (JoyDriverConfig, StructJoyDriverConfig);
ELEKTRA_SET_SIGNATURE (const JoyDriverConfig *, StructJoyDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const JoyDriverConfig *, StructJoyDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Lb216DriverConfig, StructLb216DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Lb216DriverConfig, StructLb216DriverConfig);
ELEKTRA_SET_SIGNATURE (const Lb216DriverConfig *, StructLb216DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Lb216DriverConfig *, StructLb216DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Lcdm001DriverConfig, StructLcdm001DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Lcdm001DriverConfig, StructLcdm001DriverConfig);
ELEKTRA_SET_SIGNATURE (const Lcdm001DriverConfig *, StructLcdm001DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Lcdm001DriverConfig *, StructLcdm001DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (LctermDriverConfig, StructLctermDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LctermDriverConfig, StructLctermDriverConfig);
ELEKTRA_SET_SIGNATURE (const LctermDriverConfig *, StructLctermDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LctermDriverConfig *, StructLctermDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Linux_inputDriverConfig, StructLinux_inputDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Linux_inputDriverConfig, StructLinux_inputDriverConfig);
ELEKTRA_SET_SIGNATURE (const Linux_inputDriverConfig *, StructLinux_inputDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Linux_inputDriverConfig *, StructLinux_inputDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (LircDriverConfig, StructLircDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LircDriverConfig, StructLircDriverConfig);
ELEKTRA_SET_SIGNATURE (const LircDriverConfig *, StructLircDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LircDriverConfig *, StructLircDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (LisDriverConfig, StructLisDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (LisDriverConfig, StructLisDriverConfig);
ELEKTRA_SET_SIGNATURE (const LisDriverConfig *, StructLisDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const LisDriverConfig *, StructLisDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (MD8800DriverConfig, StructMD8800DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (MD8800DriverConfig, StructMD8800DriverConfig);
ELEKTRA_SET_SIGNATURE (const MD8800DriverConfig *, StructMD8800DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const MD8800DriverConfig *, StructMD8800DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Mdm166aDriverConfig, StructMdm166aDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mdm166aDriverConfig, StructMdm166aDriverConfig);
ELEKTRA_SET_SIGNATURE (const Mdm166aDriverConfig *, StructMdm166aDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mdm166aDriverConfig *, StructMdm166aDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Ms6931DriverConfig, StructMs6931DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ms6931DriverConfig, StructMs6931DriverConfig);
ELEKTRA_SET_SIGNATURE (const Ms6931DriverConfig *, StructMs6931DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ms6931DriverConfig *, StructMs6931DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Mtc_s16209xDriverConfig, StructMtc_s16209xDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mtc_s16209xDriverConfig, StructMtc_s16209xDriverConfig);
ELEKTRA_SET_SIGNATURE (const Mtc_s16209xDriverConfig *, StructMtc_s16209xDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mtc_s16209xDriverConfig *, StructMtc_s16209xDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (MtxOrbDriverConfig, StructMtxOrbDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (MtxOrbDriverConfig, StructMtxOrbDriverConfig);
ELEKTRA_SET_SIGNATURE (const MtxOrbDriverConfig *, StructMtxOrbDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const MtxOrbDriverConfig *, StructMtxOrbDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Mx5000DriverConfig, StructMx5000DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Mx5000DriverConfig, StructMx5000DriverConfig);
ELEKTRA_SET_SIGNATURE (const Mx5000DriverConfig *, StructMx5000DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Mx5000DriverConfig *, StructMx5000DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (NoritakeVFDDriverConfig, StructNoritakeVFDDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (NoritakeVFDDriverConfig, StructNoritakeVFDDriverConfig);
ELEKTRA_SET_SIGNATURE (const NoritakeVFDDriverConfig *, StructNoritakeVFDDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const NoritakeVFDDriverConfig *, StructNoritakeVFDDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Olimex_MOD_LCD1x9DriverConfig, StructOlimex_MOD_LCD1x9DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Olimex_MOD_LCD1x9DriverConfig, StructOlimex_MOD_LCD1x9DriverConfig);
ELEKTRA_SET_SIGNATURE (const Olimex_MOD_LCD1x9DriverConfig *, StructOlimex_MOD_LCD1x9DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Olimex_MOD_LCD1x9DriverConfig *, StructOlimex_MOD_LCD1x9DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (PicolcdDriverConfig, StructPicolcdDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (PicolcdDriverConfig, StructPicolcdDriverConfig);
ELEKTRA_SET_SIGNATURE (const PicolcdDriverConfig *, StructPicolcdDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const PicolcdDriverConfig *, StructPicolcdDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (PyramidDriverConfig, StructPyramidDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (PyramidDriverConfig, StructPyramidDriverConfig);
ELEKTRA_SET_SIGNATURE (const PyramidDriverConfig *, StructPyramidDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const PyramidDriverConfig *, StructPyramidDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (RawserialDriverConfig, StructRawserialDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (RawserialDriverConfig, StructRawserialDriverConfig);
ELEKTRA_SET_SIGNATURE (const RawserialDriverConfig *, StructRawserialDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const RawserialDriverConfig *, StructRawserialDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Sed1330DriverConfig, StructSed1330DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Sed1330DriverConfig, StructSed1330DriverConfig);
ELEKTRA_SET_SIGNATURE (const Sed1330DriverConfig *, StructSed1330DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Sed1330DriverConfig *, StructSed1330DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Sed1520DriverConfig, StructSed1520DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Sed1520DriverConfig, StructSed1520DriverConfig);
ELEKTRA_SET_SIGNATURE (const Sed1520DriverConfig *, StructSed1520DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Sed1520DriverConfig *, StructSed1520DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (SerialPOSDriverConfig, StructSerialPOSDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SerialPOSDriverConfig, StructSerialPOSDriverConfig);
ELEKTRA_SET_SIGNATURE (const SerialPOSDriverConfig *, StructSerialPOSDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SerialPOSDriverConfig *, StructSerialPOSDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (SerialVFDDriverConfig, StructSerialVFDDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SerialVFDDriverConfig, StructSerialVFDDriverConfig);
ELEKTRA_SET_SIGNATURE (const SerialVFDDriverConfig *, StructSerialVFDDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SerialVFDDriverConfig *, StructSerialVFDDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (SliDriverConfig, StructSliDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SliDriverConfig, StructSliDriverConfig);
ELEKTRA_SET_SIGNATURE (const SliDriverConfig *, StructSliDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SliDriverConfig *, StructSliDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Stv5730DriverConfig, StructStv5730DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Stv5730DriverConfig, StructStv5730DriverConfig);
ELEKTRA_SET_SIGNATURE (const Stv5730DriverConfig *, StructStv5730DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Stv5730DriverConfig *, StructStv5730DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (SureElecDriverConfig, StructSureElecDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SureElecDriverConfig, StructSureElecDriverConfig);
ELEKTRA_SET_SIGNATURE (const SureElecDriverConfig *, StructSureElecDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SureElecDriverConfig *, StructSureElecDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (SvgaDriverConfig, StructSvgaDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (SvgaDriverConfig, StructSvgaDriverConfig);
ELEKTRA_SET_SIGNATURE (const SvgaDriverConfig *, StructSvgaDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const SvgaDriverConfig *, StructSvgaDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (T6963DriverConfig, StructT6963DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (T6963DriverConfig, StructT6963DriverConfig);
ELEKTRA_SET_SIGNATURE (const T6963DriverConfig *, StructT6963DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const T6963DriverConfig *, StructT6963DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (TextDriverConfig, StructTextDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (TextDriverConfig, StructTextDriverConfig);
ELEKTRA_SET_SIGNATURE (const TextDriverConfig *, StructTextDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const TextDriverConfig *, StructTextDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (TyanDriverConfig, StructTyanDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (TyanDriverConfig, StructTyanDriverConfig);
ELEKTRA_SET_SIGNATURE (const TyanDriverConfig *, StructTyanDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const TyanDriverConfig *, StructTyanDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Ula200DriverConfig, StructUla200DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Ula200DriverConfig, StructUla200DriverConfig);
ELEKTRA_SET_SIGNATURE (const Ula200DriverConfig *, StructUla200DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Ula200DriverConfig *, StructUla200DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Vlsys_m428DriverConfig, StructVlsys_m428DriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Vlsys_m428DriverConfig, StructVlsys_m428DriverConfig);
ELEKTRA_SET_SIGNATURE (const Vlsys_m428DriverConfig *, StructVlsys_m428DriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Vlsys_m428DriverConfig *, StructVlsys_m428DriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (XosdDriverConfig, StructXosdDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (XosdDriverConfig, StructXosdDriverConfig);
ELEKTRA_SET_SIGNATURE (const XosdDriverConfig *, StructXosdDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const XosdDriverConfig *, StructXosdDriverConfig);


ELEKTRA_GET_OUT_PTR_SIGNATURE (Yard2LCDDriverConfig, StructYard2LCDDriverConfig);
ELEKTRA_GET_OUT_PTR_ARRAY_ELEMENT_SIGNATURE (Yard2LCDDriverConfig, StructYard2LCDDriverConfig);
ELEKTRA_SET_SIGNATURE (const Yard2LCDDriverConfig *, StructYard2LCDDriverConfig);
ELEKTRA_SET_ARRAY_ELEMENT_SIGNATURE (const Yard2LCDDriverConfig *, StructYard2LCDDriverConfig);



// clang-format off

// clang-format on

// clang-format off

/**
* Tag name for 'bayrad/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_BAYRAD Bayrad

/**
* Tag name for 'bayrad/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_BAYRAD_DEVICE BayradDevice

/**
* Tag name for 'bayrad/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_BAYRAD_FILE BayradFile

/**
* Tag name for 'bayrad/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_BAYRAD_SPEED BayradSpeed

/**
* Tag name for 'cfontz/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ Cfontz

/**
* Tag name for 'cfontz/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_BRIGHTNESS CfontzBrightness

/**
* Tag name for 'cfontz/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_CONTRAST CfontzContrast

/**
* Tag name for 'cfontz/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_DEVICE CfontzDevice

/**
* Tag name for 'cfontz/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_FILE CfontzFile

/**
* Tag name for 'cfontz/#/newfirmware'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_NEWFIRMWARE CfontzNewfirmware

/**
* Tag name for 'cfontz/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_OFFBRIGHTNESS CfontzOffbrightness

/**
* Tag name for 'cfontz/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_REBOOT CfontzReboot

/**
* Tag name for 'cfontz/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_SIZE CfontzSize

/**
* Tag name for 'cfontz/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_SPEED CfontzSpeed

/**
* Tag name for 'cfontz/#/usb'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZ_USB CfontzUsb

/**
* Tag name for 'cfontzpacket/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET Cfontzpacket

/**
* Tag name for 'cfontzpacket/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_BRIGHTNESS CfontzpacketBrightness

/**
* Tag name for 'cfontzpacket/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_CONTRAST CfontzpacketContrast

/**
* Tag name for 'cfontzpacket/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_DEVICE CfontzpacketDevice

/**
* Tag name for 'cfontzpacket/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_FILE CfontzpacketFile

/**
* Tag name for 'cfontzpacket/#/model'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_MODEL CfontzpacketModel

/**
* Tag name for 'cfontzpacket/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_OFFBRIGHTNESS CfontzpacketOffbrightness

/**
* Tag name for 'cfontzpacket/#/oldfirmware'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_OLDFIRMWARE CfontzpacketOldfirmware

/**
* Tag name for 'cfontzpacket/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_REBOOT CfontzpacketReboot

/**
* Tag name for 'cfontzpacket/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_SIZE CfontzpacketSize

/**
* Tag name for 'cfontzpacket/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_SPEED CfontzpacketSpeed

/**
* Tag name for 'cfontzpacket/#/usb'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CFONTZPACKET_USB CfontzpacketUsb

/**
* Tag name for 'curses/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES Curses

/**
* Tag name for 'curses/#/background'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_BACKGROUND CursesBackground

/**
* Tag name for 'curses/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_BACKLIGHT CursesBacklight

/**
* Tag name for 'curses/#/drawborder'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_DRAWBORDER CursesDrawborder

/**
* Tag name for 'curses/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_FILE CursesFile

/**
* Tag name for 'curses/#/foreground'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_FOREGROUND CursesForeground

/**
* Tag name for 'curses/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_SIZE CursesSize

/**
* Tag name for 'curses/#/topleftx'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_TOPLEFTX CursesTopleftx

/**
* Tag name for 'curses/#/toplefty'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_TOPLEFTY CursesToplefty

/**
* Tag name for 'curses/#/useacs'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CURSES_USEACS CursesUseacs

/**
* Tag name for 'cwlnx/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX Cwlnx

/**
* Tag name for 'cwlnx/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_DEVICE CwlnxDevice

/**
* Tag name for 'cwlnx/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_FILE CwlnxFile

/**
* Tag name for 'cwlnx/#/keymap_a'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_A CwlnxKeymapA

/**
* Tag name for 'cwlnx/#/keymap_b'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_B CwlnxKeymapB

/**
* Tag name for 'cwlnx/#/keymap_c'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_C CwlnxKeymapC

/**
* Tag name for 'cwlnx/#/keymap_d'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_D CwlnxKeymapD

/**
* Tag name for 'cwlnx/#/keymap_e'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_E CwlnxKeymapE

/**
* Tag name for 'cwlnx/#/keymap_f'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYMAP_F CwlnxKeymapF

/**
* Tag name for 'cwlnx/#/keypad'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYPAD CwlnxKeypad

/**
* Tag name for 'cwlnx/#/keypad_test_mode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_KEYPAD_TEST_MODE CwlnxKeypadTestMode

/**
* Tag name for 'cwlnx/#/model'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_MODEL CwlnxModel

/**
* Tag name for 'cwlnx/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_SIZE CwlnxSize

/**
* Tag name for 'cwlnx/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_CWLNX_SPEED CwlnxSpeed

/**
* Tag name for 'ea65/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65 Ea65

/**
* Tag name for 'ea65/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65_BRIGHTNESS Ea65Brightness

/**
* Tag name for 'ea65/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65_CONTRAST Ea65Contrast

/**
* Tag name for 'ea65/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65_FILE Ea65File

/**
* Tag name for 'ea65/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65_OFFBRIGHTNESS Ea65Offbrightness

/**
* Tag name for 'ea65/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EA65_REBOOT Ea65Reboot

/**
* Tag name for 'eyeboxone/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE Eyeboxone

/**
* Tag name for 'eyeboxone/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_BACKLIGHT EyeboxoneBacklight

/**
* Tag name for 'eyeboxone/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_BRIGHTNESS EyeboxoneBrightness

/**
* Tag name for 'eyeboxone/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_CONTRAST EyeboxoneContrast

/**
* Tag name for 'eyeboxone/#/cursor'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_CURSOR EyeboxoneCursor

/**
* Tag name for 'eyeboxone/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_DEVICE EyeboxoneDevice

/**
* Tag name for 'eyeboxone/#/downkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_DOWNKEY EyeboxoneDownkey

/**
* Tag name for 'eyeboxone/#/escapekey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_ESCAPEKEY EyeboxoneEscapekey

/**
* Tag name for 'eyeboxone/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_FILE EyeboxoneFile

/**
* Tag name for 'eyeboxone/#/keypad_test_mode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_KEYPAD_TEST_MODE EyeboxoneKeypadTestMode

/**
* Tag name for 'eyeboxone/#/leftkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_LEFTKEY EyeboxoneLeftkey

/**
* Tag name for 'eyeboxone/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_OFFBRIGHTNESS EyeboxoneOffbrightness

/**
* Tag name for 'eyeboxone/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_REBOOT EyeboxoneReboot

/**
* Tag name for 'eyeboxone/#/rightkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_RIGHTKEY EyeboxoneRightkey

/**
* Tag name for 'eyeboxone/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_SIZE EyeboxoneSize

/**
* Tag name for 'eyeboxone/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_SPEED EyeboxoneSpeed

/**
* Tag name for 'eyeboxone/#/upkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_EYEBOXONE_UPKEY EyeboxoneUpkey

/**
* Tag name for 'futaba/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_FUTABA Futaba

/**
* Tag name for 'futaba/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_FUTABA_FILE FutabaFile

/**
* Tag name for 'g15/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15 G15

/**
* Tag name for 'g15/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_BRIGHTNESS G15Brightness

/**
* Tag name for 'g15/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_CONTRAST G15Contrast

/**
* Tag name for 'g15/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_FILE G15File

/**
* Tag name for 'g15/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_OFFBRIGHTNESS G15Offbrightness

/**
* Tag name for 'g15/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_REBOOT G15Reboot

/**
* Tag name for 'g15/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_G15_SIZE G15Size

/**
* Tag name for 'glcd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD Glcd

/**
* Tag name for 'glcd/#/bidirectional'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_BIDIRECTIONAL GlcdBidirectional

/**
* Tag name for 'glcd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_BRIGHTNESS GlcdBrightness

/**
* Tag name for 'glcd/#/cellsize'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_CELLSIZE GlcdCellsize

/**
* Tag name for 'glcd/#/connectiontype'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_CONNECTIONTYPE GlcdConnectiontype

/**
* Tag name for 'glcd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_CONTRAST GlcdContrast

/**
* Tag name for 'glcd/#/delaybus'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_DELAYBUS GlcdDelaybus

/**
* Tag name for 'glcd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_FILE GlcdFile

/**
* Tag name for 'glcd/#/fonthasicons'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_FONTHASICONS GlcdFonthasicons

/**
* Tag name for 'glcd/#/keymap_a'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_A GlcdKeymapA

/**
* Tag name for 'glcd/#/keymap_b'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_B GlcdKeymapB

/**
* Tag name for 'glcd/#/keymap_c'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_C GlcdKeymapC

/**
* Tag name for 'glcd/#/keymap_d'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_D GlcdKeymapD

/**
* Tag name for 'glcd/#/keymap_e'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_E GlcdKeymapE

/**
* Tag name for 'glcd/#/keymap_f'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYMAP_F GlcdKeymapF

/**
* Tag name for 'glcd/#/keyrepeatdelay'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYREPEATDELAY GlcdKeyrepeatdelay

/**
* Tag name for 'glcd/#/keyrepeatinterval'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_KEYREPEATINTERVAL GlcdKeyrepeatinterval

/**
* Tag name for 'glcd/#/normal_font'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_NORMAL_FONT GlcdNormalFont

/**
* Tag name for 'glcd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_OFFBRIGHTNESS GlcdOffbrightness

/**
* Tag name for 'glcd/#/picolcdgfx_inverted'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_PICOLCDGFX_INVERTED GlcdPicolcdgfxInverted

/**
* Tag name for 'glcd/#/picolcdgfx_keytimeout'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_PICOLCDGFX_KEYTIMEOUT GlcdPicolcdgfxKeytimeout

/**
* Tag name for 'glcd/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_PORT GlcdPort

/**
* Tag name for 'glcd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_REBOOT GlcdReboot

/**
* Tag name for 'glcd/#/serdisp_device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_SERDISP_DEVICE GlcdSerdispDevice

/**
* Tag name for 'glcd/#/serdisp_name'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_SERDISP_NAME GlcdSerdispName

/**
* Tag name for 'glcd/#/serdisp_options'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_SERDISP_OPTIONS GlcdSerdispOptions

/**
* Tag name for 'glcd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_SIZE GlcdSize

/**
* Tag name for 'glcd/#/useft2'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_USEFT2 GlcdUseft2

/**
* Tag name for 'glcd/#/x11_backlightcolor'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_X11_BACKLIGHTCOLOR GlcdX11Backlightcolor

/**
* Tag name for 'glcd/#/x11_border'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_X11_BORDER GlcdX11Border

/**
* Tag name for 'glcd/#/x11_inverted'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_X11_INVERTED GlcdX11Inverted

/**
* Tag name for 'glcd/#/x11_pixelcolor'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_X11_PIXELCOLOR GlcdX11Pixelcolor

/**
* Tag name for 'glcd/#/x11_pixelsize'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCD_X11_PIXELSIZE GlcdX11Pixelsize

/**
* Tag name for 'glcdlib/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB Glcdlib

/**
* Tag name for 'glcdlib/#/CharEncoding'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_CHARENCODING GlcdlibCharEncoding

/**
* Tag name for 'glcdlib/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_BACKLIGHT GlcdlibBacklight

/**
* Tag name for 'glcdlib/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_BRIGHTNESS GlcdlibBrightness

/**
* Tag name for 'glcdlib/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_CONTRAST GlcdlibContrast

/**
* Tag name for 'glcdlib/#/driver'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_DRIVER GlcdlibDriver

/**
* Tag name for 'glcdlib/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_FILE GlcdlibFile

/**
* Tag name for 'glcdlib/#/fontfile'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_FONTFILE GlcdlibFontfile

/**
* Tag name for 'glcdlib/#/invert'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_INVERT GlcdlibInvert

/**
* Tag name for 'glcdlib/#/minfontfacesize'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_MINFONTFACESIZE GlcdlibMinfontfacesize

/**
* Tag name for 'glcdlib/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_OFFBRIGHTNESS GlcdlibOffbrightness

/**
* Tag name for 'glcdlib/#/pixelshiftx'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_PIXELSHIFTX GlcdlibPixelshiftx

/**
* Tag name for 'glcdlib/#/pixelshifty'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_PIXELSHIFTY GlcdlibPixelshifty

/**
* Tag name for 'glcdlib/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_REBOOT GlcdlibReboot

/**
* Tag name for 'glcdlib/#/showbigborder'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_SHOWBIGBORDER GlcdlibShowbigborder

/**
* Tag name for 'glcdlib/#/showdebugframe'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_SHOWDEBUGFRAME GlcdlibShowdebugframe

/**
* Tag name for 'glcdlib/#/showthinborder'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_SHOWTHINBORDER GlcdlibShowthinborder

/**
* Tag name for 'glcdlib/#/textresolution'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_TEXTRESOLUTION GlcdlibTextresolution

/**
* Tag name for 'glcdlib/#/upsidedown'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_UPSIDEDOWN GlcdlibUpsidedown

/**
* Tag name for 'glcdlib/#/useft2'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLCDLIB_USEFT2 GlcdlibUseft2

/**
* Tag name for 'glk/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLK Glk

/**
* Tag name for 'glk/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLK_CONTRAST GlkContrast

/**
* Tag name for 'glk/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLK_DEVICE GlkDevice

/**
* Tag name for 'glk/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLK_FILE GlkFile

/**
* Tag name for 'glk/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_GLK_SPEED GlkSpeed

/**
* Tag name for 'hd44780/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780 Hd44780

/**
* Tag name for 'hd44780/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_BACKLIGHT Hd44780Backlight

/**
* Tag name for 'hd44780/#/backlightcmdoff'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_BACKLIGHTCMDOFF Hd44780Backlightcmdoff

/**
* Tag name for 'hd44780/#/backlightcmdon'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_BACKLIGHTCMDON Hd44780Backlightcmdon

/**
* Tag name for 'hd44780/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_BRIGHTNESS Hd44780Brightness

/**
* Tag name for 'hd44780/#/charmap'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_CHARMAP Hd44780Charmap

/**
* Tag name for 'hd44780/#/connectiontype'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_CONNECTIONTYPE Hd44780Connectiontype

/**
* Tag name for 'hd44780/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_CONTRAST Hd44780Contrast

/**
* Tag name for 'hd44780/#/delaybus'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_DELAYBUS Hd44780Delaybus

/**
* Tag name for 'hd44780/#/delaymult'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_DELAYMULT Hd44780Delaymult

/**
* Tag name for 'hd44780/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_DEVICE Hd44780Device

/**
* Tag name for 'hd44780/#/extendedmode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_EXTENDEDMODE Hd44780Extendedmode

/**
* Tag name for 'hd44780/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_FILE Hd44780File

/**
* Tag name for 'hd44780/#/fontbank'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_FONTBANK Hd44780Fontbank

/**
* Tag name for 'hd44780/#/keepalivedisplay'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEEPALIVEDISPLAY Hd44780Keepalivedisplay

/**
* Tag name for 'hd44780/#/keymatrix_4_1'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEYMATRIX_4_1 Hd44780Keymatrix41

/**
* Tag name for 'hd44780/#/keymatrix_4_2'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEYMATRIX_4_2 Hd44780Keymatrix42

/**
* Tag name for 'hd44780/#/keymatrix_4_3'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEYMATRIX_4_3 Hd44780Keymatrix43

/**
* Tag name for 'hd44780/#/keymatrix_4_4'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEYMATRIX_4_4 Hd44780Keymatrix44

/**
* Tag name for 'hd44780/#/keypad'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_KEYPAD Hd44780Keypad

/**
* Tag name for 'hd44780/#/lastline'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_LASTLINE Hd44780Lastline

/**
* Tag name for 'hd44780/#/lineaddress'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_LINEADDRESS Hd44780Lineaddress

/**
* Tag name for 'hd44780/#/model'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_MODEL Hd44780Model

/**
* Tag name for 'hd44780/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_OFFBRIGHTNESS Hd44780Offbrightness

/**
* Tag name for 'hd44780/#/outputport'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_OUTPUTPORT Hd44780Outputport

/**
* Tag name for 'hd44780/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_PORT Hd44780Port

/**
* Tag name for 'hd44780/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_REBOOT Hd44780Reboot

/**
* Tag name for 'hd44780/#/refreshdisplay'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_REFRESHDISPLAY Hd44780Refreshdisplay

/**
* Tag name for 'hd44780/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_SIZE Hd44780Size

/**
* Tag name for 'hd44780/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_SPEED Hd44780Speed

/**
* Tag name for 'hd44780/#/vspan'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_HD44780_VSPAN Hd44780Vspan

/**
* Tag name for 'icp_a106/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106 IcpA106

/**
* Tag name for 'icp_a106/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_BRIGHTNESS IcpA106Brightness

/**
* Tag name for 'icp_a106/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_CONTRAST IcpA106Contrast

/**
* Tag name for 'icp_a106/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_DEVICE IcpA106Device

/**
* Tag name for 'icp_a106/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_FILE IcpA106File

/**
* Tag name for 'icp_a106/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_OFFBRIGHTNESS IcpA106Offbrightness

/**
* Tag name for 'icp_a106/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_REBOOT IcpA106Reboot

/**
* Tag name for 'icp_a106/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ICP_A106_SIZE IcpA106Size

/**
* Tag name for 'imon/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON Imon

/**
* Tag name for 'imon/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_BRIGHTNESS ImonBrightness

/**
* Tag name for 'imon/#/charmap'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_CHARMAP ImonCharmap

/**
* Tag name for 'imon/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_CONTRAST ImonContrast

/**
* Tag name for 'imon/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_DEVICE ImonDevice

/**
* Tag name for 'imon/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_FILE ImonFile

/**
* Tag name for 'imon/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_OFFBRIGHTNESS ImonOffbrightness

/**
* Tag name for 'imon/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_REBOOT ImonReboot

/**
* Tag name for 'imon/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMON_SIZE ImonSize

/**
* Tag name for 'imonlcd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD Imonlcd

/**
* Tag name for 'imonlcd/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_BACKLIGHT ImonlcdBacklight

/**
* Tag name for 'imonlcd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_BRIGHTNESS ImonlcdBrightness

/**
* Tag name for 'imonlcd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_CONTRAST ImonlcdContrast

/**
* Tag name for 'imonlcd/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_DEVICE ImonlcdDevice

/**
* Tag name for 'imonlcd/#/discmode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_DISCMODE ImonlcdDiscmode

/**
* Tag name for 'imonlcd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_FILE ImonlcdFile

/**
* Tag name for 'imonlcd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_OFFBRIGHTNESS ImonlcdOffbrightness

/**
* Tag name for 'imonlcd/#/onexit'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_ONEXIT ImonlcdOnexit

/**
* Tag name for 'imonlcd/#/protocol'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_PROTOCOL ImonlcdProtocol

/**
* Tag name for 'imonlcd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_REBOOT ImonlcdReboot

/**
* Tag name for 'imonlcd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IMONLCD_SIZE ImonlcdSize

/**
* Tag name for 'iowarrior/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR Iowarrior

/**
* Tag name for 'iowarrior/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_BRIGHTNESS IowarriorBrightness

/**
* Tag name for 'iowarrior/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_CONTRAST IowarriorContrast

/**
* Tag name for 'iowarrior/#/extendedmode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_EXTENDEDMODE IowarriorExtendedmode

/**
* Tag name for 'iowarrior/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_FILE IowarriorFile

/**
* Tag name for 'iowarrior/#/lastline'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_LASTLINE IowarriorLastline

/**
* Tag name for 'iowarrior/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_OFFBRIGHTNESS IowarriorOffbrightness

/**
* Tag name for 'iowarrior/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_REBOOT IowarriorReboot

/**
* Tag name for 'iowarrior/#/serialnumber'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_SERIALNUMBER IowarriorSerialnumber

/**
* Tag name for 'iowarrior/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IOWARRIOR_SIZE IowarriorSize

/**
* Tag name for 'irman/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN Irman

/**
* Tag name for 'irman/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_BRIGHTNESS IrmanBrightness

/**
* Tag name for 'irman/#/config'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_CONFIG IrmanConfig

/**
* Tag name for 'irman/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_CONTRAST IrmanContrast

/**
* Tag name for 'irman/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_DEVICE IrmanDevice

/**
* Tag name for 'irman/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_FILE IrmanFile

/**
* Tag name for 'irman/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_OFFBRIGHTNESS IrmanOffbrightness

/**
* Tag name for 'irman/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRMAN_REBOOT IrmanReboot

/**
* Tag name for 'irtrans/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS Irtrans

/**
* Tag name for 'irtrans/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_BACKLIGHT IrtransBacklight

/**
* Tag name for 'irtrans/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_BRIGHTNESS IrtransBrightness

/**
* Tag name for 'irtrans/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_CONTRAST IrtransContrast

/**
* Tag name for 'irtrans/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_FILE IrtransFile

/**
* Tag name for 'irtrans/#/hostname'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_HOSTNAME IrtransHostname

/**
* Tag name for 'irtrans/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_OFFBRIGHTNESS IrtransOffbrightness

/**
* Tag name for 'irtrans/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_REBOOT IrtransReboot

/**
* Tag name for 'irtrans/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_IRTRANS_SIZE IrtransSize

/**
* Tag name for 'joy/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY Joy

/**
* Tag name for 'joy/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_BRIGHTNESS JoyBrightness

/**
* Tag name for 'joy/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_CONTRAST JoyContrast

/**
* Tag name for 'joy/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_DEVICE JoyDevice

/**
* Tag name for 'joy/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_FILE JoyFile

/**
* Tag name for 'joy/#/map_axis1neg'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_AXIS1NEG JoyMapAxis1neg

/**
* Tag name for 'joy/#/map_axis1pos'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_AXIS1POS JoyMapAxis1pos

/**
* Tag name for 'joy/#/map_axis2neg'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_AXIS2NEG JoyMapAxis2neg

/**
* Tag name for 'joy/#/map_axis2pos'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_AXIS2POS JoyMapAxis2pos

/**
* Tag name for 'joy/#/map_button1'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_BUTTON1 JoyMapButton1

/**
* Tag name for 'joy/#/map_button2'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_MAP_BUTTON2 JoyMapButton2

/**
* Tag name for 'joy/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_OFFBRIGHTNESS JoyOffbrightness

/**
* Tag name for 'joy/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_JOY_REBOOT JoyReboot

/**
* Tag name for 'lb216/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216 Lb216

/**
* Tag name for 'lb216/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216_BRIGHTNESS Lb216Brightness

/**
* Tag name for 'lb216/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216_DEVICE Lb216Device

/**
* Tag name for 'lb216/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216_FILE Lb216File

/**
* Tag name for 'lb216/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216_REBOOT Lb216Reboot

/**
* Tag name for 'lb216/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LB216_SPEED Lb216Speed

/**
* Tag name for 'lcdm001/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001 Lcdm001

/**
* Tag name for 'lcdm001/#/backkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_BACKKEY Lcdm001Backkey

/**
* Tag name for 'lcdm001/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_DEVICE Lcdm001Device

/**
* Tag name for 'lcdm001/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_FILE Lcdm001File

/**
* Tag name for 'lcdm001/#/forwardkey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_FORWARDKEY Lcdm001Forwardkey

/**
* Tag name for 'lcdm001/#/mainmenukey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_MAINMENUKEY Lcdm001Mainmenukey

/**
* Tag name for 'lcdm001/#/pausekey'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCDM001_PAUSEKEY Lcdm001Pausekey

/**
* Tag name for 'lcterm/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM Lcterm

/**
* Tag name for 'lcterm/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_BRIGHTNESS LctermBrightness

/**
* Tag name for 'lcterm/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_CONTRAST LctermContrast

/**
* Tag name for 'lcterm/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_DEVICE LctermDevice

/**
* Tag name for 'lcterm/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_FILE LctermFile

/**
* Tag name for 'lcterm/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_OFFBRIGHTNESS LctermOffbrightness

/**
* Tag name for 'lcterm/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_REBOOT LctermReboot

/**
* Tag name for 'lcterm/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LCTERM_SIZE LctermSize

/**
* Tag name for 'linux_input/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT LinuxInput

/**
* Tag name for 'linux_input/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_BRIGHTNESS LinuxInputBrightness

/**
* Tag name for 'linux_input/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_CONTRAST LinuxInputContrast

/**
* Tag name for 'linux_input/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_DEVICE LinuxInputDevice

/**
* Tag name for 'linux_input/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_FILE LinuxInputFile

/**
* Tag name for 'linux_input/#/key/_'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* - const char * name1: Replaces occurence no. 1 of _ in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_KEY LinuxInputKey

/**
* Tag name for 'linux_input/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_OFFBRIGHTNESS LinuxInputOffbrightness

/**
* Tag name for 'linux_input/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LINUX_INPUT_REBOOT LinuxInputReboot

/**
* Tag name for 'lirc/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC Lirc

/**
* Tag name for 'lirc/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_BRIGHTNESS LircBrightness

/**
* Tag name for 'lirc/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_CONTRAST LircContrast

/**
* Tag name for 'lirc/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_FILE LircFile

/**
* Tag name for 'lirc/#/lircrc'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_LIRCRC LircLircrc

/**
* Tag name for 'lirc/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_OFFBRIGHTNESS LircOffbrightness

/**
* Tag name for 'lirc/#/prog'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_PROG LircProg

/**
* Tag name for 'lirc/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIRC_REBOOT LircReboot

/**
* Tag name for 'lis/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS Lis

/**
* Tag name for 'lis/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_BRIGHTNESS LisBrightness

/**
* Tag name for 'lis/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_CONTRAST LisContrast

/**
* Tag name for 'lis/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_FILE LisFile

/**
* Tag name for 'lis/#/lastline'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_LASTLINE LisLastline

/**
* Tag name for 'lis/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_OFFBRIGHTNESS LisOffbrightness

/**
* Tag name for 'lis/#/productid'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_PRODUCTID LisProductid

/**
* Tag name for 'lis/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_REBOOT LisReboot

/**
* Tag name for 'lis/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_SIZE LisSize

/**
* Tag name for 'lis/#/vendorid'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_LIS_VENDORID LisVendorid

/**
* Tag name for 'md8800/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800 Md8800

/**
* Tag name for 'md8800/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_BRIGHTNESS Md8800Brightness

/**
* Tag name for 'md8800/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_CONTRAST Md8800Contrast

/**
* Tag name for 'md8800/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_DEVICE Md8800Device

/**
* Tag name for 'md8800/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_FILE Md8800File

/**
* Tag name for 'md8800/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_OFFBRIGHTNESS Md8800Offbrightness

/**
* Tag name for 'md8800/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_REBOOT Md8800Reboot

/**
* Tag name for 'md8800/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MD8800_SIZE Md8800Size

/**
* Tag name for 'mdm166a/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A Mdm166a

/**
* Tag name for 'mdm166a/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_BRIGHTNESS Mdm166aBrightness

/**
* Tag name for 'mdm166a/#/clock'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_CLOCK Mdm166aClock

/**
* Tag name for 'mdm166a/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_CONTRAST Mdm166aContrast

/**
* Tag name for 'mdm166a/#/dimming'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_DIMMING Mdm166aDimming

/**
* Tag name for 'mdm166a/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_FILE Mdm166aFile

/**
* Tag name for 'mdm166a/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_OFFBRIGHTNESS Mdm166aOffbrightness

/**
* Tag name for 'mdm166a/#/offdimming'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_OFFDIMMING Mdm166aOffdimming

/**
* Tag name for 'mdm166a/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MDM166A_REBOOT Mdm166aReboot

/**
* Tag name for 'menu/downkey'
* 
*/// 
#define ELEKTRA_TAG_MENU_DOWNKEY MenuDownkey

/**
* Tag name for 'menu/enterkey'
* 
*/// 
#define ELEKTRA_TAG_MENU_ENTERKEY MenuEnterkey

/**
* Tag name for 'menu/leftkey'
* 
*/// 
#define ELEKTRA_TAG_MENU_LEFTKEY MenuLeftkey

/**
* Tag name for 'menu/menukey'
* 
*/// 
#define ELEKTRA_TAG_MENU_MENUKEY MenuMenukey

/**
* Tag name for 'menu/permissivegoto'
* 
*/// 
#define ELEKTRA_TAG_MENU_PERMISSIVEGOTO MenuPermissivegoto

/**
* Tag name for 'menu/rightkey'
* 
*/// 
#define ELEKTRA_TAG_MENU_RIGHTKEY MenuRightkey

/**
* Tag name for 'menu/upkey'
* 
*/// 
#define ELEKTRA_TAG_MENU_UPKEY MenuUpkey

/**
* Tag name for 'ms6931/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931 Ms6931

/**
* Tag name for 'ms6931/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_BRIGHTNESS Ms6931Brightness

/**
* Tag name for 'ms6931/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_CONTRAST Ms6931Contrast

/**
* Tag name for 'ms6931/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_DEVICE Ms6931Device

/**
* Tag name for 'ms6931/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_FILE Ms6931File

/**
* Tag name for 'ms6931/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_OFFBRIGHTNESS Ms6931Offbrightness

/**
* Tag name for 'ms6931/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_REBOOT Ms6931Reboot

/**
* Tag name for 'ms6931/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MS6931_SIZE Ms6931Size

/**
* Tag name for 'mtc_s16209x/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X MtcS16209x

/**
* Tag name for 'mtc_s16209x/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_BRIGHTNESS MtcS16209xBrightness

/**
* Tag name for 'mtc_s16209x/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_CONTRAST MtcS16209xContrast

/**
* Tag name for 'mtc_s16209x/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_DEVICE MtcS16209xDevice

/**
* Tag name for 'mtc_s16209x/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_FILE MtcS16209xFile

/**
* Tag name for 'mtc_s16209x/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_OFFBRIGHTNESS MtcS16209xOffbrightness

/**
* Tag name for 'mtc_s16209x/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTC_S16209X_REBOOT MtcS16209xReboot

/**
* Tag name for 'mtxorb/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB Mtxorb

/**
* Tag name for 'mtxorb/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_BRIGHTNESS MtxorbBrightness

/**
* Tag name for 'mtxorb/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_CONTRAST MtxorbContrast

/**
* Tag name for 'mtxorb/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_DEVICE MtxorbDevice

/**
* Tag name for 'mtxorb/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_FILE MtxorbFile

/**
* Tag name for 'mtxorb/#/hasadjustablebacklight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_HASADJUSTABLEBACKLIGHT MtxorbHasadjustablebacklight

/**
* Tag name for 'mtxorb/#/keymap_a'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_A MtxorbKeymapA

/**
* Tag name for 'mtxorb/#/keymap_b'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_B MtxorbKeymapB

/**
* Tag name for 'mtxorb/#/keymap_c'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_C MtxorbKeymapC

/**
* Tag name for 'mtxorb/#/keymap_d'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_D MtxorbKeymapD

/**
* Tag name for 'mtxorb/#/keymap_e'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_E MtxorbKeymapE

/**
* Tag name for 'mtxorb/#/keymap_f'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYMAP_F MtxorbKeymapF

/**
* Tag name for 'mtxorb/#/keypad_test_mode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_KEYPAD_TEST_MODE MtxorbKeypadTestMode

/**
* Tag name for 'mtxorb/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_OFFBRIGHTNESS MtxorbOffbrightness

/**
* Tag name for 'mtxorb/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_SIZE MtxorbSize

/**
* Tag name for 'mtxorb/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_SPEED MtxorbSpeed

/**
* Tag name for 'mtxorb/#/type'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MTXORB_TYPE MtxorbType

/**
* Tag name for 'mx5000/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000 Mx5000

/**
* Tag name for 'mx5000/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_BRIGHTNESS Mx5000Brightness

/**
* Tag name for 'mx5000/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_CONTRAST Mx5000Contrast

/**
* Tag name for 'mx5000/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_DEVICE Mx5000Device

/**
* Tag name for 'mx5000/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_FILE Mx5000File

/**
* Tag name for 'mx5000/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_OFFBRIGHTNESS Mx5000Offbrightness

/**
* Tag name for 'mx5000/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_REBOOT Mx5000Reboot

/**
* Tag name for 'mx5000/#/waitafterrefresh'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_MX5000_WAITAFTERREFRESH Mx5000Waitafterrefresh

/**
* Tag name for 'noritakevfd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD Noritakevfd

/**
* Tag name for 'noritakevfd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_BRIGHTNESS NoritakevfdBrightness

/**
* Tag name for 'noritakevfd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_CONTRAST NoritakevfdContrast

/**
* Tag name for 'noritakevfd/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_DEVICE NoritakevfdDevice

/**
* Tag name for 'noritakevfd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_FILE NoritakevfdFile

/**
* Tag name for 'noritakevfd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_OFFBRIGHTNESS NoritakevfdOffbrightness

/**
* Tag name for 'noritakevfd/#/parity'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_PARITY NoritakevfdParity

/**
* Tag name for 'noritakevfd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_REBOOT NoritakevfdReboot

/**
* Tag name for 'noritakevfd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_SIZE NoritakevfdSize

/**
* Tag name for 'noritakevfd/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_NORITAKEVFD_SPEED NoritakevfdSpeed

/**
* Tag name for 'olimex_mod_lcd1x9/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9 OlimexModLcd1x9

/**
* Tag name for 'olimex_mod_lcd1x9/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_BRIGHTNESS OlimexModLcd1x9Brightness

/**
* Tag name for 'olimex_mod_lcd1x9/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_CONTRAST OlimexModLcd1x9Contrast

/**
* Tag name for 'olimex_mod_lcd1x9/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_DEVICE OlimexModLcd1x9Device

/**
* Tag name for 'olimex_mod_lcd1x9/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_FILE OlimexModLcd1x9File

/**
* Tag name for 'olimex_mod_lcd1x9/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_OFFBRIGHTNESS OlimexModLcd1x9Offbrightness

/**
* Tag name for 'olimex_mod_lcd1x9/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_OLIMEX_MOD_LCD1X9_REBOOT OlimexModLcd1x9Reboot

/**
* Tag name for 'picolcd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD Picolcd

/**
* Tag name for 'picolcd/#/backlight'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_BACKLIGHT PicolcdBacklight

/**
* Tag name for 'picolcd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_BRIGHTNESS PicolcdBrightness

/**
* Tag name for 'picolcd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_CONTRAST PicolcdContrast

/**
* Tag name for 'picolcd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_FILE PicolcdFile

/**
* Tag name for 'picolcd/#/key0light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY0LIGHT PicolcdKey0light

/**
* Tag name for 'picolcd/#/key1light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY1LIGHT PicolcdKey1light

/**
* Tag name for 'picolcd/#/key2light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY2LIGHT PicolcdKey2light

/**
* Tag name for 'picolcd/#/key3light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY3LIGHT PicolcdKey3light

/**
* Tag name for 'picolcd/#/key4light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY4LIGHT PicolcdKey4light

/**
* Tag name for 'picolcd/#/key5light'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEY5LIGHT PicolcdKey5light

/**
* Tag name for 'picolcd/#/keylights'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEYLIGHTS PicolcdKeylights

/**
* Tag name for 'picolcd/#/keyrepeatdelay'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEYREPEATDELAY PicolcdKeyrepeatdelay

/**
* Tag name for 'picolcd/#/keyrepeatinterval'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEYREPEATINTERVAL PicolcdKeyrepeatinterval

/**
* Tag name for 'picolcd/#/keytimeout'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_KEYTIMEOUT PicolcdKeytimeout

/**
* Tag name for 'picolcd/#/linklights'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_LINKLIGHTS PicolcdLinklights

/**
* Tag name for 'picolcd/#/lircflushthreshold'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_LIRCFLUSHTHRESHOLD PicolcdLircflushthreshold

/**
* Tag name for 'picolcd/#/lirchost'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_LIRCHOST PicolcdLirchost

/**
* Tag name for 'picolcd/#/lircport'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_LIRCPORT PicolcdLircport

/**
* Tag name for 'picolcd/#/lirctime_us'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_LIRCTIME_US PicolcdLirctimeUs

/**
* Tag name for 'picolcd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_OFFBRIGHTNESS PicolcdOffbrightness

/**
* Tag name for 'picolcd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PICOLCD_REBOOT PicolcdReboot

/**
* Tag name for 'pyramid/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PYRAMID Pyramid

/**
* Tag name for 'pyramid/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PYRAMID_DEVICE PyramidDevice

/**
* Tag name for 'pyramid/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_PYRAMID_FILE PyramidFile

/**
* Tag name for 'rawserial/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL Rawserial

/**
* Tag name for 'rawserial/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_BRIGHTNESS RawserialBrightness

/**
* Tag name for 'rawserial/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_CONTRAST RawserialContrast

/**
* Tag name for 'rawserial/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_DEVICE RawserialDevice

/**
* Tag name for 'rawserial/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_FILE RawserialFile

/**
* Tag name for 'rawserial/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_OFFBRIGHTNESS RawserialOffbrightness

/**
* Tag name for 'rawserial/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_REBOOT RawserialReboot

/**
* Tag name for 'rawserial/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_SIZE RawserialSize

/**
* Tag name for 'rawserial/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_SPEED RawserialSpeed

/**
* Tag name for 'rawserial/#/updaterate'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_RAWSERIAL_UPDATERATE RawserialUpdaterate

/**
* Tag name for 'sed1330/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330 Sed1330

/**
* Tag name for 'sed1330/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_BRIGHTNESS Sed1330Brightness

/**
* Tag name for 'sed1330/#/cellsize'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_CELLSIZE Sed1330Cellsize

/**
* Tag name for 'sed1330/#/connectiontype'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_CONNECTIONTYPE Sed1330Connectiontype

/**
* Tag name for 'sed1330/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_CONTRAST Sed1330Contrast

/**
* Tag name for 'sed1330/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_FILE Sed1330File

/**
* Tag name for 'sed1330/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_OFFBRIGHTNESS Sed1330Offbrightness

/**
* Tag name for 'sed1330/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_PORT Sed1330Port

/**
* Tag name for 'sed1330/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_REBOOT Sed1330Reboot

/**
* Tag name for 'sed1330/#/type'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1330_TYPE Sed1330Type

/**
* Tag name for 'sed1520/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520 Sed1520

/**
* Tag name for 'sed1520/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_BRIGHTNESS Sed1520Brightness

/**
* Tag name for 'sed1520/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_CONTRAST Sed1520Contrast

/**
* Tag name for 'sed1520/#/delaymult'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_DELAYMULT Sed1520Delaymult

/**
* Tag name for 'sed1520/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_FILE Sed1520File

/**
* Tag name for 'sed1520/#/haveinverter'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_HAVEINVERTER Sed1520Haveinverter

/**
* Tag name for 'sed1520/#/interfacetype'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_INTERFACETYPE Sed1520Interfacetype

/**
* Tag name for 'sed1520/#/invertedmapping'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_INVERTEDMAPPING Sed1520Invertedmapping

/**
* Tag name for 'sed1520/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_OFFBRIGHTNESS Sed1520Offbrightness

/**
* Tag name for 'sed1520/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_PORT Sed1520Port

/**
* Tag name for 'sed1520/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_REBOOT Sed1520Reboot

/**
* Tag name for 'sed1520/#/usehardreset'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SED1520_USEHARDRESET Sed1520Usehardreset

/**
* Tag name for 'serialpos/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS Serialpos

/**
* Tag name for 'serialpos/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_BRIGHTNESS SerialposBrightness

/**
* Tag name for 'serialpos/#/cellsize'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_CELLSIZE SerialposCellsize

/**
* Tag name for 'serialpos/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_CONTRAST SerialposContrast

/**
* Tag name for 'serialpos/#/custom_chars'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_CUSTOM_CHARS SerialposCustomChars

/**
* Tag name for 'serialpos/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_DEVICE SerialposDevice

/**
* Tag name for 'serialpos/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_FILE SerialposFile

/**
* Tag name for 'serialpos/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_OFFBRIGHTNESS SerialposOffbrightness

/**
* Tag name for 'serialpos/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_REBOOT SerialposReboot

/**
* Tag name for 'serialpos/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_SIZE SerialposSize

/**
* Tag name for 'serialpos/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_SPEED SerialposSpeed

/**
* Tag name for 'serialpos/#/type'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALPOS_TYPE SerialposType

/**
* Tag name for 'serialvfd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD Serialvfd

/**
* Tag name for 'serialvfd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_BRIGHTNESS SerialvfdBrightness

/**
* Tag name for 'serialvfd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_CONTRAST SerialvfdContrast

/**
* Tag name for 'serialvfd/#/custom-characters'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_CUSTOM_CHARACTERS SerialvfdCustomCharacters

/**
* Tag name for 'serialvfd/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_DEVICE SerialvfdDevice

/**
* Tag name for 'serialvfd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_FILE SerialvfdFile

/**
* Tag name for 'serialvfd/#/iso_8859_1'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_ISO_8859_1 SerialvfdIso88591

/**
* Tag name for 'serialvfd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_OFFBRIGHTNESS SerialvfdOffbrightness

/**
* Tag name for 'serialvfd/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_PORT SerialvfdPort

/**
* Tag name for 'serialvfd/#/portwait'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_PORTWAIT SerialvfdPortwait

/**
* Tag name for 'serialvfd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_REBOOT SerialvfdReboot

/**
* Tag name for 'serialvfd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_SIZE SerialvfdSize

/**
* Tag name for 'serialvfd/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_SPEED SerialvfdSpeed

/**
* Tag name for 'serialvfd/#/type'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_TYPE SerialvfdType

/**
* Tag name for 'serialvfd/#/use_parallel'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERIALVFD_USE_PARALLEL SerialvfdUseParallel

/**
* Tag name for 'server/autorotate'
* 
*/// 
#define ELEKTRA_TAG_SERVER_AUTOROTATE ServerAutorotate

/**
* Tag name for 'server/backlight'
* 
*/// 
#define ELEKTRA_TAG_SERVER_BACKLIGHT ServerBacklight

/**
* Tag name for 'server/bind'
* 
*/// 
#define ELEKTRA_TAG_SERVER_BIND ServerBind

/**
* Tag name for 'server/driverpath'
* 
*/// 
#define ELEKTRA_TAG_SERVER_DRIVERPATH ServerDriverpath

/**
* Tag name for 'server/drivers/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERVER_DRIVERS ServerDrivers

/**
* Tag name for 'server/foreground'
* 
*/// 
#define ELEKTRA_TAG_SERVER_FOREGROUND ServerForeground

/**
* Tag name for 'server/frameinterval'
* 
*/// 
#define ELEKTRA_TAG_SERVER_FRAMEINTERVAL ServerFrameinterval

/**
* Tag name for 'server/goodbye/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERVER_GOODBYE ServerGoodbye

/**
* Tag name for 'server/heartbeat'
* 
*/// 
#define ELEKTRA_TAG_SERVER_HEARTBEAT ServerHeartbeat

/**
* Tag name for 'server/hello/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SERVER_HELLO ServerHello

/**
* Tag name for 'server/nextscreenkey'
* 
*/// 
#define ELEKTRA_TAG_SERVER_NEXTSCREENKEY ServerNextscreenkey

/**
* Tag name for 'server/port'
* 
*/// 
#define ELEKTRA_TAG_SERVER_PORT ServerPort

/**
* Tag name for 'server/prevscreenkey'
* 
*/// 
#define ELEKTRA_TAG_SERVER_PREVSCREENKEY ServerPrevscreenkey

/**
* Tag name for 'server/reportlevel'
* 
*/// 
#define ELEKTRA_TAG_SERVER_REPORTLEVEL ServerReportlevel

/**
* Tag name for 'server/reporttosyslog'
* 
*/// 
#define ELEKTRA_TAG_SERVER_REPORTTOSYSLOG ServerReporttosyslog

/**
* Tag name for 'server/scrolldownkey'
* 
*/// 
#define ELEKTRA_TAG_SERVER_SCROLLDOWNKEY ServerScrolldownkey

/**
* Tag name for 'server/scrollupkey'
* 
*/// 
#define ELEKTRA_TAG_SERVER_SCROLLUPKEY ServerScrollupkey

/**
* Tag name for 'server/serverscreen'
* 
*/// 
#define ELEKTRA_TAG_SERVER_SERVERSCREEN ServerServerscreen

/**
* Tag name for 'server/titlespeed'
* 
*/// 
#define ELEKTRA_TAG_SERVER_TITLESPEED ServerTitlespeed

/**
* Tag name for 'server/togglerotatekey'
* 
*/// 
#define ELEKTRA_TAG_SERVER_TOGGLEROTATEKEY ServerTogglerotatekey

/**
* Tag name for 'server/user'
* 
*/// 
#define ELEKTRA_TAG_SERVER_USER ServerUser

/**
* Tag name for 'server/waittime'
* 
*/// 
#define ELEKTRA_TAG_SERVER_WAITTIME ServerWaittime

/**
* Tag name for 'sli/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI Sli

/**
* Tag name for 'sli/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_BRIGHTNESS SliBrightness

/**
* Tag name for 'sli/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_CONTRAST SliContrast

/**
* Tag name for 'sli/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_DEVICE SliDevice

/**
* Tag name for 'sli/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_FILE SliFile

/**
* Tag name for 'sli/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_OFFBRIGHTNESS SliOffbrightness

/**
* Tag name for 'sli/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_REBOOT SliReboot

/**
* Tag name for 'sli/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SLI_SPEED SliSpeed

/**
* Tag name for 'stv5730/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730 Stv5730

/**
* Tag name for 'stv5730/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_BRIGHTNESS Stv5730Brightness

/**
* Tag name for 'stv5730/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_CONTRAST Stv5730Contrast

/**
* Tag name for 'stv5730/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_FILE Stv5730File

/**
* Tag name for 'stv5730/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_OFFBRIGHTNESS Stv5730Offbrightness

/**
* Tag name for 'stv5730/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_PORT Stv5730Port

/**
* Tag name for 'stv5730/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_STV5730_REBOOT Stv5730Reboot

/**
* Tag name for 'sureelec/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC Sureelec

/**
* Tag name for 'sureelec/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_BRIGHTNESS SureelecBrightness

/**
* Tag name for 'sureelec/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_CONTRAST SureelecContrast

/**
* Tag name for 'sureelec/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_DEVICE SureelecDevice

/**
* Tag name for 'sureelec/#/edition'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_EDITION SureelecEdition

/**
* Tag name for 'sureelec/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_FILE SureelecFile

/**
* Tag name for 'sureelec/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_OFFBRIGHTNESS SureelecOffbrightness

/**
* Tag name for 'sureelec/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_REBOOT SureelecReboot

/**
* Tag name for 'sureelec/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SUREELEC_SIZE SureelecSize

/**
* Tag name for 'svga/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA Svga

/**
* Tag name for 'svga/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_BRIGHTNESS SvgaBrightness

/**
* Tag name for 'svga/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_CONTRAST SvgaContrast

/**
* Tag name for 'svga/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_FILE SvgaFile

/**
* Tag name for 'svga/#/mode'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_MODE SvgaMode

/**
* Tag name for 'svga/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_OFFBRIGHTNESS SvgaOffbrightness

/**
* Tag name for 'svga/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_REBOOT SvgaReboot

/**
* Tag name for 'svga/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_SVGA_SIZE SvgaSize

/**
* Tag name for 't6963/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963 T6963

/**
* Tag name for 't6963/#/bidirectional'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_BIDIRECTIONAL T6963Bidirectional

/**
* Tag name for 't6963/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_BRIGHTNESS T6963Brightness

/**
* Tag name for 't6963/#/cleargraphic'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_CLEARGRAPHIC T6963Cleargraphic

/**
* Tag name for 't6963/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_CONTRAST T6963Contrast

/**
* Tag name for 't6963/#/delaybus'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_DELAYBUS T6963Delaybus

/**
* Tag name for 't6963/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_FILE T6963File

/**
* Tag name for 't6963/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_OFFBRIGHTNESS T6963Offbrightness

/**
* Tag name for 't6963/#/port'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_PORT T6963Port

/**
* Tag name for 't6963/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_REBOOT T6963Reboot

/**
* Tag name for 't6963/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_T6963_SIZE T6963Size

/**
* Tag name for 'text/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT Text

/**
* Tag name for 'text/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_BRIGHTNESS TextBrightness

/**
* Tag name for 'text/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_CONTRAST TextContrast

/**
* Tag name for 'text/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_FILE TextFile

/**
* Tag name for 'text/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_OFFBRIGHTNESS TextOffbrightness

/**
* Tag name for 'text/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_REBOOT TextReboot

/**
* Tag name for 'text/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TEXT_SIZE TextSize

/**
* Tag name for 'tyan/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN Tyan

/**
* Tag name for 'tyan/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_BRIGHTNESS TyanBrightness

/**
* Tag name for 'tyan/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_CONTRAST TyanContrast

/**
* Tag name for 'tyan/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_DEVICE TyanDevice

/**
* Tag name for 'tyan/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_FILE TyanFile

/**
* Tag name for 'tyan/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_OFFBRIGHTNESS TyanOffbrightness

/**
* Tag name for 'tyan/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_REBOOT TyanReboot

/**
* Tag name for 'tyan/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_SIZE TyanSize

/**
* Tag name for 'tyan/#/speed'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_TYAN_SPEED TyanSpeed

/**
* Tag name for 'ula200/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200 Ula200

/**
* Tag name for 'ula200/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_BRIGHTNESS Ula200Brightness

/**
* Tag name for 'ula200/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_CONTRAST Ula200Contrast

/**
* Tag name for 'ula200/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_FILE Ula200File

/**
* Tag name for 'ula200/#/keymap_a'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_A Ula200KeymapA

/**
* Tag name for 'ula200/#/keymap_b'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_B Ula200KeymapB

/**
* Tag name for 'ula200/#/keymap_c'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_C Ula200KeymapC

/**
* Tag name for 'ula200/#/keymap_d'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_D Ula200KeymapD

/**
* Tag name for 'ula200/#/keymap_e'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_E Ula200KeymapE

/**
* Tag name for 'ula200/#/keymap_f'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_KEYMAP_F Ula200KeymapF

/**
* Tag name for 'ula200/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_OFFBRIGHTNESS Ula200Offbrightness

/**
* Tag name for 'ula200/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_REBOOT Ula200Reboot

/**
* Tag name for 'ula200/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_ULA200_SIZE Ula200Size

/**
* Tag name for 'vlsys_m428/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428 VlsysM428

/**
* Tag name for 'vlsys_m428/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_BRIGHTNESS VlsysM428Brightness

/**
* Tag name for 'vlsys_m428/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_CONTRAST VlsysM428Contrast

/**
* Tag name for 'vlsys_m428/#/device'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_DEVICE VlsysM428Device

/**
* Tag name for 'vlsys_m428/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_FILE VlsysM428File

/**
* Tag name for 'vlsys_m428/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_OFFBRIGHTNESS VlsysM428Offbrightness

/**
* Tag name for 'vlsys_m428/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_VLSYS_M428_REBOOT VlsysM428Reboot

/**
* Tag name for 'xosd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD Xosd

/**
* Tag name for 'xosd/#/Font'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_FONT XosdFont

/**
* Tag name for 'xosd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_BRIGHTNESS XosdBrightness

/**
* Tag name for 'xosd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_CONTRAST XosdContrast

/**
* Tag name for 'xosd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_FILE XosdFile

/**
* Tag name for 'xosd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_OFFBRIGHTNESS XosdOffbrightness

/**
* Tag name for 'xosd/#/offset'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_OFFSET XosdOffset

/**
* Tag name for 'xosd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_REBOOT XosdReboot

/**
* Tag name for 'xosd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_XOSD_SIZE XosdSize

/**
* Tag name for 'yard2lcd/#'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD Yard2lcd

/**
* Tag name for 'yard2lcd/#/brightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_BRIGHTNESS Yard2lcdBrightness

/**
* Tag name for 'yard2lcd/#/contrast'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_CONTRAST Yard2lcdContrast

/**
* Tag name for 'yard2lcd/#/file'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_FILE Yard2lcdFile

/**
* Tag name for 'yard2lcd/#/offbrightness'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_OFFBRIGHTNESS Yard2lcdOffbrightness

/**
* Tag name for 'yard2lcd/#/reboot'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_REBOOT Yard2lcdReboot

/**
* Tag name for 'yard2lcd/#/size'
* 
* Required arguments:
* 
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
* 
* 
*/// 
#define ELEKTRA_TAG_YARD2LCD_SIZE Yard2lcdSize
// clang-format on


// clang-format off

// clang-format on

// clang-format off

// clang-format on


// clang-format off

// clang-format on




// clang-format off

// clang-format on

// local helper macros to determine the length of a 64 bit integer
#define elektra_len19(x) ((x) < 10000000000000000000ULL ? 19 : 20)
#define elektra_len18(x) ((x) < 1000000000000000000ULL ? 18 : elektra_len19 (x))
#define elektra_len17(x) ((x) < 100000000000000000ULL ? 17 : elektra_len18 (x))
#define elektra_len16(x) ((x) < 10000000000000000ULL ? 16 : elektra_len17 (x))
#define elektra_len15(x) ((x) < 1000000000000000ULL ? 15 : elektra_len16 (x))
#define elektra_len14(x) ((x) < 100000000000000ULL ? 14 : elektra_len15 (x))
#define elektra_len13(x) ((x) < 10000000000000ULL ? 13 : elektra_len14 (x))
#define elektra_len12(x) ((x) < 1000000000000ULL ? 12 : elektra_len13 (x))
#define elektra_len11(x) ((x) < 100000000000ULL ? 11 : elektra_len12 (x))
#define elektra_len10(x) ((x) < 10000000000ULL ? 10 : elektra_len11 (x))
#define elektra_len09(x) ((x) < 1000000000ULL ? 9 : elektra_len10 (x))
#define elektra_len08(x) ((x) < 100000000ULL ? 8 : elektra_len09 (x))
#define elektra_len07(x) ((x) < 10000000ULL ? 7 : elektra_len08 (x))
#define elektra_len06(x) ((x) < 1000000ULL ? 6 : elektra_len07 (x))
#define elektra_len05(x) ((x) < 100000ULL ? 5 : elektra_len06 (x))
#define elektra_len04(x) ((x) < 10000ULL ? 4 : elektra_len05 (x))
#define elektra_len03(x) ((x) < 1000ULL ? 3 : elektra_len04 (x))
#define elektra_len02(x) ((x) < 100ULL ? 2 : elektra_len03 (x))
#define elektra_len01(x) ((x) < 10ULL ? 1 : elektra_len02 (x))
#define elektra_len00(x) ((x) < 0ULL ? 0 : elektra_len01 (x))
#define elektra_len(x) elektra_len00 (x)

#define ELEKTRA_SIZE(tagName) ELEKTRA_CONCAT (elektraSize, tagName)

// clang-format off

// clang-format on



/**
 * Get the value of 'bayrad/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Bayrad) (Elektra * elektra, BayradDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructBayradDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'bayrad/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'bayrad/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Bayrad) (Elektra * elektra, const BayradDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructBayradDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'bayrad/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Bayrad) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "bayrad");
}



/**
 * Get the value of 'bayrad/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'bayrad/#/device'.
 */// 
static inline const char * ELEKTRA_GET (BayradDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'bayrad/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'bayrad/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (BayradDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'bayrad/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'bayrad/#/file'.
 */// 
static inline const char * ELEKTRA_GET (BayradFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'bayrad/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'bayrad/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (BayradFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'bayrad/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'bayrad/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (BayradSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'bayrad/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'bayrad/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (BayradSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("bayrad/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'cfontz/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Cfontz) (Elektra * elektra, CFontzDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructCFontzDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'cfontz/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Cfontz) (Elektra * elektra, const CFontzDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructCFontzDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'cfontz/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Cfontz) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "cfontz");
}



/**
 * Get the value of 'cfontz/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/device'.
 */// 
static inline const char * ELEKTRA_GET (CfontzDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/file'.
 */// 
static inline const char * ELEKTRA_GET (CfontzFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/newfirmware'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/newfirmware'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzNewfirmware) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/newfirmware",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/newfirmware'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/newfirmware'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzNewfirmware) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/newfirmware",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/size'.
 */// 
static inline const char * ELEKTRA_GET (CfontzSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontz/#/usb'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontz/#/usb'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzUsb) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/usb",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontz/#/usb'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontz/#/usb'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzUsb) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontz/%*.*s%lld/usb",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'cfontzpacket/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Cfontzpacket) (Elektra * elektra, CFontzPacketDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructCFontzPacketDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'cfontzpacket/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Cfontzpacket) (Elektra * elektra, const CFontzPacketDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructCFontzPacketDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'cfontzpacket/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Cfontzpacket) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "cfontzpacket");
}



/**
 * Get the value of 'cfontzpacket/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzpacketBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzpacketContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/device'.
 */// 
static inline const char * ELEKTRA_GET (CfontzpacketDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/file'.
 */// 
static inline const char * ELEKTRA_GET (CfontzpacketFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/model'.
 */// 
static inline CFontzPacketModel ELEKTRA_GET (CfontzpacketModel) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	CFontzPacketModel result = ELEKTRA_GET (EnumCFontzPacketModel) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/model'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketModel) (Elektra * elektra,
						    CFontzPacketModel value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumCFontzPacketModel) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzpacketOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/oldfirmware'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/oldfirmware'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzpacketOldfirmware) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/oldfirmware",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/oldfirmware'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/oldfirmware'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketOldfirmware) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/oldfirmware",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzpacketReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/size'.
 */// 
static inline const char * ELEKTRA_GET (CfontzpacketSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CfontzpacketSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cfontzpacket/#/usb'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cfontzpacket/#/usb'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CfontzpacketUsb) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/usb",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cfontzpacket/#/usb'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cfontzpacket/#/usb'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CfontzpacketUsb) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cfontzpacket/%*.*s%lld/usb",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'curses/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Curses) (Elektra * elektra, CursesDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructCursesDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'curses/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Curses) (Elektra * elektra, const CursesDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructCursesDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'curses/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Curses) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "curses");
}



/**
 * Get the value of 'curses/#/background'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/background'.
 */// 
static inline CursesColor ELEKTRA_GET (CursesBackground) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/background",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	CursesColor result = ELEKTRA_GET (EnumCursesColor) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/background'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/background'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesBackground) (Elektra * elektra,
						    CursesColor value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/background",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumCursesColor) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/backlight'.
 */// 
static inline CursesColor ELEKTRA_GET (CursesBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	CursesColor result = ELEKTRA_GET (EnumCursesColor) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesBacklight) (Elektra * elektra,
						    CursesColor value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumCursesColor) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/drawborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/drawborder'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CursesDrawborder) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/drawborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/drawborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/drawborder'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesDrawborder) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/drawborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/file'.
 */// 
static inline const char * ELEKTRA_GET (CursesFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/foreground'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/foreground'.
 */// 
static inline CursesColor ELEKTRA_GET (CursesForeground) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/foreground",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	CursesColor result = ELEKTRA_GET (EnumCursesColor) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/foreground'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/foreground'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesForeground) (Elektra * elektra,
						    CursesColor value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/foreground",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumCursesColor) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/size'.
 */// 
static inline const char * ELEKTRA_GET (CursesSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/topleftx'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/topleftx'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CursesTopleftx) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/topleftx",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/topleftx'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/topleftx'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesTopleftx) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/topleftx",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/toplefty'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/toplefty'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CursesToplefty) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/toplefty",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/toplefty'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/toplefty'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesToplefty) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/toplefty",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'curses/#/useacs'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'curses/#/useacs'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CursesUseacs) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/useacs",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'curses/#/useacs'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'curses/#/useacs'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CursesUseacs) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("curses/%*.*s%lld/useacs",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'cwlnx/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Cwlnx) (Elektra * elektra, CwLnxDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructCwLnxDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'cwlnx/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Cwlnx) (Elektra * elektra, const CwLnxDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructCwLnxDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'cwlnx/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Cwlnx) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "cwlnx");
}



/**
 * Get the value of 'cwlnx/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/device'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/file'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_a'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapA) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_a'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapA) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_b'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapB) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_b'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapB) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_c'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapC) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_c'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapC) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_d'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapD) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_d'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapD) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_e'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapE) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_e'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapE) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keymap_f'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxKeymapF) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keymap_f'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeymapF) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keypad'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keypad'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CwlnxKeypad) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keypad",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keypad'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keypad'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeypad) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keypad",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/keypad_test_mode'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (CwlnxKeypadTestMode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/keypad_test_mode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxKeypadTestMode) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/model'.
 */// 
static inline CwLnxModel ELEKTRA_GET (CwlnxModel) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	CwLnxModel result = ELEKTRA_GET (EnumCwLnxModel) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/model'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxModel) (Elektra * elektra,
						    CwLnxModel value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumCwLnxModel) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/size'.
 */// 
static inline const char * ELEKTRA_GET (CwlnxSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'cwlnx/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'cwlnx/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (CwlnxSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'cwlnx/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'cwlnx/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (CwlnxSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("cwlnx/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'ea65/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Ea65) (Elektra * elektra, Ea65DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructEa65DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'ea65/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65) (Elektra * elektra, const Ea65DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructEa65DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'ea65/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Ea65) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "ea65");
}



/**
 * Get the value of 'ea65/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ea65/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ea65Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ea65/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ea65/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ea65/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ea65Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ea65/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ea65/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ea65/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Ea65File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ea65/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ea65/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ea65/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ea65Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ea65/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ea65/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ea65/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Ea65Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ea65/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ea65/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ea65Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ea65/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'eyeboxone/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Eyeboxone) (Elektra * elektra, EyeboxOneDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructEyeboxOneDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'eyeboxone/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Eyeboxone) (Elektra * elektra, const EyeboxOneDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructEyeboxOneDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'eyeboxone/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Eyeboxone) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "eyeboxone");
}



/**
 * Get the value of 'eyeboxone/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/backlight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (EyeboxoneBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneBacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (EyeboxoneBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (EyeboxoneContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/cursor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/cursor'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (EyeboxoneCursor) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/cursor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/cursor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/cursor'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneCursor) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/cursor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/device'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/downkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/downkey'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneDownkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/downkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/downkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/downkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneDownkey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/downkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/escapekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/escapekey'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneEscapekey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/escapekey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/escapekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/escapekey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneEscapekey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/escapekey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/file'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/keypad_test_mode'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (EyeboxoneKeypadTestMode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/keypad_test_mode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneKeypadTestMode) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/leftkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/leftkey'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneLeftkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/leftkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/leftkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/leftkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneLeftkey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/leftkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (EyeboxoneOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (EyeboxoneReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/rightkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/rightkey'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneRightkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/rightkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/rightkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/rightkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneRightkey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/rightkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/size'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (EyeboxoneSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'eyeboxone/#/upkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'eyeboxone/#/upkey'.
 */// 
static inline const char * ELEKTRA_GET (EyeboxoneUpkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/upkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'eyeboxone/#/upkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'eyeboxone/#/upkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (EyeboxoneUpkey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("eyeboxone/%*.*s%lld/upkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'futaba/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Futaba) (Elektra * elektra, FutabaDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("futaba/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructFutabaDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'futaba/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'futaba/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Futaba) (Elektra * elektra, const FutabaDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("futaba/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructFutabaDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'futaba/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Futaba) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "futaba");
}



/**
 * Get the value of 'futaba/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'futaba/#/file'.
 */// 
static inline const char * ELEKTRA_GET (FutabaFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("futaba/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'futaba/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'futaba/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (FutabaFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("futaba/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'g15/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (G15) (Elektra * elektra, G15DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructG15DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'g15/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15) (Elektra * elektra, const G15DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructG15DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'g15/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (G15) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "g15");
}



/**
 * Get the value of 'g15/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (G15Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'g15/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (G15Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'g15/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/file'.
 */// 
static inline const char * ELEKTRA_GET (G15File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'g15/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (G15Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'g15/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (G15Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'g15/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'g15/#/size'.
 */// 
static inline const char * ELEKTRA_GET (G15Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'g15/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'g15/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (G15Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("g15/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'glcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Glcd) (Elektra * elektra, GlcdDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructGlcdDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'glcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Glcd) (Elektra * elektra, const GlcdDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructGlcdDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'glcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Glcd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "glcd");
}



/**
 * Get the value of 'glcd/#/bidirectional'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/bidirectional'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdBidirectional) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/bidirectional",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/bidirectional'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/bidirectional'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdBidirectional) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/bidirectional",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/cellsize'.
 */// 
static inline const char * ELEKTRA_GET (GlcdCellsize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/cellsize'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdCellsize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/connectiontype'.
 */// 
static inline GlcdConnectionType ELEKTRA_GET (GlcdConnectiontype) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	GlcdConnectionType result = ELEKTRA_GET (EnumGlcdConnectionType) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/connectiontype'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdConnectiontype) (Elektra * elektra,
						    GlcdConnectionType value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumGlcdConnectionType) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/delaybus'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdDelaybus) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/delaybus'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdDelaybus) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (GlcdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/fonthasicons'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/fonthasicons'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdFonthasicons) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/fonthasicons",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/fonthasicons'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/fonthasicons'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdFonthasicons) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/fonthasicons",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_a'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapA) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_a'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapA) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_b'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapB) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_b'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapB) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_c'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapC) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_c'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapC) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_d'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapD) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_d'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapD) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_e'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapE) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_e'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapE) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keymap_f'.
 */// 
static inline const char * ELEKTRA_GET (GlcdKeymapF) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keymap_f'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeymapF) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keyrepeatdelay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keyrepeatdelay'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdKeyrepeatdelay) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keyrepeatdelay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keyrepeatdelay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keyrepeatdelay'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeyrepeatdelay) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keyrepeatdelay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/keyrepeatinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/keyrepeatinterval'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdKeyrepeatinterval) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keyrepeatinterval",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/keyrepeatinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/keyrepeatinterval'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdKeyrepeatinterval) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/keyrepeatinterval",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/normal_font'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/normal_font'.
 */// 
static inline const char * ELEKTRA_GET (GlcdNormalFont) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/normal_font",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/normal_font'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/normal_font'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdNormalFont) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/normal_font",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/picolcdgfx_inverted'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/picolcdgfx_inverted'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdPicolcdgfxInverted) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/picolcdgfx_inverted",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/picolcdgfx_inverted'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/picolcdgfx_inverted'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdPicolcdgfxInverted) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/picolcdgfx_inverted",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/picolcdgfx_keytimeout'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/picolcdgfx_keytimeout'.
 */// 
static inline const char * ELEKTRA_GET (GlcdPicolcdgfxKeytimeout) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/picolcdgfx_keytimeout",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/picolcdgfx_keytimeout'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/picolcdgfx_keytimeout'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdPicolcdgfxKeytimeout) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/picolcdgfx_keytimeout",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/port'.
 */// 
static inline const char * ELEKTRA_GET (GlcdPort) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdPort) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/serdisp_device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/serdisp_device'.
 */// 
static inline const char * ELEKTRA_GET (GlcdSerdispDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/serdisp_device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/serdisp_device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdSerdispDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/serdisp_name'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/serdisp_name'.
 */// 
static inline const char * ELEKTRA_GET (GlcdSerdispName) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_name",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/serdisp_name'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/serdisp_name'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdSerdispName) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_name",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/serdisp_options'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/serdisp_options'.
 */// 
static inline const char * ELEKTRA_GET (GlcdSerdispOptions) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_options",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/serdisp_options'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/serdisp_options'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdSerdispOptions) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/serdisp_options",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (GlcdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/useft2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/useft2'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdUseft2) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/useft2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/useft2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/useft2'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdUseft2) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/useft2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/x11_backlightcolor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/x11_backlightcolor'.
 */// 
static inline const char * ELEKTRA_GET (GlcdX11Backlightcolor) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_backlightcolor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/x11_backlightcolor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/x11_backlightcolor'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdX11Backlightcolor) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_backlightcolor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/x11_border'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/x11_border'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (GlcdX11Border) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_border",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/x11_border'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/x11_border'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdX11Border) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_border",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/x11_inverted'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/x11_inverted'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdX11Inverted) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_inverted",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/x11_inverted'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/x11_inverted'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdX11Inverted) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_inverted",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/x11_pixelcolor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/x11_pixelcolor'.
 */// 
static inline const char * ELEKTRA_GET (GlcdX11Pixelcolor) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_pixelcolor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/x11_pixelcolor'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/x11_pixelcolor'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdX11Pixelcolor) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_pixelcolor",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcd/#/x11_pixelsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcd/#/x11_pixelsize'.
 */// 
static inline const char * ELEKTRA_GET (GlcdX11Pixelsize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_pixelsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcd/#/x11_pixelsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcd/#/x11_pixelsize'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdX11Pixelsize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcd/%*.*s%lld/x11_pixelsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'glcdlib/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Glcdlib) (Elektra * elektra, GlcdlibDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructGlcdlibDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'glcdlib/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Glcdlib) (Elektra * elektra, const GlcdlibDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructGlcdlibDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'glcdlib/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Glcdlib) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "glcdlib");
}



/**
 * Get the value of 'glcdlib/#/CharEncoding'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/CharEncoding'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibCharEncoding) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/CharEncoding",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/CharEncoding'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/CharEncoding'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibCharEncoding) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/CharEncoding",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/backlight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibBacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdlibBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdlibContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/driver'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/driver'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibDriver) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/driver",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/driver'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/driver'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibDriver) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/driver",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/file'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/fontfile'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/fontfile'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibFontfile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/fontfile",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/fontfile'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/fontfile'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibFontfile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/fontfile",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/invert'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/invert'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibInvert) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/invert",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/invert'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/invert'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibInvert) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/invert",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/minfontfacesize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/minfontfacesize'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibMinfontfacesize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/minfontfacesize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/minfontfacesize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/minfontfacesize'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibMinfontfacesize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/minfontfacesize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlcdlibOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/pixelshiftx'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/pixelshiftx'.
 */// 
static inline kdb_short_t ELEKTRA_GET (GlcdlibPixelshiftx) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/pixelshiftx",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_short_t result = ELEKTRA_GET (Short) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/pixelshiftx'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/pixelshiftx'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibPixelshiftx) (Elektra * elektra,
						    kdb_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/pixelshiftx",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Short) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/pixelshifty'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/pixelshifty'.
 */// 
static inline kdb_short_t ELEKTRA_GET (GlcdlibPixelshifty) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/pixelshifty",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_short_t result = ELEKTRA_GET (Short) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/pixelshifty'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/pixelshifty'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibPixelshifty) (Elektra * elektra,
						    kdb_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/pixelshifty",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Short) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/showbigborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/showbigborder'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibShowbigborder) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showbigborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/showbigborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/showbigborder'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibShowbigborder) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showbigborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/showdebugframe'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/showdebugframe'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibShowdebugframe) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showdebugframe",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/showdebugframe'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/showdebugframe'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibShowdebugframe) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showdebugframe",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/showthinborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/showthinborder'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibShowthinborder) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showthinborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/showthinborder'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/showthinborder'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibShowthinborder) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/showthinborder",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/textresolution'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/textresolution'.
 */// 
static inline const char * ELEKTRA_GET (GlcdlibTextresolution) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/textresolution",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/textresolution'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/textresolution'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibTextresolution) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/textresolution",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/upsidedown'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/upsidedown'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibUpsidedown) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/upsidedown",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/upsidedown'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/upsidedown'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibUpsidedown) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/upsidedown",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glcdlib/#/useft2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glcdlib/#/useft2'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (GlcdlibUseft2) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/useft2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glcdlib/#/useft2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glcdlib/#/useft2'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlcdlibUseft2) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glcdlib/%*.*s%lld/useft2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'glk/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Glk) (Elektra * elektra, GlkDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("glk/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructGlkDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'glk/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glk/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Glk) (Elektra * elektra, const GlkDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glk/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructGlkDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'glk/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Glk) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "glk");
}



/**
 * Get the value of 'glk/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glk/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlkContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glk/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glk/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlkContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glk/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glk/#/device'.
 */// 
static inline const char * ELEKTRA_GET (GlkDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glk/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glk/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlkDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glk/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glk/#/file'.
 */// 
static inline const char * ELEKTRA_GET (GlkFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glk/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glk/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlkFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'glk/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'glk/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (GlkSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'glk/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'glk/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (GlkSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("glk/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'hd44780/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Hd44780) (Elektra * elektra, Hd44780DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructHd44780DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'hd44780/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780) (Elektra * elektra, const Hd44780DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructHd44780DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'hd44780/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Hd44780) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "hd44780");
}



/**
 * Get the value of 'hd44780/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/backlight'.
 */// 
static inline HD44780Backlight ELEKTRA_GET (Hd44780Backlight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	HD44780Backlight result = ELEKTRA_GET (EnumHD44780Backlight) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Backlight) (Elektra * elektra,
						    HD44780Backlight value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumHD44780Backlight) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/backlightcmdoff'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/backlightcmdoff'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Backlightcmdoff) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlightcmdoff",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/backlightcmdoff'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/backlightcmdoff'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Backlightcmdoff) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlightcmdoff",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/backlightcmdon'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/backlightcmdon'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Backlightcmdon) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlightcmdon",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/backlightcmdon'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/backlightcmdon'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Backlightcmdon) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/backlightcmdon",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/charmap'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/charmap'.
 */// 
static inline HD44780Charmap ELEKTRA_GET (Hd44780Charmap) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/charmap",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	HD44780Charmap result = ELEKTRA_GET (EnumHD44780Charmap) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/charmap'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/charmap'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Charmap) (Elektra * elektra,
						    HD44780Charmap value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/charmap",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumHD44780Charmap) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/connectiontype'.
 */// 
static inline HD44780ConnectionType ELEKTRA_GET (Hd44780Connectiontype) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	HD44780ConnectionType result = ELEKTRA_GET (EnumHD44780ConnectionType) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/connectiontype'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Connectiontype) (Elektra * elektra,
						    HD44780ConnectionType value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumHD44780ConnectionType) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/delaybus'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Delaybus) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/delaybus'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Delaybus) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/delaymult'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/delaymult'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Delaymult) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/delaymult",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/delaymult'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/delaymult'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Delaymult) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/delaymult",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/extendedmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/extendedmode'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Extendedmode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/extendedmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/extendedmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/extendedmode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Extendedmode) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/extendedmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/fontbank'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/fontbank'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Fontbank) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/fontbank",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/fontbank'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/fontbank'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Fontbank) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/fontbank",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keepalivedisplay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keepalivedisplay'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Keepalivedisplay) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keepalivedisplay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keepalivedisplay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keepalivedisplay'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keepalivedisplay) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keepalivedisplay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keymatrix_4_1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keymatrix_4_1'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Keymatrix41) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keymatrix_4_1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keymatrix_4_1'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keymatrix41) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keymatrix_4_2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keymatrix_4_2'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Keymatrix42) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keymatrix_4_2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keymatrix_4_2'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keymatrix42) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keymatrix_4_3'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keymatrix_4_3'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Keymatrix43) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_3",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keymatrix_4_3'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keymatrix_4_3'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keymatrix43) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_3",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keymatrix_4_4'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keymatrix_4_4'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Keymatrix44) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_4",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keymatrix_4_4'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keymatrix_4_4'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keymatrix44) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keymatrix_4_4",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/keypad'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/keypad'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Keypad) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keypad",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/keypad'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/keypad'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Keypad) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/keypad",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/lastline'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Lastline) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/lastline'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Lastline) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/lineaddress'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/lineaddress'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Lineaddress) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/lineaddress",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/lineaddress'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/lineaddress'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Lineaddress) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/lineaddress",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/model'.
 */// 
static inline ElektraEnumHd44780Model ELEKTRA_GET (Hd44780Model) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumHd44780Model result = ELEKTRA_GET (EnumHd44780Model) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/model'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/model'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Model) (Elektra * elektra,
						    ElektraEnumHd44780Model value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/model",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumHd44780Model) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/outputport'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/outputport'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Outputport) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/outputport",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/outputport'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/outputport'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Outputport) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/outputport",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/port'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Port) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Port) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Hd44780Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/refreshdisplay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/refreshdisplay'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Hd44780Refreshdisplay) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/refreshdisplay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/refreshdisplay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/refreshdisplay'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Refreshdisplay) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/refreshdisplay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/size'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/speed'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (Hd44780Speed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Speed) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'hd44780/#/vspan'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'hd44780/#/vspan'.
 */// 
static inline const char * ELEKTRA_GET (Hd44780Vspan) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/vspan",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'hd44780/#/vspan'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'hd44780/#/vspan'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Hd44780Vspan) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("hd44780/%*.*s%lld/vspan",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'icp_a106/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (IcpA106) (Elektra * elektra, Icp_a106DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructIcp_a106DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'icp_a106/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106) (Elektra * elektra, const Icp_a106DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructIcp_a106DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'icp_a106/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (IcpA106) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "icp_a106");
}



/**
 * Get the value of 'icp_a106/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IcpA106Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IcpA106Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/device'.
 */// 
static inline const char * ELEKTRA_GET (IcpA106Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/file'.
 */// 
static inline const char * ELEKTRA_GET (IcpA106File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IcpA106Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IcpA106Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'icp_a106/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'icp_a106/#/size'.
 */// 
static inline const char * ELEKTRA_GET (IcpA106Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'icp_a106/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'icp_a106/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IcpA106Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("icp_a106/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'imon/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Imon) (Elektra * elektra, ImonDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructImonDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'imon/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Imon) (Elektra * elektra, const ImonDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructImonDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'imon/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Imon) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "imon");
}



/**
 * Get the value of 'imon/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/charmap'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/charmap'.
 */// 
static inline IMonCharmap ELEKTRA_GET (ImonCharmap) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/charmap",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	IMonCharmap result = ELEKTRA_GET (EnumIMonCharmap) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/charmap'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/charmap'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonCharmap) (Elektra * elektra,
						    IMonCharmap value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/charmap",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumIMonCharmap) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/device'.
 */// 
static inline const char * ELEKTRA_GET (ImonDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/file'.
 */// 
static inline const char * ELEKTRA_GET (ImonFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ImonReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imon/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imon/#/size'.
 */// 
static inline const char * ELEKTRA_GET (ImonSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imon/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imon/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imon/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'imonlcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Imonlcd) (Elektra * elektra, ImonlcdDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructImonlcdDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'imonlcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Imonlcd) (Elektra * elektra, const ImonlcdDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructImonlcdDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'imonlcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Imonlcd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "imonlcd");
}



/**
 * Get the value of 'imonlcd/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/backlight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ImonlcdBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdBacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonlcdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonlcdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/device'.
 */// 
static inline const char * ELEKTRA_GET (ImonlcdDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/discmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/discmode'.
 */// 
static inline IMonLCDDiscMode ELEKTRA_GET (ImonlcdDiscmode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/discmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	IMonLCDDiscMode result = ELEKTRA_GET (EnumIMonLCDDiscMode) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/discmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/discmode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdDiscmode) (Elektra * elektra,
						    IMonLCDDiscMode value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/discmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumIMonLCDDiscMode) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (ImonlcdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonlcdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/onexit'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/onexit'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ImonlcdOnexit) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/onexit",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/onexit'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/onexit'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdOnexit) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/onexit",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/protocol'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/protocol'.
 */// 
static inline const char * ELEKTRA_GET (ImonlcdProtocol) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/protocol",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/protocol'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/protocol'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdProtocol) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/protocol",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ImonlcdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'imonlcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'imonlcd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (ImonlcdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'imonlcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'imonlcd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ImonlcdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("imonlcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'iowarrior/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Iowarrior) (Elektra * elektra, IOWarriorDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructIOWarriorDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'iowarrior/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Iowarrior) (Elektra * elektra, const IOWarriorDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructIOWarriorDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'iowarrior/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Iowarrior) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "iowarrior");
}



/**
 * Get the value of 'iowarrior/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IowarriorBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IowarriorContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/extendedmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/extendedmode'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IowarriorExtendedmode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/extendedmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/extendedmode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/extendedmode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorExtendedmode) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/extendedmode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/file'.
 */// 
static inline const char * ELEKTRA_GET (IowarriorFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/lastline'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IowarriorLastline) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/lastline'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorLastline) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IowarriorOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IowarriorReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/serialnumber'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/serialnumber'.
 */// 
static inline const char * ELEKTRA_GET (IowarriorSerialnumber) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/serialnumber",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/serialnumber'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/serialnumber'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorSerialnumber) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/serialnumber",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'iowarrior/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'iowarrior/#/size'.
 */// 
static inline const char * ELEKTRA_GET (IowarriorSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'iowarrior/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'iowarrior/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IowarriorSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("iowarrior/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'irman/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Irman) (Elektra * elektra, IrManDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructIrManDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'irman/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Irman) (Elektra * elektra, const IrManDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructIrManDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'irman/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Irman) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "irman");
}



/**
 * Get the value of 'irman/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrmanBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/config'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/config'.
 */// 
static inline const char * ELEKTRA_GET (IrmanConfig) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/config",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/config'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/config'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanConfig) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/config",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrmanContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/device'.
 */// 
static inline const char * ELEKTRA_GET (IrmanDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/file'.
 */// 
static inline const char * ELEKTRA_GET (IrmanFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrmanOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irman/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irman/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IrmanReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irman/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irman/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrmanReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irman/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'irtrans/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Irtrans) (Elektra * elektra, IrtransDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructIrtransDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'irtrans/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Irtrans) (Elektra * elektra, const IrtransDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructIrtransDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'irtrans/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Irtrans) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "irtrans");
}



/**
 * Get the value of 'irtrans/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/backlight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IrtransBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransBacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrtransBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrtransContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/file'.
 */// 
static inline const char * ELEKTRA_GET (IrtransFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/hostname'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/hostname'.
 */// 
static inline const char * ELEKTRA_GET (IrtransHostname) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/hostname",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/hostname'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/hostname'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransHostname) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/hostname",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (IrtransOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (IrtransReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'irtrans/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'irtrans/#/size'.
 */// 
static inline const char * ELEKTRA_GET (IrtransSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'irtrans/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'irtrans/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (IrtransSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("irtrans/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'joy/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Joy) (Elektra * elektra, JoyDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructJoyDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'joy/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Joy) (Elektra * elektra, const JoyDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructJoyDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'joy/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Joy) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "joy");
}



/**
 * Get the value of 'joy/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (JoyBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (JoyContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/device'.
 */// 
static inline const char * ELEKTRA_GET (JoyDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/file'.
 */// 
static inline const char * ELEKTRA_GET (JoyFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_axis1neg'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_axis1neg'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapAxis1neg) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis1neg",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_axis1neg'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_axis1neg'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapAxis1neg) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis1neg",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_axis1pos'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_axis1pos'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapAxis1pos) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis1pos",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_axis1pos'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_axis1pos'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapAxis1pos) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis1pos",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_axis2neg'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_axis2neg'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapAxis2neg) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis2neg",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_axis2neg'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_axis2neg'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapAxis2neg) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis2neg",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_axis2pos'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_axis2pos'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapAxis2pos) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis2pos",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_axis2pos'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_axis2pos'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapAxis2pos) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_axis2pos",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_button1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_button1'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapButton1) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_button1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_button1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_button1'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapButton1) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_button1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/map_button2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/map_button2'.
 */// 
static inline const char * ELEKTRA_GET (JoyMapButton2) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_button2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/map_button2'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/map_button2'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyMapButton2) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/map_button2",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (JoyOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'joy/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'joy/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (JoyReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'joy/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'joy/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (JoyReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("joy/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'lb216/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Lb216) (Elektra * elektra, Lb216DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLb216DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'lb216/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216) (Elektra * elektra, const Lb216DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLb216DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'lb216/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Lb216) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "lb216");
}



/**
 * Get the value of 'lb216/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lb216/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Lb216Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lb216/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lb216/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lb216/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Lb216Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lb216/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lb216/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lb216/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Lb216File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lb216/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lb216/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lb216/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Lb216Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lb216/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lb216/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lb216/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Lb216Speed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lb216/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lb216/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lb216Speed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lb216/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'lcdm001/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Lcdm001) (Elektra * elektra, Lcdm001DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLcdm001DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'lcdm001/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001) (Elektra * elektra, const Lcdm001DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLcdm001DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'lcdm001/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Lcdm001) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "lcdm001");
}



/**
 * Get the value of 'lcdm001/#/backkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/backkey'.
 */// 
static inline Lcdm001Keys ELEKTRA_GET (Lcdm001Backkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/backkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	Lcdm001Keys result = ELEKTRA_GET (EnumLcdm001Keys) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/backkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/backkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001Backkey) (Elektra * elektra,
						    Lcdm001Keys value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/backkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumLcdm001Keys) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcdm001/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Lcdm001Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcdm001/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Lcdm001File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcdm001/#/forwardkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/forwardkey'.
 */// 
static inline Lcdm001Keys ELEKTRA_GET (Lcdm001Forwardkey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/forwardkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	Lcdm001Keys result = ELEKTRA_GET (EnumLcdm001Keys) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/forwardkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/forwardkey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001Forwardkey) (Elektra * elektra,
						    Lcdm001Keys value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/forwardkey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumLcdm001Keys) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcdm001/#/mainmenukey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/mainmenukey'.
 */// 
static inline Lcdm001Keys ELEKTRA_GET (Lcdm001Mainmenukey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/mainmenukey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	Lcdm001Keys result = ELEKTRA_GET (EnumLcdm001Keys) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/mainmenukey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/mainmenukey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001Mainmenukey) (Elektra * elektra,
						    Lcdm001Keys value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/mainmenukey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumLcdm001Keys) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcdm001/#/pausekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcdm001/#/pausekey'.
 */// 
static inline Lcdm001Keys ELEKTRA_GET (Lcdm001Pausekey) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/pausekey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	Lcdm001Keys result = ELEKTRA_GET (EnumLcdm001Keys) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcdm001/#/pausekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcdm001/#/pausekey'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcdm001Pausekey) (Elektra * elektra,
						    Lcdm001Keys value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcdm001/%*.*s%lld/pausekey",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumLcdm001Keys) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'lcterm/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Lcterm) (Elektra * elektra, LctermDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLctermDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'lcterm/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lcterm) (Elektra * elektra, const LctermDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLctermDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'lcterm/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Lcterm) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "lcterm");
}



/**
 * Get the value of 'lcterm/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LctermBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LctermContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/device'.
 */// 
static inline const char * ELEKTRA_GET (LctermDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/file'.
 */// 
static inline const char * ELEKTRA_GET (LctermFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LctermOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (LctermReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lcterm/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lcterm/#/size'.
 */// 
static inline const char * ELEKTRA_GET (LctermSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lcterm/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lcterm/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LctermSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lcterm/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'linux_input/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (LinuxInput) (Elektra * elektra, Linux_inputDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLinux_inputDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'linux_input/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInput) (Elektra * elektra, const Linux_inputDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLinux_inputDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'linux_input/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (LinuxInput) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "linux_input");
}



/**
 * Get the value of 'linux_input/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LinuxInputBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LinuxInputContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/device'.
 */// 
static inline const char * ELEKTRA_GET (LinuxInputDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/file'.
 */// 
static inline const char * ELEKTRA_GET (LinuxInputFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/key/_'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param name1 Replaces occurence no. 1 of _ in the keyname.
 *
 * @return the value of 'linux_input/#/key/_'.
 */// 
static inline const char * ELEKTRA_GET (LinuxInputKey) (Elektra * elektra ,
								     kdb_long_long_t index1 ,
								      const char * name1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/key/%s",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1 ,
				       name1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/key/_'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/key/_'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param name1 Replaces occurence no. 1 of _ in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputKey) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1, 
						    const char * name1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/key/%s",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1 ,
				       name1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LinuxInputOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'linux_input/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'linux_input/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (LinuxInputReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'linux_input/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'linux_input/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LinuxInputReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("linux_input/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'lirc/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Lirc) (Elektra * elektra, LircDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLircDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'lirc/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lirc) (Elektra * elektra, const LircDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLircDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'lirc/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Lirc) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "lirc");
}



/**
 * Get the value of 'lirc/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LircBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LircContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/file'.
 */// 
static inline const char * ELEKTRA_GET (LircFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/lircrc'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/lircrc'.
 */// 
static inline const char * ELEKTRA_GET (LircLircrc) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/lircrc",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/lircrc'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/lircrc'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircLircrc) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/lircrc",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LircOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/prog'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/prog'.
 */// 
static inline const char * ELEKTRA_GET (LircProg) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/prog",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/prog'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/prog'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircProg) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/prog",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lirc/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lirc/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (LircReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lirc/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lirc/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LircReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lirc/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'lis/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Lis) (Elektra * elektra, LisDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructLisDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'lis/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Lis) (Elektra * elektra, const LisDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructLisDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'lis/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Lis) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "lis");
}



/**
 * Get the value of 'lis/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LisBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LisContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/file'.
 */// 
static inline const char * ELEKTRA_GET (LisFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/lastline'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (LisLastline) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/lastline'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/lastline'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisLastline) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/lastline",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (LisOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/productid'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/productid'.
 */// 
static inline const char * ELEKTRA_GET (LisProductid) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/productid",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/productid'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/productid'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisProductid) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/productid",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (LisReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/size'.
 */// 
static inline const char * ELEKTRA_GET (LisSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'lis/#/vendorid'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'lis/#/vendorid'.
 */// 
static inline const char * ELEKTRA_GET (LisVendorid) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/vendorid",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'lis/#/vendorid'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'lis/#/vendorid'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (LisVendorid) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("lis/%*.*s%lld/vendorid",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'md8800/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Md8800) (Elektra * elektra, MD8800DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMD8800DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'md8800/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800) (Elektra * elektra, const MD8800DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMD8800DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'md8800/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Md8800) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "md8800");
}



/**
 * Get the value of 'md8800/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Md8800Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Md8800Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Md8800Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Md8800File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Md8800Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Md8800Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'md8800/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'md8800/#/size'.
 */// 
static inline const char * ELEKTRA_GET (Md8800Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'md8800/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'md8800/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Md8800Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("md8800/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'mdm166a/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Mdm166a) (Elektra * elektra, Mdm166aDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMdm166aDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'mdm166a/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166a) (Elektra * elektra, const Mdm166aDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMdm166aDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'mdm166a/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Mdm166a) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "mdm166a");
}



/**
 * Get the value of 'mdm166a/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mdm166aBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/clock'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/clock'.
 */// 
static inline ElektraEnumMdm166aClock ELEKTRA_GET (Mdm166aClock) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/clock",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumMdm166aClock result = ELEKTRA_GET (EnumMdm166aClock) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/clock'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/clock'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aClock) (Elektra * elektra,
						    ElektraEnumMdm166aClock value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/clock",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumMdm166aClock) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mdm166aContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/dimming'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/dimming'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Mdm166aDimming) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/dimming",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/dimming'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/dimming'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aDimming) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/dimming",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Mdm166aFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mdm166aOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/offdimming'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/offdimming'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Mdm166aOffdimming) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/offdimming",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/offdimming'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/offdimming'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aOffdimming) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/offdimming",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mdm166a/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mdm166a/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Mdm166aReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mdm166a/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mdm166a/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mdm166aReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mdm166a/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'menu/downkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/downkey'.
 */// 
static inline const char * ELEKTRA_GET (MenuDownkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/downkey");
}


/**
 * Set the value of 'menu/downkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/downkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuDownkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/downkey", value, error);
}




/**
 * Get the value of 'menu/enterkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/enterkey'.
 */// 
static inline const char * ELEKTRA_GET (MenuEnterkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/enterkey");
}


/**
 * Set the value of 'menu/enterkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/enterkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuEnterkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/enterkey", value, error);
}




/**
 * Get the value of 'menu/leftkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/leftkey'.
 */// 
static inline const char * ELEKTRA_GET (MenuLeftkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/leftkey");
}


/**
 * Set the value of 'menu/leftkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/leftkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuLeftkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/leftkey", value, error);
}




/**
 * Get the value of 'menu/menukey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/menukey'.
 */// 
static inline const char * ELEKTRA_GET (MenuMenukey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/menukey");
}


/**
 * Set the value of 'menu/menukey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/menukey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuMenukey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/menukey", value, error);
}




/**
 * Get the value of 'menu/permissivegoto'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/permissivegoto'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (MenuPermissivegoto) (Elektra * elektra )
{
	
	return ELEKTRA_GET (Boolean) (elektra, "menu/permissivegoto");
}


/**
 * Set the value of 'menu/permissivegoto'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/permissivegoto'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuPermissivegoto) (Elektra * elektra,
						    kdb_boolean_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (Boolean) (elektra, "menu/permissivegoto", value, error);
}




/**
 * Get the value of 'menu/rightkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/rightkey'.
 */// 
static inline const char * ELEKTRA_GET (MenuRightkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/rightkey");
}


/**
 * Set the value of 'menu/rightkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/rightkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuRightkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/rightkey", value, error);
}




/**
 * Get the value of 'menu/upkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'menu/upkey'.
 */// 
static inline const char * ELEKTRA_GET (MenuUpkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "menu/upkey");
}


/**
 * Set the value of 'menu/upkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'menu/upkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MenuUpkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "menu/upkey", value, error);
}

// clang-format off

// clang-format on



/**
 * Get the value of 'ms6931/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Ms6931) (Elektra * elektra, Ms6931DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMs6931DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'ms6931/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931) (Elektra * elektra, const Ms6931DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMs6931DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'ms6931/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Ms6931) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "ms6931");
}



/**
 * Get the value of 'ms6931/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ms6931Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ms6931Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Ms6931Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Ms6931File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ms6931Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Ms6931Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ms6931/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ms6931/#/size'.
 */// 
static inline const char * ELEKTRA_GET (Ms6931Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ms6931/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ms6931/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ms6931Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ms6931/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'mtc_s16209x/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (MtcS16209x) (Elektra * elektra, Mtc_s16209xDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMtc_s16209xDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'mtc_s16209x/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209x) (Elektra * elektra, const Mtc_s16209xDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMtc_s16209xDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'mtc_s16209x/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (MtcS16209x) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "mtc_s16209x");
}



/**
 * Get the value of 'mtc_s16209x/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtcS16209xBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtc_s16209x/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtcS16209xContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtc_s16209x/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/device'.
 */// 
static inline const char * ELEKTRA_GET (MtcS16209xDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtc_s16209x/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/file'.
 */// 
static inline const char * ELEKTRA_GET (MtcS16209xFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtc_s16209x/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtcS16209xOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtc_s16209x/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtc_s16209x/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (MtcS16209xReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtc_s16209x/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtc_s16209x/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtcS16209xReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtc_s16209x/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'mtxorb/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Mtxorb) (Elektra * elektra, MtxOrbDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMtxOrbDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'mtxorb/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mtxorb) (Elektra * elektra, const MtxOrbDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMtxOrbDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'mtxorb/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Mtxorb) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "mtxorb");
}



/**
 * Get the value of 'mtxorb/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtxorbBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtxorbContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/device'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/file'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/hasadjustablebacklight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/hasadjustablebacklight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (MtxorbHasadjustablebacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/hasadjustablebacklight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/hasadjustablebacklight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/hasadjustablebacklight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbHasadjustablebacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/hasadjustablebacklight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_a'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapA) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_a'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapA) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_b'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapB) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_b'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapB) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_c'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapC) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_c'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapC) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_d'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapD) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_d'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapD) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_e'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapE) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_e'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapE) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keymap_f'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbKeymapF) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keymap_f'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeymapF) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/keypad_test_mode'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (MtxorbKeypadTestMode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/keypad_test_mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/keypad_test_mode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbKeypadTestMode) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/keypad_test_mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtxorbOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/size'.
 */// 
static inline const char * ELEKTRA_GET (MtxorbSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (MtxorbSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mtxorb/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mtxorb/#/type'.
 */// 
static inline ElektraEnumMtxorbType ELEKTRA_GET (MtxorbType) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumMtxorbType result = ELEKTRA_GET (EnumMtxorbType) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mtxorb/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mtxorb/#/type'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (MtxorbType) (Elektra * elektra,
						    ElektraEnumMtxorbType value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mtxorb/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumMtxorbType) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'mx5000/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Mx5000) (Elektra * elektra, Mx5000DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructMx5000DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'mx5000/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000) (Elektra * elektra, const Mx5000DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructMx5000DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'mx5000/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Mx5000) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "mx5000");
}



/**
 * Get the value of 'mx5000/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mx5000Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mx5000Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/device'.
 */// 
static inline const char * ELEKTRA_GET (Mx5000Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Mx5000File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Mx5000Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Mx5000Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'mx5000/#/waitafterrefresh'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'mx5000/#/waitafterrefresh'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (Mx5000Waitafterrefresh) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/waitafterrefresh",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'mx5000/#/waitafterrefresh'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'mx5000/#/waitafterrefresh'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Mx5000Waitafterrefresh) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("mx5000/%*.*s%lld/waitafterrefresh",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'noritakevfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Noritakevfd) (Elektra * elektra, NoritakeVFDDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructNoritakeVFDDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'noritakevfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Noritakevfd) (Elektra * elektra, const NoritakeVFDDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructNoritakeVFDDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'noritakevfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Noritakevfd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "noritakevfd");
}



/**
 * Get the value of 'noritakevfd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (NoritakevfdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (NoritakevfdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/device'.
 */// 
static inline const char * ELEKTRA_GET (NoritakevfdDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (NoritakevfdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (NoritakevfdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/parity'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/parity'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (NoritakevfdParity) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/parity",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/parity'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/parity'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdParity) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/parity",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (NoritakevfdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (NoritakevfdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'noritakevfd/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'noritakevfd/#/speed'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (NoritakevfdSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'noritakevfd/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'noritakevfd/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (NoritakevfdSpeed) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("noritakevfd/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'olimex_mod_lcd1x9/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (OlimexModLcd1x9) (Elektra * elektra, Olimex_MOD_LCD1x9DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructOlimex_MOD_LCD1x9DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9) (Elektra * elektra, const Olimex_MOD_LCD1x9DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructOlimex_MOD_LCD1x9DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'olimex_mod_lcd1x9/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (OlimexModLcd1x9) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "olimex_mod_lcd1x9");
}



/**
 * Get the value of 'olimex_mod_lcd1x9/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (OlimexModLcd1x9Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'olimex_mod_lcd1x9/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (OlimexModLcd1x9Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'olimex_mod_lcd1x9/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/device'.
 */// 
static inline const char * ELEKTRA_GET (OlimexModLcd1x9Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'olimex_mod_lcd1x9/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/file'.
 */// 
static inline const char * ELEKTRA_GET (OlimexModLcd1x9File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'olimex_mod_lcd1x9/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (OlimexModLcd1x9Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'olimex_mod_lcd1x9/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'olimex_mod_lcd1x9/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (OlimexModLcd1x9Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'olimex_mod_lcd1x9/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'olimex_mod_lcd1x9/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (OlimexModLcd1x9Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("olimex_mod_lcd1x9/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'picolcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Picolcd) (Elektra * elektra, PicolcdDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructPicolcdDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'picolcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Picolcd) (Elektra * elektra, const PicolcdDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructPicolcdDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'picolcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Picolcd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "picolcd");
}



/**
 * Get the value of 'picolcd/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/backlight'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdBacklight) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/backlight'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdBacklight) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/backlight",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (PicolcdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key0light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key0light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey0light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key0light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key0light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key0light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey0light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key0light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key1light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key1light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey1light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key1light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key1light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key1light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey1light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key1light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key2light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key2light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey2light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key2light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key2light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key2light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey2light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key2light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key3light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key3light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey3light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key3light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key3light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key3light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey3light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key3light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key4light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key4light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey4light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key4light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key4light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key4light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey4light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key4light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/key5light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/key5light'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKey5light) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key5light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/key5light'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/key5light'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKey5light) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/key5light",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/keylights'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/keylights'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdKeylights) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keylights",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/keylights'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/keylights'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKeylights) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keylights",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/keyrepeatdelay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/keyrepeatdelay'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdKeyrepeatdelay) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keyrepeatdelay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/keyrepeatdelay'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/keyrepeatdelay'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKeyrepeatdelay) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keyrepeatdelay",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/keyrepeatinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/keyrepeatinterval'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdKeyrepeatinterval) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keyrepeatinterval",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/keyrepeatinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/keyrepeatinterval'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKeyrepeatinterval) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keyrepeatinterval",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/keytimeout'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/keytimeout'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdKeytimeout) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keytimeout",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/keytimeout'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/keytimeout'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdKeytimeout) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/keytimeout",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/linklights'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/linklights'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdLinklights) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/linklights",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/linklights'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/linklights'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdLinklights) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/linklights",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/lircflushthreshold'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/lircflushthreshold'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (PicolcdLircflushthreshold) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lircflushthreshold",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/lircflushthreshold'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/lircflushthreshold'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdLircflushthreshold) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lircflushthreshold",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/lirchost'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/lirchost'.
 */// 
static inline const char * ELEKTRA_GET (PicolcdLirchost) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lirchost",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/lirchost'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/lirchost'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdLirchost) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lirchost",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/lircport'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/lircport'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdLircport) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lircport",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/lircport'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/lircport'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdLircport) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lircport",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/lirctime_us'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/lirctime_us'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdLirctimeUs) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lirctime_us",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/lirctime_us'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/lirctime_us'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdLirctimeUs) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/lirctime_us",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (PicolcdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'picolcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'picolcd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (PicolcdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'picolcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'picolcd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PicolcdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("picolcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'pyramid/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Pyramid) (Elektra * elektra, PyramidDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructPyramidDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'pyramid/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'pyramid/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Pyramid) (Elektra * elektra, const PyramidDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructPyramidDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'pyramid/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Pyramid) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "pyramid");
}



/**
 * Get the value of 'pyramid/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'pyramid/#/device'.
 */// 
static inline const char * ELEKTRA_GET (PyramidDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'pyramid/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'pyramid/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PyramidDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'pyramid/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'pyramid/#/file'.
 */// 
static inline const char * ELEKTRA_GET (PyramidFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'pyramid/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'pyramid/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (PyramidFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("pyramid/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'rawserial/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Rawserial) (Elektra * elektra, RawserialDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructRawserialDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'rawserial/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Rawserial) (Elektra * elektra, const RawserialDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructRawserialDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'rawserial/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Rawserial) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "rawserial");
}



/**
 * Get the value of 'rawserial/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (RawserialBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (RawserialContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/device'.
 */// 
static inline const char * ELEKTRA_GET (RawserialDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/file'.
 */// 
static inline const char * ELEKTRA_GET (RawserialFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (RawserialOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (RawserialReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/size'.
 */// 
static inline const char * ELEKTRA_GET (RawserialSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/speed'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (RawserialSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialSpeed) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'rawserial/#/updaterate'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'rawserial/#/updaterate'.
 */// 
static inline kdb_float_t ELEKTRA_GET (RawserialUpdaterate) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/updaterate",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_float_t result = ELEKTRA_GET (Float) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'rawserial/#/updaterate'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'rawserial/#/updaterate'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (RawserialUpdaterate) (Elektra * elektra,
						    kdb_float_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("rawserial/%*.*s%lld/updaterate",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Float) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'sed1330/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Sed1330) (Elektra * elektra, Sed1330DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSed1330DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'sed1330/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330) (Elektra * elektra, const Sed1330DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSed1330DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'sed1330/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Sed1330) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "sed1330");
}



/**
 * Get the value of 'sed1330/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1330Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/cellsize'.
 */// 
static inline const char * ELEKTRA_GET (Sed1330Cellsize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/cellsize'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Cellsize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/connectiontype'.
 */// 
static inline ElektraEnumSed1330Connectiontype ELEKTRA_GET (Sed1330Connectiontype) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumSed1330Connectiontype result = ELEKTRA_GET (EnumSed1330Connectiontype) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/connectiontype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/connectiontype'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Connectiontype) (Elektra * elektra,
						    ElektraEnumSed1330Connectiontype value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/connectiontype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumSed1330Connectiontype) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1330Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Sed1330File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1330Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/port'.
 */// 
static inline const char * ELEKTRA_GET (Sed1330Port) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Port) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Sed1330Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1330/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1330/#/type'.
 */// 
static inline ElektraEnumSed1330Type ELEKTRA_GET (Sed1330Type) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumSed1330Type result = ELEKTRA_GET (EnumSed1330Type) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1330/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1330/#/type'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1330Type) (Elektra * elektra,
						    ElektraEnumSed1330Type value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1330/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumSed1330Type) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'sed1520/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Sed1520) (Elektra * elektra, Sed1520DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSed1520DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'sed1520/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520) (Elektra * elektra, const Sed1520DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSed1520DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'sed1520/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Sed1520) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "sed1520");
}



/**
 * Get the value of 'sed1520/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1520Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1520Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/delaymult'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/delaymult'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1520Delaymult) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/delaymult",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/delaymult'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/delaymult'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Delaymult) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/delaymult",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Sed1520File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/haveinverter'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/haveinverter'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Sed1520Haveinverter) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/haveinverter",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/haveinverter'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/haveinverter'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Haveinverter) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/haveinverter",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/interfacetype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/interfacetype'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1520Interfacetype) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/interfacetype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/interfacetype'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/interfacetype'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Interfacetype) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/interfacetype",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/invertedmapping'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/invertedmapping'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Sed1520Invertedmapping) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/invertedmapping",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/invertedmapping'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/invertedmapping'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Invertedmapping) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/invertedmapping",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Sed1520Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/port'.
 */// 
static inline const char * ELEKTRA_GET (Sed1520Port) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Port) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Sed1520Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sed1520/#/usehardreset'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sed1520/#/usehardreset'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Sed1520Usehardreset) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/usehardreset",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sed1520/#/usehardreset'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sed1520/#/usehardreset'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sed1520Usehardreset) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sed1520/%*.*s%lld/usehardreset",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'serialpos/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Serialpos) (Elektra * elektra, SerialPOSDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSerialPOSDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'serialpos/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Serialpos) (Elektra * elektra, const SerialPOSDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSerialPOSDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'serialpos/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Serialpos) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "serialpos");
}



/**
 * Get the value of 'serialpos/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialposBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/cellsize'.
 */// 
static inline const char * ELEKTRA_GET (SerialposCellsize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/cellsize'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/cellsize'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposCellsize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/cellsize",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialposContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/custom_chars'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/custom_chars'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (SerialposCustomChars) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/custom_chars",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/custom_chars'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/custom_chars'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposCustomChars) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/custom_chars",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/device'.
 */// 
static inline const char * ELEKTRA_GET (SerialposDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/file'.
 */// 
static inline const char * ELEKTRA_GET (SerialposFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialposOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SerialposReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/size'.
 */// 
static inline const char * ELEKTRA_GET (SerialposSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialposSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialpos/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialpos/#/type'.
 */// 
static inline ElektraEnumSerialposType ELEKTRA_GET (SerialposType) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ElektraEnumSerialposType result = ELEKTRA_GET (EnumSerialposType) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialpos/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialpos/#/type'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialposType) (Elektra * elektra,
						    ElektraEnumSerialposType value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialpos/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (EnumSerialposType) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'serialvfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Serialvfd) (Elektra * elektra, SerialVFDDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSerialVFDDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'serialvfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Serialvfd) (Elektra * elektra, const SerialVFDDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSerialVFDDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'serialvfd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Serialvfd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "serialvfd");
}



/**
 * Get the value of 'serialvfd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/custom-characters'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/custom-characters'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (SerialvfdCustomCharacters) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/custom-characters",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_long_t result = ELEKTRA_GET (UnsignedLong) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/custom-characters'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/custom-characters'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdCustomCharacters) (Elektra * elektra,
						    kdb_unsigned_long_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/custom-characters",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedLong) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/device'.
 */// 
static inline const char * ELEKTRA_GET (SerialvfdDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (SerialvfdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/iso_8859_1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/iso_8859_1'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SerialvfdIso88591) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/iso_8859_1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/iso_8859_1'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/iso_8859_1'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdIso88591) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/iso_8859_1",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/port'.
 */// 
static inline const char * ELEKTRA_GET (SerialvfdPort) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdPort) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/portwait'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/portwait'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdPortwait) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/portwait",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/portwait'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/portwait'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdPortwait) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/portwait",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SerialvfdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (SerialvfdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/type'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SerialvfdType) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/type'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/type'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdType) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/type",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'serialvfd/#/use_parallel'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'serialvfd/#/use_parallel'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SerialvfdUseParallel) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/use_parallel",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'serialvfd/#/use_parallel'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'serialvfd/#/use_parallel'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SerialvfdUseParallel) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("serialvfd/%*.*s%lld/use_parallel",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'server/autorotate'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/autorotate'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ServerAutorotate) (Elektra * elektra )
{
	
	return ELEKTRA_GET (Boolean) (elektra, "server/autorotate");
}


/**
 * Set the value of 'server/autorotate'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/autorotate'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerAutorotate) (Elektra * elektra,
						    kdb_boolean_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (Boolean) (elektra, "server/autorotate", value, error);
}




/**
 * Get the value of 'server/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/backlight'.
 */// 
static inline ElektraEnumServerBacklight ELEKTRA_GET (ServerBacklight) (Elektra * elektra )
{
	
	return ELEKTRA_GET (EnumServerBacklight) (elektra, "server/backlight");
}


/**
 * Set the value of 'server/backlight'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/backlight'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerBacklight) (Elektra * elektra,
						    ElektraEnumServerBacklight value,  ElektraError ** error)
{
	
	ELEKTRA_SET (EnumServerBacklight) (elektra, "server/backlight", value, error);
}




/**
 * Get the value of 'server/bind'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/bind'.
 */// 
static inline const char * ELEKTRA_GET (ServerBind) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/bind");
}


/**
 * Set the value of 'server/bind'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/bind'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerBind) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/bind", value, error);
}




/**
 * Get the value of 'server/driverpath'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/driverpath'.
 */// 
static inline const char * ELEKTRA_GET (ServerDriverpath) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/driverpath");
}


/**
 * Set the value of 'server/driverpath'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/driverpath'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerDriverpath) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/driverpath", value, error);
}




/**
 * Get the value of 'server/drivers/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'server/drivers/#'.
 */// 
static inline const char * ELEKTRA_GET (ServerDrivers) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("server/drivers/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'server/drivers/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/drivers/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerDrivers) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("server/drivers/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

/**
 * Get the size of the array 'server/drivers/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (ServerDrivers) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "server/drivers");
}



/**
 * Get the value of 'server/foreground'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/foreground'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ServerForeground) (Elektra * elektra )
{
	
	return ELEKTRA_GET (Boolean) (elektra, "server/foreground");
}


/**
 * Set the value of 'server/foreground'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/foreground'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerForeground) (Elektra * elektra,
						    kdb_boolean_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (Boolean) (elektra, "server/foreground", value, error);
}




/**
 * Get the value of 'server/frameinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/frameinterval'.
 */// 
static inline kdb_unsigned_long_t ELEKTRA_GET (ServerFrameinterval) (Elektra * elektra )
{
	
	return ELEKTRA_GET (UnsignedLong) (elektra, "server/frameinterval");
}


/**
 * Set the value of 'server/frameinterval'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/frameinterval'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerFrameinterval) (Elektra * elektra,
						    kdb_unsigned_long_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (UnsignedLong) (elektra, "server/frameinterval", value, error);
}




/**
 * Get the value of 'server/goodbye/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'server/goodbye/#'.
 */// 
static inline const char * ELEKTRA_GET (ServerGoodbye) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("server/goodbye/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'server/goodbye/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/goodbye/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerGoodbye) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("server/goodbye/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

/**
 * Get the size of the array 'server/goodbye/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (ServerGoodbye) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "server/goodbye");
}



/**
 * Get the value of 'server/heartbeat'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/heartbeat'.
 */// 
static inline ElektraEnumServerHeartbeat ELEKTRA_GET (ServerHeartbeat) (Elektra * elektra )
{
	
	return ELEKTRA_GET (EnumServerHeartbeat) (elektra, "server/heartbeat");
}


/**
 * Set the value of 'server/heartbeat'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/heartbeat'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerHeartbeat) (Elektra * elektra,
						    ElektraEnumServerHeartbeat value,  ElektraError ** error)
{
	
	ELEKTRA_SET (EnumServerHeartbeat) (elektra, "server/heartbeat", value, error);
}




/**
 * Get the value of 'server/hello/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'server/hello/#'.
 */// 
static inline const char * ELEKTRA_GET (ServerHello) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("server/hello/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'server/hello/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/hello/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerHello) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("server/hello/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

/**
 * Get the size of the array 'server/hello/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (ServerHello) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "server/hello");
}



/**
 * Get the value of 'server/nextscreenkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/nextscreenkey'.
 */// 
static inline const char * ELEKTRA_GET (ServerNextscreenkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/nextscreenkey");
}


/**
 * Set the value of 'server/nextscreenkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/nextscreenkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerNextscreenkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/nextscreenkey", value, error);
}




/**
 * Get the value of 'server/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/port'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ServerPort) (Elektra * elektra )
{
	
	return ELEKTRA_GET (UnsignedShort) (elektra, "server/port");
}


/**
 * Set the value of 'server/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/port'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerPort) (Elektra * elektra,
						    kdb_unsigned_short_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (UnsignedShort) (elektra, "server/port", value, error);
}




/**
 * Get the value of 'server/prevscreenkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/prevscreenkey'.
 */// 
static inline const char * ELEKTRA_GET (ServerPrevscreenkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/prevscreenkey");
}


/**
 * Set the value of 'server/prevscreenkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/prevscreenkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerPrevscreenkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/prevscreenkey", value, error);
}




/**
 * Get the value of 'server/reportlevel'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/reportlevel'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ServerReportlevel) (Elektra * elektra )
{
	
	return ELEKTRA_GET (UnsignedShort) (elektra, "server/reportlevel");
}


/**
 * Set the value of 'server/reportlevel'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/reportlevel'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerReportlevel) (Elektra * elektra,
						    kdb_unsigned_short_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (UnsignedShort) (elektra, "server/reportlevel", value, error);
}




/**
 * Get the value of 'server/reporttosyslog'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/reporttosyslog'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (ServerReporttosyslog) (Elektra * elektra )
{
	
	return ELEKTRA_GET (Boolean) (elektra, "server/reporttosyslog");
}


/**
 * Set the value of 'server/reporttosyslog'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/reporttosyslog'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerReporttosyslog) (Elektra * elektra,
						    kdb_boolean_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (Boolean) (elektra, "server/reporttosyslog", value, error);
}




/**
 * Get the value of 'server/scrolldownkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/scrolldownkey'.
 */// 
static inline const char * ELEKTRA_GET (ServerScrolldownkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/scrolldownkey");
}


/**
 * Set the value of 'server/scrolldownkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/scrolldownkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerScrolldownkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/scrolldownkey", value, error);
}




/**
 * Get the value of 'server/scrollupkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/scrollupkey'.
 */// 
static inline const char * ELEKTRA_GET (ServerScrollupkey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/scrollupkey");
}


/**
 * Set the value of 'server/scrollupkey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/scrollupkey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerScrollupkey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/scrollupkey", value, error);
}




/**
 * Get the value of 'server/serverscreen'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/serverscreen'.
 */// 
static inline ElektraEnumServerServerscreen ELEKTRA_GET (ServerServerscreen) (Elektra * elektra )
{
	
	return ELEKTRA_GET (EnumServerServerscreen) (elektra, "server/serverscreen");
}


/**
 * Set the value of 'server/serverscreen'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/serverscreen'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerServerscreen) (Elektra * elektra,
						    ElektraEnumServerServerscreen value,  ElektraError ** error)
{
	
	ELEKTRA_SET (EnumServerServerscreen) (elektra, "server/serverscreen", value, error);
}




/**
 * Get the value of 'server/titlespeed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/titlespeed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (ServerTitlespeed) (Elektra * elektra )
{
	
	return ELEKTRA_GET (UnsignedShort) (elektra, "server/titlespeed");
}


/**
 * Set the value of 'server/titlespeed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/titlespeed'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerTitlespeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (UnsignedShort) (elektra, "server/titlespeed", value, error);
}




/**
 * Get the value of 'server/togglerotatekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/togglerotatekey'.
 */// 
static inline const char * ELEKTRA_GET (ServerTogglerotatekey) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/togglerotatekey");
}


/**
 * Set the value of 'server/togglerotatekey'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/togglerotatekey'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerTogglerotatekey) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/togglerotatekey", value, error);
}




/**
 * Get the value of 'server/user'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/user'.
 */// 
static inline const char * ELEKTRA_GET (ServerUser) (Elektra * elektra )
{
	
	return ELEKTRA_GET (String) (elektra, "server/user");
}


/**
 * Set the value of 'server/user'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/user'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerUser) (Elektra * elektra,
						    const char * value,  ElektraError ** error)
{
	
	ELEKTRA_SET (String) (elektra, "server/user", value, error);
}




/**
 * Get the value of 'server/waittime'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 *
 * @return the value of 'server/waittime'.
 */// 
static inline kdb_float_t ELEKTRA_GET (ServerWaittime) (Elektra * elektra )
{
	
	return ELEKTRA_GET (Float) (elektra, "server/waittime");
}


/**
 * Set the value of 'server/waittime'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'server/waittime'.

 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (ServerWaittime) (Elektra * elektra,
						    kdb_float_t value,  ElektraError ** error)
{
	
	ELEKTRA_SET (Float) (elektra, "server/waittime", value, error);
}

// clang-format off

// clang-format on



/**
 * Get the value of 'sli/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Sli) (Elektra * elektra, SliDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSliDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'sli/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sli) (Elektra * elektra, const SliDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSliDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'sli/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Sli) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "sli");
}



/**
 * Get the value of 'sli/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SliBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SliContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/device'.
 */// 
static inline const char * ELEKTRA_GET (SliDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/file'.
 */// 
static inline const char * ELEKTRA_GET (SliFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SliOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SliReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sli/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sli/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SliSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sli/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sli/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SliSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sli/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'stv5730/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Stv5730) (Elektra * elektra, Stv5730DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructStv5730DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'stv5730/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730) (Elektra * elektra, const Stv5730DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructStv5730DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'stv5730/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Stv5730) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "stv5730");
}



/**
 * Get the value of 'stv5730/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Stv5730Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'stv5730/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Stv5730Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'stv5730/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Stv5730File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'stv5730/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Stv5730Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'stv5730/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/port'.
 */// 
static inline const char * ELEKTRA_GET (Stv5730Port) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730Port) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'stv5730/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'stv5730/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Stv5730Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'stv5730/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'stv5730/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Stv5730Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("stv5730/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'sureelec/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Sureelec) (Elektra * elektra, SureElecDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSureElecDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'sureelec/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Sureelec) (Elektra * elektra, const SureElecDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSureElecDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'sureelec/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Sureelec) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "sureelec");
}



/**
 * Get the value of 'sureelec/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SureelecBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SureelecContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/device'.
 */// 
static inline const char * ELEKTRA_GET (SureelecDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/edition'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/edition'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SureelecEdition) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/edition",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/edition'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/edition'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecEdition) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/edition",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/file'.
 */// 
static inline const char * ELEKTRA_GET (SureelecFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SureelecOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SureelecReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'sureelec/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'sureelec/#/size'.
 */// 
static inline const char * ELEKTRA_GET (SureelecSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'sureelec/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'sureelec/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SureelecSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("sureelec/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'svga/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Svga) (Elektra * elektra, SvgaDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructSvgaDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'svga/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Svga) (Elektra * elektra, const SvgaDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructSvgaDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'svga/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Svga) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "svga");
}



/**
 * Get the value of 'svga/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SvgaBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SvgaContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/file'.
 */// 
static inline const char * ELEKTRA_GET (SvgaFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/mode'.
 */// 
static inline const char * ELEKTRA_GET (SvgaMode) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/mode'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/mode'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaMode) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/mode",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (SvgaOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (SvgaReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'svga/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'svga/#/size'.
 */// 
static inline const char * ELEKTRA_GET (SvgaSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'svga/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'svga/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (SvgaSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("svga/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 't6963/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (T6963) (Elektra * elektra, T6963DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructT6963DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 't6963/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963) (Elektra * elektra, const T6963DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructT6963DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 't6963/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (T6963) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "t6963");
}



/**
 * Get the value of 't6963/#/bidirectional'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/bidirectional'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (T6963Bidirectional) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/bidirectional",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/bidirectional'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/bidirectional'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Bidirectional) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/bidirectional",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (T6963Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/cleargraphic'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/cleargraphic'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (T6963Cleargraphic) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/cleargraphic",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/cleargraphic'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/cleargraphic'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Cleargraphic) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/cleargraphic",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (T6963Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/delaybus'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (T6963Delaybus) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/delaybus'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/delaybus'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Delaybus) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/delaybus",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/file'.
 */// 
static inline const char * ELEKTRA_GET (T6963File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (T6963Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/port'.
 */// 
static inline const char * ELEKTRA_GET (T6963Port) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/port'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/port'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Port) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/port",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (T6963Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 't6963/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 't6963/#/size'.
 */// 
static inline const char * ELEKTRA_GET (T6963Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 't6963/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 't6963/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (T6963Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("t6963/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'text/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Text) (Elektra * elektra, TextDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("text/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructTextDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'text/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Text) (Elektra * elektra, const TextDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructTextDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'text/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Text) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "text");
}



/**
 * Get the value of 'text/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TextBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'text/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TextContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'text/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/file'.
 */// 
static inline const char * ELEKTRA_GET (TextFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'text/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TextOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'text/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (TextReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'text/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'text/#/size'.
 */// 
static inline const char * ELEKTRA_GET (TextSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("text/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'text/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'text/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TextSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("text/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'tyan/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Tyan) (Elektra * elektra, TyanDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructTyanDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'tyan/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Tyan) (Elektra * elektra, const TyanDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructTyanDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'tyan/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Tyan) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "tyan");
}



/**
 * Get the value of 'tyan/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TyanBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TyanContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/device'.
 */// 
static inline const char * ELEKTRA_GET (TyanDevice) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanDevice) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/file'.
 */// 
static inline const char * ELEKTRA_GET (TyanFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TyanOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (TyanReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/size'.
 */// 
static inline const char * ELEKTRA_GET (TyanSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'tyan/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'tyan/#/speed'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (TyanSpeed) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'tyan/#/speed'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'tyan/#/speed'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (TyanSpeed) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("tyan/%*.*s%lld/speed",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'ula200/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Ula200) (Elektra * elektra, Ula200DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructUla200DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'ula200/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200) (Elektra * elektra, const Ula200DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructUla200DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'ula200/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Ula200) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "ula200");
}



/**
 * Get the value of 'ula200/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ula200Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ula200Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Ula200File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_a'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapA) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_a'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_a'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapA) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_a",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_b'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapB) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_b'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_b'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapB) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_b",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_c'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapC) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_c'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_c'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapC) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_c",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_d'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapD) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_d'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_d'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapD) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_d",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_e'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapE) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_e'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_e'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapE) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_e",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/keymap_f'.
 */// 
static inline const char * ELEKTRA_GET (Ula200KeymapF) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/keymap_f'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/keymap_f'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200KeymapF) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/keymap_f",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Ula200Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Ula200Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'ula200/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'ula200/#/size'.
 */// 
static inline const char * ELEKTRA_GET (Ula200Size) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'ula200/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'ula200/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Ula200Size) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("ula200/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'vlsys_m428/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (VlsysM428) (Elektra * elektra, Vlsys_m428DriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructVlsys_m428DriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'vlsys_m428/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428) (Elektra * elektra, const Vlsys_m428DriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructVlsys_m428DriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'vlsys_m428/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (VlsysM428) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "vlsys_m428");
}



/**
 * Get the value of 'vlsys_m428/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (VlsysM428Brightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428Brightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'vlsys_m428/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (VlsysM428Contrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428Contrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'vlsys_m428/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/device'.
 */// 
static inline const char * ELEKTRA_GET (VlsysM428Device) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/device'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/device'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428Device) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/device",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'vlsys_m428/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/file'.
 */// 
static inline const char * ELEKTRA_GET (VlsysM428File) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428File) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'vlsys_m428/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (VlsysM428Offbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428Offbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'vlsys_m428/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'vlsys_m428/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (VlsysM428Reboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'vlsys_m428/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'vlsys_m428/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (VlsysM428Reboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("vlsys_m428/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'xosd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Xosd) (Elektra * elektra, XosdDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructXosdDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'xosd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Xosd) (Elektra * elektra, const XosdDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructXosdDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'xosd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Xosd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "xosd");
}



/**
 * Get the value of 'xosd/#/Font'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/Font'.
 */// 
static inline const char * ELEKTRA_GET (XosdFont) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/Font",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/Font'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/Font'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdFont) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/Font",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (XosdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (XosdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (XosdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (XosdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/offset'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/offset'.
 */// 
static inline const char * ELEKTRA_GET (XosdOffset) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/offset",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/offset'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/offset'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdOffset) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/offset",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (XosdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'xosd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'xosd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (XosdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'xosd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'xosd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (XosdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("xosd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}

// clang-format off

// clang-format on



/**
 * Get the value of 'yard2lcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param result  The value will be stored in the referenced variable.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 */// 
static inline void ELEKTRA_GET (Yard2lcd) (Elektra * elektra, Yard2LCDDriverConfig *result ,
						     kdb_long_long_t index1 
						     )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_GET (StructYard2LCDDriverConfig) (elektra, name, result);
	elektraFree (name);
	
}


/**
 * Set the value of 'yard2lcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcd) (Elektra * elektra, const Yard2LCDDriverConfig * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (StructYard2LCDDriverConfig) (elektra, name, value, error);
	elektraFree (name);
	
}



/**
 * Get the size of the array 'yard2lcd/#'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().

 */// 
static inline kdb_long_long_t ELEKTRA_SIZE (Yard2lcd) (Elektra * elektra )
{
	
	return elektraArraySize (elektra, "yard2lcd");
}



/**
 * Get the value of 'yard2lcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/brightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Yard2lcdBrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/brightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/brightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdBrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/brightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'yard2lcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/contrast'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Yard2lcdContrast) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/contrast'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/contrast'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdContrast) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/contrast",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'yard2lcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/file'.
 */// 
static inline const char * ELEKTRA_GET (Yard2lcdFile) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/file'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/file'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdFile) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/file",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'yard2lcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/offbrightness'.
 */// 
static inline kdb_unsigned_short_t ELEKTRA_GET (Yard2lcdOffbrightness) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_unsigned_short_t result = ELEKTRA_GET (UnsignedShort) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/offbrightness'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/offbrightness'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdOffbrightness) (Elektra * elektra,
						    kdb_unsigned_short_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/offbrightness",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (UnsignedShort) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'yard2lcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/reboot'.
 */// 
static inline kdb_boolean_t ELEKTRA_GET (Yard2lcdReboot) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	kdb_boolean_t result = ELEKTRA_GET (Boolean) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/reboot'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/reboot'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdReboot) (Elektra * elektra,
						    kdb_boolean_t value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/reboot",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (Boolean) (elektra, name, value, error);
	elektraFree (name);
	
}




/**
 * Get the value of 'yard2lcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 *
 * @return the value of 'yard2lcd/#/size'.
 */// 
static inline const char * ELEKTRA_GET (Yard2lcdSize) (Elektra * elektra ,
								     kdb_long_long_t index1   )
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	const char * result = ELEKTRA_GET (String) (elektra, name);
	elektraFree (name);
	return result;
	
}


/**
 * Set the value of 'yard2lcd/#/size'.
 *
 * @param elektra Instance of Elektra. Create with loadConfiguration().
 * @param value   The value of 'yard2lcd/#/size'.
 * @param index1 Replaces occurence no. 1 of # in the keyname.
 * @param error   Pass a reference to an ElektraError pointer.
 *                Will only be set in case of an error.
 */// 
static inline void ELEKTRA_SET (Yard2lcdSize) (Elektra * elektra,
						    const char * value,  
						    kdb_long_long_t index1,   ElektraError ** error)
{
	

	char * name = elektraFormat ("yard2lcd/%*.*s%lld/size",  elektra_len (index1), elektra_len (index1), "#___________________", (long long) index1  );
	ELEKTRA_SET (String) (elektra, name, value, error);
	elektraFree (name);
	
}


#undef elektra_len19
#undef elektra_len18
#undef elektra_len17
#undef elektra_len16
#undef elektra_len15
#undef elektra_len14
#undef elektra_len13
#undef elektra_len12
#undef elektra_len11
#undef elektra_len10
#undef elektra_len09
#undef elektra_len08
#undef elektra_len07
#undef elektra_len06
#undef elektra_len05
#undef elektra_len04
#undef elektra_len03
#undef elektra_len02
#undef elektra_len01
#undef elektra_len00
#undef elektra_len


int loadConfiguration (Elektra ** elektra, ElektraError ** error);
void printHelpMessage (const char * usage, const char * prefix);
void doSpecloadCheck (int argc, const char ** argv);


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param tag     The tag to look up.
 *
 * @return The value stored at the given key.
 */// 
#define elektraGet(elektra, tag) ELEKTRA_GET (tag) (elektra)


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param tag     The tag to look up.
 * @param ...     Variable arguments depending on the given tag.
 *
 * @return The value stored at the given key.
 */// 
#define elektraGetV(elektra, tag, ...) ELEKTRA_GET (tag) (elektra, __VA_ARGS__)


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param tag     The tag to look up.
 * @param result  Points to the struct into which results will be stored.
 */// 
#define elektraGet2(elektra, result, tag) ELEKTRA_GET (tag) (elektra, result)


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param result  Points to the struct into which results will be stored.
 * @param tag     The tag to look up.
 * @param ...     Variable arguments depending on the given tag.
 */// 
#define elektraGet2V(elektra, result, tag, ...) ELEKTRA_GET (tag) (elektra, result, __VA_ARGS__)


/**
 * @param elektra The elektra instance initialized with the loadConfiguration().
 * @param tag     The tag to write to.
 * @param value   The new value.
 * @param error   Pass a reference to an ElektraError pointer.
 */// 
#define elektraSet(elektra, tag, value, error) ELEKTRA_SET (tag) (elektra, value, error)


/**
 * @param elektra The elektra instance initialized with the loadConfiguration().
 * @param tag     The tag to write to.
 * @param value   The new value.
 * @param error   Pass a reference to an ElektraError pointer.
 * @param ...     Variable arguments depending on the given tag.
 */// 
#define elektraSetV(elektra, tag, value, error, ...) ELEKTRA_SET (tag) (elektra, value, __VA_ARGS__, error)


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param tag     The array tag to look up.
 *
 * @return The size of the array below the given key.
 */// 
#define elektraSize(elektra, tag) ELEKTRA_SIZE (tag) (elektra)


/**
 * @param elektra The elektra instance initialized with loadConfiguration().
 * @param tag     The array tag to look up.
 * @param ...     Variable arguments depending on the given tag.
 *
 * @return The size of the array below the given key.
 */// 
#define elektraSizeV(elektra, tag, ...) ELEKTRA_SIZE (tag) (elektra, __VA_ARGS__)


/**
 * @param elektra    The elektra instance initialized with loadConfiguration().
 * @param contextTag The context tag for the contextual value you want to set.
 * @param value	     The actual value you want to set.
 */// 
#define elektraContextSet(elektra, contextTag, value) ELEKTRA_CONTEXT_SET (contextTag) (elektra, value)


/**
 * @param elektra    The elektra instance initialized with loadConfiguration().
 * @param contextTag The context tag for the contextual value you want to set.
 * @param value	     The actual value you want to set.
 * @param ...     Variable arguments depending on the given tag.
 */// 
#define elektraContextSetV(elektra, contextTag, value, ...) ELEKTRA_CONTEXT_SET (contextTag) (elektra, value, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // ELEKTRAGEN_H
