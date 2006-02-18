#ifndef CPU_H
#define CPU_H

#ifdef LCDPROC_MENUS
int cpu_screen (int rep, int display, int * flags_ptr);
int cpu_graph_screen (int rep, int display, int * flags_ptr);
#else
int cpu_screen (int rep, int display);
int cpu_graph_screen (int rep, int display);
#endif

#endif
