/*
 * fileio.c
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

#include <stdlib.h>
#include <string.h>
#include "fileio.h"

#define FILECHUNKSIZE 10

buffile *buffile_open( const char *path, const char *mode ) {
	buffile * file = NULL;

	file = malloc(sizeof(*file));
	if ( NULL == file )
		return NULL;
	
	file->pos=0;
	file->bytesread=0;
	
	file->f = fopen( path, mode );
	if( NULL == file->f ) {
		free(file);
		return NULL;
	}

	return file;
}

int buffile_close ( buffile * file ) {
	int retval=0;

	if (NULL == file)
		return -1;
	if ( NULL != file->f )
		retval = fclose( file->f );
	if ( NULL != file->buf )
		free ( file->buf );
	free (file);
	file = NULL;

	return retval;
}

char * buffile_read ( buffile * file, size_t n ) {
	char * returnbuf=NULL;

	if( NULL == file )
		return NULL;

	returnbuf = malloc(n);
	if( NULL == returnbuf)
		return NULL;

	if( NULL == file->buf ) {
		file->buf = malloc(FILECHUNKSIZE);
		if( NULL == file->buf )
			return NULL;
	}

	if( file->pos >= file->bytesread ) {
		if( !( file->bytesread = fread( file->buf, 1, FILECHUNKSIZE, file->f ))) {
			/* We're at the end*/
			returnbuf[0]=0;
			return returnbuf;
		}
		file->pos = 0;
	}
	strncpy(returnbuf, file->buf + (file->pos++), sizeof(returnbuf));
	return returnbuf;
}

