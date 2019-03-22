/**
 * \file server/drivers/serialPOS_epson.c
 *
 * LCDd Helper functions for driving ESC/POS displays through
 * \c serialPOS
 *
 * See \file server/drivers/serialPOS_common.h for information on
 * functions.
 */

/*-
 * These are the helper functions for driving Point Of Sale ("POS") devices
 * using the LCDproc serialPOS driver through the ESC / POS protocol
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

#include "serialPOS_common.h"

/**
 * Maximum width of an ESC/POS display - usually there are no
 * displays outside of this size and displays may crash if given invalid
 * arguments to commands. Limit all these settings to prevent crashing
 * and damaging displays.
 */
static const uint8_t EPSON_MAX_WIDTH = 20;

/**
 * Maximum height of an ESC/POS display - usually there are no
 * displays outside of this size and displays may crash if given invalid
 * arguments to commands. Limit all these settings to prevent crashing
 * and damaging displays.
 */
static const uint8_t EPSON_MAX_HEIGHT = 4;

/**
 * Cell height of an ESC/POS display - protocol limited
 */
static const uint8_t EPSON_CELLHGT = 7;

/**
 * Cell width of an ESC/POS display - protocol limited
 */
static const uint8_t EPSON_CELLWID = 5;

/**
 * Default brightness of an ESC/POS display
 */
static const uint8_t EPSON_DEF_BRIGHTNESS = 0x04;

/**
 * ESC/POS display initialization command sequence
 */
static const uint8_t EPSON_INIT_DISPLAY[] = {0x1b, 0x40};

/**
 * ESC/POS command sequence to set cursor state.
 *
 * Must be followed by \c 0x01 or \c 0x00 for cursor states of on and off,
 * respectively.
 */
static const uint8_t EPSON_CURSOR_STATE[] = {0x1f, 0x43};

/**
 * ESC/POS command sequence to set cursor position
 *
 * Must be followed by two bytes representing the desired cursor position,
 * encoded in straight binary, with the desired x-coordinate
 * sent before the desired y-coordinate.
 */
static const uint8_t EPSON_CURSOR_POSITION[] = {0x1f, 0x24};

/**
 * ESC/POS command sequence to home the cursor to (1, 1) (1-based)
 */
static const uint8_t EPSON_CURSOR_HOME[] = {0x0b};

/**
 * ESC/POS command sequence to move the cursor down one row
 */
static const uint8_t EPSON_CURSOR_DOWN[] = {0x0a};

/*
 * The ESC/POS command set does not have any commands that atomically update
 * single lines, and hence, in this driver, line updates are handled through
 * moving the cursor to a designated display line and updating the line
 */

/**
 * ESC/POS command sequence to adjust brightness
 *
 * Must be followed by one byte in the range [\c 0x01, \c 0x04] representing
 * the desired brightness level, starting from 25% with a step of
 * 25%.
 */
static const uint8_t EPSON_BRIGHTNESS_ADJUST[] = {0x1f, 0x58};

/**
 * ESC/POS command sequence to upload custom characters to the display
 *
 * Must be followed by:
 * - Two bytes in the range [\c 0x20, \c 0xff],
 *   henceforth called \c CSTART, \c CEND representing the
 *   start and end codes of the characters whose glyphs we want to replace
 * - (\c CEND - \c CSTART + 1) repetitons of 6 bytes, consisting of:
 *   - The byte \c 0x05
 *   - Five bytes defining the custom character
 */
static const uint8_t EPSON_UPLOAD_CUSTOM_CHARS[] = {0x1b, 0x26, 0x01};

/**
 * ESC/POS command sequence to set whether the screen uses custom glyphs
 * to render characters.
 *
 * Must be followed by \c 0x00 or \c 0x01 to disable / enable custom characters,
 * respectively.
 */
static const uint8_t EPSON_RENDER_CUSTOM_CHARS[] = {0x1b, 0x25};

/**
 * Custom character starting region
 *
 * Allocate characters from high-ASCII region for custom characters
 *
 * \note Some displays support assigning characters from high ASCII region,
 * some displays do not. In the case that the display does not, we don't
 * support custom characters on the display, at all.
 */
static const uint8_t CUSTOM_START = (0xff - MAX_CUSTOM_CHARS);

static int command_buffer_sz(PrivateData* data);

static int init(PrivateData* data, uint8_t* buffer);

static int flush(PrivateData* data, uint8_t* buffer);

static int cust_char_code(PrivateData* data, int idx);

const ops serialPOS_epson_ops = {command_buffer_sz, init, flush,
				 cust_char_code};

static int
command_buffer_sz(PrivateData* data)
{
	/*
	 * Two scenarios will consume the most buffer space
	 *
	 * - Updating the whole display, plus updating the cursor state and
	 *   position, plus updating the brightness.
	 *
	 * - Initializing the display, plus sending custom characters
	 *   for the maximum number of custom characters defined, plus
	 *   sending the custom character enable sequence.
	 *
	 * - We choose the largest amount of space required to return.
	 */
	int full_update_size = (
	    /*
	     * Take into account size of updating the display via the dumbest
	     * way possible, one cursor shift for each line.
	     */
	    ((sizeof(EPSON_CURSOR_POSITION) + 2) * (data->height))
	    + (data->height * data->width)
	    /*
	     * Take into account cursor position save / restore
	     * to avoid cursor flash when performing operations
	     */
	    + (1 * (sizeof(EPSON_CURSOR_POSITION) + 2))
	    + (2 * (sizeof(EPSON_CURSOR_STATE) + 1))
	    /* Take into account adjusting the display brightness */
	    + sizeof(EPSON_BRIGHTNESS_ADJUST) + 1);
	int display_init_size =
	    (sizeof(EPSON_INIT_DISPLAY) + sizeof(EPSON_UPLOAD_CUSTOM_CHARS) + 2
	     + (6 * data->custom_chars_supported)
	     + sizeof(EPSON_RENDER_CUSTOM_CHARS) + 1);
	return ((full_update_size > display_init_size) ? full_update_size :
							 display_init_size);
}

static int
init(PrivateData* data, uint8_t* buffer)
{
	/*
	 * EPSON displays limited to max 20x4 only
	 */
	if ((data->width > EPSON_MAX_WIDTH)
	    || (data->height > EPSON_MAX_HEIGHT)) {
		return -1;
	}
	/*
	 * EPSON displays limited to 5x7 custom characters if we
	 * are going to send custom characters. If there's no requirement
	 * for custom character support, we ignore this limitation - because
	 * it's not going to affect what we're going to send to the display.
	 */
	if (data->custom_chars_supported)
		if ((data->cellheight != EPSON_CELLHGT)
		    || (data->cellwidth != EPSON_CELLWID)) {
			return -1;
		}

	/*
	 * Store initial display state
	 */
	data->display_misc_state.backlight_state = 1;
	data->display_misc_state.brightness      = EPSON_DEF_BRIGHTNESS;
	/*
	 * Write in the initialization sequence
	 */
	unsigned char* start = buffer;
	buffer		     = bytecpy_advance_ptr(buffer, EPSON_INIT_DISPLAY,
					   sizeof(EPSON_INIT_DISPLAY));

	/*
	 * If possible, reserve characters for hbar
	 */
	int required_chars = 0;
	if (data->custom_chars_supported >= data->cellwidth) {
		required_chars += data->cellwidth;
		data->hbar_custom = 1;
	}
	/*
	 * If possible, reserve characters for vbar
	 */
	if ((data->custom_chars_supported - required_chars)
	    >= (data->cellheight - 1)) {
		required_chars += (data->cellheight - 1);
		data->vbar_custom = 1;
	}

	/*
	 * Upload custom characters
	 */
	uint8_t cus_codes_start = CUSTOM_START;
	uint8_t cus_codes_end   = cus_codes_start;
	if (data->hbar_custom)
		cus_codes_end += (data->cellwidth);
	if (data->vbar_custom)
		cus_codes_end += (data->cellheight - 1);
	if (required_chars) {
		buffer = bytecpy_advance_ptr(buffer, EPSON_UPLOAD_CUSTOM_CHARS,
					     sizeof(EPSON_UPLOAD_CUSTOM_CHARS));
		*(buffer++) = cus_codes_start;
		*(buffer++) = (cus_codes_end - 1);
		/* Generate custom characters */
		if (data->hbar_custom) {
			for (int width = 1; width <= data->cellwidth; width++) {
				*(buffer++) = 0x05;
				serialPOS_generate_horizontal_block_glyph(
				    buffer, width);
				buffer += 0x05;
			}
		}
		if (data->vbar_custom) {
			for (int height = 1; height <= (data->cellheight - 1);
			     height++) {
				*(buffer++) = 0x05;
				serialPOS_generate_vertical_block_glyph(buffer,
									height);
				buffer += 0x05;
			}
		}
		/*
		 * Enable custom charcters
		 */
		buffer = bytecpy_advance_ptr(buffer, EPSON_RENDER_CUSTOM_CHARS,
					     sizeof(EPSON_RENDER_CUSTOM_CHARS));
		*(buffer++) = 0x01;
	}
	return (buffer - start);
}

static int
flush(PrivateData* data, uint8_t* buffer)
{
	uint8_t* start	 = buffer;
	uint32_t lines_flushed = serialPOS_lines_to_flush(data);
	/*
	 * If cursor is on, and we're doing a character-by-character update,
	 * the cursor is going to be visible throughout the update in a
	 * very annoying fashion.
	 *
	 * Turn the cursor off before we update the screen.
	 */
	if (data->display_misc_state.cursor_state && (lines_flushed)) {
		buffer      = bytecpy_advance_ptr(buffer, EPSON_CURSOR_STATE,
						  sizeof(EPSON_CURSOR_STATE));
		*(buffer++) = 0x00;
		/*
		 * Set tracking so it is automatically turned on again
		 */
		data->display_misc_state.cursor_state = 0;
	}

	int cursor_position = -1;
	for (int h = 0; h < data->height; h++) {
		if (lines_flushed & (0x01 << h)) {
			/* Advance cursor to h-th line */
			if (cursor_position < 0) {
				buffer = bytecpy_advance_ptr(
				    buffer, EPSON_CURSOR_HOME,
				    sizeof(EPSON_CURSOR_HOME));
				cursor_position = 0;
			}
			for (int down_cmds = (h - cursor_position);
			     down_cmds > 0; down_cmds--) {
				buffer = bytecpy_advance_ptr(
				    buffer, EPSON_CURSOR_DOWN,
				    sizeof(EPSON_CURSOR_DOWN));
			}
			/* Flush data */
			buffer = bytecpy_advance_ptr(
			    buffer, data->framebuf + (h * data->width),
			    data->width);
			cursor_position = (h + 1);
		}
	}

	/* Flush cursor data */
	if (data->display_misc_state.cursor_state
	    != ((data->buffered_misc_state.cursor_state != CURSOR_OFF) ? 1 :
									 0)) {
		/* Cursor state mismatch */
		buffer      = bytecpy_advance_ptr(buffer, EPSON_CURSOR_STATE,
						  sizeof(EPSON_CURSOR_STATE));
		*(buffer++) = !data->display_misc_state.cursor_state;
		data->display_misc_state.cursor_state =
		    !data->display_misc_state.cursor_state;
		if (data->display_misc_state.cursor_state) {
			/* Force cursor position sync */
			data->display_misc_state.cx = -1;
		}
	}
	/* Cursor state match, check position */
	if ((data->display_misc_state.cx != data->buffered_misc_state.cx)
	    || (data->display_misc_state.cy != data->buffered_misc_state.cy)) {
		if (data->display_misc_state.cursor_state) {
			buffer =
			    bytecpy_advance_ptr(buffer, EPSON_CURSOR_POSITION,
						sizeof(EPSON_CURSOR_POSITION));
			*(buffer++) = data->buffered_misc_state.cx;
			*(buffer++) = data->buffered_misc_state.cy;
		}
		data->display_misc_state.cx = data->buffered_misc_state.cx;
		data->display_misc_state.cy = data->buffered_misc_state.cy;
	}

	/* Flush brightness data*/
	int target_brightness;
	if (data->buffered_misc_state.backlight_state != BACKLIGHT_OFF) {
		target_brightness = serialPOS_convert_brightness(
		    data->buffered_misc_state.brightness);
	}
	else {
		target_brightness = 1;
	}
	if (data->display_misc_state.brightness != target_brightness) {
		buffer = bytecpy_advance_ptr(buffer, EPSON_BRIGHTNESS_ADJUST,
					     sizeof(EPSON_BRIGHTNESS_ADJUST));
		*(buffer++)			    = target_brightness;
		data->display_misc_state.brightness = target_brightness;
	}
	return (buffer - start);
}

static int
cust_char_code(PrivateData* data, int idx)
{
	if (idx < data->custom_chars_supported) {
		return (CUSTOM_START + idx);
	}
	else {
		return -1;
	}
}
