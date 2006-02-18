#ifndef BATT_H
#define BATT_H

#ifdef LCDPROC_MENUS
int battery_screen (int rep, int display, int * flags_ptr);
#else
int battery_screen (int rep, int display);
#endif

#endif
