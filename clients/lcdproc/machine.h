
#ifndef _lcdproc_machine_h_
#define _lcdproc_machine_h_

#include "shared/LL.h"

#ifndef LOADAVG_NSTATS
# define LOADAVG_NSTATS	3
#endif

#ifndef LOADAVG_1MIN
# define LOADAVG_1MIN	0
#endif

#ifndef LOADAVG_5MIN
# define LOADAVG_5MIN	1
#endif

#ifndef LOADAVG_15MIN
# define LOADAVG_15MIN	2
#endif


typedef struct
{
	unsigned long total;	/**< total time (in USER_HZ; since last call) */
	unsigned long user;	/**< time in user mode (in USER_HZ; since last call) */
	unsigned long system;	/**< time in kernel mode (in USER_HZ; since last call) */
	unsigned long nice;	/**< time in 'niced' user mode (in USER_HZ; since last call) */
	unsigned long idle;	/**< time idling (in USER_HZ; since last call) */
} load_type;


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


typedef struct
{
	int total;		/**< total memory (in kB) */
	int cache;		/**< memory in page cache (in kB) */
	int buffers;		/**< memory in buffer cache (in kB) */
	int free;		/**< free memory (in kB) */
	int shared;		/**< ??? (in kB) */
} meminfo_type;


typedef struct
{
	char name[16];		/**< process name */
	int totl;		/**< process memory usage (in kB) */
	int number;		/**< incstances of the process */
} procinfo_type;


/* status definitions for network interfaces */
typedef enum {
	down = 0,
	up = 1,
} IfaceStatus;

/* Struct for network interface values (transmision, reception, etc..) */
typedef struct iface_info
{
	/* interface name and alias (=display name) */
	char *name;
	char *alias;

	IfaceStatus status;

	time_t last_online;

	/* received bytes */
	double rc_byte;
	double rc_byte_old;

	/* transmited bytes */
	double tr_byte;
	double tr_byte_old;

	/* received packets */
	double rc_pkt;
	double rc_pkt_old;

	/* transmited packets */
	double tr_pkt;
	double tr_pkt_old;
} IfaceInfo;


int machine_init();
int machine_close();

int machine_get_battstat(int *acstat, int *battflag, int *percent);
int machine_get_fs(mounts_type fs[], int *cnt);
int machine_get_load(load_type *cur_load);
int machine_get_loadavg(double *load);
int machine_get_meminfo(meminfo_type *result);
int machine_get_procs(LinkedList *procs);
int machine_get_smpload(load_type *result, int *numcpus);
int machine_get_uptime(double *up, double *idle);
int machine_get_iface_stats (IfaceInfo *interface);


#endif /* _lcdproc_machine_h_ */
