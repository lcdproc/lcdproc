#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include "stat.h"
#include "lcd.h"


/*
  Don't use this yet!  It's barely even started...
 */



struct load { unsigned long total, user, system, nice, idle; };
struct meminfo { int total, cache, buffers, free, shared; };
static char buffer[1024];

// Nothing else can see these...
static int meminfo_fd, load_fd, loadavg_fd, uptime_fd;

static char kver[SYS_NMLN];
static char sysname[SYS_NMLN];

static void reread(int f, char *errmsg);
static int getentry(const char *tag, const char *bufptr);
static void get_mem_info(struct meminfo *result);
static double get_loadavg(void);
static double get_uptime(void);
static void get_load(struct load * result);



int stat_init() 
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

void stat_close() 
{
  close(meminfo_fd);
  close(loadavg_fd);
  close(load_fd);
  close(uptime_fd);
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

static double get_uptime(void) 
{
  double uptime;

  reread(uptime_fd, "get_uptime:");
  sscanf(buffer, "%lf", &uptime);
  return uptime; 
}

static void get_load(struct load * result) 
{
  static struct load last_load = { 0, 0, 0, 0, 0 }; struct load curr_load;

  reread(load_fd, "get_load:");
  sscanf(buffer, "%*s %lu %lu %lu %lu\n",
	 &curr_load.user, &curr_load.nice, &curr_load.system, &curr_load.idle);
  curr_load.total = curr_load.user + curr_load.nice
                  + curr_load.system + curr_load.idle;
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
// TODO: 24-hour time, if desired.
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
    lcd.string(1, 4, "E       F  E       F");
  }

  get_mem_info(mem);
  sprintf(tmp, "%6dk", mem[0].total);
  lcd.string(1, 2, tmp);
  sprintf(tmp, "%6dk", mem[0].free);
  lcd.string(1, 3, tmp);
  sprintf(tmp, "%6dk", mem[1].total);
  lcd.string(14, 2, tmp);
  sprintf(tmp, "%6dk", mem[1].free);
  lcd.string(14, 3, tmp);

// This gives just main memory usage
//  n = (int)(50.0 - (float)mem[0].free / (float)mem[0].total * 50.0);
// This uses main + swap usage...

  lcd.string(1, 4, "E       F  E       F");

  n = (int)(35.0 -
              (float)mem[0].free / (float)mem[0].total
            * 35.0);
  lcd.hbar(2, 4, n);

  n = (int)(35.0 -
              (float)mem[1].free / (float)mem[1].total
            * 35.0);
  lcd.hbar(13, 4, n);


/*
  {
    int i;
    for(i=0; i<100; i++)
    {
      lcd.hbar(1,1,i);
      lcd.hbar(1,2,i);
      lcd.hbar(1,3,i);
      lcd.hbar(1,4,i);
      usleep(100000);
    }
  }
*/

  return 0;
} // End mem_screen()


////////////////////////////////////////////////////////////////////
// Uptime Screen shows info about system uptime and OS version
//
int uptime_screen(int rep)
{
  int i;
  char date[16], hour[8], min[8], sec[8];
  double uptime;


  if(!rep)
  {
    lcd.clear();
    sprintf(tmp, "%c%c SYSTEM UPTIME %c%c%c", PAD, PAD, PAD, PAD, PAD);
    lcd.string(1, 1, tmp);

    sprintf(tmp, "%s %s", sysname, kver);
    lcd.string(5, 4, tmp);
  }

  uptime = get_uptime();
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

  if(loadmax < 0.05) status = BACKLIGHT_OFF;
  if(loadmax > 0.05) status = BACKLIGHT_ON;
  if(loads[lcd.wid-2] > LOAD_THRESHOLD) status = BLINK_ON;

  // This must be drawn *after* the vertical bars...  (?)
  sprintf(tmp, "%c%c LOAD AVG %2.2f %c%c", PAD, PAD, loads[lcd.wid-2], PAD, PAD);
  lcd.string(1, 1, tmp);

  if(!rep)
    lcd.init_vbar();


  return status;
} // End xload_screen()



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


