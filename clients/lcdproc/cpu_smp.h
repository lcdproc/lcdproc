#ifndef CPU_SMP_H
#define CPU_SMP_H

#ifdef LCDPROC_MENUS
int cpu_smp_screen (int rep, int display, int * flags_ptr);
#else
int cpu_smp_screen (int rep, int display);
#endif

#endif
