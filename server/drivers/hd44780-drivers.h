/** \file server/drivers/hd44780-drivers.h
 * Interface to low-level driver types, headers and names.
 *
 * To add support for a new driver in this file:
 * -# include your header file
 * -# Add a new connectionType
 * -# Add an entry in the \c ConnectionMapping structure
 */

#ifndef HD44780_DRIVERS_H
#define HD44780_DRIVERS_H

/* hd44780 specific header files */
#ifdef HAVE_PCSTYLE_LPT_CONTROL
# include "hd44780-4bit.h"
# include "hd44780-ext8bit.h"
# include "hd44780-serialLpt.h"
# include "hd44780-winamp.h"
#endif
#include "hd44780-serial.h"
#include "hd44780-lis2.h"
#ifdef HAVE_LIBUSB
# include "hd44780-bwct-usb.h"
# include "hd44780-lcd2usb.h"
# include "hd44780-usbtiny.h"
# include "hd44780-uss720.h"
# include "hd44780-usb4all.h"
#endif
#ifdef HAVE_LIBFTDI
# include "hd44780-ftdi.h"
#endif
#ifdef HAVE_I2C
# include "hd44780-i2c.h"
# include "hd44780-piplate.h"
#endif
#ifdef HAVE_SPI
# include "hd44780-spi.h"
# include "hd44780-pifacecad.h"
#endif
#ifdef WITH_ETHLCD
# include "hd44780-ethlcd.h"
#endif
#include "hd44780-usblcd.h"
#ifdef WITH_RASPBERRYPI
# include "hd44780-rpi.h"
#endif
/* add new connection type header files to the correct section above or here */


/** connectionType mapping table:
 * - string to identify connection in config file
 * - connection type identifier
 * - interface type
 * - initialisation function
 */
static const ConnectionMapping connectionMapping[] = {
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	/* parallel connection types */
	{ "4bit",          HD44780_CT_4BIT,          IF_TYPE_PARPORT, hd_init_4bit      },
	{ "8bit",          HD44780_CT_8BIT,          IF_TYPE_PARPORT, hd_init_ext8bit   },
	{ "serialLpt",     HD44780_CT_SERIALLPT,     IF_TYPE_PARPORT, hd_init_serialLpt },
	{ "winamp",        HD44780_CT_WINAMP,        IF_TYPE_PARPORT, hd_init_winamp    },
#endif
	/* serial connection types */
	{ "picanlcd",      HD44780_CT_PICANLCD,      IF_TYPE_SERIAL,  hd_init_serial    },
	{ "lcdserializer", HD44780_CT_LCDSERIALIZER, IF_TYPE_SERIAL,  hd_init_serial    },
	{ "los-panel",     HD44780_CT_LOS_PANEL,     IF_TYPE_SERIAL,  hd_init_serial    },
	{ "vdr-lcd",       HD44780_CT_VDR_LCD,       IF_TYPE_SERIAL,  hd_init_serial    },
	{ "vdr-wakeup",    HD44780_CT_VDR_WAKEUP,    IF_TYPE_SERIAL,  hd_init_serial    },
	{ "pertelian",     HD44780_CT_PERTELIAN,     IF_TYPE_SERIAL,  hd_init_serial    },
	/* USB connection types */
	{ "lis2",          HD44780_CT_LIS2,          IF_TYPE_USB,     hd_init_lis2      },
	{ "mplay",         HD44780_CT_MPLAY,         IF_TYPE_USB,     hd_init_lis2      },
	{ "usblcd",        HD44780_CT_USBLCD,        IF_TYPE_USB,     hd_init_usblcd    },
#ifdef HAVE_LIBUSB
	{ "bwctusb",       HD44780_CT_BWCTUSB,       IF_TYPE_USB,     hd_init_bwct_usb  },
	{ "lcd2usb",       HD44780_CT_LCD2USB,       IF_TYPE_USB,     hd_init_lcd2usb   },
	{ "usbtiny",       HD44780_CT_USBTINY,       IF_TYPE_USB,     hd_init_usbtiny   },
	{ "uss720",        HD44780_CT_USS720,        IF_TYPE_USB,     hd_init_uss720    },
	{ "usb4all",       HD44780_CT_USB4ALL,       IF_TYPE_USB,     hd_init_usb4all   },
#endif
#ifdef HAVE_LIBFTDI
	{ "ftdi",          HD44780_CT_FTDI,          IF_TYPE_USB,     hd_init_ftdi      },
#endif
	/* I2C connection types */
#ifdef HAVE_I2C
	{ "i2c",           HD44780_CT_I2C,           IF_TYPE_I2C,     hd_init_i2c       },
        { "piplate",       HD44780_CT_PIPLATE,       IF_TYPE_I2C,     hd_init_i2c_piplate },
#endif
#ifdef HAVE_SPI
	{ "spi",           HD44780_CT_SPI,           IF_TYPE_SPI,     hd_init_spi       },
	{ "pifacecad",     HD44780_CT_PIFACECAD,     IF_TYPE_SPI,     hd_init_pifacecad },
#endif
	/* TCP socket connection types */
#ifdef WITH_ETHLCD
	{ "ethlcd",        HD44780_CT_ETHLCD,        IF_TYPE_TCP,     hd_init_ethlcd    },
#endif
#ifdef WITH_RASPBERRYPI
	{ "raspberrypi",   HD44780_CT_RASPBERRYPI,   IF_TYPE_PARPORT,  hd_init_rpi      },
#endif
	/* add new connection types in the correct section above or here */

	/* default, end of structure element (do not delete) */
	{ NULL, HD44780_CT_UNKNOWN, IF_TYPE_UNKNOWN, NULL }
};

#endif
