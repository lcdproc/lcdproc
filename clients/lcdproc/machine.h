/** \file clients/lcdproc/machine.h
 * Common data types and function declarations
 * for OS specific sources in \c machine_{Darwin,Linux,SunOS,*BSD}.c.
 *
 * In order to port the \e lcdproc client to a new operating system,
 * a new file \c machine_XXX.c (where \c XXX = OS name) needs to be
 * created, that provides functions to the function declarations below.
 * The aleady existing \c machine_*.c files can be used as examples.
 *
 */

#ifndef _lcdproc_machine_h_
#define _lcdproc_machine_h_

#include "shared/LL.h"

#ifndef LOADAVG_NSTATS
# define LOADAVG_NSTATS	3	/**< size of loadavg[] parameter to getloadavg() */
#endif

#ifndef LOADAVG_1MIN
# define LOADAVG_1MIN	0	/**< index in loadavg[] parameter to getloadavg() for 1 minute load average */
#endif

#ifndef LOADAVG_5MIN
# define LOADAVG_5MIN	1	/**< index in loadavg[] parameter to getloadavg() for 5 minute load average */
#endif

#ifndef LOADAVG_15MIN
# define LOADAVG_15MIN	2	/**< index in loadavg[] parameter to getloadavg() for 15 minute load average */
#endif

#ifndef MAX_CPUS
# define MAX_CPUS	16	/**< maximal number of CPUs for which load history is kept */
#endif


/** Information about CPU load */
typedef struct
{
	unsigned long total;	/**< total time (in USER_HZ; since last call) */
	unsigned long user;	/**< time in user mode (in USER_HZ; since last call) */
	unsigned long system;	/**< time in kernel mode (in USER_HZ; since last call) */
	unsigned long nice;	/**< time in 'niced' user mode (in USER_HZ; since last call) */
	unsigned long idle;	/**< time idling (in USER_HZ; since last call) */
} load_type;


/** Information about mounted file systems */
typedef struct
{
	char dev[256];		/**< device name */
	char type[64];		/**< file system type (as string) */
	char mpoint[256];	/**< mount point name */
	long bsize;		/**< transfer block size */
	long blocks;		/**< total data blocks in file system */
	long bfree;		/**< free blocks in fs */
	long files;		/**< total file nodes in file system */
	long ffree;		/**< free file nodes in fs */
} mounts_type;


/** Information about memory status */
typedef struct
{
	long total;		/**< total memory (in kB) */
	long cache;		/**< memory in page cache (in kB) */
	long buffers;		/**< memory in buffer cache (in kB) */
	long free;		/**< free memory (in kB) */
	long shared;		/**< ??? (in kB) */
} meminfo_type;


/** Information about processes and their size */
typedef struct
{
	char name[16];		/**< process name */
	long totl;		/**< process memory usage (in kB) */
	int number;		/**< incstances of the process */
} procinfo_type;


/** Status definitions for network interfaces */
typedef enum {
	down = 0,
	up = 1,
} IfaceStatus;

/* Network Interface information */
typedef struct iface_info
{
	char *name;		/**< physical interface name */
	char *alias;		/**< displayed name of interface */

	IfaceStatus status;	/**< status of the interface */

	time_t last_online;

	double rc_byte;		/**< currently received bytes */
	double rc_byte_old;	/**< previously received bytes */

	double tr_byte;		/**< currently sent bytes */
	double tr_byte_old;	/**< previously sent bytes */

	double rc_pkt;		/**< currently received packages */
	double rc_pkt_old;	/**< previously received packages */

	double tr_pkt;		/**< currently sent packages */
	double tr_pkt_old;	/**< previously sent packages */
} IfaceInfo;


/**
 * Set up OS specific functions.
 * \retval  FALSE  Error
 * \retval  TRUE   OK
 */
int machine_init(void);

/**
 * Tear down (clean up) OS specific functions.
 * \retval  FALSE  Error
 * \retval  TRUE   OK
 */
int machine_close(void);


/**
 * Get battery information.
 * \param  acstat   Pointer to information whether the system runs on AC power or on battery.
 * \param  battflag Pointer to information about the battery load status.
 * \param  percent  Pointer to the battery fill state.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_battstat(int *acstat, int *battflag, int *percent);

/**
 * get information about mounted file systems.
 * \param  fs   Pointer to array where file system information gets stored.
 * \param  cnt  Number of mounted file systems.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_fs(mounts_type fs[], int *cnt);

/**
 * Get total CPU load (added over all CPUs).
 * \param  cur_load  Pointer where to store current load information.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_load(load_type *cur_load);

/**
 * Get load 1min load average.
 * \param  load  Pointer where to store the current 1min load average.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_loadavg(double *load);

/**
 * Get information about memory.
 * \param  result  Pointer where meminfo is to be stored.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_meminfo(meminfo_type *result);

/**
 * Get list of processes and their sizes.
 * \param  procs  Pointer where to store the linked List of processes.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_procs(LinkedList *procs);

/**
 * Get CPU load split up for each CPU.
 * \param  result  Pointer to array of CPU load info.
 * \param  numcpus Number of CPUs found.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_smpload(load_type *result, int *numcpus);

/**
 * Get uptime.
 * \param  up    Pointer to store the uptime in seconds.
 * \param  idle  Pointer to store the percentage in which the CPUs idled since boot.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_uptime(double *up, double *idle);

/**
 * Read network interface statistics for a single interface from system and
 * store results in the struct IfaceInfo.
 *
 * \param  interface Pointer where to store interface info.
 * \retval  FALSE  Error, do not trust the contents of the parameter pointers.
 * \retval  TRUE   OK, parameter pointers are filled with sensible data.
 */
int machine_get_iface_stats(IfaceInfo *interface);


#endif /* _lcdproc_machine_h_ */
