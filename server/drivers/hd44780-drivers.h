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
# include "hd44780-lcm162.h"
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
#ifdef HAVE_UGPIO
# include "hd44780-gpio.h"
#endif
/* add new connection type header files to the correct section above or here */

#include "../elektragen.h"

/** connectionType mapping table:
 * - string to identify connection in config file
 * - connection type identifier
 * - interface type
 * - initialisation function
 */
static const ConnectionMapping connectionMapping[] = {
#ifdef HAVE_PCSTYLE_LPT_CONTROL
	/* parallel connection types */
	{ HD44780_CONNECTION_TYPE_4BIT,          HD44780_CT_4BIT,          IF_TYPE_PARPORT, hd_init_4bit        },
	{ HD44780_CONNECTION_TYPE_8BIT,          HD44780_CT_8BIT,          IF_TYPE_PARPORT, hd_init_ext8bit     },
	{ HD44780_CONNECTION_TYPE_SERIAL_LPT,    HD44780_CT_SERIALLPT,     IF_TYPE_PARPORT, hd_init_serialLpt   },
	{ HD44780_CONNECTION_TYPE_WINAMP,        HD44780_CT_WINAMP,        IF_TYPE_PARPORT, hd_init_winamp      },
	{ HD44780_CONNECTION_TYPE_LCM162,        HD44780_CT_LCM162,        IF_TYPE_PARPORT, hd_init_lcm162      },
#endif
	/* serial connection types */
	{ HD44780_CONNECTION_TYPE_4BIT,          HD44780_CT_PICANLCD,      IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_LCDSERIALIZER, HD44780_CT_LCDSERIALIZER, IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_LOS_PANEL,     HD44780_CT_LOS_PANEL,     IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_VDR_LCD,       HD44780_CT_VDR_LCD,       IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_VDR_WAKEUP,    HD44780_CT_VDR_WAKEUP,    IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_PERTELIAN,     HD44780_CT_PERTELIAN,     IF_TYPE_SERIAL,  hd_init_serial      },
	{ HD44780_CONNECTION_TYPE_EZIO,          HD44780_CT_EZIO,          IF_TYPE_SERIAL,  hd_init_serial      },
	/* USB connection types */
	{ HD44780_CONNECTION_TYPE_LIS2,          HD44780_CT_LIS2,          IF_TYPE_USB,     hd_init_lis2        },
	{ HD44780_CONNECTION_TYPE_MPLAY,         HD44780_CT_MPLAY,         IF_TYPE_USB,     hd_init_lis2        },
	{ HD44780_CONNECTION_TYPE_USBLCD,        HD44780_CT_USBLCD,        IF_TYPE_USB,     hd_init_usblcd      },
#ifdef HAVE_LIBUSB
	{ HD44780_CONNECTION_TYPE_BWCTUSB,       HD44780_CT_BWCTUSB,       IF_TYPE_USB,     hd_init_bwct_usb    },
	{ HD44780_CONNECTION_TYPE_LCD2USB,       HD44780_CT_LCD2USB,       IF_TYPE_USB,     hd_init_lcd2usb     },
	{ HD44780_CONNECTION_TYPE_USBTINY,       HD44780_CT_USBTINY,       IF_TYPE_USB,     hd_init_usbtiny     },
	{ HD44780_CONNECTION_TYPE_USS720,        HD44780_CT_USS720,        IF_TYPE_USB,     hd_init_uss720      },
	{ HD44780_CONNECTION_TYPE_U_S_B_4_ALL,   HD44780_CT_USB4ALL,       IF_TYPE_USB,     hd_init_usb4all     },
#endif
#ifdef HAVE_LIBFTDI
	{ HD44780_CONNECTION_TYPE_FTDI,          HD44780_CT_FTDI,          IF_TYPE_USB,     hd_init_ftdi        },
#endif
	/* I2C connection types */
#ifdef HAVE_I2C
	{ HD44780_CONNECTION_TYPE_I2C,           HD44780_CT_I2C,           IF_TYPE_I2C,     hd_init_i2c         },
	{ HD44780_CONNECTION_TYPE_PIPLATE,       HD44780_CT_PIPLATE,       IF_TYPE_I2C,     hd_init_i2c_piplate },
#endif
#ifdef HAVE_SPI
	{ HD44780_CONNECTION_TYPE_SPI,           HD44780_CT_SPI,           IF_TYPE_SPI,     hd_init_spi         },
	{ HD44780_CONNECTION_TYPE_PIFACECAD,     HD44780_CT_PIFACECAD,     IF_TYPE_SPI,     hd_init_pifacecad   },
#endif
	/* TCP socket connection types */
#ifdef WITH_ETHLCD
	{ HD44780_CONNECTION_TYPE_ETHLCD,        HD44780_CT_ETHLCD,        IF_TYPE_TCP,     hd_init_ethlcd      },
#endif
#ifdef WITH_RASPBERRYPI
	{ HD44780_CONNECTION_TYPE_RASPBERRYPI,   HD44780_CT_RASPBERRYPI,   IF_TYPE_PARPORT,  hd_init_rpi        },
#endif
#ifdef HAVE_UGPIO
	{ HD44780_CONNECTION_TYPE_GPIO,          HD44780_CT_GPIO,          IF_TYPE_PARPORT, hd_init_gpio        },
#endif
	/* add new connection types in the correct section above or here */

	/* default, end of structure element (do not delete) */
	/* init_fn must be != NULL everywhere else in the array */
	{ 0, HD44780_CT_UNKNOWN, IF_TYPE_UNKNOWN, NULL }
};

#endif
