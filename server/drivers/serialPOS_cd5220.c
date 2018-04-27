/**
 * \file server/drivers/serialPOS_cd5220.c
 *
 * LCDd Helper functions for driving cd5220 displays through
 * \c serialPOS
 *
 * See \file server/drivers/serialPOS_common.h for information on
 * functions.
 */

/*-
 * These are the helper functions for driving Point Of Sale ("POS") devices
 * using the LCDproc serialPOS driver through the CD5220 protocol
 *
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
 * Maximum width of a CD5220 display - protocol limited
 */
static const uint8_t CD5220_MAX_WIDTH = 20;

/**
 * Maximum height of a CD5220 display - protocol limited
 */
static const uint8_t CD5220_MAX_HEIGHT = 2;

/**
 * Cell height of a CD5220 display - protocol limited
 */
static const uint8_t CD5220_CELLHGT = 7;

/**
 * Cell width of a CD5220 display - protocol limited
 */
static const uint8_t CD5220_CELLWID = 5;

/**
 * Default brightness of a CD5220 display
 */
static const uint8_t CD5220_DEF_BRIGHTNESS = 0x04;

/**
 * CD5220 display initialization command sequence
 */
static const uint8_t CD5220_INIT_DISPLAY[] = {0x1b, 0x40};

/**
 * CD5220 command sequence to set cursor state.
 *
 * Must be followed by \c 0x01 or \c 0x00 for cursor states of on and off,
 * respectively.
 */
static const uint8_t CD5220_CURSOR_STATE[] = {0x1b, 0x5f};

/**
 * CD5220 command sequence to set cursor position
 *
 * Must be followed by two bytes representing the desired cursor position,
 * encoded in straight binary, with the desired x-coordinate
 * sent before the desired y-coordinate.
 */
static const uint8_t CD5220_CURSOR_POSITION[] = {0x1b, 0x6c};

/**
 * CD5220 command sequence to update the upper line
 *
 * Must be followed by up to twenty bytes representing the desired
 * characters on the upper line of the display, followed by
 * \ref CD5220_END_UPDATE_LINE
 */
static const uint8_t CD5220_UPDATE_UPPER_LINE[] = {0x1b, 0x51, 0x41};

/**
 * CD5220 command sequence to update the lower line
 *
 * Must be followed by up to twenty bytes representing the desired
 * characters on the lower line of the display, followed by
 * \ref CD5220_END_UPDATE_LINE
 */
static const uint8_t CD5220_UPDATE_LOWER_LINE[] = {0x1b, 0x51, 0x42};

/**
 * CD5220 command sequence to update the lower line
 *
 * Must be followed by up to twenty bytes representing the desired
 * charcters on the lower line of the display, followed by
 * \ref CD5220_END_UPDATE_LINE
 */
static const uint8_t CD5220_END_UPDATE_LINE[] = {0x0d};

/**
 * CD5220 command sequence to adjust brightness
 *
 * Must be followed by one byte in the range [\c 0x01, \c 0x04] representing
 * the desired brightness level, starting from 25% with a step of
 * 25%.
 */
static const uint8_t CD5220_BRIGHTNESS_ADJUST[] = {0x1b, 0x2a};

/**
 * CD5220 command sequence to upload custom characters to the display
 *
 * Must be followed by:
 * - Two bytes in the range [\c 0x20, \c 0xff],
 *   henceforth called \c CSTART, \c CEND representing the
 *   start and end codes of the characters whose glyphs we want to replace
 * - (\c CEND - \c CSTART + 1) repetitons of 6 bytes, consisting of:
 *   - The byte \c 0x05
 *   - Five bytes defining the custom character
 */
static const uint8_t CD5220_UPLOAD_CUSTOM_CHARS[] = {0x1b, 0x26, 0x01};

/**
 * CD5220 command sequence to set whether the screen uses custom glyphs
 * to render characters.
 *
 * Must be followed by \c 0x00 or \c 0x01 to disable / enable custom characters,
 * respectively.
 */
static const uint8_t CD5220_RENDER_CUSTOM_CHARS[] = {0x1b, 0x25};

/**
 * Custom character starting region
 *
 * Allocate characters from high-ASCII region for custom characters
 */
static const uint8_t CUSTOM_START = (0xff - MAX_CUSTOM_CHARS);

static int command_buffer_sz(PrivateData* data);

static int init(PrivateData* data, uint8_t* buffer);

static int flush(PrivateData* data, uint8_t* buffer);

static int cust_char_code(PrivateData* data, int idx);

const ops serialPOS_cd5220_ops = {
    command_buffer_sz,
    init,
    flush,
    cust_char_code,
};

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
	int full_update_size =
	    (sizeof(CD5220_UPDATE_UPPER_LINE) + data->width
	     + sizeof(CD5220_UPDATE_LOWER_LINE) + data->width
	     + (2 * sizeof(CD5220_END_UPDATE_LINE))
	     + sizeof(CD5220_CURSOR_POSITION) + 2 + sizeof(CD5220_CURSOR_STATE)
	     + 1 + sizeof(CD5220_BRIGHTNESS_ADJUST) + 1);
	int display_init_size =
	    (sizeof(CD5220_INIT_DISPLAY) + sizeof(CD5220_UPLOAD_CUSTOM_CHARS)
	     + 2 + (6 * data->custom_chars_supported)
	     + sizeof(CD5220_RENDER_CUSTOM_CHARS) + 1);
	return ((full_update_size > display_init_size) ? full_update_size :
							 display_init_size);
}

static int
init(PrivateData* data, uint8_t* buffer)
{
	/*
	 * CD5220 displays limited to max 20x2 only
	 */
	if ((data->width > CD5220_MAX_WIDTH)
	    || (data->height > CD5220_MAX_HEIGHT)) {
		return -1;
	}
	/*
	 * CD5220 displays have custom characters limited to 5x7.
	 * Limit not observed if custom character support disabled.
	 */
	if (data->custom_chars_supported)
		if ((data->cellheight != CD5220_CELLHGT)
		    || (data->cellwidth != CD5220_CELLWID)) {
			return -1;
		}

	data->display_misc_state.backlight_state = 1;
	data->display_misc_state.brightness      = CD5220_DEF_BRIGHTNESS;
	/*
	 * Write in the initialization sequence
	 */
	uint8_t* const start = buffer;
	buffer		     = bytecpy_advance_ptr(buffer, CD5220_INIT_DISPLAY,
					   sizeof(CD5220_INIT_DISPLAY));

	/*
	 * If possible, reserve characters for hbar
	 */
	int custom_reserved = 0;
	if (data->custom_chars_supported >= data->cellwidth) {
		custom_reserved += data->cellwidth;
		data->hbar_custom = 1;
	}
	/*
	 * If possible, reserve characters for vbar
	 */
	if ((data->custom_chars_supported - custom_reserved)
	    >= (data->cellheight - 1)) {
		custom_reserved += (data->cellheight - 1);
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
	if (custom_reserved) {
		buffer =
		    bytecpy_advance_ptr(buffer, CD5220_UPLOAD_CUSTOM_CHARS,
					sizeof(CD5220_UPLOAD_CUSTOM_CHARS));
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
		 * Enable custom characters
		 */
		buffer =
		    bytecpy_advance_ptr(buffer, CD5220_RENDER_CUSTOM_CHARS,
					sizeof(CD5220_RENDER_CUSTOM_CHARS));
		*(buffer++) = 0x01;
	}
	return (buffer - start);
}

static int
flush(PrivateData* data, uint8_t* buffer)
{
	uint8_t* const start   = buffer;
	uint32_t lines_flushed = 0;

	lines_flushed = serialPOS_lines_to_flush(data);
	/* Flush display data */
	if (lines_flushed & 0x01) {
		/* Flush first display line */
		buffer = bytecpy_advance_ptr(buffer, CD5220_UPDATE_UPPER_LINE,
					     sizeof(CD5220_UPDATE_UPPER_LINE));
		buffer =
		    bytecpy_advance_ptr(buffer, data->framebuf, data->width);
		buffer = bytecpy_advance_ptr(buffer, CD5220_END_UPDATE_LINE,
					     sizeof(CD5220_END_UPDATE_LINE));
	}
	if (lines_flushed & 0x02) {
		/* Flush second display line */
		buffer = bytecpy_advance_ptr(buffer, CD5220_UPDATE_LOWER_LINE,
					     sizeof(CD5220_UPDATE_LOWER_LINE));
		buffer = bytecpy_advance_ptr(
		    buffer, data->framebuf + (data->width), data->width);
		buffer = bytecpy_advance_ptr(buffer, CD5220_END_UPDATE_LINE,
					     sizeof(CD5220_END_UPDATE_LINE));
	}

	/* Flush cursor data */
	if (data->display_misc_state.cursor_state
	    != ((data->buffered_misc_state.cursor_state != CURSOR_OFF) ? 1 :
									 0)) {
		/* Cursor state mismatch */
		buffer      = bytecpy_advance_ptr(buffer, CD5220_CURSOR_STATE,
						  sizeof(CD5220_CURSOR_STATE));
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
			    bytecpy_advance_ptr(buffer, CD5220_CURSOR_POSITION,
						sizeof(CD5220_CURSOR_POSITION));
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
		buffer = bytecpy_advance_ptr(buffer, CD5220_BRIGHTNESS_ADJUST,
					     sizeof(CD5220_BRIGHTNESS_ADJUST));
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
