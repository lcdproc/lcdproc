/*
 * configfile.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2001, Joris Robijn
 *
 *
 * Defines routines to read ini-file-like files.
 *
 */

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

int config_read_file( char *filename );
/* Opens the specified file and reads everything into memory.
 * Returns -1 on parsing errors.
 * Returns -2 if the file could not be opened or a read error occured.
 * Returns -16 if a malloc went wrong.
 */

int config_read_string( char *sectionname, char *str );
/* Reads everything in the string into memory.
 * Returns -1 on parsing errors.
 * Returns -16 if a malloc went wrong.
 */

unsigned char config_get_bool( char *sectionname, char *keyname,
		int skip, unsigned char default_value );
/* Tries to interpret a value in the config file as a boolean.
 * 0, false, no, n = false
 * 1, true, yes, y = true
 * If the key is not found or cannot be interpreted, the given default value is
 * returned.
 * The skip value can be used to iterate over multiple values with the same
 * key. Should be 0 to get the first one, 1 for the second etc. and -1 for the
 * last.
 */

long int config_get_int( char *sectionname, char *keyname,
		int skip, long int default_value );
/* Tries to interpret a value in the config file as an integer.*/

double config_get_float( char *sectionname, char *keyname,
		int skip, double default_value );
/* Tries to interpret a value in the config file as a float.*/

char *config_get_string( char * sectionname, char * keyname,
		int skip, char * default_value );
/* Returns a pointer to the string associated with the specified key.
 * The string should never be modified, and used only short-term. You can
 * for example scan it or copy it. In successive calls this function can
 * re-use the data space !
 *
 * found in a mail from Joris:
 *
 * The strings it returns are always terminated.
 *
 * You can do some things with them:
 * 1. Scan them:
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
 * 
 */

int config_has_section( char *sectionname );
/* Checks if a specified section exists.
 * Returns whether it exists.
 */

int config_has_key( char *sectionname, char *keyname );
/* Checks if a specified key within the specified section exists.
 * Returns the number of times the key exists.
 */

void config_clear();
/* Clears all data stored by the config_read_* functions.
 * Should be called if the config should be reread.
 */

#endif
