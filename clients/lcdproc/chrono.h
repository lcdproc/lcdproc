#ifndef CHRONO_H
#define CHRONO_H

int chrono_init();
int chrono_close();

int clock_screen(int rep, int display);
int uptime_screen(int rep, int display);
int time_screen(int rep, int display);
int big_clock_screen(int rep, int display);

#endif
