#ifndef CHRONO_H
#define CHRONO_H

#ifdef LCDPROC_MENUS
int clock_screen (int rep, int display, int * flags_ptr);
int uptime_screen (int rep, int display, int * flags_ptr);
int time_screen (int rep, int display, int * flags_ptr);
int big_clock_screen (int rep, int display, int * flags_ptr);
int essential_clock_screen (int rep, int display, int * flags_ptr);
#else
int clock_screen (int rep, int display);
int uptime_screen (int rep, int display);
int time_screen (int rep, int display);
int big_clock_screen (int rep, int display);
int essential_clock_screen (int rep, int display);
#endif

#endif
