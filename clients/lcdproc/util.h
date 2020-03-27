/** \file clients/lcdproc/util.h
 * Header file for \c util.c
 */

/*-
 * Copyright (C) 2005  Peter Marschall
 *
 * This file is part of lcdproc, the lcdproc client.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 */

#ifndef LCDPROC_UTIL_H
#define LCDPROC_UTIL_H

#include <stdio.h>
#include <string.h>

/** print a memory value with the correct unit to a given string */
char *sprintf_memory(char *dst, double value, double roundlimit);

/** print a percentage value to a given string */
char *sprintf_percent(char *dst, double percent);

/** converts value into power-of-x representation */
char *convert_double(double *value, int base, double roundlimit);

/** add a pbar widget, auto-falling back to hbar if the server is too old */
void pbar_widget_add(const char *screen, const char *name);

/** set parameters for a pbar widget added with pbar_widget_add() */
void pbar_widget_set(const char *screen, const char *name, int x, int y, int width,
		     int promille, char *begin_label, char *end_label);

#endif
