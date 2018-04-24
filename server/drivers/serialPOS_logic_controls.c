/**
 * \file server/drivers/serialPOS_logic_controls.c
 *
 * LCDd Helper functions for driving logic controls displays through
 * \c serialPOS
 *
 * See \file server/drivers/serialPOS_common.h for information on
 * functions.
 */

/*-
 * These are the helper functions for driving Point Of Sale ("POS") devices
 * using the LCDproc serialPOS driver through the logic controls protocol.
 *
 * This helper implements the basic logic controls
 * command set, which should work with all displays. It does not
 * support custom characters, be cause logic controls displays
 * require overriding standard ASCII characters, which could be
 * very confusing to clients if overridden characters were displayed
 * when clients expected normal glyphs.
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
#include <stdio.h>

/**
 * Maximum width of a Logic controls display - protocol limited
 */
static const uint8_t LOGIC_MAX_WIDTH = 20;

/**
 * Maximum height of a Logic controls display - protocol limited
 */
static const uint8_t LOGIC_MAX_HEIGHT = 2;

/*
 * No cell height limits enforced for logic controls displays
 */

/**
 * Default brightness of a logic controls display
 */
static const uint8_t LOGIC_DEF_BRIGHTNESS = 0xff;

/**
 * Logic controls display initialization command sequence
 *
 * Resets the display and puts it into normal mode, instead of
 * vertical scroll mode
 */
static const uint8_t LOGIC_INIT_DISPLAY[] = {0x1f, 0x11};

/**
 * Logic controls command sequence to advance the cursor to the
 * next row
 */
static const uint8_t LOGIC_CURSOR_DOWN[] = {0x0a};

/**
 * Logic controls command sequence to turn on the cursor.
 */
static const uint8_t LOGIC_CURSOR_ON[] = {0x13};

/**
 * Logic controls command sequence to turn off the cursor
 */
static const uint8_t LOGIC_CURSOR_OFF[] = {0x14};

/**
 * Logic controls command sequence to move the cursor to a desginated
 * location.
 *
 * Must be followed by a byte in the range [0x00, 0x27], representing
 * the location to move to.
 *
 * Bytes in the range [0x00, 0x13] map to the first 20 characters on
 * the first row.
 *
 * Bytes in the range [0x14, 0x27] map to the next 20 characters.
 */
static const uint8_t LOGIC_CURSOR_MOVE[] = {0x10};

/**
 * Logic controls command sequence to adjust brightness
 *
 * Must be followed by one byte in the set {0x20, 0x40, 0x60, 0xff}
 * to set the actual brightness.
 *
 * Higher values represent a brighter display, with each value step
 * except the last representing a jump in brightness of 20%,
 * starting from an initial display brightness of 20% at 0x20. The last value
 * step represents a jump in brightness of 40%.
 */
static const uint8_t LOGIC_BRIGHTNESS_ADJUST[] = {0x04};

static int command_buffer_sz(PrivateData* data);

static int init(PrivateData* data, uint8_t* buffer);

static int flush(PrivateData* data, uint8_t* buffer);

const ops serialPOS_logic_controls_ops = {command_buffer_sz, init, flush, NULL};

static int
command_buffer_sz(PrivateData* data)
{
	/*
	 * Performing a full screen state update plus a cursor state
	 * push / pop and a brightness adjustment will consume the
	 * most amount of buffer space.
	 */
	int full_update_size =
	    (sizeof(LOGIC_CURSOR_MOVE) + 0x01 + sizeof(LOGIC_CURSOR_DOWN)
	     + (data->width * 2) + sizeof(LOGIC_CURSOR_ON)
	     + sizeof(LOGIC_CURSOR_OFF) + (sizeof(LOGIC_CURSOR_MOVE) + 1)
	     + sizeof(LOGIC_BRIGHTNESS_ADJUST) + 1);
	return full_update_size;
}

static int
init(PrivateData* data, uint8_t* buffer)
{
	/*
	 * Logic controls displays limited to max 20x2 only, there's no
	 * comamnd sequence to move the cursor beyond the 20x2 display range.
	 */
	if ((data->width > LOGIC_MAX_WIDTH)
	    || (data->height > LOGIC_MAX_HEIGHT)) {
		return -1;
	}

	/*
	 * Setup initial display tracking
	 */
	data->display_misc_state.backlight_state = 1;
	data->display_misc_state.brightness      = LOGIC_DEF_BRIGHTNESS;
	/*
	 * Write in the initialization sequence
	 */
	uint8_t* const start = buffer;
	buffer		     = bytecpy_advance_ptr(buffer, LOGIC_INIT_DISPLAY,
					   sizeof(LOGIC_INIT_DISPLAY));

	return (buffer - start);
}

static int
flush(PrivateData* data, uint8_t* buffer)
{
	uint8_t* const start   = buffer;
	uint32_t lines_flushed = 0;

	lines_flushed = serialPOS_lines_to_flush(data);
	/* disable cursor, if enabled */
	if (lines_flushed && data->display_misc_state.cursor_state) {
		buffer = bytecpy_advance_ptr(buffer, LOGIC_CURSOR_OFF,
					     sizeof(LOGIC_CURSOR_OFF));
		data->display_misc_state.cursor_state = 0;
	}

	/* Flush display data */
	int cursor_position = -1;
	for (int h = 0; h < data->height; h++) {
		if (lines_flushed & (0x01 << h)) {
			/* Advance cursor to h-th line */
			if (cursor_position < 0) {
				buffer = bytecpy_advance_ptr(
				    buffer, LOGIC_CURSOR_MOVE,
				    sizeof(LOGIC_CURSOR_MOVE));
				*(buffer++)     = 0x00;
				cursor_position = 0;
			}
			for (int down_cmds = (h - cursor_position);
			     down_cmds > 0; down_cmds--) {
				buffer = bytecpy_advance_ptr(
				    buffer, LOGIC_CURSOR_DOWN,
				    sizeof(LOGIC_CURSOR_DOWN));
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
	    != ((data->buffered_misc_state.cursor_state != CURSOR_OFF)
		    ? 1 : 0)) {
		/* Cursor state mismatch */
		if (data->display_misc_state.cursor_state) {
			buffer = bytecpy_advance_ptr(buffer, LOGIC_CURSOR_OFF,
						     sizeof(LOGIC_CURSOR_OFF));
		}
		else {
			buffer = bytecpy_advance_ptr(buffer, LOGIC_CURSOR_ON,
						     sizeof(LOGIC_CURSOR_ON));
		}
		data->display_misc_state.cursor_state =
		    !data->display_misc_state.cursor_state;
		if (data->display_misc_state.cursor_state) {
			/* Force cursor position sync */
			data->display_misc_state.cx = -1;
		}
	}

	/* Cursor position mismatch - check position */
	if ((data->display_misc_state.cx != data->buffered_misc_state.cx)
	    || (data->display_misc_state.cy != data->buffered_misc_state.cy)) {
		if (data->display_misc_state.cursor_state) {
			buffer = bytecpy_advance_ptr(buffer, LOGIC_CURSOR_MOVE,
						     sizeof(LOGIC_CURSOR_MOVE));
			*(buffer++) =
			    (data->buffered_misc_state.cy > 1) ?
				(0x13 + data->buffered_misc_state.cx) :
				(-1 + data->buffered_misc_state.cx);
		}
		data->display_misc_state.cx = data->buffered_misc_state.cx;
		data->display_misc_state.cy = data->buffered_misc_state.cy;
	}

	/* Flush brightness data*/
	int target_brightness = 0x20;
	if (data->buffered_misc_state.backlight_state != BACKLIGHT_OFF) {
		switch (serialPOS_convert_brightness(
		    data->buffered_misc_state.brightness)) {
		    /*
		     * Not the best mapping, but it'll have to do, I don't
		     * want to get SIGFPE or something
		     */
		    case 0x01:
			target_brightness = 0x20;
			break;
		    case 0x02:
			target_brightness = 0x40;
			break;
		    case 0x03:
			target_brightness = 0x60;
			break;
		    case 0x04:
			target_brightness = 0xff;
			break;
		}
	}
	else {
		target_brightness = 0x20;
	}
	if (data->display_misc_state.brightness != target_brightness) {
		buffer = bytecpy_advance_ptr(buffer, LOGIC_BRIGHTNESS_ADJUST,
					     sizeof(LOGIC_BRIGHTNESS_ADJUST));
		*(buffer++)			    = target_brightness;
		data->display_misc_state.brightness = target_brightness;
	}

	return (buffer - start);
}
