#ifndef MODE_H
#define MODE_H


//TODO: Net stats screen...
//TODO: "Who"
//TODO: biff / mail checking

// Character to use for padding title bars, etc...
extern int PAD;
// Character for the "..." symbol.
extern int ELLIPSIS;

int mode_init();
void mode_close();

int cpu_screen(int rep);
int cpu_graph_screen(int rep);
int clock_screen(int rep);
int mem_screen(int rep);
int uptime_screen(int rep);
int time_screen(int rep);
int disk_screen(int rep);
int xload_screen(int rep);
int battery_screen(int rep);
int credit_screen(int rep);
int dumbass_screen(int rep);
int goodbye_screen(int rep);

#endif
