#ifndef LOAD_H
#define LOAD_H

#ifndef LOAD_MAX
#define LOAD_MAX 1.3
#endif
#ifndef LOAD_MIN
#define LOAD_MIN 0.5
#endif

#ifdef LCDPROC_MENUS
int xload_screen (int rep, int display, int * flags_ptr);
#else
int xload_screen (int rep, int display);
#endif

#endif
