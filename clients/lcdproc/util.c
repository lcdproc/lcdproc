/** \file clients/lcdproc/util.c
 * Utility functions to print some numerical values in a nice fashion
 */

/*-
 *  Copyright (C) 2005  Peter Marschall
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301.
 */

#include <sys/types.h>
#include "shared/sockets.h"
#include "main.h"
#include "util.h"


/** Print a memory value with the correct unit to a given string.
 * \param *dst        String to print the value to.
 * \param value       Value to print.
 * \param roundlimit  Set < 1.0 if precision of original input value is not sufficient.
 * \return  dst
 */
char *
sprintf_memory(char *dst, double value, double roundlimit)
{
	if (dst != NULL) {
		char *format = "%.1f%s";

		char *unit = convert_double(&value, 1024, roundlimit);

		if (value <= 99.994999999)
			format = "%.2f%s";
		if (value <= 9.9994999999)
			format = "%.3f%s";

		sprintf(dst, format, value, unit);
	}
	return dst;
}


/** Print a percentage value to a given string.
 * \param *dst     String to print the percentage value to.
 * \param percent  Value to print.
 * \return  dst
 */
char *
sprintf_percent(char *dst, double percent)
{
	if (dst != NULL) {
		if (percent > 99.9)
			strcpy(dst, "100%");
		else
			sprintf(dst, "%.1f%%", (percent >= 0) ? percent : 0);
	}
	return dst;
}


/** Convert a value with unit value.
 * does not do formatting
 * \param *value       Pointer to the value to be scaled
 * \param base         Base used for scaling (e.g. 1000 for decimal, 1024 for binary units)
 * \param roundlimit   Set < 1.0 if precision of original input value is not sufficient
 * \return       Unit string to be used when displaying value
 */
char *
convert_double(double *value, int base, double roundlimit)
{
	static char *units[] = {"", "k", "M", "G", "T", "P", "E", "Z", "Y", NULL};
	int off = 0;

	if ((roundlimit <= 0.0) || (roundlimit > 1.0))
		roundlimit = 0.5;

	/* Get value's order of magnitude */
	while (units[off] != NULL) {
		/*
		 * Note: the check for 1000 is to idenfity the number of
		 * characters in the output needed, not to decide if to scale
		 * value.
		 */
		if (*value < 1000 * roundlimit)
			break;
		off++;
		*value /= base;
	}
	return units[off];
}

/**
 * Add a pbar widget, auto-falling back to hbar if the server is too old.
 * \param screen       Name of the screen to add the widget to.
 * \param name         Name of the widget.
 */
void
pbar_widget_add(const char *screen, const char *name)
{

	if (check_protocol_version(0, 4)) {
		sock_printf(sock, "widget_add %s %s pbar\n", screen, name);
	} else {
		sock_printf(sock, "widget_add %s %s-begin-label string\n",
			    screen, name);
		sock_printf(sock, "widget_add %s %s hbar\n",
			    screen, name);
		sock_printf(sock, "widget_add %s %s-end-label string\n",
			    screen, name);
	}
}

/**
 * Set parameters for a pbar widget added with pbar_widget_add().
 * \param screen       Name of the screen to add the widget to.
 * \param name         Name of the widget.
 * \param x            Horizontal character position (column) of the starting point.
 * \param y            Vertical character position (row) of the starting point.
 * \param width        Width of the widget in characters, including the
 *                     optional begin and end-labels.
 * \param promille     Current length level of the bar in promille.
 * \param begin_label  Optional (may be NULL) text to render in front of /
 *                     at the beginning of the percentage-bar.
 * \param end_label    Optional text to render at the end of the pbar.
 */
void
pbar_widget_set(const char *screen, const char *name, int x, int y, int width,
		int promille, char *begin_label, char *end_label)
{
	int begin_length, end_length, len, hbar_pixels;

	if (check_protocol_version(0, 4)) {
		if (begin_label || end_label)
			sock_printf(sock, "widget_set %s %s %d %d %d %d {%s} {%s}\n",
				    screen, name, x, y, width, promille,
				    begin_label ? begin_label : "",
				    end_label ? end_label : "");
		else
			sock_printf(sock, "widget_set %s %s %d %d %d %d\n",
				    screen, name, x, y, width, promille);
		return;
	}

	/* pbar widget is not supported by the server, emulate it using a hbar */

	/* since we are falling back to drawing a hbar we need some sort of
	 * begin / end markers for the hbar. If neither label is given,
	 * draw [] around the hbar to mark the beginning and end of the bar.
	 */
	if (begin_label == NULL && end_label == NULL) {
		begin_label = "[";
		end_label = "]";
	}

	if (begin_label == NULL)
		begin_label = "";

	if (end_label == NULL)
		end_label = "";

	begin_length = strlen(begin_label);
	end_length = strlen(end_label);

	/* We want at least 2 chars for the bar itself */
	if ((begin_length + end_length + 2) > width) {
		begin_label = end_label = "";
		begin_length = end_length = 0;
	}

	len = width - begin_length - end_length;

	sock_printf(sock, "widget_set %s %s-begin-label %d %d {%s}\n",
		    screen, name, x, y, begin_label);
	x += begin_length;

	/* hbar takes number of pixels to fill as 3th argument */
	hbar_pixels = (promille * lcd_cellwid * len + 500) / 1000;
	sock_printf(sock, "widget_set %s %s %d %d %d\n",
		    screen, name, x, y, hbar_pixels);
	x += len;

	sock_printf(sock, "widget_set %s %s-end-label %d %d {%s}\n",
		    screen, name, x, y, end_label);
}

/* EOF */
