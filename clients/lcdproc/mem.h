#ifndef MEM_H
#define MEM_H

#ifdef LCDPROC_MENUS
int mem_screen (int rep, int display, int * flags_ptr);
int mem_top_screen (int rep, int display, int * flags_ptr);
#else
int mem_screen (int rep, int display);
int mem_top_screen (int rep, int display);
#endif

#endif
