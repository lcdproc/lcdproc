#ifndef CHRONO_H
#define CHRONO_H

#include <elektra/elektra.h>

int clock_screen(int rep, int display, int *flags_ptr, Elektra * elektra);
int uptime_screen(int rep, int display, int *flags_ptr, Elektra * elektra);
int time_screen(int rep, int display, int *flags_ptr, Elektra * elektra);
int big_clock_screen(int rep, int display, int *flags_ptr, Elektra * elektra);
int mini_clock_screen(int rep, int display, int *flags_ptr, Elektra * elektra);

#endif
