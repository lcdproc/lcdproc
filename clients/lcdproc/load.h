#ifndef LOAD_H
#define LOAD_H

#ifndef LOAD_MAX
#define LOAD_MAX 1.3
#endif
#ifndef LOAD_MIN
#define LOAD_MIN 0.5
#endif

int load_init ();
int load_close ();

int xload_screen (int rep, int display);

#endif
