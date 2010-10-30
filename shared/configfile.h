/** \file configfile.h
 * Declare routines to read INI-file like files.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *           (c) 2006,2007 Peter Marschall
 *
 */

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Opens the specified file and reads everything into memory.
 * Returns 0  when config file was successfully parsed
 * Returns <0 on errors
 */
int config_read_file(const char *filename);

#if defined(LCDPROC_CONFIG_READ_STRING)
/* Reads everything in the string into memory.
 * Returns 0  when config file was successfully parsed
 * Returns <0 on errors
 */
int config_read_string(const char *sectionname, const char *str);
#endif

/* Tries to interpret a value in the config file as a boolean.
 * 0, false, off, no, n = false
 * 1, true, on, yes, y = true
 * If the key is not found or cannot be interpreted, the given default value is
 * returned.
 * The skip value can be used to iterate over multiple values with the same
 * key. Should be 0 to get the first one, 1 for the second etc. and -1 for the
 * last.
 */
short config_get_bool(const char *sectionname, const char *keyname,
		int skip, short default_value);

/* Tries to interpret a value in the config file as a boolean.
 * 0, false, off, no, n = 0
 * 1, true, on, yes, y = 1
 * 2, or the given 3rd name = 2
 * If the key is not found or cannot be interpreted, the given default value is
 * returned.
 * The skip value can be used to iterate over multiple values with the same
 * key. Should be 0 to get the first one, 1 for the second etc. and -1 for the
 * last.
 */
short config_get_tristate(const char *sectionname, const char *keyname,
		int skip, const char *name3rd, short default_value);

/* Tries to interpret a value in the config file as an integer.*/
long int config_get_int(const char *sectionname, const char *keyname,
		int skip, long int default_value);

/* Tries to interpret a value in the config file as a float.*/
double config_get_float(const char *sectionname, const char *keyname,
		int skip, double default_value);

/* Returns a pointer to the string associated with the specified key.
 * The strings returned are always NUL-terminated.
 * The string should never be modified, and used only short-term.
 * In successive calls this function can * re-use the data space !
 *
 * You can do some things with the returned string:
 * 1. Scan or parse it:
 *    s = config_get_string(...);
 *    sscanf( s, "%dx%d", &w, &h );  // scan format like: 20x4
 *    ...and check the w and h values...
 * 2. Copy it to a preallocated buffer like device[256]:
 *    s = config_get_string(...);
 *    strncpy( device, s, sizeof(device));
 *    device[sizeof(device)-1] = 0;
 * 3. Copy it to a newly allocated space in char *device:
 *    s = config_get_string(...);
 *    device = malloc(strlen(s)+1);
 *    if( device == NULL ) return -5; // or whatever < 0
 *    strcpy( device, s );
 */
const char *config_get_string(const char *sectionname, const char *keyname,
		int skip, const char *default_value);

/* Checks if a specified section exists.
 * Returns whether it exists.
 */
int config_has_section(const char *sectionname);

/* Checks if a specified key within the specified section exists.
 * Returns the number of times the key exists.
 */
int config_has_key(const char *sectionname, const char *keyname);

/* Clears all data stored by the config_read_* functions.
 * Should be called if the config should be reread.
 */
void config_clear(void);

#endif
