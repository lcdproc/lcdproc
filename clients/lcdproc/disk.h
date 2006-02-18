#ifndef DISK_H
#define DISK_H

#ifdef LCDPROC_MENUS
int disk_screen (int rep, int display, int * flags_ptr);
#else
int disk_screen (int rep, int display);
#endif

#endif
