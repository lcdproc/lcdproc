/*
 * This file contains status-gathering code *and* modescreen functions.
 * It's long, and messy; but that will change with V0.4.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/utsname.h>

#ifdef LINUX
#include <sys/vfs.h>
#else
#include <sys/statfs.h>
#endif

#include "mode.h"
#include "lcd.h"
#include "main.h"

#ifndef NETDEVICE
#define NETDEVICE "ppp0"
#endif

#ifndef LOAD_MAX
#define LOAD_MAX 1.3
#endif
#ifndef LOAD_MIN
#define LOAD_MIN 0.5
#endif


int PAD=255;
int ELLIPSIS=7;
int TwentyFourHour=0;

// TODO: Clean this up...  Support multiple display sizes..

struct load { unsigned long total, user, system, nice, idle; };
struct meminfo { int total, cache, buffers, free, shared; };
static char buffer[1024];

// Nothing else can see these...
static int meminfo_fd, load_fd, loadavg_fd, uptime_fd, batt_fd=0;
static FILE *mtab_fd;

static char kver[SYS_NMLN];
static char sysname[SYS_NMLN];

static void reread(int f, char *errmsg);
static int getentry(const char *tag, const char *bufptr);
static void get_mem_info(struct meminfo *result);
static double get_loadavg(void);
static double get_uptime(double *up, double *idle);
static void get_load(struct load * result);
static int get_batt_stat(int *acstat, int *battstat,
			 int *battflag, int *percent);


typedef struct mounts 
{ 
  char dev[256], type[64], mpoint[256];
  long bsize, blocks, bfree, files, ffree; 
} mounts;


int mode_init() 
{
  struct utsname *unamebuf = 
    (struct utsname *) malloc( sizeof(struct utsname) );

  meminfo_fd = open("/proc/meminfo",O_RDONLY);
  loadavg_fd = open("/proc/loadavg",O_RDONLY);
  load_fd = open("/proc/stat",O_RDONLY);
  uptime_fd = open("/proc/uptime",O_RDONLY);

#if 0
  kversion_fd = open("/proc/sys/kernel/osrelease",O_RDONLY);

  reread(kversion_fd, "main:");
  sscanf(buffer, "%s", kver);

  close(kversion_fd);
# endif

  /* Get OS name and version from uname() */
  if( uname( unamebuf ) != 0 ) {
    perror( "Error calling uname:" );
  }
  strcpy( kver, unamebuf->release );
  strcpy( sysname, unamebuf->sysname );

  return 0; 
}

void mode_close() 
{
  close(meminfo_fd);
  close(loadavg_fd);
  close(load_fd);
  close(uptime_fd);
  if(batt_fd) close(batt_fd);
  
}


static void reread(int f, char *errmsg) 
{
  if (lseek(f, 0L, 0) == 0 && read(f, buffer, sizeof(buffer) - 1 ) > 0 )
    return;
  perror(errmsg);
  exit(1); 
}

static int getentry(const char *tag, const char *bufptr) 
{
  char *tail;
  int retval, len = strlen(tag);

  while (bufptr) 
  {
    if (*bufptr == '\n') bufptr++;
    if (!strncmp(tag, bufptr, len)) 
    {
      retval = strtol(bufptr + len, &tail, 10);
      if (tail == bufptr + len) return -1; 
      else return retval; 
    }
    bufptr = strchr( bufptr, '\n'); 
  }
  return -1; 
}

static void get_mem_info(struct meminfo *result) 
{
//  int i, res; char *bufptr;

  reread(meminfo_fd, "get_meminfo:");
  result[0].total   = getentry("MemTotal:", buffer);
  result[0].free    = getentry("MemFree:", buffer);
  result[0].shared  = getentry("MemShared:", buffer);
  result[0].buffers = getentry("Buffers:", buffer);
  result[0].cache   = getentry("Cached:", buffer);
  result[1].total   = getentry("SwapTotal:", buffer);
  result[1].free    = getentry("SwapFree:", buffer); 
}

static double get_loadavg(void) 
{
  double load;

  reread(loadavg_fd, "get_load:");
  sscanf(buffer, "%lf", &load);
  return load; 
}

static double get_uptime(double *up, double *idle)
{
  double uptime;

  reread(uptime_fd, "get_uptime:");
  sscanf(buffer, "%lf %lf", &uptime, idle);

  *up = uptime;
  
  return uptime; 
}

static int get_fs(mounts fs[])
{
  struct statfs fsinfo;
  char line[256];
  int x = 0, y;
  
  mtab_fd = fopen("/etc/mtab", "r");

  // Get rid of old, unmounted filesystems...
  memset(fs, 0, sizeof(mounts)*256);
  
  while (x < 256)
  {
    if(fgets(line, 256, mtab_fd) == NULL)
      {
	fclose(mtab_fd);
	return x;
      }
    
    sscanf(line, "%s %s %s", fs[x].dev, fs[x].mpoint, fs[x].type);
    
    if(   strcmp(fs[x].type, "proc")
#ifndef STAT_NFS
       && strcmp(fs[x].type, "nfs")
#endif
#ifndef STAT_SMBFS
       && strcmp(fs[x].type, "smbfs")
#endif
      ) 
    {
#ifdef LINUX
      y = statfs(fs[x].mpoint, &fsinfo); 
#else
      y = statfs(fs[x].mpoint, &fsinfo, sizeof(fsinfo), 0); 
#endif
      fs[x].bsize = fsinfo.f_bsize; 
      fs[x].blocks = fsinfo.f_blocks;
      fs[x].bfree = fsinfo.f_bfree; 
      fs[x].files = fsinfo.f_files;
      fs[x].ffree = fsinfo.f_ffree; 
      x++;
    }
  }

  fclose(mtab_fd);
  return x;
}

static void get_load(struct load * result) 
{
  static struct load last_load = { 0, 0, 0, 0, 0 }; struct load curr_load;

  reread(load_fd, "get_load:");
  sscanf(buffer, "%*s %lu %lu %lu %lu\n", &curr_load.user, &curr_load.nice, &curr_load.system, &curr_load.idle);
  curr_load.total = curr_load.user + curr_load.nice + curr_load.system + curr_load.idle;
  result->total  = curr_load.total  - last_load.total;
  result->user   = curr_load.user   - last_load.user;
  result->nice   = curr_load.nice   - last_load.nice;
  result->system = curr_load.system - last_load.system;
  result->idle   = curr_load.idle   - last_load.idle;
  last_load.total  = curr_load.total;
  last_load.user   = curr_load.user;
  last_load.nice   = curr_load.nice;
  last_load.system = curr_load.system;
  last_load.idle   = curr_load.idle; 
}


static int get_batt_stat(int *acstat, int *battstat,
			 int *battflag, int *percent)
{
  char str[64];
  
  if(!batt_fd)
    batt_fd = open("/proc/apm", O_RDONLY);
  if(batt_fd <= 0) return -1;

  if(lseek(batt_fd, 0, 0) != 0) return -1;

  if(read(batt_fd, str, sizeof(str)-1) < 0) return -1;
  
  if(3 > sscanf(str+13, "0x%x 0x%x 0x%x %d",
		acstat, battstat, battflag, percent))
    return -1;

  return 0;
  
}


///////////////////////////////////////////////////////////////////////////
//////////////////////// Let the Modes Begin! /////////////////////////////
///////////////////////////////////////////////////////////////////////////

// TODO: Implement more return values for behavior changing...
// TODO: more modes...

static char tmp[256];

// A couple of tables for later use...
static char *days[] = {
  "Sunday,   ",
  "Monday,   ",
  "Tuesday,  ",
  "Wednesday,",
  "Thursday, ",
  "Friday,   ",
  "Saturday, ",
};
static char *shortdays[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
};

static char *months[] = {
  "  January",
  " February",
  "    March",
  "    April",
  "      May",
  "     June",
  "     July",
  "   August",
  "September",
  "  October",
  " November",
  " December",
};
static char *shortmonths[] = {
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};


//////////////////////////////////////////////////////////////////////////
// CPU screen shows info about percentage of the CPU being used
//
int cpu_screen(int rep)
{
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 4
  int i, j, n;
  float value;
  static float cpu[CPU_BUF_SIZE + 1][5];// last buffer is scratch
  struct load load;



  get_load(&load);

  // Shift values over by one
  for(i=0; i<(CPU_BUF_SIZE-1); i++)
    for(j=0; j<5; j++)
      cpu[i][j] = cpu[i+1][j];

  // Read new data
  cpu[CPU_BUF_SIZE-1][0] = ((float)load.user / (float)load.total) * 100.0;
  cpu[CPU_BUF_SIZE-1][1] = ((float)load.system / (float)load.total) * 100.0;
  cpu[CPU_BUF_SIZE-1][2] = ((float)load.nice / (float)load.total) * 100.0;
  cpu[CPU_BUF_SIZE-1][3] = ((float)load.idle / (float)load.total) * 100.0;
  cpu[CPU_BUF_SIZE-1][4] = (((float)load.user + (float)load.system +
  (float)load.nice) / (float)load.total) * 100.0;

  // Only clear on first display...
  if(!rep)
  {
    lcd.clear(); 
    lcd.init_hbar();

    sprintf(tmp, "%c%c CPU LOAD       %c%c", PAD, PAD, PAD, PAD);

    lcd.string(1, 1, tmp);
    lcd.string(1, 2, "Usr  0.0% Nice  0.0%");
    lcd.string(1, 3, "Sys  0.0% Idle  0.0%");
    lcd.string(1, 4, "0%              100%");

    // Make all the same, if this is the first time...
    for(i=0; i<CPU_BUF_SIZE-1; i++)
      for(j=0; j<5; j++)
        cpu[i][j] = cpu[CPU_BUF_SIZE-1][j];
  }



  // Average values for final result
  for(i=0; i<5; i++)
  {
    value = 0;
    for(j=0; j<CPU_BUF_SIZE; j++)
    {
      value += cpu[j][i];
    }
    value /= CPU_BUF_SIZE;
    cpu[CPU_BUF_SIZE][i] = value;
  }


  value = cpu[CPU_BUF_SIZE][4];
  n = (int)(value * 70.0);
  if (value >= 99.9) { lcd.string(13, 1, " 100%"); } 
  else { sprintf(tmp, "%4.1f%%", value); lcd.string(13, 1, tmp); }

  value = cpu[CPU_BUF_SIZE][0];
  if (value >= 99.9) { lcd.string(5, 2, " 100%"); } 
  else { sprintf(tmp, "%4.1f%%", value); lcd.string(5, 2, tmp); }

  value = cpu[CPU_BUF_SIZE][1];
  if (value >= 99.9) { lcd.string(5, 3, " 100%"); } 
  else { sprintf(tmp, "%4.1f%%", value); lcd.string(5, 3, tmp); }

  value = cpu[CPU_BUF_SIZE][2];
  if (value >= 99.9) { lcd.string(16, 2, " 100%"); } 
  else { sprintf(tmp, "%4.1f%%", value); lcd.string(16, 2, tmp); }

  value = cpu[CPU_BUF_SIZE][3];
  if (value >= 99.9) { lcd.string(16, 3, " 100%"); } 
  else { sprintf(tmp, "%4.1f%%", value); lcd.string(16, 3, tmp); }

  value = cpu[CPU_BUF_SIZE][4];
  n = (int)(value * 70.0 / 100.0);
  lcd.string(1, 4, "0%              100%");
  lcd.hbar(3, 4, n);

  return 0;
} // End cpu_screen()



//////////////////////////////////////////////////////////////////////////
// Cpu Graph Screen shows a quick-moving histogram of CPU use.
//
int cpu_graph_screen(int rep)
{
  int i, j, n;
  float value, maxload;
#undef CPU_BUF_SIZE
#define CPU_BUF_SIZE 2
  static float cpu[CPU_BUF_SIZE + 1];// last buffer is scratch
  static float cpu_past[LCD_MAX_WIDTH];
  struct load load;
  int status=0;
  char out[LCD_MAX_WIDTH];
  


  get_load(&load);

  // Shift values over by one
  for(i=0; i<(CPU_BUF_SIZE-1); i++)
      cpu[i] = cpu[i+1];

  // Read new data
  cpu[CPU_BUF_SIZE-1] = ((float)load.user + (float)load.system 
                      + (float)load.nice) / (float)load.total;


  // Only clear on first display...
  if(!rep)
  {
    lcd.init_vbar();

    // Make all the same, if this is the first time...
    for(i=0; i<CPU_BUF_SIZE-1; i++)
        cpu[i] = cpu[CPU_BUF_SIZE-1];

  }

  //lcd.clear();
  for(i=2; i<=lcd.hgt; i++)
    lcd.string(1,i,"                    ");
  

  // Average values for final result
  value = 0;
  for(j=0; j<CPU_BUF_SIZE; j++)
  {
    value += cpu[j];
  }
  value /= (float)CPU_BUF_SIZE;
  cpu[CPU_BUF_SIZE] = value;


  maxload=0;
  for(i=0; i<lcd.wid-1; i++)
  {
    cpu_past[i] = cpu_past[i+1];
    lcd.vbar(i+1, cpu_past[i]);
    if(cpu_past[i] > maxload) maxload = cpu_past[i];
  }

  value = cpu[CPU_BUF_SIZE];
  n = (int)(value * 8.0 * (float)(lcd.hgt-1));

  cpu_past[lcd.wid-1] = n;
  lcd.vbar(lcd.wid, cpu_past[lcd.wid-1]);

    sprintf(out, "%c%c CPU GRAPH %c%c%c%c%c%c", PAD,PAD,
	    PAD,PAD,PAD,PAD,PAD,PAD);
    lcd.string(1,1,out);
    


  if(n > maxload) maxload = n;

  if(cpu_past[lcd.wid-1] > 0 ) status = BACKLIGHT_ON;  
  if(maxload < 1) status = BACKLIGHT_OFF;

//  return status;
  return 0;
} // End cpu_graph_screen()



//////////////////////////////////////////////////////////////////////
// Clock Screen displays current time and date...
//
// TODO: 24-hour time on old clock screen, if desired.
int clock_screen(int rep)
{
  char hr[8], min[8], sec[8], ampm[8];
  char day[16], month[16];
  time_t thetime;
  struct tm *rtime;
  int i;


  if(!rep)
  {
    lcd.clear();
    sprintf(tmp, "%c%c DATE & TIME %c%c%c%c%c", PAD, PAD, PAD, PAD, PAD, PAD, PAD);
    lcd.string(1, 1, tmp);
  }

  time(&thetime);
  rtime = localtime(&thetime);

  strcpy(day, days[rtime->tm_wday]);

  if (rtime->tm_hour > 12) { i = 1; sprintf(hr, "%02d", (rtime->tm_hour - 12)); }
  else { i = 0; sprintf(hr, "%02d", rtime->tm_hour); }
  if(rtime->tm_hour == 12) i=1;
  sprintf(min, "%02d", rtime->tm_min);
  sprintf(sec, "%02d", rtime->tm_sec);
  if (i == 1) { sprintf(ampm, "%s", "P"); } 
  else { sprintf(ampm, "%s", "A"); }
  if (rep & 1) { sprintf(tmp, "%s:%s:%s%s %s", hr, min, sec, ampm, day); }
  else { sprintf(tmp, "%s %s %s%s %s", hr, min, sec, ampm, day); }

  strcpy(month, months[rtime->tm_mon]);

  lcd.string(1, 3, tmp);

  sprintf(tmp, "%s %d, %d", month, rtime->tm_mday, (rtime->tm_year + 1900));
  lcd.string(2, 4, tmp); 

  return 0;
} // End clock_screen()



//////////////////////////////////////////////////////////////////////
// Time Screen displays current time and date, uptime, OS ver...
//
//+--------------------+
//|## Linux 2.0.33 ###@|
//|Up xxx days hh:mm:ss|
//|  Wed May 17, 1998  |
//|11:32:57a  100% idle|
//+--------------------+
//

int time_screen(int rep)
{
  char hr[8], min[8], sec[8], ampm[8];
  char day[16], month[16];
  time_t thetime;
  struct tm *rtime;
  double uptime, idle;
  int i;



  if(!rep)
  {
    lcd.clear();
  }


  get_uptime(&uptime, &idle);
  idle = (idle * 100) / uptime;

  
  ///////////////////// Write the title bar (os name and version)
  memset(tmp, 0, lcd.wid+1);
  sprintf(tmp, "%c%c %s %s ", 
	  PAD, PAD, sysname, kver);
  for(i=1; i<(20-strlen(sysname)-strlen(kver)-4); i++)
    tmp[lcd.wid-i]=PAD;
  
  lcd.string(1, 1, tmp);

  /////////////////////// Display the time...
  time(&thetime);
  rtime = localtime(&thetime);

  if(TwentyFourHour)
    {
      sprintf(hr, "%02d", rtime->tm_hour);
    }
  else
    {
      if (rtime->tm_hour > 12) 
	{ i = 1; sprintf(hr, "%02d", (rtime->tm_hour - 12)); }
      else { i = 0; sprintf(hr, "%02d", rtime->tm_hour); }
    }
  if(rtime->tm_hour == 12) i=1;
  if (i == 1) { sprintf(ampm, "%s", "P"); } 
  else { sprintf(ampm, "%s", "A"); }

  sprintf(min, "%02d", rtime->tm_min);
  sprintf(sec, "%02d", rtime->tm_sec);
  if (rep & 1) 
    { 
      if(TwentyFourHour)
	sprintf(tmp, "%s:%s:%s   %2i%% idle", hr, min, sec, (int)idle); 
      else
	sprintf(tmp, "%s:%s:%s%s  %2i%% idle", hr, min, sec, ampm, (int)idle); 
    }
  else
    { 
      if(TwentyFourHour)
	sprintf(tmp, "%s %s %s   %2i%% idle", hr, min, sec, (int)idle); 
      else
	sprintf(tmp, "%s %s %s%s  %2i%% idle", hr, min, sec, ampm, (int)idle); 
    }
  // Center the output line...
  i = ((lcd.wid - strlen(tmp)) / 2) + 1;
  lcd.string(i, 4, tmp);




  /////////////////////// Display the uptime...
  strcpy(day, shortdays[rtime->tm_wday]);
  strcpy(month, shortmonths[rtime->tm_mon]);

  sprintf(tmp, "%s %s %d, %d", day, month, 
	  rtime->tm_mday, (rtime->tm_year + 1900));
  lcd.string(3, 3, tmp); 


  i = (int)uptime / 86400;
  sprintf(day, "%d day%s,", i, (i != 1 ? "s" : ""));
  i = ((int)uptime % 86400) / 60 / 60;
  sprintf(hr, "%02i",i);
  i = (((int)uptime % 86400) % 3600) / 60;
  sprintf(min, "%02i",i);
  i = ((int)uptime % 60);
  sprintf(sec, "%02i",i);
  if (rep & 1)
    sprintf(tmp, "%s %s:%s:%s", day, hr, min, sec);
  else
    sprintf(tmp, "%s %s %s %s", day, hr, min, sec);
  // Center this line automatically...
  i = ((lcd.wid - strlen(tmp)) / 2) + 1;
  lcd.string(i, 2, tmp);
  
  return 0;
} // End time_screen()


/////////////////////////////////////////////////////////////////////////
// Mem Screen displays info about memory and swap usage...
//
int mem_screen(int rep)
{
  int n;
  struct meminfo mem[2];


  if(!rep)
  {
    lcd.clear();
    lcd.init_hbar();
    sprintf(tmp, "%c%c%c MEM %c%c%c%c SWAP %c%c",
      PAD,PAD,PAD,PAD,PAD,PAD,PAD,PAD,PAD);
    lcd.string(1, 1, tmp);
    lcd.string(9, 2, "Totl");
    lcd.string(9, 3, "Free");
    //lcd.string(1, 4, "E       F  E       F");
  }


  // Total memory
  get_mem_info(mem);
  sprintf(tmp, "%6dk", mem[0].total);
  lcd.string(1, 2, tmp);

  // Free memory (plus buffers and cache)
  sprintf(tmp, "%6dk",
	  mem[0].free + mem[0].buffers + mem[0].cache);
  lcd.string(1, 3, tmp);

  // Total swap
  sprintf(tmp, "%6dk", mem[1].total);
  lcd.string(14, 2, tmp);

  // Free swap
  sprintf(tmp, "%6dk", mem[1].free);
  lcd.string(14, 3, tmp);


  // Empty or full?
  lcd.string(1, 4, "E       F  E       F");

  // Free memory graph
  n = (int)(35.0 -
              ((float)mem[0].free + (float)mem[0].buffers + (float)mem[0].cache)
	    / (float)mem[0].total
            * 35.0);
  lcd.hbar(2, 4, n);

  // Free swap graph
  n = (int)(35.0 -
              (float)mem[1].free / (float)mem[1].total
            * 35.0);
  lcd.hbar(13, 4, n);


  return 0;
} // End mem_screen()


////////////////////////////////////////////////////////////////////
// Uptime Screen shows info about system uptime and OS version
//
int uptime_screen(int rep)
{
  int i;
  char date[16], hour[8], min[8], sec[8];
  double uptime, idle;


  if(!rep)
  {
    lcd.clear();
    sprintf(tmp, "%c%c SYSTEM UPTIME %c%c%c", PAD, PAD, PAD, PAD, PAD);
    lcd.string(1, 1, tmp);

    sprintf(tmp, "%s %s", sysname, kver);
    lcd.string(5, 4, tmp);
  }

  get_uptime(&uptime, &idle);
  i = (int)uptime / 86400;
  sprintf(date, "%d day%s,", i, (i != 1 ? "s" : ""));
  i = ((int)uptime % 86400) / 60 / 60;
  sprintf(hour, "%02i",i);
  i = (((int)uptime % 86400) % 3600) / 60;
  sprintf(min, "%02i",i);
  i = ((int)uptime % 60);
  sprintf(sec, "%02i",i);
  if (rep & 1)
    sprintf(tmp, "%s %s:%s:%s", date, hour, min, sec);
  else
    sprintf(tmp, "%s %s %s %s", date, hour, min, sec);
  i = ((20 - strlen(tmp)) / 2) + 1;
  lcd.string(i, 3, tmp);


  return 0;
} // End uptime_screen()


///////////////////////////////////////////////////////////////////////////
// Gives disk stats. 
//
// Stays onscreen until it is done.
//
int disk_screen(int rep)
{
  static mounts mnt[256]; 
  static int count=0;

  struct disp  // Holds info to display (avoid recalculating it)
  { 
    char dev[8]; 
    char cap[8];
    int full; 
  } table[256];
  int i, y;
  static int first=0;  // First line to display, sort of.

  #define huge long long int
  huge size;


  // Grab disk stats on first display, and fill "table".
  if(!rep) 
  {
    lcd.clear();
    lcd.init_hbar();
    lcd.icon(2,7);

    // Get rid of old, unmounted filesystems...
    memset(table, 0, sizeof(struct disp)*256);
    
    count = get_fs(mnt);
    first = 0;
    
    sprintf(tmp, "%c%c FILESYSTEMS %c%c%c%c", 
	    PAD, PAD, PAD, PAD, PAD, PAD);
    lcd.string(1, 1, tmp);

    // Fill the display structure...
    if(count)
    {
      for(i=0; i<count; i++)
      {
	if(strlen(mnt[i].mpoint) > 6)
	{
	  sprintf(table[i].dev, "%c%s", ELLIPSIS,
		  (mnt[i].mpoint)+(strlen(mnt[i].mpoint)-5));
	}
	else
	{
	  sprintf(table[i].dev, "%s", mnt[i].mpoint);
	}
	
	table[i].full = (lcd.cellwid * 4)
	              * (huge)(mnt[i].blocks - mnt[i].bfree)
	              / (huge)mnt[i].blocks;

	size = (huge)mnt[i].bsize * (huge)mnt[i].blocks;
	memset(table[i].cap, 0, 8);

	// Kilobytes
	if(size > 0  &&  size < (huge)1000*(huge)1000)
	  sprintf(table[i].cap, "%3.1fk", 
		  (double)(size)/1024.0);
	// Megabytes
	else if(size >= (huge)1000*(huge)1000
		&&  size < (huge)1000*(huge)1000*(huge)1000)
	  sprintf(table[i].cap, "%3.1fM", 
		  (float)(size/(huge)1024)/1024.0);
	// Gigabytes
	else if(size >= (huge)1000*(huge)1000*(huge)1000  
		&&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000)
	  sprintf(table[i].cap, "%3.1fG", 
		  (float)(size/((huge)1024*(huge)1024))/1024.0);
	// Terabytes
	else if(size >= (huge)1000*(huge)1000*(huge)1000*(huge)1000  
		&&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000)
	  sprintf(table[i].cap, "%3.1fT", 
		  (float)(size/((huge)1024*(huge)1024*(huge)1024))/1024.0);
	  	
	// PectaBytes -- Yeah!  I want some!
	else if(size >= (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000
		&&  size < (huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000*(huge)1000)
	  sprintf(table[i].cap, "%3.1fP", 
		  (float)(size/((huge)1024*(huge)1024*(huge)1024*(huge)1024))/1024.0);
	  	
      }
    }
  } // End if(!rep)


  // Increment this once per second.
  if((rep&7) == 0) first++;
  

  if (!count) 
  { 
    lcd.string(1, 2, "Error Retrieving");
    lcd.string(5, 3, "Filesystem Stats");
    return 0;
  }

  // If we are done, tell the system to keep going.
  if (first >= count+1)
    return CONTINUE;


  // Display stuff...  (show for two seconds, then scroll once per
  //  second, then hold at the end for two seconds)
  for(y=0; y<3; y++)
  {
    i = first - 2;
    if(i>count-3) i=count-3;
    if(i<0) i=0;
    i += y;

    if(table[i].dev[0] == 0) continue;
    sprintf(tmp, "%-6s %6s E    F", table[i].dev, table[i].cap);

    lcd.string(1, y+2, tmp);
    lcd.hbar(16, y+2, table[i].full);
  }


  #undef huge

/*    
// ** FILESYSTEMS *****
// /      543.2M E----F
// /dos/c   2.1G E----F
// /stuff   4.3G E----F
*/
  return HOLD_SCREEN;
}


///////////////////////////////////////////////////////////////////////////
// Shows a display very similar to "xload"'s histogram.
//

int xload_screen(int rep)
{
  static float loads[LCD_MAX_WIDTH];
  static int first_time=1;
  int n;
  float loadmax=0, factor, x;
  int status = 0;

  
  if(first_time)  // Only the first time this is ever called...
  {
    memset(loads, 0, sizeof(float)*LCD_MAX_WIDTH);
    first_time = 0;
  }

  if(!rep)
  {
    lcd.clear();
  }


  for(n=0; n<(lcd.wid-2); n++) loads[n] = loads[n+1];
  loads[lcd.wid-2] = get_loadavg();

  for(n=0; n<lcd.wid-1; n++)
    if(loads[n] > loadmax) loadmax = loads[n];

  lcd.string(20, 4, "0");
  n = (int)loadmax;
  if ((float)n < loadmax) { n++; } 
  sprintf(tmp, "%i", n); lcd.string(20, 2, tmp);

  if (loadmax < 1.0) factor = 24.0;
  else factor = 24 / (float)n;

  for(n=0; n<lcd.wid-1; n++)
  {
    x = (loads[n] * factor);
    lcd.vbar(n+1, (int)x); 
  }

  if(loadmax < LOAD_MIN) status = BACKLIGHT_OFF;
  if(loadmax > LOAD_MIN) status = BACKLIGHT_ON;
  if(loads[lcd.wid-2] > LOAD_MAX) status = BLINK_ON;

  // This must be drawn *after* the vertical bars...  (?)
  sprintf(tmp, "%c%c LOAD AVG %2.2f %c%c", PAD, PAD, loads[lcd.wid-2], PAD, PAD);
  lcd.string(1, 1, tmp);

  if(!rep)
    lcd.init_vbar();


  return status;
} // End xload_screen()





////////////////////////////////////////////////////////////////////////
// Battery Screen shows apm battery status...
//

//####################
//## Battery: 100% ###
//AC: Unknown
//Batt: Low (Charging)
//E------------------F

int battery_screen(int rep)
{
  int acstat=0, battstat=0, battflag=0, percent=0;
  
  
  // Only run once every 16 frames...
  if(rep&0x0f) return 0;
  
  get_batt_stat(&acstat, &battstat, &battflag, &percent);

  lcd.clear();


  sprintf(tmp, "%c%c Battery: ", PAD, PAD);
  
  if(percent >= 0) sprintf(tmp+strlen(tmp), "%i%% %c%c%c%c",
			   percent, PAD, PAD, PAD, PAD);
  else sprintf(tmp+strlen(tmp), "??%% %c%c%c", PAD, PAD, PAD);
  
  lcd.string(1,1,tmp);
  

  switch(acstat)
    {
      case 0: sprintf(tmp, "AC: Off");
        break;
      case 1: sprintf(tmp, "AC: On");
        break;
      case 2: sprintf(tmp, "AC: Backup");
        break;
      default: sprintf(tmp, "AC: Unknown");
        break;
    }
  
  lcd.string(1,2,tmp);


  if(battflag == 0xff)
    {
      sprintf(tmp, "Battery Stat Unknown");
    }
  else
    {
      sprintf(tmp, "Batt:");
      if(battflag & 1)  sprintf(tmp+strlen(tmp), " High");
      if(battflag & 2)  sprintf(tmp+strlen(tmp), " Low");
      if(battflag & 4)  sprintf(tmp+strlen(tmp), " Critical");
      if(battflag & 8
       ||battstat == 3)  sprintf(tmp+strlen(tmp), " Charging");
      if(battflag & 128) sprintf(tmp+strlen(tmp), " (NONE)");
    } 
  lcd.string(1,3,tmp);

  lcd.chr(1,4, 'E');
  lcd.chr(lcd.wid,4, 'F');
  
  if(percent > 0)
    {
      lcd.hbar(2,4, (percent * ((lcd.wid-2)*lcd.cellwid)/100));
    }


  return 0;
}



////////////////////////////////////////////////////////////////////////
// Credit Screen shows who wrote this...
//
int credit_screen(int rep)
{

  if(!rep)
  {
    lcd.clear();
    sprintf(tmp, "%c%c LCDPROC %s %c%c%c%c", 
            PAD, PAD, version, PAD, PAD, PAD, PAD);
    lcd.string(1, 1, tmp);
    lcd.string(1, 2, "     for Linux      ");
    lcd.string(1, 3, " by William Ferrell ");
    lcd.string(1, 4, " and Scott Scriven  ");
  }

  return 0;
} // End credit_screen()


//////////////////////////////////////////////////////////////////////
// This is mostly for debugging.  It should never show up.
//
int dumbass_screen(int rep)
{

  lcd.string(1,1, "---=== Haha... ==---");
  lcd.string(1,2, "  You specified an  ");
  lcd.string(1,3, "   INVALID MODE!!!  ");
  lcd.string(1,4, "---== Ya LOSER! ==--");
/*
  lcd.string(1,1, "--===GO AWAY!!!===--");
  lcd.string(1,2, "   You're a slimy,  ");
  lcd.string(1,3, "     mold-ridden    ");
  lcd.string(1,4, "      pop-tart!     ");
*/

  return BLINK_ON;
}


//////////////////////////////////////////////////////////////////////////
// This gets called upon program exit, to say "goodbye"
//
int goodbye_screen(int rep)
{

  lcd.clear();
/*
  lcd.string(1,1, "---===SHUTDOWN===---");
  lcd.string(1,2, "    DANGER, WILL    ");
  lcd.string(1,3, "      ROBINSON!     ");
  lcd.string(1,4, "---===EJECT!!!===---");
  usleep(250000);
*/

  lcd.string(1,1, "                    ");
  lcd.string(1,2, "  Thanks for using  ");
  lcd.string(1,3, " LCDproc and Linux! ");
  lcd.string(1,4, "                    ");

  return 0;
}


