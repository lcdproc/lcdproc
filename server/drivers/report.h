#ifndef REPORT_H
#define REPORT_H

/* DEBUGGING / REPORTING FOR DRIVERS
 *
 * This file uses the reporting functions from the server core.
 * See the file shared/report.h for details.
 *
 * This file assumes that the drivers have a drvthis parameter that contains the
 * current driver structure. It redefines report to make its use simple:
 *
 * report( RPT_ERR, "report this: %s", str );
 * debug( RPT_ERR, "report this if debug enabled: %s", str );
 *
 */

// Reporting levels
#define RPT_CRIT 0
#define RPT_ERR 1
#define RPT_WARNING 2
#define RPT_NOTICE 3
#define RPT_INFO 4
#define RPT_DEBUG 5

#define report drvthis->report
// This assumes drvthis is locally defined... Anyone has a better idea ?

static inline void dont_report( const int level, const char *format, .../*args*/ )
{} // The idea is that this gets optimized out

#ifdef DEBUG
#  define debug report
#else
#  define debug dont_report
#endif /*DEBUG*/

#endif
