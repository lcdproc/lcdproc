/*
 * report.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 1999, William Ferrell, Scott Scriven
 *		 2001, Joris Robijn
 *		 2005, Peter Marschall
 *
 * Contains reporting functions
 *
 */

#include <stdlib.h>
#include "report.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
# include <syslog.h>
#else
# include <windows.h>
#endif

static int report_level = RPT_INFO;
static int report_dest = RPT_DEST_STORE;

#ifdef WIN32
HANDLE event_log_handle = NULL;
#endif

#define MAX_STORED_MSGS 200

static char *stored_msgs[MAX_STORED_MSGS];
static int stored_levels[MAX_STORED_MSGS];
static int num_stored_msgs = 0;


/* local functions */
static void store_report_message( int level, const char *message );
static void flush_messages();

void report( const int level, const char *format, .../*args*/ )
{
	/* Check if we should report it */
	if( level <= report_level || report_dest == RPT_DEST_STORE ) {

	  	char buf[1024];

		/* Following functions appear to work on RedHat and Debian
		 * Linux, FreeBSD and Solaris
		 */

		va_list ap;
		va_start(ap, format); /* measure the required size (the number of elements of format) */

		switch( report_dest ) {
		  case RPT_DEST_STDERR:
			vfprintf( stderr, format, ap );
			fprintf( stderr, "\n" );
			break;
		  case RPT_DEST_SYSLOG:
                  {
#ifndef WIN32
			vsyslog( LOG_USER|(level+2), format, ap );
#else
                        LPTSTR tmpArray[1];
                        WORD eventType;
                        switch (level)
                        {
                        case RPT_CRIT:
                        case RPT_ERR:
                            eventType = EVENTLOG_ERROR_TYPE;
                            break;
                        case RPT_WARNING:
                            eventType = EVENTLOG_WARNING_TYPE;
                            break;
                        case RPT_NOTICE:
                        case RPT_INFO:
                        case RPT_DEBUG:
                        default:
                            eventType = EVENTLOG_INFORMATION_TYPE;
                            break;
                        }

                        vsnprintf( buf, sizeof(buf), format, ap );
                        tmpArray[0] = buf;
                        ReportEvent(event_log_handle,
                                    eventType,
                                    0,        /* category */
                                    0,        /* event ID */
                                    NULL,     /* user ID */
                                    1,        /* number of strings */
                                    0,         /* binary data size */
                                    (LPCTSTR*) tmpArray,   /* error string */
                                    NULL       /* binary data */
                            );
#endif
                  }
			break;
		  case RPT_DEST_STORE:
			vsnprintf( buf, sizeof(buf), format, ap );
			buf[sizeof(buf)-1] = 0; /* be sure to have a terminating 0 */
			store_report_message( level, buf );
			break;
		}
		va_end(ap);
	}
}


int set_reporting( char *application_name, int new_level, int new_dest )
{
	if( new_level < RPT_CRIT || new_level > RPT_DEBUG ) {
		report( RPT_ERR, "report level invalid: %d", new_level );
		return -1;
	}

	if( report_dest != RPT_DEST_SYSLOG && new_dest == RPT_DEST_SYSLOG ) {
#ifdef WIN32
                if (event_log_handle)
                {
                        CloseEventLog(event_log_handle);
                } else {
                        event_log_handle = RegisterEventSource(NULL, "LcdProc");
                }
#else
		openlog( application_name, 0, LOG_USER );
#endif
	}
	else if( report_dest == RPT_DEST_SYSLOG && new_dest != RPT_DEST_SYSLOG ) {
#ifdef WIN32
                if (event_log_handle)
                {
                        CloseEventLog(event_log_handle);
                        event_log_handle = NULL;
                }                
#else
		closelog();
#endif
	}

	report_level = new_level;
	report_dest = new_dest;

	if( report_dest != RPT_DEST_STORE )
		flush_messages();

	return 0;
}


static void store_report_message( int level, const char *message )
{
	if( num_stored_msgs < MAX_STORED_MSGS ) {
		stored_msgs[num_stored_msgs] = malloc(strlen( message )+1);
		strcpy( stored_msgs[num_stored_msgs], message );
		stored_levels[num_stored_msgs] = level;
		num_stored_msgs ++;
	}
}


static void flush_messages()
{
	int i;
	for( i=0; i<num_stored_msgs; i++ ) {
		report( stored_levels[i], "%s", stored_msgs[i] );
		free( stored_msgs[i] );
	}
	num_stored_msgs = 0;
}
