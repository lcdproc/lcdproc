#ifndef MODE_H
#define MODE_H

/*
  Don't use this yet!  It's barely even started!
 */

int stat_init();
void stat_close();

typedef struct status
{
  // should hold persistent data somehow...
//  data_type data;
  
  // Takes position onscreen (x,y), and
  // format (i=int, f=float, h=horz bar, v=vert bar, 2=24-hour, 1=12-hour,
  //         x=xload-style, etc...)
  // Each stat will only draw valid formats...
  // size is max size (graphs) or num digits, etc...  (0 is "max" size)
  // reread specifies to re-grab the value first.
  void (*draw)(int x, int y, int size, char format, int reread);
  void (*read)(int reread);

  // Should also maybe be able to return the data?
//  return_type (*get)();
  
};


extern status cpu;
extern status cpu_nice;
extern status cpu_idle;
extern status cpu_sys;
extern status cpu_usr;

extern status mem_free;
extern status mem_total;
extern status swap_free;
extern status swap_total;

extern status load;

extern status date;
extern status uptime;

extern status os_ver;

extern status disk_activity;
extern status disk_free;
extern status disk_total;

extern status net_sent;
extern status net_recv;
extern status net_rate;


#endif
