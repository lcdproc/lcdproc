/**
 * \file server/drivers/serialPOS_common.c
 *
 * Common functionality used by serialPOS protocol drivers
 */

/*-
 * These are the common helper functions for driving Point Of Sale ("POS")
 * devices using the LCDproc serialPOS driver.
 *
 * Some protocols share common initialization sequences / operations, and,
 * hence, their common functions are defined here to avoid code replication.
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

uint8_t
serialPOS_convert_brightness(int promille)
{
	return ((promille / 250 == 0) ? 1 : (promille / 250));
}

void
serialPOS_generate_horizontal_block_glyph(uint8_t* buffer, int width)
{
	uint8_t single_column = 0x7f;
	memset(buffer, 0, 0x05);
	memset(buffer, single_column, width);
}

void
serialPOS_generate_vertical_block_glyph(uint8_t* buffer, int height)
{
	uint8_t column = 0;
	for (int bits_set = 1; bits_set <= height; bits_set++) {
		column |= (0x01 << (bits_set - 1));
	}
	memset(buffer, column, 0x05);
}

uint32_t
serialPOS_lines_to_flush(PrivateData* data)
{
	unsigned int flush_lines = 0;
	for (int h = 0; h < data->height; h++) {
		uint8_t* framebuf_line_start =
		    data->framebuf + ((data->width) * h);
		uint8_t* backingstore_line_start =
		    data->backingstore + ((data->width) * h);
		if (memcmp(framebuf_line_start, backingstore_line_start,
			   data->width)) {
			memcpy(backingstore_line_start, framebuf_line_start,
			       data->width);
			flush_lines |= (0x01 << h);
		}
	}
	return flush_lines;
}
