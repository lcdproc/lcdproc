/*
 * fileio.h
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2003, Rene Wagner
 *
 *
 * Defines routines to read from buffered files.
 *
 */

#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>

typedef struct buffile {
	FILE * f;
	char * buf;
	int bytesread, pos;
} buffile;

buffile *buffile_open( const char *path, const char *mode );
/* Opens the given file and returns a pointer to the corresponding
 * buffile object.
 */
int buffile_close ( buffile * file );
/* Tries to close the file corresponding to the given buffile
 * object and to free the memory used by the buffile object itself.
 * Returns 0 on success, return value of fclose(3) in case of errors
 * with fclose.
 */
char * buffile_read ( buffile * file, size_t n );
/* Reads n bytes from the given buffile object.
 */

#endif
