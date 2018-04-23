/**
 * \file server/drivers/serialPOS_common.h
 *
 * Common header file for serialPOS driver
 */

#ifndef SERVER_DRIVERS_SERIALPOS_COMMON_H_
#define SERVER_DRIVERS_SERIALPOS_COMMON_H_

/*-
 * Common declarations and shared data structures for serialPOS
 *
 * Copyright (C) 2006, 2007 Eric Pooch
 * Copyright (C) 2018, Shenghao Yang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

#include "lcd.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_DEVICE 		"/dev/ttyS0"
#define DEFAULT_SPEED 		9800
#define DEFAULT_SIZE 		"16x2"
#define DEFAULT_TYPE 		"AEDEX"
#define DEFAULT_CELL_SIZE	"5x8"
#define DEFAULT_CUSTOM_CHARS 	0
#define MAX_CUSTOM_CHARS 	32
#define MAX_CELLWID 		5
#define MAX_CELLHGT 		8

/*
 * Max width is limited because the following expression can not evaluate
 * to zero - used in hbar()
 *
 * 1000 / (cellwid * width) [Integer division]
 */
#define MAX_WIDTH (1000 / MAX_CELLWID)
/*
 * Max height limited to the maximum number of in \ref uint32_t
 * used to track which lines changed
 */
#define MAX_HEIGHT 32

/**
 * Common operations for serialPOS displays.
 *
 * Essential operations are:
 * \ref serialPOS_ops::command_buffer_sz
 * \ref serialPOS_ops::init
 * \ref serialPOS_ops::flush
 *
 * The other operations can be \c NULL
 */
struct serialPOS_private_data;
typedef struct serialPOS_ops {
	/**
	 * Obtain the command buffer size, in bytes, that is large enough
	 * to store a command returned by any of the functions in the structure.
	 *
	 * \param data private data structure used by serialPOS
	 * \return number of bytes requiring allocation
	 */
	int (*command_buffer_sz)(struct serialPOS_private_data* data);

	/**
	 * Obtain the command required to initialize the display, as well as
	 * initialize any data used by subsequent functions whose pointers are
	 * contained in this structure.
	 *
	 * \param data private data structure used by serialPOS
	 * \param buffer buffer to write the initialization command to
	 * \return number of bytes written to the buffer, or -1
	 * on error.
	 */
	int (*init)(struct serialPOS_private_data* data, uint8_t* buffer);

	/**
	 * Obtain the command required to flush the framebuffer
	 * to the screen.
	 *
	 * This function must transfer to the display information necessary
	 * to changes tracked by these two buffers:
	 * \ref PrivateData::framebuf to \ref PrivateData::backingstore,
	 * updating the \ref PrivateData::backingstore with changes
	 * written to the display.
	 *
	 * Cursor information is provided through
	 * \ref PrivateData::cx, \ref PrivateData::cy, and
	 * \ref PrivateData::cursor_state
	 *
	 * The function should also command the display's cursor to
	 * have the desired state after flushing the text data to the display,
	 * if supported.
	 *
	 * Brightness information is provided through
	 * \ref PrivateData::brightness and \ref PrivateData::backlight_state
	 *
	 * The function should also command the display to switch to the
	 * selected brightness, if supported.
	 *
	 * As far as possible, attempt to avoid performing complex multi-command
	 * delta updates to the display by moving the display cursor. Some
	 * displays have problems dealing with high command rates and will
	 * drop commands, leading to malformed data displayed on screen. The
	 * "write-line" commands that allow atomic updates of single lines
	 * should be preferred, instead.
	 *
	 * \param data private data used by the display
	 * \param buffer buffer to write data to
	 * \return number of bytes written to the buffer, or -1 on error
	 */
	int (*flush)(struct serialPOS_private_data* data, uint8_t* buffer);

	/**
	 * Obtain the character code assigned to a custom character
	 * at a particular index.
	 *
	 * \param data private display data
	 * \param idx index of the custom character, from \c 0 to
	 * \ref PrivateData::custom_chars_supported.
	 * \return character code, or -1 on error
	 */
	int (*cust_char_code)(struct serialPOS_private_data* data, int idx);
} ops;

/*
 * Externally defined operations structures
 */
extern const ops serialPOS_aedex_ops;
extern const ops serialPOS_cd5220_ops;
extern const ops serialPOS_epson_ops;
extern const ops serialPOS_logic_controls_ops;

/**
 * Structure use by \c serialPOS to track miscellaneous display state.
 */
typedef struct serialPOS_display_state_misc {
	int cx;		  /**< Cursor x, 1-based */
	int cy;		  /**< Cursor y, 1-based */
	int cursor_state; /**< Cursor state, see LCD.h */

	int brightness;      /**< Display brightness */
	int backlight_state; /**< Backlight state, see LCD.h */
} serialPOS_state_misc;

/**
 * Private data for the \c serialPOS driver
 */
typedef struct serialPOS_private_data {
	int fd; /**< LCD file descriptor */

	/* dimensions */
	int width, height;
	int cellwidth, cellheight;

	/* number of custom characters supported by the display */
	int custom_chars_supported; /**< number of custom character supported */
	/**
	 * Whether display supports custom chars for hbar
	 *
	 * If display supports custom chars for hbar, then custom character
	 * codes returned for
	 * indices 0 through \ref PrivateData::cellwidth - 1 correspond to
	 * the custom character codes that will draw
	 * \ref PrivateData::cellheight
	 * pixels high bars, of increasing width, with a width step of
	 * 1 pixel and a start of 1 pixel.
	 *
	 */
	int hbar_custom;
	/**
	 * Whether display supports custom chars for vbar
	 *
	 * The display must support custom chars for hbar as well to support
	 * custom chars for vbar.
	 *
	 * If display supports custom chars for vbar, then custom character
	 * codes returned for indices \ref PrivateData::cellwidth through
	 * \ref PrivateData::cellwidth + \ref PrivateData::cellheight - 2
	 * (inclusive) correspond to the custom character codes that will draw
	 * \ref PrivateData::cellwidth pixel wide bars, of increasing height,
	 * with a height step of 1 pixel and a start of 1 pixel,
	 * and a height end of \ref PrivateData::cellheight - 1 pixels
	 *
	 * To draw a full pixel block, use the custom character generated
	 * for a full hbar block.
	 */
	int vbar_custom; /**< display supports custom chars for vbar */

	/* Display state tracking */
	/**
	 * Actual state of the display, with regards to cursor position
	 * and brightness.
	 */
	serialPOS_state_misc display_misc_state;
	/**
	 * Buffered state of the display, changes which will be committed
	 * on the next flush.
	 */
	serialPOS_state_misc buffered_misc_state;

	/* framebuffer and buffer for old LCD contents */
	uint8_t* framebuf;     /**< Framebuffer mutated by server calls */
	uint8_t* backingstore; /**< real LCD display buffer */

	/* protocol specific data */
	int buffer_size;	 /**< Buffer size required by the protocol */
	const ops* protocol_ops; /**< Protocol specific operations */

	char info[255]; /**< static data from serialPOS_get_info */
} PrivateData;

/**
 * Copy bytes from one region to another, using the same semantics as
 * \ref memcpy()
 *
 * \param to destination region
 * \param from source region
 * \param len number of bytes to copy
 * \return pointer to the byte after writing copied memory, len bytes after
 * \p to
 */
static inline uint8_t*
bytecpy_advance_ptr(uint8_t* restrict to, const uint8_t* restrict from,
		    size_t len)
{
	memcpy(to, from, len);
	return (to + len);
}

/**
 * Convert a brightness value in promille to a brightness value
 * that can be sent to the display.
 *
 * This works with any protocol that expects a brightness value in the
 * returned range, with the same meaning to those values, that is,
 * higher values give progressively brighter display outputs.
 *
 * \param data
 * \return brightness value in range [\c 0x01, \c 0x04]
 */
uint8_t serialPOS_convert_brightness(int promille);

/**
 * Generate a custom character sequence, \c 0x05 bytes wide,
 * that can be sent to the display in order for that display
 * to show a horizontal block \p width pixels wide.
 *
 * Compatible with protocols that send custom characters in the same format
 * as the ESC/POS / CD5220 / ... protocols.
 *
 * \param buffer buffer to write to
 * \param width  width of block in pixels, in range [\c 1, \c 5]
 */
void serialPOS_generate_horizontal_block_glyph(uint8_t* buffer, int width);

/**
 * Generate a custom character sequence, \c 0x05 bytes wide,
 * that can be sent to the display in order for that display
 * to show a vertical block \p height pixels tall.
 *
 * Compatible with protocols that send custom characters in the same format
 * as the ESC/POS / CD5220 / ... protocols.
 *
 * \param buffer buffer to write to
 * \param height height of block in pixels, in range [\c 1, \c 7]
 */
void serialPOS_generate_vertical_block_glyph(uint8_t* buffer, int height);

/**
 * Obtain a bitset indicating which lines should be flushed to the display
 * from the framebuffer.
 *
 * Updates the backing store with the framebuffer's lines if the
 * display is indeed flushed.
 *
 * \param data private data
 * \return 32-bit unsigned integer. Each bit set represents a line to flush.
 * Bit 0 set means that line 0 must be flushed, bit 1 corresponds to line 1,
 * and so on. (0 - based)
 */
uint32_t serialPOS_lines_to_flush(PrivateData* data);
#endif /* SERVER_DRIVERS_SERIALPOS_COMMON_H_ */
